/*
 *
 * cd64io.c
 *
 * I/O routines for CD64 device
 *
 * (c) 2004 Ryan Underwood
 * Portions (c) 2004 - 2005, 2015 - 2019 Daniel Horchner (OpenBSD, NetBSD,
 *                                       FreeBSD, BeOS, Win32, DOS)
 *
 * May be distributed under the terms of the GNU Lesser/Library General Public
 * License, or any later version of the same, as published by the Free Software
 * Foundation.
 */

#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#endif
#include <stdlib.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4820) /* 'bytes' bytes padding added after construct 'member_name' */
#endif
#include <sys/stat.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#if defined __unix__ || defined __BEOS__ /* ioctl() */
#include <unistd.h>
#endif

#include <ultra64/host/cd64lib.h>
#include "cd64io.h"

#define DEBUG_LOWLEVEL 0
#define BUSY_THRESHOLD 10000
#define MAX_TRIES 5

#ifdef CD64_USE_RAWIO

#if defined _WIN32 || defined __CYGWIN__
#ifdef __CYGWIN__
#include <dlfcn.h>

#define DIR_SEPARATOR_S "/"
#else
#define snprintf _snprintf
#define DIR_SEPARATOR_S "\\"
#endif

/* The next union is a portable means to convert between function and data
 * pointers and the only way to silence Visual C++ 2012 other than
 *   #pragma warning(disable: 4152)
 * That is, with /W4. */
typedef union u_func_ptr {
	void (*func_ptr)(void);
	void *void_ptr;
} u_func_ptr_t;

static void *io_driver = NULL;
static int io_driver_found = 0;
/* inpout32.dll */
static short (__stdcall *Inp32)(short) = NULL;
static void (__stdcall *Outp32)(short, short) = NULL;
/* io.dll */
static char (WINAPI *PortIn)(short int) = NULL;
static void (WINAPI *PortOut)(short int, char) = NULL;
static short int (WINAPI *IsDriverInstalled)(void) = NULL;
/* DlPortIO.dll */
static unsigned char (__stdcall *DlPortReadPortUchar)(unsigned short) = NULL;
static void (__stdcall *DlPortWritePortUchar)(unsigned short, unsigned char) = NULL;

static INLINE uint8_t inb(uint16_t);
static INLINE void outb(uint8_t, uint16_t);
static uint8_t (*input_byte)(uint16_t) = inb;
static void (*output_byte)(uint8_t, uint16_t) = outb;
#endif

#ifdef  __BEOS__
static int io_portfd;

typedef struct st_ioport {
	unsigned int port;
	unsigned char data8;
	unsigned short data16;
} st_ioport_t;
#endif

#endif /* CD64_USE_RAWIO */

int cd64_send_byte(struct cd64_t *cd64, uint8_t what) {
	return cd64->xfer(cd64, &what, NULL, 0);
}

int cd64_send_dword(struct cd64_t *cd64, uint32_t what) {

	int ret = 1;
	ret &= cd64_send_byte(cd64, (uint8_t) (what>>24));
	ret &= cd64_send_byte(cd64, (uint8_t) (what>>16));
	ret &= cd64_send_byte(cd64, (uint8_t) (what>>8));
	ret &= cd64_send_byte(cd64, (uint8_t) what);
	return ret;
}

int cd64_grab_byte(struct cd64_t *cd64, uint8_t *val) {
	return cd64->xfer(cd64, NULL, val, 0);
}

int cd64_grab_dword(struct cd64_t *cd64, uint32_t *val) {

	int ret = 1;
	uint8_t grab;
	if (val == NULL) return 0;
	*val = 0;

	ret &= cd64_grab_byte(cd64, &grab);
	*val |= grab << 24;
	ret &= cd64_grab_byte(cd64, &grab);
	*val |= grab << 16;
	ret &= cd64_grab_byte(cd64, &grab);
	*val |= grab << 8;
	ret &= cd64_grab_byte(cd64, &grab);
	*val |= grab;
	return ret;
}

int cd64_trade_bytes(struct cd64_t *cd64, uint8_t give, uint8_t *recv) {
	return cd64->xfer(cd64, &give, recv, 0);
}

/* Backend-specific defs go down here. */

#ifdef CD64_USE_LIBIEEE1284

int cd64_open_ieee1284(struct cd64_t *cd64) {

	struct parport_list pplist;
	int ppflags = F1284_EXCL;
	int ppcaps = 0;
	int i;
	int opened = 0;

	if (cd64->ppdev || !cd64->using_ppa) return 0;

	if (ieee1284_find_ports(&pplist, 0) < 0) {
		cd64->notice_callback2("Could not get port list.");
		return 0;
	}

	if (cd64->port < pplist.portc) {
		/* Just use it as an index. */
		cd64->ppdev = pplist.portv[cd64->port];
	}
	else {
		/* Search for the ppdev matching its base address. */
		for (i = 0; i < pplist.portc; i++) {
			if (cd64->port == (int) pplist.portv[i]->base_addr) {
				cd64->ppdev = pplist.portv[i];
			}
		}
	}

	if (cd64->ppdev) {
		if (ieee1284_open(cd64->ppdev, ppflags, &ppcaps) < 0) {
			cd64->notice_callback2("Failed opening ieee1284 port %d.", cd64->port);
			cd64->ppdev = NULL;
		}
		else {
			opened = 1;
		}
	}

	ieee1284_free_ports(&pplist);

	if (opened && ieee1284_claim(cd64->ppdev) < 0) return 0;
	else return opened;
}

int cd64_close_ieee1284(struct cd64_t *cd64) {

	int ret;

	if (cd64->ppdev == NULL) return 1;

	ieee1284_release(cd64->ppdev);
	ret = ieee1284_close(cd64->ppdev);
	if (ret < 0) ret = 0;
	else {
		cd64->ppdev = NULL;
		ret = 1;
	}

	return ret;
}

static INLINE int cd64_wait_ieee(struct cd64_t *cd64) {

	/* With ppdev, could we use an interrupt instead?  The PPA
	 * could be modified... */

	int i = 0;
	int reset_tries = 0;
	while (i < 10000) i++; /* FIXME is this necessary? */
	i = 0;

	while ((ieee1284_read_status(cd64->ppdev)^S1284_INVERTED) & S1284_BUSY) {
		i++;
		if (i >= BUSY_THRESHOLD) {
			/* The PPA is in a weird state.
			 * Try to knock some sense into it. */
			ieee1284_write_control(cd64->ppdev, (C1284_NINIT|C1284_NAUTOFD)^C1284_INVERTED);
			ieee1284_write_control(cd64->ppdev, C1284_NINIT^C1284_INVERTED);
			ieee1284_write_control(cd64->ppdev, (C1284_NINIT|C1284_NSTROBE)^C1284_INVERTED);
			ieee1284_write_control(cd64->ppdev, C1284_NINIT^C1284_INVERTED);
			reset_tries++;
			i = 0;
			MSLEEP(1);
		}
		if (reset_tries > MAX_TRIES) break;
		if (cd64->abort) return 0;
	}

	return (reset_tries < MAX_TRIES);
}

int cd64_xfer_ieee1284(struct cd64_t *cd64, uint8_t *wr, uint8_t *rd, int delayms) {

	if (!cd64_wait_ieee(cd64)) { return 0; }

	if (delayms) MSLEEP(delayms);
	ieee1284_data_dir(cd64->ppdev, 1);
	if (delayms) MSLEEP(delayms);
	ieee1284_write_control(cd64->ppdev, (C1284_NINIT|C1284_NAUTOFD)^C1284_INVERTED);
	if (delayms) MSLEEP(delayms);
	if (rd) {
		*rd = ieee1284_read_data(cd64->ppdev);
#if DEBUG_LOWLEVEL
		printf("got %xh", *rd);
		if (*rd > 0x20) printf(" (%c)", *rd);
		fputc('\n', stdout);
#endif
	}

	if (delayms) MSLEEP(delayms);
	ieee1284_data_dir(cd64->ppdev, 0);
	if (delayms) MSLEEP(delayms);
	ieee1284_write_control(cd64->ppdev, C1284_NINIT^C1284_INVERTED);
	if (delayms) MSLEEP(delayms);
	if (wr) {
		ieee1284_write_data(cd64->ppdev, *wr);
#if DEBUG_LOWLEVEL
		printf("put %xh", *wr);
		if (*wr > 0x20) printf(" (%c)", *wr);
		fputc('\n', stdout);
#endif
	}
	if (delayms) MSLEEP(delayms);
	ieee1284_write_control(cd64->ppdev, (C1284_NINIT|C1284_NSTROBE)^C1284_INVERTED);
	if (delayms) MSLEEP(delayms);
	ieee1284_write_control(cd64->ppdev, C1284_NINIT^C1284_INVERTED);

	return 1;
}

#endif /* CD64_USE_LIBIEEE1284 */


#ifdef CD64_USE_PPDEV

int cd64_open_ppdev(struct cd64_t *cd64) {

	char *device = "/dev/parport%d";
	char realdev[128+1];

	if (cd64->ppdevfd || !cd64->using_ppa) return 0;
	/* This should be a port number only, not an address */
	if (cd64->port > PARPORT_MAX) return 0;

	snprintf(realdev, 128+1, device, cd64->port);
	realdev[128] = 0;

	if ((cd64->ppdevfd = open(realdev, O_RDWR)) == -1) {
		cd64->notice_callback2("open: %s", strerror(errno));
		cd64->ppdevfd = 0;
		return 0;
	}

	if (ioctl(cd64->ppdevfd, PPEXCL) != 0) {
		cd64->notice_callback2("PPEXCL: %s", strerror(errno));
		close(cd64->ppdevfd);
		cd64->ppdevfd = 0;
		return 0;
	}

	if (ioctl(cd64->ppdevfd, PPCLAIM) != 0) {
		cd64->notice_callback2("PPCLAIM: %s", strerror(errno));
		close(cd64->ppdevfd);
		cd64->ppdevfd = 0;
		return 0;
	}

	return 1;
}

int cd64_close_ppdev(struct cd64_t *cd64) {

	int ret = 1;

	if (cd64->ppdevfd == 0) return 1;

	if (ioctl(cd64->ppdevfd, PPRELEASE) != 0) {
		cd64->notice_callback2("PPRELEASE: %s", strerror(errno));
		ret = 0;
	}

	close(cd64->ppdevfd);
	cd64->ppdevfd = 0;
	return ret;
}

static INLINE int cd64_wait_ppdev(struct cd64_t *cd64) {

	/* With ppdev, could we use an interrupt instead?  The PPA
	 * could be modified... */

	int i = 0;
	int reset_tries = 0;
	uint8_t status;
	int dir;
	i = 0;

	if (ioctl(cd64->ppdevfd, PPRSTATUS, &status) != 0) cd64->notice_callback2("PPRSTATUS: %s", strerror(errno));

	while (status & 0x80) {
		i++;
		if (i >= BUSY_THRESHOLD) {
			/* The PPA is in a weird state.
			 * Try to knock some sense into it. */
			dir = 1;
			if (ioctl(cd64->ppdevfd, PPDATADIR, &dir) != 0) cd64->notice_callback2("PPDATADIR: %s", strerror(errno));
			status = PARPORT_CONTROL_INIT | PARPORT_CONTROL_AUTOFD; /* 0x26 */
			if (ioctl(cd64->ppdevfd, PPWCONTROL, &status) != 0) cd64->notice_callback2("PPWCONTROL: %s", strerror(errno));

			dir = 0;
			if (ioctl(cd64->ppdevfd, PPDATADIR, &dir) != 0) cd64->notice_callback2("PPDATADIR: %s", strerror(errno));
			status = PARPORT_CONTROL_INIT; /* 0x04 */
			if (ioctl(cd64->ppdevfd, PPWCONTROL, &status) != 0) cd64->notice_callback2("PPWCONTROL: %s", strerror(errno));
			status = PARPORT_CONTROL_INIT | PARPORT_CONTROL_STROBE; /* 0x05 */
			if (ioctl(cd64->ppdevfd, PPWCONTROL, &status) != 0) cd64->notice_callback2("PPWCONTROL: %s", strerror(errno));
			status = PARPORT_CONTROL_INIT; /* 0x04 */
			if (ioctl(cd64->ppdevfd, PPWCONTROL, &status) != 0) cd64->notice_callback2("PPWCONTROL: %s", strerror(errno));
			reset_tries++;
			i = 0;
			MSLEEP(1);
		}
		if (cd64->abort) return 0;
		if (reset_tries > MAX_TRIES) break;

		if (ioctl(cd64->ppdevfd, PPRSTATUS, &status) != 0) cd64->notice_callback2("PPRSTATUS: %s", strerror(errno));
	}

	return (reset_tries < MAX_TRIES);
}

int cd64_xfer_ppdev(struct cd64_t *cd64, uint8_t *wr, uint8_t *rd, int delayms) {

	uint8_t ctl;
	int dir;

	if (!cd64_wait_ppdev(cd64)) { return 0; }

	if (delayms) MSLEEP(delayms);
	dir = 1;
	if (ioctl(cd64->ppdevfd, PPDATADIR, &dir) != 0) cd64->notice_callback2("PPDATADIR: %s", strerror(errno));
	if (delayms) MSLEEP(delayms);
	ctl = PARPORT_CONTROL_INIT | PARPORT_CONTROL_AUTOFD;
	if (ioctl(cd64->ppdevfd, PPWCONTROL, &ctl) != 0) cd64->notice_callback2("PPWCONTROL: %s", strerror(errno));
	if (delayms) MSLEEP(delayms);
	if (rd) {
		if (ioctl(cd64->ppdevfd, PPRDATA, rd) != 0) cd64->notice_callback2("PPRDATA: %s", strerror(errno));
#if DEBUG_LOWLEVEL
		printf("got %xh", *rd);
		if (*rd > 0x20) printf(" (%c)", *rd);
		fputc('\n', stdout);
#endif
	}

	if (delayms) MSLEEP(delayms);
	dir = 0;
	if (ioctl(cd64->ppdevfd, PPDATADIR, &dir) != 0) cd64->notice_callback2("PPDATADIR: %s", strerror(errno));
	if (delayms) MSLEEP(delayms);
	ctl = PARPORT_CONTROL_INIT;
	if (ioctl(cd64->ppdevfd, PPWCONTROL, &ctl) != 0) cd64->notice_callback2("PPWCONTROL: %s", strerror(errno));
	if (delayms) MSLEEP(delayms);
	if (wr) {
		if (ioctl(cd64->ppdevfd, PPWDATA, wr) != 0) cd64->notice_callback2("PPWDATA: %s", strerror(errno));
#if DEBUG_LOWLEVEL
		printf("put %xh", *wr);
		if (*wr > 0x20) printf(" (%c)", *wr);
		fputc('\n', stdout);
#endif
	}
	if (delayms) MSLEEP(delayms);
	ctl = PARPORT_CONTROL_INIT | PARPORT_CONTROL_STROBE;
	if (ioctl(cd64->ppdevfd, PPWCONTROL, &ctl) != 0) cd64->notice_callback2("PPWCONTROL: %s", strerror(errno));
	if (delayms) MSLEEP(delayms);
	ctl = PARPORT_CONTROL_INIT;
	if (ioctl(cd64->ppdevfd, PPWCONTROL, &ctl) != 0) cd64->notice_callback2("PPWCONTROL: %s", strerror(errno));

	return 1;
}

#endif /* CD64_USE_PPDEV */


#ifdef CD64_USE_PORTDEV

int cd64_open_portdev(struct cd64_t *cd64) {

	if (cd64->portdevfd || cd64->port == 0) return 0;

	if ((cd64->portdevfd = open("/dev/port", O_RDWR)) == -1) {
		cd64->notice_callback2("open: %s", strerror(errno));
		cd64->notice_callback2("portdev requires CAP_SYS_RAWIO capability");
		cd64->portdevfd = 0;
		return 0;
	}

	return 1;
}

int cd64_close_portdev(struct cd64_t *cd64) {

	if (cd64->portdevfd == 0) return 1;

	if (close(cd64->portdevfd) == -1) {
		cd64->notice_callback2("close: %s", strerror(errno));
		return 0;
	}
	cd64->portdevfd = 0;
	return 1;
}

static INLINE ssize_t read2(struct cd64_t *cd64, void *buf) {

	size_t i = 0;

	do {
		ssize_t n = read(cd64->portdevfd, buf, 1);
		if (n >= 0) i += n;
		else if (errno != EINTR) {
			cd64->notice_callback2("read: %s", strerror(errno));
			break;
		}
	}
	while (i < 1);

	return i;
}

static INLINE ssize_t write2(struct cd64_t *cd64, const void *buf) {

	size_t i = 0;

	do {
		ssize_t n = write(cd64->portdevfd, buf, 1);
		if (n >= 0) i += n;
		else if (errno != EINTR) {
			cd64->notice_callback2("write: %s", strerror(errno));
			break;
		}
	}
	while (i < 1);

	return i;
}

static INLINE int cd64_wait_portdev(struct cd64_t *cd64) {

	int i = 0;
	int reset_tries = 0;
	uint8_t status;
	int dir;
	i = 0;

	if (cd64->using_ppa) {
		lseek(cd64->portdevfd, cd64->port+1, SEEK_SET);
		read2(cd64, &status);

		while (status & 0x80) {
			i++;
			if (i >= BUSY_THRESHOLD) {
				/* The PPA is in a weird state.
				 * Try to knock some sense into it. */
				dir = 1;
				status = 0x06 | (dir << 5);
				lseek(cd64->portdevfd, cd64->port+2, SEEK_SET);
				write2(cd64, &status);

				dir = 0;
				status = 0x04 | (dir << 5);
				lseek(cd64->portdevfd, cd64->port+2, SEEK_SET);
				write2(cd64, &status);
				status = 0x05 | (dir << 5);
				lseek(cd64->portdevfd, cd64->port+2, SEEK_SET);
				write2(cd64, &status);
				status = 0x04 | (dir << 5);
				lseek(cd64->portdevfd, cd64->port+2, SEEK_SET);
				write2(cd64, &status);

				reset_tries++;
				i = 0;
				MSLEEP(1);
			}
			if (cd64->abort) return 0;
			if (reset_tries > MAX_TRIES) {
				break;
			}

			lseek(cd64->portdevfd, cd64->port+1, SEEK_SET);
			read2(cd64, &status);
		}
	}
	else { /* Comms link */
		lseek(cd64->portdevfd, cd64->port+2, SEEK_SET);
		read2(cd64, &status);
		while (status & 1) {
			/* Do we need to handle a stuck situation here? */
			lseek(cd64->portdevfd, cd64->port+2, SEEK_SET);
			read2(cd64, &status);
		}
	}

	return (reset_tries < MAX_TRIES);
}

int cd64_xfer_portdev(struct cd64_t *cd64, uint8_t *wr, uint8_t *rd, int delayms) {

	uint8_t ctl;
	int dir;

	if (cd64->using_ppa) {

		if (!cd64_wait_portdev(cd64)) { return 0; }

		if (delayms) MSLEEP(delayms);
		dir = 1;
		ctl = 0x06 | (dir << 5);
		lseek(cd64->portdevfd, cd64->port+2, SEEK_SET);
		write2(cd64, &ctl);
		if (delayms) MSLEEP(delayms);
		if (rd) {
			lseek(cd64->portdevfd, cd64->port, SEEK_SET);
			read2(cd64, rd);
#if DEBUG_LOWLEVEL
			printf("got %xh", *rd);
			if (*rd > 0x20) printf(" (%c)", *rd);
			fputc('\n', stdout);
#endif
		}

		if (delayms) MSLEEP(delayms);
		dir = 0;
		ctl = 0x04 | (dir << 5);
		lseek(cd64->portdevfd, cd64->port+2, SEEK_SET);
		write2(cd64, &ctl);
		if (delayms) MSLEEP(delayms);
		if (wr) {
			lseek(cd64->portdevfd, cd64->port, SEEK_SET);
			write2(cd64, wr);
#if DEBUG_LOWLEVEL
			printf("put %xh", *wr);
			if (*wr > 0x20) printf(" (%c)", *wr);
			fputc('\n', stdout);
#endif
		}
		if (delayms) MSLEEP(delayms);
		ctl = 0x05 | (dir << 5);
		lseek(cd64->portdevfd, cd64->port+2, SEEK_SET);
		write2(cd64, &ctl);
		if (delayms) MSLEEP(delayms);
		ctl = 0x04 | (dir << 5);
		lseek(cd64->portdevfd, cd64->port+2, SEEK_SET);
		write2(cd64, &ctl);
	}
	else { /* Comms link */
		lseek(cd64->portdevfd, cd64->port, SEEK_SET);
		write2(cd64, wr);
		if (!cd64_wait_portdev(cd64)) { return 0; }
		lseek(cd64->portdevfd, cd64->port, SEEK_SET);
		read2(cd64, rd);
	}

	return 1;
}

#endif /* CD64_USE_PORTDEV */


#ifdef CD64_USE_RAWIO

#if defined _WIN32 || defined __CYGWIN__
static void *open_module(char *module_name, struct cd64_t *cd64) {

	void *handle;
#ifdef __CYGWIN__
	if ((handle = dlopen(module_name, RTLD_LAZY)) == NULL) {
		cd64->notice_callback2("dlopen: %s", dlerror());
		exit(1);
	}
#else
	if ((handle = LoadLibrary(module_name)) == NULL) {
		DWORD errorcode = GetLastError();
		LPTSTR strptr;

		cd64->notice_callback2("LoadLibrary: %s", strerror(errno));
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		              FORMAT_MESSAGE_FROM_SYSTEM |
		              FORMAT_MESSAGE_IGNORE_INSERTS,
		              NULL, errorcode,
		              MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
		              (LPTSTR) &strptr, 0, NULL);
		cd64->notice_callback2("LoadLibrary: %s", strptr);
		LocalFree(strptr);
		exit(1);
	}
#endif
	return handle;
}

static void close_module(void *handle, struct cd64_t *cd64) {

#ifdef __CYGWIN__
	if (dlclose(handle)) {
		cd64->notice_callback2("dlclose: %s", dlerror());
		exit(1);
	}
#else
	if (!FreeLibrary((HINSTANCE) handle)) {
		DWORD errorcode = GetLastError();
		LPTSTR strptr;

		cd64->notice_callback2("FreeLibrary: %s", strerror(errno));
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		              FORMAT_MESSAGE_FROM_SYSTEM |
		              FORMAT_MESSAGE_IGNORE_INSERTS,
		              NULL, errorcode,
		              MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
		              (LPTSTR) &strptr, 0, NULL);
		cd64->notice_callback2("FreeLibrary: %s", strptr);
		LocalFree(strptr);
		exit(1);
	}
#endif
}

static void *get_symbol(void *handle, char *symbol_name, struct cd64_t *cd64) {

	void *symptr;
#ifdef __CYGWIN__
	char *strptr;

	symptr = dlsym(handle, symbol_name);
	if ((strptr = dlerror()) != NULL) {            /* this is "the correct way" */
		cd64->notice_callback2("dlsym: %s", strptr); /*  according to the info page */
		exit(1);
	}
#else
	u_func_ptr_t sym;
	sym.func_ptr = (void (*)(void)) GetProcAddress((HINSTANCE) handle, symbol_name);
	symptr = sym.void_ptr;
	if (symptr == NULL) {
		DWORD errorcode = GetLastError();
		LPTSTR strptr;

		cd64->notice_callback2("GetProcAddress: %s", strerror(errno));
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		              FORMAT_MESSAGE_FROM_SYSTEM |
		              FORMAT_MESSAGE_IGNORE_INSERTS,
		              NULL, errorcode,
		              MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
		              (LPTSTR) &strptr, 0, NULL);
		cd64->notice_callback2("GetProcAddress: %s", strptr);
		LocalFree(strptr);
		exit(1);
	}
#endif
	return symptr;
}

static void *has_symbol(void *handle, char *symbol_name) {

	void *symptr;
#ifdef __CYGWIN__
	symptr = dlsym(handle, symbol_name);
	if (dlerror() != NULL) symptr = (void *) -1;  /* this is "the correct way" */
#else                                           /*  according to the info page */
	u_func_ptr_t sym;
	sym.func_ptr = (void (*)(void)) GetProcAddress((HINSTANCE) handle, symbol_name);
	symptr = sym.void_ptr;
	if (symptr == NULL) symptr = (void *) -1;
#endif
	return symptr;
}

/* inpout32.dll */
static uint8_t inpout32_input_byte(uint16_t port) {

	return (uint8_t) Inp32((short) port);
}

static void inpout32_output_byte(uint8_t byte, uint16_t port) {

	Outp32((short) port, (short) byte);
}

/* io.dll */
static uint8_t io_input_byte(uint16_t port) {

	return PortIn(port);
}

static void io_output_byte(uint8_t byte, uint16_t port) {

	PortOut(port, byte);
}

/* DlPortIO.dll */
static uint8_t dlportio_input_byte(uint16_t port) {

	return DlPortReadPortUchar(port);
}

static void dlportio_output_byte(uint8_t byte, uint16_t port) {

	DlPortWritePortUchar(port, byte);
}

#define NODRIVER_MSG "No (working) I/O port driver"

#if defined __CYGWIN__ && defined __i386__      /* 32-bit */
static EXCEPTION_DISPOSITION NTAPI new_exception_handler(PEXCEPTION_RECORD exception_record,
		void *establisher_frame, PCONTEXT context_record, void *dispatcher_context) {

	(void) establisher_frame;
	(void) dispatcher_context;
	if (exception_record->ExceptionCode == EXCEPTION_PRIV_INSTRUCTION &&
			*(uint8_t *) context_record->Eip == 0xec) { /* in al, dx */
		io_driver_found = -1;
		context_record->Eip++;
#if 1 /* EXCEPTION_CONTINUE_EXECUTION does not work with Cygwin... */
		fputs("ERROR: "NODRIVER_MSG"\n", stderr);
		exit(1);
#else
		return EXCEPTION_CONTINUE_EXECUTION; /* continue at context_record->Eip */
#endif
	}
	return EXCEPTION_CONTINUE_SEARCH;
}
#elif defined _WIN32 && (defined __i386__ || defined _M_IX86) /* 32-bit */
static LONG new_exception_filter(LPEXCEPTION_POINTERS exception_pointers) {

	if (exception_pointers->ExceptionRecord->ExceptionCode == EXCEPTION_PRIV_INSTRUCTION &&
			*(uint8_t *) exception_pointers->ContextRecord->Eip == 0xec) { /* in al, dx */
		io_driver_found = -1;
		exception_pointers->ContextRecord->Eip++;
		return EXCEPTION_CONTINUE_EXECUTION; /* continue at ContextRecord->Eip */
	}
	return EXCEPTION_CONTINUE_SEARCH;
}
#endif
#endif /* _WIN32 || __CYGWIN__ */

#if ((defined _WIN32 || defined __CYGWIN__ || defined __BEOS__) && \
    (defined __i386__ || defined __x86_64__)) || defined _MSC_VER
static INLINE uint8_t inb(uint16_t port) {

#ifdef __BEOS__
	st_ioport_t temp;

	temp.port = port;
	ioctl(io_portfd, 'r', &temp, 0);

	return temp.data8;
#else /* Win32 */
	if (io_driver_found) return input_byte(port);
	else {
#ifdef _MSC_VER
#ifdef _M_IX86
		return (unsigned char) _inp(port);
#endif
#else
		unsigned char byte;
		__asm__ __volatile__
		("inb %1, %0"
		  : "=a" (byte)
		  : "d" (port)
		);
		return byte;
#endif
	}
#endif
}

static INLINE void outb(uint8_t byte, uint16_t port) {

#ifdef __BEOS__
	st_ioport_t temp;

	temp.port = port;
	temp.data8 = byte;
	ioctl(io_portfd, 'w', &temp, 0);
#else /* Win32 */
	if (io_driver_found) output_byte(byte, port);
	else {
#ifdef _MSC_VER
#ifdef _M_IX86
		_outp(port, byte);
#endif
#else
		__asm__ __volatile__
		("outb %1, %0"
		  :
		  : "d" (port), "a" (byte)
		);
#endif
	}
#endif
}
#endif /* inb/outb defs */

int cd64_open_rawio(struct cd64_t *cd64) {

	int ret;
	(void) ret;

	/* NOTE: we will soon be able to use ioperm on the entire
	 * 16-bit port range.  Find out what Linux kernels support it. */

	if (cd64->port < 0x200) {
		cd64->notice_callback2("Erroneous port %xh", cd64->port);
		return 0;
	}

#ifdef __linux__
	if (cd64->port < 0x3fd) {
		if (cd64->using_ppa) {
			ret = ioperm(cd64->port, 3, 1);
		}
		else {
			ret = ioperm(cd64->port, 1, 1);
			ret |= ioperm(cd64->port+2, 1, 1);
		}

		if (ret == -1) {
			cd64->notice_callback2("ioperm: %s", strerror(errno));
			cd64->notice_callback2("rawio requires CAP_SYS_RAWIO capability");
			return 0;
		}
	}
	else {
		ret = iopl(3);
		if (ret == -1) {
			cd64->notice_callback2("iopl: %s", strerror(errno));
			cd64->notice_callback2("rawio requires CAP_SYS_RAWIO capability");
			return 0;
		}
	}
#elif (defined __OpenBSD__ || defined __NetBSD__) && defined __i386__
	ret = i386_iopl(3);
	if (ret == -1) {
		cd64->notice_callback2("i386_iopl: %s", strerror(errno));
		return 0;
	}
#endif
#ifdef __x86_64__
#ifdef __OpenBSD__
	ret = amd64_iopl(3);
	if (ret == -1) {
		cd64->notice_callback2("amd64_iopl: %s", strerror(errno));
		return 0;
	}
#elif defined __NetBSD__
	ret = x86_64_iopl(3);
	if (ret == -1) {
		cd64->notice_callback2("x86_64_iopl: %s", strerror(errno));
		return 0;
	}
#endif
#endif
#ifdef __FreeBSD__
	cd64->portdevfd = open("/dev/io", O_RDWR);
	if (cd64->portdevfd == -1) {
		cd64->portdevfd = 0;
		cd64->notice_callback2("open: %s", strerror(errno));
		cd64->notice_callback2("Could not open I/O port device (/dev/io)");
		return 0;
	}
#elif defined __BEOS__
	io_portfd = open("/dev/misc/ioport", O_RDWR | O_NONBLOCK);
	if (io_portfd == -1) {
		io_portfd = 0;
		cd64->notice_callback2("open: %s", strerror(errno));
		cd64->notice_callback2("Could not open I/O port device (no driver)");
		return 0;
	}
#elif defined _WIN32 || defined __CYGWIN__
#ifdef _MSC_VER
#define access  _access
#endif
	{
		char fname[FILENAME_MAX];
		u_func_ptr_t sym;
		io_driver_found = 0;

		if (!cd64->io_driver_dir[0]) strcpy(cd64->io_driver_dir, ".");
		snprintf(fname, FILENAME_MAX, "%s" DIR_SEPARATOR_S "%s",
		         cd64->io_driver_dir, "dlportio.dll");
		fname[FILENAME_MAX-1] = 0;
		if (access(fname, F_OK) == 0) {
			io_driver = open_module(fname, cd64);

			io_driver_found = 1;
			cd64->notice_callback("Using %s\n", fname);

			sym.void_ptr = get_symbol(io_driver, "DlPortReadPortUchar", cd64);
			DlPortReadPortUchar = (unsigned char (__stdcall *)(unsigned short)) sym.func_ptr;
			sym.void_ptr = get_symbol(io_driver, "DlPortWritePortUchar", cd64);
			DlPortWritePortUchar = (void (__stdcall *)(unsigned short, unsigned char)) sym.func_ptr;

			input_byte = dlportio_input_byte;
			output_byte = dlportio_output_byte;
		}

		if (!io_driver_found) {
			snprintf(fname, FILENAME_MAX, "%s" DIR_SEPARATOR_S "%s",
			         cd64->io_driver_dir, "io.dll");
			fname[FILENAME_MAX-1] = 0;
			if (access(fname, F_OK) == 0) {
				io_driver = open_module(fname, cd64);

				sym.void_ptr = get_symbol(io_driver, "IsDriverInstalled", cd64);
				IsDriverInstalled = (short int (WINAPI *)(void)) sym.func_ptr;
				if (IsDriverInstalled()) {
					io_driver_found = 1;
					cd64->notice_callback("Using %s\n", fname);

					sym.void_ptr = get_symbol(io_driver, "PortIn", cd64);
					PortIn = (char (WINAPI *)(short int)) sym.func_ptr;
					sym.void_ptr = get_symbol(io_driver, "PortOut", cd64);
					PortOut = (void (WINAPI *)(short int, char)) sym.func_ptr;

					input_byte = io_input_byte;
					output_byte = io_output_byte;
				}
			}
		}

		if (!io_driver_found) {
			snprintf(fname, FILENAME_MAX, "%s" DIR_SEPARATOR_S "%s",
			         cd64->io_driver_dir, "inpout32.dll");
			fname[FILENAME_MAX-1] = 0;
			if (access(fname, F_OK) == 0) {
				io_driver = open_module(fname, cd64);

				io_driver_found = 1;
				cd64->notice_callback("Using %s\n", fname);

				/* Newer ports of inpout32.dll also contain the API provided by
				 * DlPortIO.dll. Since the API of DlPortIO.dll does not have
				 * the flaws of inpout32.dll (*signed* short return value and
				 * arguments), we prefer it if it is present. */
				sym.void_ptr = has_symbol(io_driver, "DlPortReadPortUchar");
				DlPortReadPortUchar = (unsigned char (__stdcall *)(unsigned short)) sym.func_ptr;
				if (DlPortReadPortUchar != (void *) -1) input_byte = dlportio_input_byte;
				else {
					sym.void_ptr = get_symbol(io_driver, "Inp32", cd64);
					Inp32 = (short (__stdcall *)(short)) sym.func_ptr;

					input_byte = inpout32_input_byte;
				}

				sym.void_ptr = has_symbol(io_driver, "DlPortWritePortUchar");
				DlPortWritePortUchar = (void (__stdcall *)(unsigned short, unsigned char)) sym.func_ptr;
				if (DlPortWritePortUchar != (void *) -1) output_byte = dlportio_output_byte;
				else {
					sym.void_ptr = get_symbol(io_driver, "Out32", cd64);
					Outp32 = (void (__stdcall *)(short, short)) sym.func_ptr;

					output_byte = inpout32_output_byte;
				}
			}
		}
	}

	{
		/* __try and __except are not supported by MinGW and Cygwin. MinGW has
		 * __try1 and __except1, but using them requires more code than we
		 * currently have. Cygwin does something stupid which breaks
		 * SetUnhandledExceptionFilter()... */
#ifdef __CYGWIN__                               /* Cygwin */
#ifdef __i386__                                 /* 32-bit */
		EXCEPTION_REGISTRATION exception_registration;
		exception_registration.handler = new_exception_handler;

		__asm__ __volatile__
		("movl %%fs:0, %0\n"
		 "movl %1, %%fs:0"
		  : "=a" (exception_registration.prev)
		  : "b" (&exception_registration)
		);
		input_byte(0x378 + 0x402);                  /* 0x378 + 0x402 is okay */
		__asm__ __volatile__
		("movl %0, %%fs:0"
		  :
		  : "r" (exception_registration.prev)
		);
#else                                           /* 64-bit */
		if (!io_driver_found) io_driver_found = -1;
#endif /* __i386__ */
#elif defined _WIN32                            /* MinGW & Visual C++ */
#if defined __i386__ || defined _M_IX86         /* 32-bit */
		LPTOP_LEVEL_EXCEPTION_FILTER org_exception_filter =
			SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER) new_exception_filter);
		input_byte(0x378 + 0x402);                  /* 0x378 + 0x402 is okay */
		SetUnhandledExceptionFilter(org_exception_filter);
#else                                           /* 64-bit */
		if (!io_driver_found) io_driver_found = -1;
#endif /* __i386__ || _M_IX86 */
#endif
		/* input_byte() can succeed without triggering an exception with giveio64. */
		if (io_driver_found == -1) {
			io_driver_found = 0;
			cd64->notice_callback2(NODRIVER_MSG);
			return 0;
		}
	}
#ifdef _MSC_VER
#undef  access
#endif
#endif /* _WIN32 || __CYGWIN__ */

	return 1;
}

int cd64_close_rawio(struct cd64_t *cd64) {

	int ret;
	(void) ret;
	(void) cd64;

#ifdef __linux__
	if (cd64->port < 0x3fd) {
		if (cd64->using_ppa) {
			ret = ioperm(cd64->port, 3, 0);
		}
		else {
			ret = ioperm(cd64->port, 1, 0);
			ret |= ioperm(cd64->port+2, 1, 0);
		}

		if (ret == -1) {
			cd64->notice_callback2("ioperm: %s", strerror(errno));
			return 0;
		}
	}
	else {
		ret = iopl(0);
		if (ret == -1) {
			cd64->notice_callback2("iopl: %s", strerror(errno));
			return 0;
		}
	}
#elif (defined __OpenBSD__ || defined __NetBSD__) && defined __i386__
	ret = i386_iopl(0);
	if (ret == -1) {
		cd64->notice_callback2("i386_iopl: %s", strerror(errno));
		return 0;
	}
#endif
#ifdef __x86_64__
#ifdef __OpenBSD__
	ret = amd64_iopl(0);
	if (ret == -1) {
		cd64->notice_callback2("amd64_iopl: %s", strerror(errno));
		return 0;
	}
#elif defined __NetBSD__
	ret = x86_64_iopl(0);
	if (ret == -1) {
		cd64->notice_callback2("x86_64_iopl: %s", strerror(errno));
		return 0;
	}
#endif
#endif
#ifdef __FreeBSD__
	if (close(cd64->portdevfd) == -1) {
		cd64->notice_callback2("close: %s", strerror(errno));
		return 0;
	}
	cd64->portdevfd = 0;
#elif defined __BEOS__
	if (close(io_portfd) == -1) {
		cd64->notice_callback2("close: %s", strerror(errno));
		return 0;
	}
	io_portfd = 0;
#elif defined _WIN32 || defined __CYGWIN__
	close_module(io_driver, cd64);
	io_driver = NULL;
	io_driver_found = 0;
	input_byte = inb;
	output_byte = outb;
#endif

	return 1;
}

static INLINE int cd64_wait_rawio(struct cd64_t *cd64) {

	int i = 0;
	int reset_tries = 0;
	uint8_t status;
	i = 0;

	if (cd64->using_ppa) {
		status = inb((uint16_t) (cd64->port+1));

		while (status & 0x80) {
			i++;
			if (i >= BUSY_THRESHOLD) {
				/* The PPA is in a weird state.
				 * Try to knock some sense into it. */
				uint8_t dir = 1;
				status = 0x06 | (dir << 5);
				outb(status, (uint16_t) (cd64->port+2));

				dir = 0;
				status = 0x04 | (dir << 5);
				outb(status, (uint16_t) (cd64->port+2));
				status = 0x05 | (dir << 5);
				outb(status, (uint16_t) (cd64->port+2));
				status = 0x04 | (dir << 5);
				outb(status, (uint16_t) (cd64->port+2));

				reset_tries++;
				i = 0;
				MSLEEP(1);
			}
			if (cd64->abort) return 0;
			if (reset_tries > MAX_TRIES) {
				break;
			}

			status = inb((uint16_t) (cd64->port+1));
		}
	}
	else { /* Comms link */
		status = inb((uint16_t) (cd64->port+2));
		while (status & 1) {
			/* Do we need to handle a stuck situation here? */
			status = inb((uint16_t) (cd64->port+2));
		}
	}

	return (reset_tries < MAX_TRIES);
}

int cd64_xfer_rawio(struct cd64_t *cd64, uint8_t *wr, uint8_t *rd, int delayms) {

	if (cd64->using_ppa) {

		uint8_t ctl;
		uint8_t dir;

		if (!cd64_wait_rawio(cd64)) { return 0; }

		if (delayms) MSLEEP(delayms);
		dir = 1;
		ctl = 0x06 | (dir << 5);
		outb(ctl, (uint16_t) (cd64->port+2));
		if (delayms) MSLEEP(delayms);
		if (rd) {
			*rd = inb((uint16_t) cd64->port);
#if DEBUG_LOWLEVEL
			printf("got %xh", *rd);
			if (*rd > 0x20) printf(" (%c)", *rd);
			fputc('\n', stdout);
#endif
		}

		if (delayms) MSLEEP(delayms);
		dir = 0;
		ctl = 0x04 | (dir << 5);
		outb(ctl, (uint16_t) (cd64->port+2));
		if (delayms) MSLEEP(delayms);
		if (wr) {
			outb(*wr, (uint16_t) cd64->port);
#if DEBUG_LOWLEVEL
			printf("put %xh", *wr);
			if (*wr > 0x20) printf(" (%c)", *wr);
			fputc('\n', stdout);
#endif
		}
		if (delayms) MSLEEP(delayms);
		ctl = 0x05 | (dir << 5);
		outb(ctl, (uint16_t) (cd64->port+2));
		if (delayms) MSLEEP(delayms);
		ctl = 0x04 | (dir << 5);
		outb(ctl, (uint16_t) (cd64->port+2));
	}
	else { /* Comms link */
		outb(*wr, (uint16_t) cd64->port);
		if (!cd64_wait_rawio(cd64)) { return 0; }
		*rd = inb((uint16_t) cd64->port);
	}

	return 1;
}

#endif /* CD64_USE_RAWIO */
