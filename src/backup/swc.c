#define INPUT_MASK      0x78
#define IBUSY_BIT       0x80
#define STROBE_BIT      1

#define N_TRY_MAX       65536                   // # times to test if swc ready

#define BUFFERSIZE      8192                    // don't change, only 8192 works!
#define HEADERSIZE      512                     // swc header is 512 bytes

#ifdef  __UNIX__
#define STDERR          stderr
#else
#define STDERR          stdout                  // Stupid DOS has no error
#endif                                          //  stream (direct video writes)
                                                //  this makes redir possible
#include "swc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>                             // access(), STDIN_FILENO, isatty(), ioperm() (libc5)
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __linux__
#ifdef __GLIBC__
#include <sys/io.h>				// ioperm() (glibc)
#endif
#endif

#ifdef  __UNIX__
#include <termios.h>

typedef struct termios tty_t;

#define getch           getchar
#endif
#ifdef DJGPP
#include <conio.h>                              // getch()
#include <pc.h>                                 // kbhit(), inportb()
#endif

static void init_io(unsigned int port);
static void checkabort(int status);
static void send_address(unsigned short address1, unsigned short address2);
static void send_block(unsigned short address, unsigned char *buffer, int len);
static void send_command(unsigned char command_code, unsigned short a, unsigned short l);
static void sendb(unsigned char byte);
static int receive_rom_info(unsigned char *buffer);
static int get_rom_size(unsigned char *info_block);
static int check1(unsigned char *info_block, int index);
static int check2(unsigned char *info_block, int index, unsigned char value);
static int check3(unsigned char *info_block, int index1, int index2, int size);
static unsigned char get_emu_mode_select(unsigned char byte, int size);
static void receive_block(unsigned short address, unsigned char *buffer, int len);
static unsigned char receiveb(void);
static inline unsigned char wait_while_busy(void);
static inline void wait_for_ready(void);

#ifdef  __UNIX__
static void init_conio(void);
static void deinit_conio(void);
static void set_tty(tty_t param);
static int kbhit(void);
#endif

static int swc_port, interleaved;       	// the name `interleaved' is just a guess :)

void init_io(unsigned int port)
/*
  - sets static global `swc_port'. Then the send/receive functions don't need to pass `port' all
    the way to sendb()/receiveb().
  - calls init_conio(). Necessary for kbhit() and DOS-like behaviour of getch().
*/
{
  swc_port = port;
  if (swc_port != 0x3bc && swc_port != 0x378 && swc_port != 0x278)
  {
    fprintf(STDERR, "PORT must be 0x3bc, 0x378 or 0x278\n");
    fflush(stdout);
    exit(1);
  }

#ifdef	__UNIX__
  init_conio();
#endif

  printf("Using I/O port 0x%x\n", swc_port);
}

int swc_write_rom(char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int bytesread, bytessend = 0, size, emu_mode_select;
  unsigned short address;
  struct stat fstate;
  time_t starttime;

  init_io(parport);

  if ((file = fopen(filename, "rb")) == NULL)
  {
    fprintf(STDERR, "Can't open %s for reading\n", filename);
    exit(1);
  }
  if ((buffer = malloc(BUFFERSIZE)) == NULL)
  {
    fprintf(STDERR, "Not enough memory for file buffer (%d bytes)\n", BUFFERSIZE);
    exit(1);
  }

  stat(filename, &fstate);
  size = fstate.st_size - HEADERSIZE;		// size of ROM in bytes
  printf("Send (excl. header): %d Bytes (%.4f Mb)\n\n", size, (float) size/MBIT);

  fread(buffer, 1, HEADERSIZE, file);
  emu_mode_select = buffer[2];                  // this byte is needed later
  send_address(0xc008, 0);
  send_block(0x400, buffer, HEADERSIZE);        // send header

  address = 0x200;                              // vgs '00 uses 0x200, vgs '96 uses 0,
  starttime = time(NULL);                       //  but then some ROMs don't work
  while ((bytesread = fread(buffer, 1, BUFFERSIZE, file)))
  {
    send_address(0xc010, address);
    send_block(0x8000, buffer, bytesread);
    address++;

    bytessend += bytesread;
    parport_gauge(starttime, bytessend, size);
    checkabort(2);
  }

  send_command(5, 0, 0);
  size = (size + BUFFERSIZE - 1) / BUFFERSIZE;	// size in 8KB blocks (rounded up)
  send_command(6, 5 | (size << 8), size >> 8);	// bytes: 6, 5, #8K L, #8K H, 0
  send_command(6, 1 | (emu_mode_select << 8), 0);

  wait_for_ready();
  outportb(swc_port + PARPORT_DATA, 0);

  free(buffer);
  fclose(file);
  return 0;
}

int swc_write_sram(char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int bytesread, bytessend = 0;
  unsigned short address;
  time_t starttime;

  init_io(parport);

  if ((file = fopen(filename, "rb")) == NULL)
  {
    fprintf(STDERR, "Can't open %s for reading\n", filename);
    exit(1);
  }
  if ((buffer = malloc(BUFFERSIZE)) == NULL)
  {
    fprintf(STDERR, "Not enough memory for file buffer (%d bytes)\n", BUFFERSIZE);
    exit(1);
  }

  printf("Send: %d Bytes\n\n", 32*1024);
  fseek(file, HEADERSIZE, SEEK_SET);            // skip the header

  send_command(5, 0, 0);

  send_command(0, 0xe00d, 1);                   // see note in send_address()
  sendb(0);
  sendb(0x81);

  send_command(0, 0xc008, 1);                   // see note in send_address()
  sendb(0);
  sendb(0x81);

  address = 0x100;
  starttime = time(NULL);
  while ((bytesread = fread(buffer, 1, BUFFERSIZE, file)))
  {
    send_command(5, address, 0);
    send_block(0x2000, buffer, bytesread);
    address++;

    bytessend += bytesread;
    parport_gauge(starttime, bytessend, 32*1024);
    checkabort(2);
  }

  free(buffer);
  fclose(file);
  return 0;
}

void send_address(unsigned short address1, unsigned short address2)
{
  send_command(0, address1, 1);                 // these three statements could be
  sendb(0);                                     //  replaced by one send_block() as vgs
  sendb(0x81);                                  //  does, but this is a little bit faster
  send_command(5, address2, 0);
}

void send_block(unsigned short address, unsigned char *buffer, int len)
{
  int checksum = 0x81, n;

  send_command(0, address, len);
  for (n = 0; n < len; n++)
  {
    sendb(buffer[n]);
    checksum ^= buffer[n];
  }
  sendb(checksum);
}

void send_command(unsigned char command_code, unsigned short a, unsigned short l)
{
  sendb(0xd5);
  sendb(0xaa);
  sendb(0x96);
  sendb(command_code);
  sendb(a & 0xff);
  sendb(a >> 8);
  sendb(l & 0xff);
  sendb(l >> 8);
  sendb(0x81^command_code^(a & 0xff)^(a >> 8)^(l & 0xff)^(l >> 8)); // checksum
}

void sendb(unsigned char byte)
{
  wait_for_ready();
  outportb(swc_port + PARPORT_DATA, byte);
  outportb(swc_port + PARPORT_CONTROL, inportb(swc_port + PARPORT_CONTROL)^STROBE_BIT);
  wait_for_ready();                       	// necessary if followed by receiveb()
}

int swc_read_rom(char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer, byte;
  int n, size, blocksleft, bytesreceived = 0;
  unsigned short address1, address2;
  time_t starttime;

  init_io(parport);

  if ((file = fopen(filename, "wb")) == NULL)
  {
    fprintf(STDERR, "Can't open %s for writing\n", filename);
    exit(1);
  }
  if ((buffer = malloc(BUFFERSIZE)) == NULL)
  {
    fprintf(STDERR, "Not enough memory for file buffer (%d bytes)\n", BUFFERSIZE);
    exit(1);
  }

  size = receive_rom_info(buffer);
  if (size == 0)
  {
    fprintf(STDERR, "There is no cartridge present in the Super Wild Card\n");
    remove(filename);
    exit(1);
  }
  blocksleft = size * 16; 			// 1 Mb (128KB) unit == 16 8KB units
  printf("Receive: %d Bytes (%.4f Mb) %s\n\n",
         size*MBIT, (float) size, interleaved ?  "INTERLEAVED" : "");
  size *= MBIT;					// size in bytes for parport_gauge() below

  send_command(5, 0, 0);

  send_command(0, 0xe00c, 1);
  sendb(0);
  sendb(0x81);

  send_command(0, 0xe003, 1);
  sendb(0);
  sendb(0x81);

  send_command(1, 0xbfd8, 1);
  byte = receiveb();
  if ((0x81^byte) != receiveb())
    printf("received data is corrupt\n");

  buffer[2] = get_emu_mode_select(byte, blocksleft/16);
  fwrite(buffer, 1, HEADERSIZE, file);          // write header (other necessary fields are
                                                //  filled in by receive_rom_info())
  if (interleaved)
    blocksleft >>= 1;

  address1 = 0x300;
  address2 = 0x200;
  starttime = time(NULL);
  while (blocksleft > 0)
  {
    if (interleaved)
    {
      for (n = 0; n < 4; n++)
      {
        send_command(5, address1, 0);
        receive_block(0x2000, buffer, BUFFERSIZE);
        blocksleft--;
        address1++;
        fwrite(buffer, 1, BUFFERSIZE, file);

        bytesreceived += BUFFERSIZE;
        parport_gauge(starttime, bytesreceived, size);
        checkabort(2);
      }
    }

    for (n = 0; n < 4; n++)
    {
      send_command(5, address2, 0);
      receive_block(0xa000, buffer, BUFFERSIZE);
      blocksleft--;
      address2++;
      fwrite(buffer, 1, BUFFERSIZE, file);

      bytesreceived += BUFFERSIZE;
      parport_gauge(starttime, bytesreceived, size);
      checkabort(2);
    }
  }
  send_command(5, 0, 0);

  free(buffer);
  fclose(file);
  return 0;
}

#if BUFFERSIZE < HEADERSIZE
#error receive_rom_info() and swc_read_sram() expect BUFFERSIZE to be at least \
       HEADERSIZE bytes.
#endif
int receive_rom_info(unsigned char *buffer)
/*
  - returns size of ROM in Mb (128KB) units (if interleaved returned size ==
    2 * real size)
  - returns ROM header in buffer (index 2 (emulation mode select) is not yet
    filled in)
  - sets global interleaved
*/
{
  int n, m, size;
  unsigned short address;
  unsigned char byte;

  send_command(0, 0xe00c, 1);
  sendb(0);
  sendb(0x81);

  send_command(5, 3, 0);

  send_command(1, 0xbfd5, 1);
  byte = receiveb();
  if ((0x81^byte) != receiveb())
    printf("received data is corrupt\n");
  interleaved = byte & 1;

  address = 0x200;
  for (n = 0; n < HEADERSIZE; n++)
  {
    send_command(5, address, 0);
    send_command(1, 0xa0a0, 1);
    buffer[n] = receiveb();
    if ((0x81^buffer[n]) != receiveb())
      printf("received data is corrupt\n");

    for (m = 0; m < 65536; m++)                 // a delay is necessary here
      ;
    address++;
  }

  size = get_rom_size(buffer);
  if (interleaved)
    size <<= 1;

  memset(buffer, 0, HEADERSIZE);
  buffer[0] = size << 4 & 0xff;                 // *16 for 8KB units; low byte
  buffer[1] = size >> 4;                        // *16 for 8KB units /256 for high byte
  buffer[8] = (unsigned char) 0xaa;
  buffer[9] = (unsigned char) 0xbb;
  buffer[10] = (unsigned char) 4;

  return size;
}

int get_rom_size(unsigned char *info_block)
// returns size of ROM in Mb units
{
  if (check1(info_block, 0))
    return 0;
  if (check2(info_block, 0x10, 0x84))
    return 0;
  if (check3(info_block, 0, 0x20, 0x20))
    return 2;
  if (check3(info_block, 0, 0x40, 0x20))
    return 4;
  if (check3(info_block, 0x40, 0x60, 0x20))
    return 6;
  if (check3(info_block, 0, 0x80, 0x10))
    return 8;
  if (check1(info_block, 0x80))
    return 8;
  if (check3(info_block, 0x80, 0x90, 0x10))
    return 8;
  if (check2(info_block, 0x80, 0xa0))
    return 8;
  if (check3(info_block, 0x80, 0xa0, 0x20))
    return 0xa;
  if (check1(info_block, 0xc0))
    return 0xc;
  if (check2(info_block, 0xc0, 0xb0))
    return 0xc;
  if (check3(info_block, 0x80, 0xc0, 0x20))
    return 0xc;
  if (check3(info_block, 0x100, 0, 0x10))
    return 0x10;
  if (check2(info_block, 0x100, 0xc0))
    return 0x10;
  if (check3(info_block, 0x100, 0x120, 0x10))
    return 0x12;
  if (check3(info_block, 0x100, 0x140, 0x10))
    return 0x14;
  if (check2(info_block, 0x140, 0xd0))
    return 0x14;
  if (check3(info_block, 0x100, 0x180, 0x10))
    return 0x18;
  if (check2(info_block, 0x180, 0xe0))
    return 0x18;
  if (check3(info_block, 0x180, 0x1c0, 0x10))
    return 0x1c;
  if (check3(info_block, 0x1f0, 0x1f0, 0x10))
    return 0x20;

  return 0;
}

int check1(unsigned char *info_block, int index)
{
  int n;

  for (n = 0; n < 16; n++)
    if (info_block[n + index] != info_block[index])
      return 0;

  return 1;
}

int check2(unsigned char *info_block, int index, unsigned char value)
{
  int n;

  for (n = 0; n < 4; n++)
    if (info_block[n + index] != value)
      return 0;

  return 1;
}

int check3(unsigned char *info_block, int index1, int index2, int size)
{
  int n;

  for (n = 0; n < size; n++)
    if (info_block[n + index1] != info_block[n + index2])
      return 0;

  return 1;
}

unsigned char get_emu_mode_select(unsigned char byte, int size)
{
  int x;
  unsigned char ems;

  if (byte == 0)
    x = 0xc;
  else if (byte == 1)
    x = 8;
  else if (byte == 3)
    x = 4;
  else
    x = 0;

  if (interleaved)
  {
    if (x == 0xc && size <= 0x1c)
      ems = 0x1c;
    else
      ems = x + 0x30;
  }
  else
  {
    if (x == 0xc)
      ems = 0x2c;
    else
      ems = x;

    if (size <= 8)
      ems++;
  }

  return ems;
}

int swc_read_sram(char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int blocksleft, bytesreceived = 0;
  unsigned short address;
  time_t starttime;

  init_io(parport);

  if ((file = fopen(filename, "wb")) == NULL)
  {
    fprintf(STDERR, "Can't open %s for writing\n", filename);
    exit(1);
  }
  if ((buffer = malloc(BUFFERSIZE)) == NULL)
  {
    fprintf(STDERR, "Not enough memory for file buffer (%d bytes)\n", BUFFERSIZE);
    exit(1);
  }

  printf("Receive: %d Bytes\n\n", 32*1024);
  memset(buffer, 0, HEADERSIZE);
  buffer[8] = (unsigned char) 0xaa;
  buffer[9] = (unsigned char) 0xbb;
  buffer[10] = (unsigned char) 5;
  fwrite(buffer, 1, HEADERSIZE, file);

  send_command(5, 0, 0);

  send_command(0, 0xe00d, 1);
  sendb(0);
  sendb(0x81);

  send_command(0, 0xc008, 1);
  sendb(0);
  sendb(0x81);

  blocksleft = 4;                               // SRAM is 4*8KB
  address = 0x100;
  starttime = time(NULL);
  while (blocksleft > 0)
  {
    send_command(5, address, 0);
    receive_block(0x2000, buffer, BUFFERSIZE);
    blocksleft--;
    address++;
    fwrite(buffer, 1, BUFFERSIZE, file);

    bytesreceived += BUFFERSIZE;
    parport_gauge(starttime, bytesreceived, 32*1024);
    checkabort(2);
  }

  free(buffer);
  fclose(file);
  return 0;
}

void receive_block(unsigned short address, unsigned char *buffer, int len)
{
  int checksum = 0x81, n, m;

  send_command(1, address, len);
  for (n = 0; n < len; n++)
  {
    buffer[n] = receiveb();
    checksum ^= buffer[n];
  }
  if (checksum != receiveb())
    printf(": received data is corrupt\n");

  for (m = 0; m < 65536; m++)                   // a delay is necessary here
    ;
}

unsigned char receiveb(void)
{
  unsigned char byte;

  byte = (wait_while_busy() & INPUT_MASK) >> 3; // receive low nibble
  outportb(swc_port + PARPORT_CONTROL, inportb(swc_port + PARPORT_CONTROL)^STROBE_BIT); // reverse strobe
  byte |= (wait_while_busy() & INPUT_MASK) << 1; // receive high nibble
  outportb(swc_port + PARPORT_CONTROL, inportb(swc_port + PARPORT_CONTROL)^STROBE_BIT); // reverse strobe

  return byte;
}

unsigned char wait_while_busy(void)
{
  unsigned char input;
  int n_try = 0;

  do
  {
    input = inportb(swc_port + PARPORT_STATUS);
    n_try++;
  } while (input & IBUSY_BIT && n_try < N_TRY_MAX);

#if 0
/*
  vgs doesn't check for this, and it seems to happen quite regularly, so it
  is currently commented out
*/
  if (n_try >= N_TRY_MAX)
  {
    fprintf(STDERR, "The Super Wild Card is not ready\n"	// yes, "ready" :)
                    "Turn it off for a few seconds then turn it on and try again\n");
    exit(1);
  }
#endif

  return input;
}

void wait_for_ready(void)
{
  unsigned char input;
  int n_try = 0;

  do
  {
    input = inportb(swc_port + PARPORT_STATUS);
    n_try++;
  } while (!(input & IBUSY_BIT) && n_try < N_TRY_MAX);

#if 0
  if (n_try >= N_TRY_MAX)
  {
    fprintf(STDERR, "The Super Wild Card is not ready\n"
                    "Turn it off for a few seconds then turn it on and try again\n");
    exit(1);
  }
#endif
}

void checkabort(int status)
{
  if (kbhit() && getch() == 'q')
  {
    puts("\nProgram aborted");
    exit(status);
  }
//  send_command(5, 0, 0);                      // vgs: when sending/receiving a rom
}

int swc_usage(int argc,char *argv[])
{
//  if (argcmp(argc,argv, "-help"))
  {
    printf("\n%s\n", swc_TITLE);
    printf("  -xswc    send/receive ROM to/from Super Wild Card*/(all)SWC; $FILE=PORT\n"
           "           receives automatically when $ROM does not exist\n"
           "  -xswcs   send/receive SRAM to/from Super Wild Card*/(all)SWC; $FILE=PORT\n"
           "           receives automatically when $ROM does not exist\n"
#if 1
           "\n"
           "Press q to abort sending or receiving. Don't press Ctrl-C. If you do the copier\n"
           "can get in an invalid state. Only press Ctrl-C when the program appears to\n"
           "hang. The program can appear to hang if you selected the wrong port for your\n"
           "copier or if the copier is in a wrong state.\n"
#endif
          );
    //TODO more info like technical info about cabeling and stuff for the copier
  }

  return 0;
}

#ifdef  __UNIX__
int stdin_tty = 1;                              // 1 => stdin is a tty, 0 => it's not
tty_t oldtty, newtty;

void set_tty(tty_t param)
{
  if (stdin_tty && tcsetattr(STDIN_FILENO, TCSANOW, &param) == -1)
  {
    fprintf(stderr, "Could not set tty parameters\n");
    exit(100);
  }
}

void init_conio(void)
{
  if (!isatty(STDIN_FILENO))
  {
    stdin_tty = 0;
    return;                                     // rest is nonsense if not a tty
  }

  if (tcgetattr(STDIN_FILENO, &oldtty) == -1)
  {
    fprintf(stderr, "Could not get tty parameters\n");
    exit(101);
  }

  if (atexit(deinit_conio) == -1)
  {
    fprintf(stderr, "Could not register function with atexit()\n");
    exit(102);
  }

  newtty = oldtty;
  newtty.c_lflag &= ~(ICANON | ECHO);
  newtty.c_lflag |= ISIG;
  newtty.c_cc[VMIN] = 1;                        // if VMIN != 0, read calls
  newtty.c_cc[VTIME] = 0;                       //  block (wait for input)

  set_tty(newtty);
}

void deinit_conio(void)
{
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldtty);
}

int kbhit(void)
{
  tty_t tmptty = newtty;
  int ch, key_pressed;

  tmptty.c_cc[VMIN] = 0;
  set_tty(tmptty);

  if ((ch = fgetc(stdin)) != EOF)
  {
    key_pressed = 1;
    ungetc(ch, stdin);
  }
  else
    key_pressed = 0;

  set_tty(newtty);

  return key_pressed;
}
#endif
