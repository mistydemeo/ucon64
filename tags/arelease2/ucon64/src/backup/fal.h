#include "../ucon64.h"

#ifdef DJGPP
 #include <fcntl.h>
 #include <conio.h>
 #include <dos.h>
 #include <io.h>
 #define outp(p,v)  outportb(p,v); iodelay()
 #define inp(p)   inportb(p)
 #define getchr getch
#elif __linux__
// #include <sys/types.h>
// #include <unistd.h>
// #include <curses.h>
// #include <terms.h>
 //#include <termio.h>
 #include <termios.h>
// #include <sys/ioctl.h>
// #include <ncurses/term.h>
// #include <ncurses/curses.h>

 #include <signal.h>
 #include <asm/io.h>


 #define outp(p,v)  out1byte(p,v); iodelay()
 #define inp(p)   in1byte(p)
 #define getchr cnt_getch
 //ioctl_getch
#endif

void iodelay (void);
void control_c(int n);
//void InitLinuxKbhit (init);
int kbhit();
int cnt_getch();
void ProgramExit (int code);
void usage(char *name);
void InitPort (int port);
int PPReadByte (void);
int ReadFlash (void);
void SetCartAddr (int addr);
void WriteFlash (int addr, int data);
void WriteRepeat (int addr, int data, int count);
void VisolyModePreamble (void);
void SetVisolyFlashRWMode (void);
void SetVisolyBackupRWMode (int i);
void l402684 (void);
void LinkerInit (void);
//void l4027c4 (void);
int ReadStatusRegister (int addr);
void DumpSRAM (void);
void BackupROM (FILE *fp, int SizekW);
//void dump (uchar BaseAdr);
void CheckForFC (void);
int GetFileByte (FILE *fp);
int GetFileSize (FILE *fp);
void ProgramNonIntIntelFlash (FILE *fp);
void ProgramInterleaveIntelFlash (FILE *fp);
void ProgramSharpFlash (FILE *fp);
void VerifyFlash (FILE *fp);
void clear_sig(void);
int fal_main(int argc,char *argv[]);

int fal_read(	char *filename
			,unsigned int parport
);

int fal_write(	char *filename
			,long start
			,long len
			,unsigned int parport
);

int fal_usage(int argc,char *argv[]);

#define fal_TITLE "Flash Advance Linker"