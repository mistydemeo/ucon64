/*
nes.c - Nintendo Entertainment System support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
           2002 - 2003 dbjh


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>                             // va_list()
#include <unistd.h>
#include "config.h"
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "nes.h"

#define STD_COMMENT     "Written with uCON64 "  // first part of text to put in READ chunk


const char *nes_usage[] =
  {
    "Nintendo Entertainment System/NES/Famicom/Game Axe (Redant)",
    "1983 Nintendo http://www.nintendo.com",
    "  " OPTION_LONG_S "nes         force recognition\n"
#if 0
    "  " OPTION_LONG_S "hd          force ROM has FFE header (+512 Bytes)\n"
    "  " OPTION_LONG_S "nhd         force ROM has no FFE header\n"
    "  " OPTION_LONG_S "ns          force ROM is not split\n"
#endif
    "  " OPTION_LONG_S "unif        convert to UNIF format/UNF (uses default values)\n"
    "  " OPTION_LONG_S "ines        convert to iNES format/NES (uses default values)\n"
    "  " OPTION_LONG_S "ineshd      extract iNES header from ROM (16 Bytes)\n"
    "  " OPTION_S "j           join Pasofami/PRM/700/PRG/CHR/split ROM (Pasofami -> iNES)\n"
    "  " OPTION_LONG_S "pasofami    convert to Pasofami/PRM/700/PRG/CHR\n"
    "  " OPTION_S "s           convert/split to Pasofami/PRM/700/PRG/CHR (iNES -> Pasofami)\n"
    "  " OPTION_LONG_S "ffe         convert to FFE format\n"
    "  " OPTION_LONG_S "mapr=MAPR   specify board name or mapper number for conversion options\n"
    "                  MAPR must be a board name for UNIF or a number for Pasofami\n"
    "                  and iNES\n"
    "  " OPTION_LONG_S "dint        convert to non-interleaved format\n"
    "  " OPTION_LONG_S "ctrl=TYPE   specify controller type (UNIF only)\n"
    "                  TYPE='0' regular joypad\n"
    "                  TYPE='1' zapper\n"
    "                  TYPE='2' R.O.B.\n"
    "                  TYPE='3' Arkanoid controller\n"
    "                  TYPE='4' powerpad\n"
    "                  TYPE='5' four-score adapter\n"
    "  " OPTION_LONG_S "ntsc        specify TV standard is NTSC (UNIF only)\n"
    "  " OPTION_LONG_S "pal         specify TV standard is PAL (UNIF only)\n"
    "  " OPTION_LONG_S "bat         specify battery is present\n"
    "  " OPTION_LONG_S "nbat        specify battery is not present\n"
    "  " OPTION_LONG_S "vram        specify VRAM override (UNIF only)\n"
    "  " OPTION_LONG_S "nvram       specify no VRAM override (UNIF only)\n"
    "  " OPTION_LONG_S "mirr=MTYPE  specify mirroring type\n"
    "                  MTYPE='0' horizontal mirroring\n"
    "                  MTYPE='1' vertical mirroring\n"
    "                  MTYPE='2' mirror all pages from $2000 (UNIF only)\n"
    "                  MTYPE='3' mirror all pages from $2400 (UNIF only)\n"
    "                  MTYPE='4' four screens of VRAM\n"
    "                  MTYPE='5' mirroring controlled by mapper hardware (UNIF only)\n"
#if     UNIF_REVISION > 7
    "  " OPTION_LONG_S "cmnt=TEXT   specify that TEXT should be used as comment (UNIF only)\n"
#endif
    "  " OPTION_LONG_S "dumpinfo    use dumper info when converting to UNIF; " OPTION_LONG_S "file=INFOFILE\n"
    "  " OPTION_S "n           change internal ROM name (UNIF only); " OPTION_LONG_S "file=NEWNAME\n"
    "  " OPTION_LONG_S "fds         convert Famicom Disk System file (diskimage) from FAM to FDS\n"
    "  " OPTION_LONG_S "fdsl        list Famicom Disk System/FDS (diskimage) contents\n"
#if 0
    "TODO  " OPTION_LONG_S "fam     convert Famicom Disk System file (diskimage) from FDS to FAM\n"
    "TODO:  " OPTION_LONG_S "tr     truncate doubled PRG/CHR\n"
    "TODO:  " OPTION_LONG_S "nfs    convert NFS sound to WAV; " OPTION_LONG_S "rom=NFSFILE\n"
    "  " OPTION_LONG_S "gge         encode GameGenie code; " OPTION_LONG_S "rom=AAAA:VV or " OPTION_LONG_S "rom=AAAA:VV:CC\n"
    "  " OPTION_LONG_S "ggd         decode GameGenie code; " OPTION_LONG_S "rom=XXXXXX or " OPTION_LONG_S "rom=XXXXXXXX\n"
    "  " OPTION_LONG_S "gg          apply GameGenie code (permanent);\n"
    "                  " OPTION_LONG_S "file=XXXXXX or " OPTION_LONG_S "file=XXXXXXXX\n"
#endif
      ,
      NULL};

#if 0
const char *nes_boardtypes = {
    "351258: UNROM\n"
    "351298: UNROM\n"
    "351908\n"
    "352026: TLROM (w/ LS32 for VROM enable control)\n"
    "51555: Acclaim, MMC3B mapper, PRG ROM, CHR ROM\n"
    "53361\n"
    "54425\n"
    "55741\n"
    "56504\n"
    "AMROM: LS161, VRAM, PRG-ROM\n"
    "ANROM: LS161+LS02 mapper, PRG-ROM, CHR-RAM\n"
    "AOROM: LS161 mapper, PRG-ROM, CHR-ROM\n"
    "BNROM: LS161, VRAM, PRG-ROM (Different LS161 bits?  Only used on Deadly Towers)\n"
    "CNROM: LS161 mapper, PRG-ROM, CHR-ROM?/CHR-RAM\n"
    "COB:   \"Glop Top\" style board\n"
    "CPROM: LS04, LS08, LS161, 32K ROM, 16K VRAM (bankswitched, Videomation only)\n"
    "DEIROM\n"
    "DEROM\n"
    "DRROM: MMC3, 4K of nametable RAM (for 4-screen), PRG-ROM, CHR-ROM (only in Gauntlet)\n"
    "EKROM\n"
    "ELROM: MMC5, PRG-ROM, CHR-ROM\n"
    "ETROM: MMC5, PRG-ROM, CHR-ROM, 2x 8k optionnal RAM (battery)\n"
    "EWROM: MMC5, PRG-ROM, CHR-ROM, 32k optionnal RAM (battery)\n"
    "GNROM: LS161 mapper, PRG ROM, CHR ROM\n"
    "HKROM: MMC6B, PRG-ROM, CHR-ROM, Battery\n"
    "MHROM: LS161 mapper, black blob chips. Mario Bros / Duck Hunt multi\n"
    "NES-B4: Same as TLROM\n"
    "NES-BTR: Sunsoft FME7 mapper, PRG ROM, CHR ROM, 8k optionnal RAM \n"
    "NES-QJ:\n"
    "NES-RROM: Same as NROM (Only used in Clu Clu land)\n"
    "NROM: No mapper, PRG-ROM, CHR-ROM\n"
    "PNROM: MMC2, PRG-ROM, CHR-ROM\n"
    "SAROM: MMC1B, PRG ROM, CHR ROM, optional 8k of RAM (battery)\n"
    "SBROM: MMC1A, PRG ROM, CHR ROM (only 32K of CHR ROM)\n"
    "SCEOROM\n"
    "SC1ROM: MMC1B, PRG ROM, CHR ROM\n"
    "SCROM: LS161, LS02, VRAM, PRG-ROM (Similar to UNROM)\n"
    "SEROM: MMC1B, PRG ROM, CHR ROM\n"
    "SFROM\n"
    "SGROM: MMC1B, PRG ROM, 8k CHR RAM\n"
    "SHROM\n"
    "SJROM\n"
    "SKROM: MMC1B, PRG ROM, CHR ROM, 8k optional RAM (battery)\n"
    "SL1ROM: MMC3, PRG ROM, CHR ROM, LS32 (for 128K 28 pin CHR ROMs)\n"
    "SL2ROM\n"
    "SL3ROM\n"
    "SLROM: MMC1A, PRG ROM, CHR ROM\n"
    "SLRROM\n"
    "SN1-ROM AW (Overlord only)\n"
    "SNROM: MMC1A, PRG ROM, CHR ROM/RAM ?, 8k optional RAM (battery)  \n"
    "SOROM: MMC1B2, PRG ROM, VRAM, 16K of WRAM (Battery) Only 8K battery-backed\n"
    "SVROM: MMC1B2, PRG ROM, VRAM, WRAM (Battery)\n"
    "SUROM: MMC1B2, PRG ROM, CHR RAM/(ROM?), 8k battery-backed RAM (DW4?)\n"
    "TEROM: MMC3A, PRG ROM, CHR ROM, (32k ROMs)\n"
    "TFROM: MMC3B, PRG ROM, CHR ROM (64K of CHR only)\n"
    "TGROM: MMC3C, PRG ROM, VRAM (512K of PRG)\n"
    "TKROM: MMC3A, PRG ROM, CHR ROM, 8k optional RAM (battery)\n"
    "TL1ROM: Same as TLROM\n"
    "TLROM: MMC3B, PRG ROM, CHR ROM\n"
    "TLSROM: Same as TLROM\n"
    "TQROM: MMC3B+74HC32, PRG ROM, CHR ROM + 8k of CHR-RAM\n"
    "TSROM: MMC3A, PRG ROM, CHR ROM, 8k optionnal RAM\n"
    "TVROM: MMC3B, PRG ROM, CHR ROM, 4K of Nametable RAM (4-screen)\n"
    "UNROM: 74LS32+74LS161 mapper, 128k PRG, 8k CHR-RAM\n"
    "UOROM\n"};
#endif

enum { INES, UNIF, PASOFAMI, FFE, FDS, FAM } type;

static const char *ines_usage[] = {"iNES header", NULL},
                  *unif_usage[] = {"UNIF header", NULL},
                  *ffe_usage[] = {"FFE header", NULL},
                  *pasofami_usage[] = {"Pasofami file", NULL},
                  *fds_usage[] = {"Famicom Disk System file (diskimage)", NULL};

static st_ines_header_t ines_header;
static st_unif_header_t unif_header;
static st_unknown_header_t ffe_header;

#if     UNIF_REVISION > 7
static const char unif_ucon64_sig[] =
  "uCON64" WRTR_MARKER_S UCON64_VERSION_S WRTR_MARKER_S CURRENT_OS_S;
#else
static const char unif_ucon64_sig[] = STD_COMMENT UCON64_VERSION_S " " CURRENT_OS_S;
#endif
static const int unif_prg_ids[] = {PRG0_ID, PRG1_ID, PRG2_ID, PRG3_ID,
                                   PRG4_ID, PRG5_ID, PRG6_ID, PRG7_ID,
                                   PRG8_ID, PRG9_ID, PRGA_ID, PRGB_ID,
                                   PRGC_ID, PRGD_ID, PRGE_ID, PRGF_ID};
static const int unif_pck_ids[] = {PCK0_ID, PCK1_ID, PCK2_ID, PCK3_ID,
                                   PCK4_ID, PCK5_ID, PCK6_ID, PCK7_ID,
                                   PCK8_ID, PCK9_ID, PCKA_ID, PCKB_ID,
                                   PCKC_ID, PCKD_ID, PCKE_ID, PCKF_ID};
static const int unif_chr_ids[] = {CHR0_ID, CHR1_ID, CHR2_ID, CHR3_ID,
                                   CHR4_ID, CHR5_ID, CHR6_ID, CHR7_ID,
                                   CHR8_ID, CHR9_ID, CHRA_ID, CHRB_ID,
                                   CHRC_ID, CHRD_ID, CHRE_ID, CHRF_ID};
static const int unif_cck_ids[] = {CCK0_ID, CCK1_ID, CCK2_ID, CCK3_ID,
                                   CCK4_ID, CCK5_ID, CCK6_ID, CCK7_ID,
                                   CCK8_ID, CCK9_ID, CCKA_ID, CCKB_ID,
                                   CCKC_ID, CCKD_ID, CCKE_ID, CCKF_ID};

static const char *nes_destfname = NULL, *internal_name;
static int rom_size;
static FILE *nes_destfile;


static void
remove_destfile (void)
{
  if (nes_destfname)
    {
      printf ("Removing: %s\n", nes_destfname);
      fclose (nes_destfile);                    // necessary under DOS/Win9x for DJGPP port
      remove (nes_destfname);
      nes_destfname = NULL;
    }
}


static unsigned int
read_block (unsigned char **data, unsigned int size, FILE *file, const char *format, ...)
{
  va_list argptr;
  unsigned int real_size;

  va_start (argptr, format);
  if ((*data = (unsigned char *) malloc (size)) == NULL)
    {
      vfprintf (stderr, format, argptr);
      exit (1);
    }
  real_size = fread (*data, 1, size, file);
  if (real_size != size)
    printf ("WARNING: Couldn't read %d bytes, only %d bytes were available\n",
      size, real_size);

  va_end (argptr);
  return real_size;
}


static st_unif_chunk_t *
read_chunk (unsigned long id, unsigned char *rom_buffer, int cont)
/*
  The caller is responsible for freeing the memory for the allocated
  st_unif_chunk_t. It should do that by calling free() with the pointer to
  the st_unif_chunk_t. It should do NOTHING for the struct member `data'.
*/
{
// the DEBUG_READ_CHUNK blocks are left here on purpose, don't remove!
//#define DEBUG_READ_CHUNK
#ifdef  DEBUG_READ_CHUNK
  char id_str[5] = "    ";
#endif
  struct
  {
     unsigned int id;                           // chunk identification string
     unsigned int length;                       // data length, in little endian format
  } chunk_header;
  st_unif_chunk_t *unif_chunk;
  static int pos = 0;

  if (!cont)
    pos = 0; // fseek (file, UNIF_HEADER_LEN, SEEK_SET);

#ifdef  WORDS_BIGENDIAN
  id = bswap_32 (id);                           // swap id once instead of chunk_header.id often
#endif
  do
    {
      memcpy (&chunk_header, rom_buffer + pos, sizeof (chunk_header)); // fread (&chunk_header, 1, sizeof (chunk_header), file);
      pos += sizeof (chunk_header);
#ifdef  WORDS_BIGENDIAN
      chunk_header.length = bswap_32 (chunk_header.length);
#endif
//      if (feof (file))
//        break;
#ifdef  DEBUG_READ_CHUNK
      memcpy (id_str, &chunk_header.id, 4);
#ifdef  WORDS_BIGENDIAN
      *((int *) id_str) = bswap_32 (*((int *) id_str));
#endif
      printf ("chunk header: id=%s, length=%d\n", id_str, chunk_header.length);
#endif
      if (chunk_header.id != id)
        {
          if (pos + chunk_header.length >= rom_size) // (fseek (file, chunk_header.length, SEEK_CUR) != 0) // fseek() clears EOF indicator
            break;
          else
            pos += chunk_header.length;
        }
    }
  while (chunk_header.id != id);

  if (chunk_header.id != id || pos >= rom_size) // || feof (file))
    {
#ifdef  DEBUG_READ_CHUNK
      printf ("exit1\n");
#endif
      return (st_unif_chunk_t *) NULL;
    }

  if ((unif_chunk = (st_unif_chunk_t *)
         malloc (sizeof (st_unif_chunk_t) + chunk_header.length)) == NULL)
    {
      fprintf (stderr, "ERROR: Not enough memory for chunk (%d bytes)\n",
        (int) sizeof (st_unif_chunk_t) + chunk_header.length);
      exit (1);
    }
#ifdef  WORDS_BIGENDIAN
  unif_chunk->id = bswap_32 (chunk_header.id);
#else
  unif_chunk->id = chunk_header.id;
#endif
  unif_chunk->length = chunk_header.length;
  unif_chunk->data = &((unsigned char *) unif_chunk)[sizeof (st_unif_chunk_t)];

  memcpy (unif_chunk->data, rom_buffer + pos, chunk_header.length); // fread (unif_chunk->data, 1, chunk_header.length, file);
  pos += chunk_header.length;
#ifdef  DEBUG_READ_CHUNK
  printf ("exit2\n");
#endif
  return unif_chunk;
#undef  DEBUG_READ_CHUNK
}


static void
write_chunk (st_unif_chunk_t *chunk, FILE *file)
{
#ifdef  WORDS_BIGENDIAN
  int x;
  x = bswap_32 (chunk->id);
  fwrite (&x, 1, sizeof (chunk->id), file);
  x = bswap_32 (chunk->length);
  fwrite (&x, 1, sizeof (chunk->length), file);
#else
  fwrite (&chunk->id, 1, sizeof (chunk->id), file);
  fwrite (&chunk->length, 1, sizeof (chunk->length), file);
#endif
  fwrite (chunk->data, 1, chunk->length, file);
}


static int
parse_info_file (st_dumper_info_t *info, const char *fname)
/*
  Format of info file:

  <string for name><newline>
  <d>[<d>]{-,/}<m>[<m>]{-,/}<y><y>[<y>][<y>]{-,/}<newline>
  <string for agent>[<newline>]

  <newline> can be \n (Unix), \r\n (DOS) or \r (Macintosh)

  examples of valid date line:
  22-11-1975
  1/1/01
*/
{
#define SIZE_INFO (99 + 10 + 99 + 3 * 2)        // possibly 3 lines in DOS text format
  int n, prev_n;
  unsigned char buf[SIZE_INFO], number[80];     // 4 should be enough, but don't
                                                //  be too sensitive to errors
  memset (info, 0, sizeof (st_dumper_info_t));
  if (q_fread (buf, 0, SIZE_INFO, fname) == -1)
    return -1;

  for (n = 0; n < 99; n++)
    if (buf[n] == '\n' || buf[n] == '\r')
      break;
  strncpy (info->dumper_name, buf, n);
  info->dumper_name[99] = 0;

  // handle newline, possibly in DOS format
  prev_n = n;
  for (; n < prev_n + 2; n++)
    if (buf[n] != '\n' && buf[n] != '\r')
      break;

  prev_n = n;
  for (; n < prev_n + 2; n++)
    if (buf[n] == '-' || buf[n] == '/')
      break;
  strncpy (number, &buf[prev_n], n - prev_n);
  number[n - prev_n] = 0;
  info->day = strtol (number, NULL, 10);

  n++;
  prev_n = n;
  for (; n < prev_n + 2; n++)
    if (buf[n] == '-' || buf[n] == '/')
      break;
  strncpy (number, &buf[prev_n], n - prev_n);
  number[n - prev_n] = 0;
  info->month = strtol (number, NULL, 10);

  n++;
  prev_n = n;
  for (; n < prev_n + 4; n++)
    if (buf[n] == '\n' || buf[n] == '\r')
      break;
  strncpy (number, &buf[prev_n], n - prev_n);
  number[n - prev_n] = 0;
  info->year = strtol (number, NULL, 10);

  // handle newline, possibly in DOS format
  prev_n = n;
  for (; n < prev_n + 2; n++)
    if (buf[n] != '\n' && buf[n] != '\r')
      break;

  prev_n = n;
  for (; n < prev_n + 99; n++)
    if (buf[n] == '\n' || buf[n] == '\r')
      break;
  strncpy (info->dumper_agent, &buf[prev_n], n - prev_n);
  info->dumper_agent[99] = 0;

  return 0;
#undef  SIZE_INFO
}


static int
nes_ines_unif (FILE *srcfile, FILE *destfile)
{
  int prg_size, chr_size, x;
  unsigned char *prg_data = NULL, *chr_data = NULL, b;
  st_unif_chunk_t unif_chunk;

  // read iNES file
  fread (&ines_header, 1, INES_HEADER_LEN, srcfile);
  if (ines_header.ctrl1 & INES_TRAINER)
    fseek (srcfile, 512, SEEK_CUR);             // discard trainer data (lib_unif does the same)

  prg_size = ines_header.prg_size << 14;
  prg_size = read_block (&prg_data, prg_size, srcfile,
                         "ERROR: Not enough memory for PRG buffer (%d bytes)\n", prg_size);
  chr_size = ines_header.chr_size << 13;
  if (chr_size > 0)
    chr_size = read_block (&chr_data, chr_size, srcfile,
                           "ERROR: Not enough memory for CHR buffer (%d bytes)\n", chr_size);

  // write UNIF file
  memset (&unif_header, 0, UNIF_HEADER_LEN);
  memcpy (&unif_header.signature, UNIF_SIG_S, 4);
  unif_header.revision = UNIF_REVISION;
#ifdef  WORDS_BIGENDIAN
  unif_header.revision = bswap_32 (unif_header.revision);
#endif
  fwrite (&unif_header, 1, UNIF_HEADER_LEN, destfile);

  unif_chunk.id = MAPR_ID;
  if (ucon64.mapr == NULL || strlen (ucon64.mapr) == 0)
    {
      fprintf (stderr, "ERROR: No board name specified\n");
      return -1;
    }
  unif_chunk.length = strlen (ucon64.mapr) + 1; // +1 to include ASCII-z
  unif_chunk.data = (void *) ucon64.mapr;
  if (unif_chunk.length > BOARDNAME_MAXLEN)
    {                                           // Should we give a warning?
      unif_chunk.length = BOARDNAME_MAXLEN;
      ((char *) unif_chunk.data)[BOARDNAME_MAXLEN - 1] = 0;
    }                                           // make it an ASCII-z string
  write_chunk (&unif_chunk, destfile);

#if     UNIF_REVISION > 7
  if (ucon64.comment != NULL && strlen (ucon64.comment) > 0)
    {
      unif_chunk.id = READ_ID;
      unif_chunk.length = strlen (ucon64.comment) + 1; // +1 to include ASCII-z
      unif_chunk.data = (void *) ucon64.comment;
      write_chunk (&unif_chunk, destfile);
    }

  // WRTR chunk can be helpful when debugging
  unif_chunk.id = WRTR_ID;
  unif_chunk.length = strlen (unif_ucon64_sig) + 1;
  unif_chunk.data = (char *) unif_ucon64_sig;
  write_chunk (&unif_chunk, destfile);
#else
  // READ chunk can be helpful when debugging
  unif_chunk.id = READ_ID;
  unif_chunk.length = strlen (unif_ucon64_sig) + 1;
  unif_chunk.data = (char *) unif_ucon64_sig;   // assume ASCII-z (spec is not clear)
  write_chunk (&unif_chunk, destfile);
#endif

  if (UCON64_ISSET (ucon64.tv_standard))
    {
      unif_chunk.id = TVCI_ID;
      unif_chunk.length = 1;
      b = ucon64.tv_standard;                   // necessary for big endian machines
      unif_chunk.data = &b;
      write_chunk (&unif_chunk, destfile);
    }

  if (UCON64_ISSET (ucon64.use_dump_info))
    {
      st_dumper_info_t info;
      if (ucon64.file != NULL && strlen (ucon64.file) > 0)
        {
          if (access (ucon64.file, F_OK) == 0)
            {
              parse_info_file (&info, ucon64.file);
#ifdef  WORDS_BIGENDIAN
              info.year = bswap_16 (info.year);
#endif
              unif_chunk.id = DINF_ID;
              unif_chunk.length = 204;
              unif_chunk.data = &info;
              write_chunk (&unif_chunk, destfile);
            }
          else
            printf ("WARNING: Dumper info file %s does not exist, chunk won't be written\n",
                    ucon64.file);
        }
      else                                      // Is this a warning or an error?
        printf ("WARNING: No dumper info file was specified, chunk won't be written\n");
    }

  if (UCON64_ISSET (ucon64.controller))
    {
      unif_chunk.id = CTRL_ID;
      unif_chunk.length = 1;
      b = ucon64.controller;                    // necessary for big endian machines
      unif_chunk.data = &b;
      write_chunk (&unif_chunk, destfile);
    }

  x = mem_crc32 (prg_size, 0, prg_data);
  unif_chunk.id = PCK0_ID;
  unif_chunk.length = 4;
#ifdef  WORDS_BIGENDIAN
  x = bswap_32 (x);
#endif
  unif_chunk.data = &x;
  write_chunk (&unif_chunk, destfile);

  unif_chunk.id = PRG0_ID;                      // How to detect that x > 0 for PRGx?
  unif_chunk.length = prg_size;
  unif_chunk.data = prg_data;
  write_chunk (&unif_chunk, destfile);

  if (chr_size > 0)
    {
      x = mem_crc32 (chr_size, 0, chr_data);
      unif_chunk.id = CCK0_ID;
      unif_chunk.length = 4;
#ifdef  WORDS_BIGENDIAN
      x = bswap_32 (x);
#endif
      unif_chunk.data = &x;
      write_chunk (&unif_chunk, destfile);

      unif_chunk.id = CHR0_ID;                  // How to detect that x > 0 for CHRx?
      unif_chunk.length = chr_size;
      unif_chunk.data = chr_data;
      write_chunk (&unif_chunk, destfile);
    }

  b = 0;                                        // this is a dummy
  unif_chunk.id = BATR_ID;
  unif_chunk.length = 1;
  unif_chunk.data = &b;
  if (UCON64_ISSET (ucon64.battery))
    {
      if (ucon64.battery)
        write_chunk (&unif_chunk, destfile);
    }
  else if (ines_header.ctrl1 & INES_SRAM)
    write_chunk (&unif_chunk, destfile);

  if (UCON64_ISSET (ucon64.vram))
    {
      if (ucon64.vram)
        {
          b = 0;                                // this is a dummy
          unif_chunk.id = VROR_ID;
          unif_chunk.length = 1;
          unif_chunk.data = &b;
          write_chunk (&unif_chunk, destfile);
        }
    }

  unif_chunk.id = MIRR_ID;
  unif_chunk.length = 1;
  if (UCON64_ISSET (ucon64.mirror))
    {
      if (ucon64.mirror > 5)
        {
          ucon64.mirror = 5;
          printf ("WARNING: Invalid mirroring type specified, using \"%d\"\n",
                  ucon64.mirror);
        }
      b = ucon64.mirror;                        // necessary for big endian machines
      unif_chunk.data = &b;
      write_chunk (&unif_chunk, destfile);
    }
  else if (ines_header.ctrl1 & (INES_MIRROR | INES_4SCREEN))
    {
      if (ines_header.ctrl1 & INES_MIRROR)
        b = 1;
      else                                      // it must be INES_4SCREEN
        b = 4;
      unif_chunk.data = &b;
      write_chunk (&unif_chunk, destfile);
    }

  free (prg_data);
  free (chr_data);                              // free() accepts case "chr_data == NULL"
  return 0;
}


static int
nes_unif_unif (unsigned char *rom_buffer, FILE *destfile)
{
  int x, n;
  st_unif_chunk_t *unif_chunk1, unif_chunk2, *unif_chunk3;
  unsigned char b;

  if (unif_header.revision > UNIF_REVISION)     // conversion already done on b.e. machines
    printf ("WARNING: The UNIF file is of a revision later than %d (%d), but uCON64\n"
            "         doesn't support that revision yet. Some chunks may be discarded.\n",
            UNIF_REVISION, unif_header.revision);
#ifdef  WORDS_BIGENDIAN
  unif_header.revision = bswap_32 (UNIF_REVISION);
#else
  unif_header.revision = UNIF_REVISION;
#endif
  memcpy (&unif_header.signature, UNIF_SIG_S, 4);
  fwrite (&unif_header, 1, UNIF_HEADER_LEN, destfile);

  if ((unif_chunk1 = read_chunk (MAPR_ID, rom_buffer, 0)) == NULL || // no MAPR chunk
      (ucon64.mapr != NULL && strlen (ucon64.mapr) > 0))             // MAPR, but has to change
    {
      unif_chunk2.id = MAPR_ID;
      if (ucon64.mapr == NULL || strlen (ucon64.mapr) == 0) // unif_chunk1 == NULL
        {
          fprintf (stderr, "ERROR: File has no MAPR chunk, but no board name was specified\n");
          return -1;
        }

      // ucon64.mapr != NULL && strlen (ucon64.mapr) > 0
      unif_chunk2.length = strlen (ucon64.mapr) + 1; // +1 to include ASCII-z
      unif_chunk2.data = (void *) ucon64.mapr;
      if (unif_chunk2.length > BOARDNAME_MAXLEN)
        {
          unif_chunk2.length = BOARDNAME_MAXLEN;
          ((char *) unif_chunk2.data)[BOARDNAME_MAXLEN - 1] = 0;
        }                                       // make it an ASCII-z string
      write_chunk (&unif_chunk2, destfile);
    }
  else                                          // MAPR chunk, but no board name specified
    {
      printf ("WARNING: No board name specified, using old value\n");
      write_chunk (unif_chunk1, destfile);
    }
  free (unif_chunk1);                           // case NULL is valid

#if     UNIF_REVISION > 7
  if (ucon64.comment != NULL && strlen (ucon64.comment) > 0)
    {
      unif_chunk2.id = READ_ID;
      unif_chunk2.length = strlen (ucon64.comment) + 1; // +1 to include ASCII-z
      unif_chunk2.data = (void *) ucon64.comment;
      write_chunk (&unif_chunk2, destfile);
    }
  else
    {
      if ((unif_chunk1 = read_chunk (READ_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  if ((unif_chunk1 = read_chunk (WRTR_ID, rom_buffer, 0)) == NULL)
    {
      unif_chunk2.id = WRTR_ID;
      unif_chunk2.length = strlen (unif_ucon64_sig) + 1;
      unif_chunk2.data = (char *) unif_ucon64_sig;
      write_chunk (&unif_chunk2, destfile);
    }
  else
    {
      char ucon64_name[] = "uCON64";
      int sig_added = 0;
      // find uCON64 WRTR chunk and modify it if it is present
      do
        {
          if (!strncmp (unif_chunk1->data, ucon64_name, strlen (ucon64_name)))
            {
              unif_chunk1->length = strlen (unif_ucon64_sig) + 1;
              unif_chunk1->data = (char *) unif_ucon64_sig;
              sig_added = 1; // don't break, because we want to preserve other WRTR chunks
            }
          write_chunk (unif_chunk1, destfile);
          free (unif_chunk1);
        }
      while ((unif_chunk1 = read_chunk (WRTR_ID, rom_buffer, 1)) != NULL);

      if (!sig_added)
        {
          unif_chunk2.id = WRTR_ID;
          unif_chunk2.length = strlen (unif_ucon64_sig) + 1;
          unif_chunk2.data = (char *) unif_ucon64_sig;
          write_chunk (&unif_chunk2, destfile);
        }
    }
#else
  if ((unif_chunk1 = read_chunk (READ_ID, rom_buffer, 0)) == NULL)
    {
      unif_chunk2.id = READ_ID;
      unif_chunk2.length = strlen (unif_ucon64_sig) + 1;
      unif_chunk2.data = (char *) unif_ucon64_sig;
      write_chunk (&unif_chunk2, destfile);     // assume ASCII-z (spec is not clear)
    }
  else
    {
      if (!strncmp (unif_chunk1->data, STD_COMMENT, strlen (STD_COMMENT)))
        { // overwrite uCON64 comment -> OS and version match with the used exe
          unif_chunk1->length = strlen (unif_ucon64_sig) + 1;
          unif_chunk1->data = (char *) unif_ucon64_sig;
        }
      write_chunk (unif_chunk1, destfile);
    }
  free (unif_chunk1);
#endif

  if (internal_name != NULL)
    {
      unif_chunk2.id = NAME_ID;
      unif_chunk2.length = strlen (internal_name) + 1;
      unif_chunk2.data = (char *) internal_name;
      write_chunk (&unif_chunk2, destfile);     // assume ASCII-z (spec is not clear)
    }
  else
    {
      if ((unif_chunk1 = read_chunk (NAME_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  if (UCON64_ISSET (ucon64.tv_standard))
    {
      unif_chunk2.id = TVCI_ID;
      unif_chunk2.length = 1;
      b = ucon64.tv_standard;                   // necessary for big endian machines
      unif_chunk2.data = &b;
      write_chunk (&unif_chunk2, destfile);
    }
  else
    {
      if ((unif_chunk1 = read_chunk (TVCI_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  if (UCON64_ISSET (ucon64.use_dump_info))
    {
      st_dumper_info_t info;
      if (ucon64.file != NULL && strlen (ucon64.file) > 0)
        {
          if (access (ucon64.file, F_OK) == 0)
            {
              parse_info_file (&info, ucon64.file);
/*
              printf ("Dump info:\n"
                      "  dumper: %s\n"
                      "  date: %d-%d-%d\n"
                      "  agent: %s\n",
                      info.dumper_name,
                      info.day, info.month, info.year,
                      info.dumper_agent);
*/
#ifdef  WORDS_BIGENDIAN
              info.year = bswap_16 (info.year);
#endif
              unif_chunk2.id = DINF_ID;
              unif_chunk2.length = 204;
              unif_chunk2.data = &info;
              write_chunk (&unif_chunk2, destfile);
            }
          else
            printf ("WARNING: Dumper info file %s does not exist, chunk won't be written\n",
                    ucon64.file);
        }
      else                                      // Is this a warning or an error?
        printf ("WARNING: No dumper info file was specified, chunk won't be written\n");
    }
  else
    {
      if ((unif_chunk1 = read_chunk (DINF_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  if (UCON64_ISSET (ucon64.controller))
    {
      unif_chunk2.id = CTRL_ID;
      unif_chunk2.length = 1;
      b = ucon64.controller;                    // necessary for big endian machines
      unif_chunk2.data = &b;
      write_chunk (&unif_chunk2, destfile);
    }
  else
    {
      if ((unif_chunk1 = read_chunk (CTRL_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  // copy PRG chunks
  for (n = 0; n < 16; n++)
    {
      if ((unif_chunk1 = read_chunk (unif_prg_ids[n], rom_buffer, 0)) != NULL)
        {
          if ((unif_chunk3 = read_chunk (unif_pck_ids[n], rom_buffer, 0)) == NULL)
            {
              x = mem_crc32 (unif_chunk1->length, 0, unif_chunk1->data);
              unif_chunk2.id = unif_pck_ids[n];
              unif_chunk2.length = 4;
#ifdef  WORDS_BIGENDIAN
              x = bswap_32 (x);
#endif
              unif_chunk2.data = &x;
              write_chunk (&unif_chunk2, destfile);
            }
          else
            {
              x = mem_crc32 (unif_chunk1->length, 0, unif_chunk1->data);
#ifdef  WORDS_BIGENDIAN
              x = bswap_32 (x);
#endif
              if (x != *((int *) unif_chunk3->data))
                printf ("WARNING: PRG chunk %d has a bad checksum, writing new checksum\n", n);
              unif_chunk3->length = 4;
              unif_chunk3->data = &x;
              write_chunk (unif_chunk3, destfile);
            }
          free (unif_chunk3);
          write_chunk (unif_chunk1, destfile);
        }
      free (unif_chunk1);
    }

  // copy CHR chunks
  for (n = 0; n < 16; n++)
    {
      if ((unif_chunk1 = read_chunk (unif_chr_ids[n], rom_buffer, 0)) != NULL)
        {
          if ((unif_chunk3 = read_chunk (unif_cck_ids[n], rom_buffer, 0)) == NULL)
            {
              x = mem_crc32 (unif_chunk1->length, 0, unif_chunk1->data);
              unif_chunk2.id = unif_cck_ids[n];
              unif_chunk2.length = 4;
#ifdef  WORDS_BIGENDIAN
              x = bswap_32 (x);
#endif
              unif_chunk2.data = &x;
              write_chunk (&unif_chunk2, destfile);
            }
          else
            {
              x = mem_crc32 (unif_chunk1->length, 0, unif_chunk1->data);
#ifdef  WORDS_BIGENDIAN
              x = bswap_32 (x);
#endif
              if (x != *((int *) unif_chunk3->data))
                printf ("WARNING: CHR chunk %d has a bad checksum, writing new checksum\n", n);
              unif_chunk3->length = 4;
              unif_chunk3->data = &x;
              write_chunk (unif_chunk3, destfile);
            }
          free (unif_chunk3);
          write_chunk (unif_chunk1, destfile);
        }
      free (unif_chunk1);
    }

  if (UCON64_ISSET (ucon64.battery))
    {
      if (ucon64.battery)
        {
          b = 0;                                // this is a dummy
          unif_chunk2.id = BATR_ID;
          unif_chunk2.length = 1;
          unif_chunk2.data = &b;
          write_chunk (&unif_chunk2, destfile);
        }
    }
  else
    {
      if ((unif_chunk1 = read_chunk (BATR_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  if (UCON64_ISSET (ucon64.vram))
    {
      if (ucon64.vram)
        {
          b = 0;                                // this is a dummy
          unif_chunk2.id = VROR_ID;
          unif_chunk2.length = 1;
          unif_chunk2.data = &b;
          write_chunk (&unif_chunk2, destfile);
        }
    }
  else
    {
      if ((unif_chunk1 = read_chunk (VROR_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  if (UCON64_ISSET (ucon64.mirror))
    {
      if (ucon64.mirror > 5)
        {
          ucon64.mirror = 5;
          printf ("WARNING: Invalid mirroring type specified, using \"%d\"\n",
                  ucon64.mirror);
        }
      unif_chunk2.id = MIRR_ID;
      unif_chunk2.length = 1;
      b = ucon64.mirror;                        // necessary for big endian machines
      unif_chunk2.data = &b;
      write_chunk (&unif_chunk2, destfile);
    }
  else
    {
      if ((unif_chunk1 = read_chunk (MIRR_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  return 0;
}


int
nes_unif (st_rominfo_t *rominfo)
{
  unsigned char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], *rom_buffer;
  FILE *srcfile, *destfile;

  if (type != INES && type != UNIF)
    {
      if (type == PASOFAMI)
        fprintf (stderr, "ERROR: Pasofami -> UNIF is currently not supported\n");
      else if (type == FFE)
        fprintf (stderr, "ERROR: FFE -> UNIF is currently not supported\n");
      else if (type == FDS || type == FAM)
        fprintf (stderr, "ERROR: FDS/FAM -> UNIF is currently not supported\n");
      return -1;
    }

  strcpy (dest_name, ucon64.rom);
  setext (dest_name, ".UNF");
  strcpy (src_name, ucon64.rom);
  handle_existing_file (dest_name, src_name);
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      return -1;
    }

  /*
    remove possible temp file created by handle_existing_file ()
    nes_ines_unif() and nes_unif_unif() might exit() so we use register_func()
  */
  register_func (remove_temp_file);
  nes_destfname = dest_name;
  nes_destfile = destfile;
  register_func (remove_destfile);
  /*
    Converting from UNIF to UNIF should be allowed, because the user might want
    to change some paramaters.
  */
  if (type == INES)
    {
      if ((srcfile = fopen (src_name, "rb")) == NULL)
        {
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
          return -1;
        }

      if (nes_ines_unif (srcfile, destfile) == -1) // -1 == error
        exit (1);

      fclose (srcfile);
    }
  else if (type == UNIF)
    {
      if ((rom_buffer = (unsigned char *) malloc (rom_size)) == NULL)
        {
          fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], rom_size);
          exit (1);
        }
      q_fread (rom_buffer, UNIF_HEADER_LEN, rom_size, src_name);

      if (nes_unif_unif (rom_buffer, destfile) == -1) // -1 == error
        exit (1);

      free (rom_buffer);
    }

  unregister_func (remove_destfile);
  fclose (destfile);
  unregister_func (remove_temp_file);
  remove_temp_file ();
  fprintf (stdout, ucon64_msg[WROTE], dest_name);

  return 0;
}


static void
set_mapper (st_ines_header_t *header, unsigned int mapper)
{
  header->ctrl1 &= 0x0f;                        // clear mapper bits
  header->ctrl2 &= 0x03;                        // clear reserved and mapper bits

  header->ctrl1 |= mapper << 4;
  header->ctrl2 |= mapper & 0xf0;
  if (mapper > 0xff)                            // we support mapper numbers > 255
    {
      if (mapper > 0xfff)
        {
          fprintf (stderr, "ERROR: Mapper numbers greater than 4095 can't be stored\n");
          exit (1);
        }
      // We can't just clear bits 0 & 1 of ctrl2, because they have their own
      //  meaning. So, a warning is in place here.
      printf ("WARNING: Mapper number is greater than 255\n");
      header->ctrl2 |= (mapper >> 8) & 0xf;
    }
}


static int
nes_ines_ines (FILE *srcfile, FILE *destfile, int deinterleave)
{
  int prg_size, chr_size;
  unsigned char *prg_data = NULL, *chr_data = NULL;

  // read iNES file
  fread (&ines_header, 1, INES_HEADER_LEN, srcfile);
  if (ines_header.ctrl1 & INES_TRAINER)
    {
      fseek (srcfile, 512, SEEK_CUR);           // discard trainer data
      ines_header.ctrl1 &= ~INES_TRAINER;       // clear trainer bit
    }

  prg_size = ines_header.prg_size << 14;
  prg_size = read_block (&prg_data, prg_size, srcfile,
                         "ERROR: Not enough memory for PRG buffer (%d bytes)\n", prg_size);
  chr_size = ines_header.chr_size << 13;
  if (chr_size > 0)
    chr_size = read_block (&chr_data, chr_size, srcfile,
                           "ERROR: Not enough memory for CHR buffer (%d bytes)\n", chr_size);

  if (deinterleave)
    /*
      This is a bit of a hack, i.e., putting the following code in this
      function. AFAIK (dbjh) interleaved images contain one big block of data:
      ROM data at even addresses (offsets in the file) and VROM data at odd
      addresses. Currently uCON64 supports deinterleaving of iNES files only,
      so we have to handle the improbable case that the source ROM file
      contains two blocks of interleaved ROM/VROM data.
      Due to the way the data is stored the ROM data will have the same size
      as the VROM data.
    */
    {
      unsigned char *data;
      int size, n = 0, prg = 0, chr = 0;

      size = prg_size + chr_size;
      if ((data = (unsigned char *) malloc (size)) == NULL)
        {
          fprintf (stderr, ucon64_msg[BUFFER_ERROR], size);
          exit (1);
        }
      memcpy (data, prg_data, prg_size);
      memcpy (data + prg_size, chr_data, chr_size);

      if (prg_size != chr_size)
        {
          free (prg_data);
          free (chr_data);
          prg_size = chr_size = size / 2;
          prg_data = (unsigned char *) malloc (prg_size);
          chr_data = (unsigned char *) malloc (chr_size);
          if (prg_data == NULL || chr_data == NULL)
            {
              fprintf (stderr, ucon64_msg[BUFFER_ERROR], size);
              exit (1);
            }
        }

      while (n < size)
        {                                       // deinterleave
          prg_data[prg++] = data[n++];
          chr_data[chr++] = data[n++];
        }
      free (data);
    }

  // write iNES file
  if (ucon64.mapr == NULL || strlen (ucon64.mapr) == 0)
    printf ("WARNING: No mapper number specified, using old value\n");
  else                                          // mapper specified
    set_mapper (&ines_header, strtol (ucon64.mapr, NULL, 10));
  memcpy (&ines_header.signature, INES_SIG_S, 4);
  ines_header.prg_size = prg_size >> 14;
  ines_header.chr_size = chr_size >> 13;

  if (UCON64_ISSET (ucon64.battery))
    {
      if (ucon64.battery)
        ines_header.ctrl1 |= INES_SRAM;
      else
        ines_header.ctrl1 &= ~INES_SRAM;
    }

  if (UCON64_ISSET (ucon64.mirror))
    {
      ines_header.ctrl1 &= ~(INES_MIRROR | INES_4SCREEN); // clear bits
      if (ucon64.mirror == 0)
        ;                                       // default value in ctrl1 (0) is ok
      else if (ucon64.mirror == 1)
        ines_header.ctrl1 |= INES_MIRROR;
      else if (ucon64.mirror == 4)
        ines_header.ctrl1 |= INES_4SCREEN;
      else
        printf ("WARNING: Invalid mirroring type specified, using \"0\"\n");
    }

  ines_header.reserved1 = 0;
  ines_header.reserved2 = 0;
  fwrite (&ines_header, 1, INES_HEADER_LEN, destfile);
  fwrite (prg_data, 1, prg_size, destfile);
  fwrite (chr_data, 1, chr_size, destfile);

  free (prg_data);
  free (chr_data);
  return 0;
}


static int
nes_mapper_number (const char *board_name)
{
  typedef struct
  {
    const char *string;
    int value;
  } st_string_value_t;

  int n;
  st_string_value_t name_to_mapr[] =            // TODO: expand this list
    {
      { "NROM", 0 },
      { "NES-RROM", 0 },
      { "SNROM", 1 },
      { "SOROM", 1 },
      { "SVROM", 1 },
      { "SUROM", 1 },
      { "SAROM", 1 },
      { "SBROM", 1 },
      { "UNROM", 2 },
      { "CNROM", 3 },
      { "TEROM", 4 },
      { "TFROM", 4 },
      { "TGROM", 4 },
      { "TVROM", 4 },
      { "TSROM", 4 },
      { "TQROM", 4 },
      { "TKROM", 4 },
      { "TLSROM", 4 },
      { "DRROM", 4 },
      { "TLROM", 4 },
      { "SL1ROM", 4 },
      { "SL2ROM", 4 },
      { "SL3ROM", 4 },
      { "ELROM", 5 },
      { "ETROM", 5 },
      { "EWROM", 5 },
      { "AOROM", 7 },
      { "PNROM", 9 },
      { NULL }
    };

  n = 0;
  while (name_to_mapr[n].string != NULL)
    {
      if (!strncmp(board_name, name_to_mapr[n].string, BOARDNAME_MAXLEN - 1))
        return name_to_mapr[n].value;
      n++;
    }

  return -1;
}


static int
nes_unif_ines (unsigned char *rom_buffer, FILE *destfile)
{
  int n, x, prg_size = 0, chr_size = 0;
  st_unif_chunk_t *unif_chunk;

  if (unif_header.revision > UNIF_REVISION)     // conversion already done on b.e. machines
    printf ("WARNING: The UNIF file is of a revision later than %d (%d), but uCON64\n"
            "         doesn't support that revision yet. Some chunks may be discarded.\n",
            UNIF_REVISION, unif_header.revision);

  // build iNES header
  memset (&ines_header, 0, INES_HEADER_LEN);
  memcpy (&ines_header.signature, INES_SIG_S, 4);

  if (ucon64.mapr == NULL || strlen (ucon64.mapr) == 0)
    {                                           // no mapper specified, try autodetection
      if ((unif_chunk = read_chunk (MAPR_ID, rom_buffer, 0)) != NULL)
        {
          if ((x = nes_mapper_number (unif_chunk->data)) == -1)
            {
              printf ("WARNING: Couldn't determine mapper number, writing \"0\"\n");
              x = 0;
            }
          set_mapper (&ines_header, x);
          free (unif_chunk);
        }
      else                                      // no MAPR chunk
        {
          fprintf (stderr, "ERROR: File has no MAPR chunk, but no mapper number was specified\n");
          return -1;
        }
    }
  else                                          // mapper specified
    set_mapper (&ines_header, strtol (ucon64.mapr, NULL, 10));

  if (UCON64_ISSET (ucon64.battery))
    {
      if (ucon64.battery)
        ines_header.ctrl1 |= INES_SRAM;
      else
        ines_header.ctrl1 &= ~INES_SRAM;
    }
  else
    {
      if ((unif_chunk = read_chunk (BATR_ID, rom_buffer, 0)) != NULL)
        ines_header.ctrl1 |= INES_SRAM;
      free (unif_chunk);
    }

  if (UCON64_ISSET (ucon64.mirror))
    {
      if (ucon64.mirror == 0)
        ;                                       // default value in ctrl1 (0) is ok
      else if (ucon64.mirror == 1)
        ines_header.ctrl1 |= INES_MIRROR;
      else if (ucon64.mirror == 4)
        ines_header.ctrl1 |= INES_4SCREEN;
      else
        printf ("WARNING: Invalid mirroring type specified, using \"0\"\n");
    }
  else if ((unif_chunk = read_chunk (MIRR_ID, rom_buffer, 0)) != NULL)
    {
      switch (*((unsigned char *) unif_chunk->data))
        {
        case 0:                                 // default value in ctrl1 (0) is ok
        case 2:                                 // can't express in iNES terms
        case 3:                                 // idem
        case 5:                                 // idem
          break;
        case 1:
          ines_header.ctrl1 |= INES_MIRROR;
          break;
        case 4:
          ines_header.ctrl1 |= INES_4SCREEN;
          break;
        default:
          printf ("WARNING: Unsupported value in MIRR chunk\n");
          break;
        }
      free (unif_chunk);
    }

  /*
    Determining the PRG & CHR sizes could be done in the copy loops. Doing it
    here is quite inefficient, but now we can write to a zlib stream. zlib
    doesn't support backward seeks in a compressed output stream. A backward
    seek would be necessary if the size calculation would be done in the copy
    loops.
  */
  for (n = 0; n < 16; n++)
    {
      if ((unif_chunk = read_chunk (unif_prg_ids[n], rom_buffer, 0)) != NULL)
        prg_size += unif_chunk->length;
      free (unif_chunk);
      if ((unif_chunk = read_chunk (unif_chr_ids[n], rom_buffer, 0)) != NULL)
        chr_size += unif_chunk->length;
      free (unif_chunk);
    }

  // write header
  ines_header.prg_size = prg_size >> 14;        // # 16 kB banks
  ines_header.chr_size = chr_size >> 13;        // # 8 kB banks
  fwrite (&ines_header, 1, INES_HEADER_LEN, destfile);

  // copy PRG data
  for (n = 0; n < 16; n++)
    {
      if ((unif_chunk = read_chunk (unif_prg_ids[n], rom_buffer, 0)) != NULL)
        fwrite (unif_chunk->data, 1, unif_chunk->length, destfile);
      free (unif_chunk);
    }

  // copy CHR data
  for (n = 0; n < 16; n++)
    {
      if ((unif_chunk = read_chunk (unif_chr_ids[n], rom_buffer, 0)) != NULL)
        fwrite (unif_chunk->data, 1, unif_chunk->length, destfile);
      free (unif_chunk);
    }

  return 0;
}


int
nes_ines (st_rominfo_t *rominfo)
{
  unsigned char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], *rom_buffer;
  FILE *srcfile, *destfile;

  if (type == FFE)
    {
      fprintf (stderr, "ERROR: FFE -> iNES is currently not supported\n");
      return -1;
    }
  else if (type == FDS || type == FAM)
    {
      fprintf (stderr, "ERROR: FDS/FAM -> iNES is not possible\n");
      return -1;
    }

  // Pasofami doesn't fit well in the source -> destination "paradigm"
  if (type == PASOFAMI)
    return nes_j (rominfo, NULL);

  strcpy (dest_name, ucon64.rom);
  setext (dest_name, ".NES");
  strcpy (src_name, ucon64.rom);
  handle_existing_file (dest_name, src_name);
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      return -1;
    }

  register_func (remove_temp_file);
  nes_destfname = dest_name;
  nes_destfile = destfile;
  register_func (remove_destfile);
  if (type == INES)
    {
      if ((srcfile = fopen (src_name, "rb")) == NULL)
        {
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
          return -1;
        }

      if (nes_ines_ines (srcfile, destfile, 0) == -1) // -1 == error
        exit (1);                       // calls remove_temp_file() & remove_destfile()

      fclose (srcfile);
    }
  else if (type == UNIF)
    {
      if ((rom_buffer = (unsigned char *) malloc (rom_size)) == NULL)
        {
          fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], rom_size);
          exit (1);
        }
      q_fread (rom_buffer, UNIF_HEADER_LEN, rom_size, src_name);

      if (nes_unif_ines (rom_buffer, destfile) == -1) // -1 == error
        exit (1);                       // calls remove_temp_file() & remove_destfile()

      free (rom_buffer);
    }

  unregister_func (remove_destfile);
  fclose (destfile);
  unregister_func (remove_temp_file);
  remove_temp_file ();
  fprintf (stdout, ucon64_msg[WROTE], dest_name);

  return 0;
}


int
nes_pasofami (st_rominfo_t *rominfo)
{
  // nes_s() does iNES -> Pasofami. nes_s() checks for type
  return nes_s (rominfo);
}


int
nes_ffe (st_rominfo_t *rominfo)
{
  st_unknown_header_t header;
  char buf[MAXBUFSIZE];
  long size = ucon64.file_size - rominfo->buheader_len;

  if (type != INES)
    {
      fprintf (stderr, "ERROR: Currently only iNES -> FFE is supported\n");
      return -1;
    }

  strcpy (buf, ucon64.rom);
  setext (buf, ".FFE");

  memset (&header, 0, UNKNOWN_HEADER_LEN);
  header.size_low = size / 8192;
  header.size_high = size / 8192 >> 8;
#if 1
  // TODO: verify if this is correct. It seems logical though.
  header.id1 = 0xaa;
  header.id2 = 0xbb;
#endif
  ucon64_fbackup (NULL, buf);
  q_fwrite (&header, 0, UNKNOWN_HEADER_LEN, buf, "wb");
  q_fcpy (ucon64.rom, rominfo->buheader_len, size, buf, "ab");
  fprintf (stdout, ucon64_msg[WROTE], buf);

  return 0;
}


int
nes_ineshd (st_rominfo_t *rominfo)
{
  char dest_name[FILENAME_MAX];

  if (type != INES)
    {
      fprintf (stderr, "ERROR: This option is only meaningful for iNES files\n");
      return -1;
    }

  strcpy (dest_name, ucon64.rom);
  setext (dest_name, ".HDR");
  ucon64_fbackup (NULL, dest_name);
  q_fcpy (ucon64.rom, rominfo->buheader_start, 16, dest_name, "wb");
  fprintf (stdout, ucon64_msg[WROTE], dest_name);

  return 0;
}


int
nes_dint (st_rominfo_t *rominfo)
{
  unsigned char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  FILE *srcfile, *destfile;

  if (type != INES)
    {
      // Do interleaved UNIF or Pasofami images exist?
      fprintf (stderr, "ERROR: Currently only iNES images can be deinterleaved\n");
      return -1;
    }

  strcpy (dest_name, ucon64.rom);
  setext (dest_name, ".NES");
  strcpy (src_name, ucon64.rom);
  handle_existing_file (dest_name, src_name);
  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      return -1;
    }

  register_func (remove_temp_file);
  nes_destfname = dest_name;
  nes_destfile = destfile;
  register_func (remove_destfile);
  // type == INES
  if (nes_ines_ines (srcfile, destfile, 1) == -1) // -1 == error
    exit (1);                           // calls remove_temp_file() & remove_destfile()

  unregister_func (remove_destfile);
  fclose (srcfile);
  fclose (destfile);
  unregister_func (remove_temp_file);
  remove_temp_file ();
  fprintf (stdout, ucon64_msg[WROTE], dest_name);

  return 0;
}


static int
parse_prm (st_ines_header_t *header, const char *fname)
/*
  Parse a .PRM file. The format has probably been designed be some mentally
  impaired person. I am not mentally impaired, so I just read the information
  that I can use :-)
  The meaning of the bytes comes from Marat Fayzullin's NESLIST.
*/
{
  unsigned char prm[71];                        // .PRM files are 71 bytes in size

  q_fread (prm, 0, 71, fname);

  if (prm[0] == 'V')                            // mirroring
    header->ctrl1 |= INES_MIRROR;
  else if (prm[0] == 'H')
    header->ctrl1 &= ~INES_MIRROR;

  if (prm[1] == 'T')                            // ROM mapper type (2/4/not specified)
    set_mapper (header, 2);
  else if (prm[1] == 'N')
    set_mapper (header, 4);
  else
    set_mapper (header, 0);

  // ignore VROM mapper type ('T' == mapper 2 or 4, nice design)
  // ignore music mode
  // ignore "something related to graphics"
  // ignore "unknown"
  // ignore "validity?"
  // ignore "IRQ control?"
  // ignore "something related to graphics"
  // ignore "display validity?"
  // ignore speed (NMI) control (always 'S')
  // ignore default sprite size (always 'L')
  // ignore default foreground/background (always 'R')
  // ignore "break order?"

  if (prm[14] == 'E')                           // preserve extension RAM
    header->ctrl1 |= INES_SRAM;
  else
    header->ctrl1 &= ~INES_SRAM;

  // ignore "unknown"
  // ignore "something related to interrupts" (always 'S')

  if (prm[17] != 'M')                           // bank-switched ROM?
    set_mapper (header, 0);

  // ignore 9 unknown bytes
  // ignore "partial horizontal scroll?" (always 'X')
  // ignore "don't scroll up to this scanline?" (always "02")
  // ignore "line to do a scroll in?" (always '2')
  // ignore "comment?" (always 'A')

  return 0;
}


int
nes_j (st_rominfo_t *rominfo, unsigned char **mem_image)
/*
  The Pasofami format consists of several files:
  - .PRM: header (uCON64 treats it as optional in order to support RAW images)
  - .700: trainer data (optional)
  - .PRG: ROM data
  - .CHR: VROM data (optional)
*/
{
  unsigned char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], *buffer;
  int prg_size = 0, chr_size = 0, write_file = 0, size, bytes_read = 0, nparts = 0;

  if (type != PASOFAMI)
    {
      fprintf (stderr, "ERROR: Only Pasofami files can be joined (for NES)\n");
      return -1;
    }

  if (mem_image == NULL)
    write_file = 1;

  strcpy (dest_name, ucon64.rom);
  setext (dest_name, ".NES");
  if (write_file)
    ucon64_fbackup (NULL, dest_name);

  // build iNES header
  memset (&ines_header, 0, INES_HEADER_LEN);
  memcpy (&ines_header.signature, INES_SIG_S, 4);

  strcpy (src_name, dest_name);
  setext (src_name, ".PRM");
  if (access (src_name, F_OK) == 0)
    {
      parse_prm (&ines_header, src_name);
      nparts++;
    }
  else if (write_file)                          // Don't print this from nes_init()
    printf ("WARNING: No %s, using default values\n", src_name);

  // Don't do this in parse_prm(), because there might be no .PRM file available
  if (UCON64_ISSET (ucon64.battery))
    {
      if (ucon64.battery)
        ines_header.ctrl1 |= INES_SRAM;
      else
        ines_header.ctrl1 &= ~INES_SRAM;
    }

  if (UCON64_ISSET (ucon64.mirror))
    {
      ines_header.ctrl1 &= ~(INES_MIRROR | INES_4SCREEN); // clear bits
      if (ucon64.mirror == 0)
        ;                                       // default value in ctrl1 (0) is ok
      else if (ucon64.mirror == 1)
        ines_header.ctrl1 |= INES_MIRROR;
      else if (ucon64.mirror == 4)
        ines_header.ctrl1 |= INES_4SCREEN;
      else
        printf ("WARNING: Invalid mirroring type specified, using \"0\"\n");
    }

  strcpy (src_name, dest_name);
  setext (src_name, ".700");
  if (access (src_name, F_OK) == 0 && q_fsize (src_name) >= 512)
    {
      ines_header.ctrl1 |= INES_TRAINER;
      nparts++;
    }

  setext (src_name, ".PRG");
  if (access (src_name, F_OK) != 0)             // .PRG file must exist, but
    {                                           //  not for nes_init()
      if (write_file)
        {
          fprintf (stderr, "ERROR: No %s, can't make image without it\n", src_name);
          exit (1);
        }
    }
  else
    {
      prg_size = q_fsize (src_name);
      nparts++;
    }
  ines_header.prg_size = prg_size >> 14;

  setext (src_name, ".CHR");
  if (access (src_name, F_OK) == 0)
    {
      chr_size = q_fsize (src_name);
      nparts++;
    }
  ines_header.chr_size = chr_size >> 13;

  if (ucon64.mapr == NULL || strlen (ucon64.mapr) == 0)
    {                                           // maybe .PRM contained mapper
      if (write_file)                           // Don't print this from nes_init()
        printf ("WARNING: No mapper number specified, writing \"%d\"\n",
                (ines_header.ctrl1 >> 4) | (ines_header.ctrl2 & 0xf0));
    }
  else  // mapper specified (override unreliable value from .PRM file)
    set_mapper (&ines_header, strtol (ucon64.mapr, NULL, 10));

  size = prg_size + chr_size + ((ines_header.ctrl1 & INES_TRAINER) ? 512 : 0);
  if ((buffer = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], size);
      return -1;
    }

  if (ines_header.ctrl1 & INES_TRAINER)
    {
      setext (src_name, ".700");
      q_fread (buffer, 0, 512, src_name);       // use 512 bytes at max
      bytes_read = 512;
    }

  if (prg_size > 0)
    {
      setext (src_name, ".PRG");
      q_fread (buffer + bytes_read, 0, prg_size, src_name);
      bytes_read += prg_size;
    }

  if (chr_size > 0)
    {
      setext (src_name, ".CHR");
      q_fread (buffer + bytes_read, 0, chr_size, src_name);
    }

  if (write_file)
    {
      q_fwrite (&ines_header, 0, INES_HEADER_LEN, dest_name, "wb");
      q_fwrite (buffer, INES_HEADER_LEN, size, dest_name, "ab");
      fprintf (stdout, ucon64_msg[WROTE], dest_name);
      free (buffer);
    }
  else
    *mem_image = buffer;

  if (!UCON64_ISSET (ucon64.split))
    ucon64.split = nparts;

  return 0;
}


static int
write_prm (st_ines_header_t *header, const char *fname)
{
  unsigned char prm[71] =                       // .PRM files are 71 bytes in size
    "          SLR   S          X022A\r\n"
    "1234567890123456789012345678901234\r\n\x1a";
  int mapper;

  if (UCON64_ISSET (ucon64.mirror))
    {
      header->ctrl1 &= ~(INES_MIRROR | INES_4SCREEN); // clear bits
      if (ucon64.mirror == 0)
        ;                                       // default value in ctrl1 (0) is ok
      else if (ucon64.mirror == 1)
        header->ctrl1 |= INES_MIRROR;
// Pasofami only stores horizontal or vertical mirroring types
//    else if (ucon64.mirror == 4)
//      header->ctrl1 |= INES_4SCREEN;
      else
        printf ("WARNING: Invalid mirroring type specified, using \"0\"\n");
    }
  if (header->ctrl1 & INES_MIRROR)              // mirroring
    prm[0] = 'V';
  else
    prm[0] = 'H';

  mapper = (header->ctrl1 >> 4) | (header->ctrl2 & 0xf0);
  if (mapper % 16 == 2)                         // mod of 16 to do the same as NESLIST,
    {                                           //  but I (dbjh) think this is a bug
      prm[1] = 'T';                             // ROM mapper type (2/4/not specified)
      prm[2] = 'T';                             // VROM mapper type (2/4/not specified)
      prm[4] = 'C';                             // "something related to graphics"
    }                                           //  (2/4/not specified)
  else if (mapper % 16 == 4)
    {
      prm[1] = 'N';
      prm[2] = 'T';
      prm[4] = 'N';
    }

  if (UCON64_ISSET (ucon64.battery))
    {
      if (ucon64.battery)
        header->ctrl1 |= INES_SRAM;
      else
        header->ctrl1 &= ~INES_SRAM;
    }
  if (header->ctrl1 & INES_SRAM)                // preserve extension RAM
    prm[14] = 'E';

  if (mapper)                                   // bank-switched ROM?
    prm[17] = 'M';

  // don't write backups of parts, because one name is used
  if (q_fwrite (prm, 0, sizeof (prm), fname, "wb") == -1)
    {
      fprintf (stderr, ucon64_msg[WRITE_ERROR], fname);
      return -1;                                // try to continue
    }
  else
    fprintf (stdout, ucon64_msg[WROTE], fname);

  return 0;
}


int
nes_s (st_rominfo_t *rominfo)
{
  unsigned char dest_name[FILENAME_MAX], *trainer_data = NULL,
                *prg_data = NULL, *chr_data = NULL;
  int prg_size = 0, chr_size = 0, x;
  FILE *srcfile;

  if (type != INES)
    {
      fprintf (stderr, "ERROR: Currently only iNES -> Pasofami is supported\n");
      return -1;
    }

  if ((srcfile = fopen (ucon64.rom, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ucon64.rom);
      return -1;
    }

  // read iNES file
  fread (&ines_header, 1, INES_HEADER_LEN, srcfile);
  if (ines_header.ctrl1 & INES_TRAINER)
    {
      if (read_block (&trainer_data, 512, srcfile,
                      "ERROR: Not enough memory for trainer buffer (%d bytes)\n", 512) != 512)
        {
          fprintf (stderr, "ERROR: %s is not a valid iNES file", ucon64.rom);
          exit (1);
        }
    }
  prg_size = ines_header.prg_size << 14;
  prg_size = read_block (&prg_data, prg_size, srcfile,
                         "ERROR: Not enough memory for PRG buffer (%d bytes)\n", prg_size);
  chr_size = ines_header.chr_size << 13;
  if (chr_size > 0)
    chr_size = read_block (&chr_data, chr_size, srcfile,
                           "ERROR: Not enough memory for CHR buffer (%d bytes)\n", chr_size);

  if (ucon64.mapr != NULL && strlen (ucon64.mapr) > 0) // mapper specified
    {
      x = strtol (ucon64.mapr, NULL, 10);
      if (x == 0 || x == 2 || x == 4)
        set_mapper (&ines_header, x);
      else
        printf ("WARNING: Pasofami can only store mapper numbers 0, 2 or 4; using old value\n");
    }

  strcpy (dest_name, ucon64.rom);
  setext (dest_name, ".PRM");
  write_prm (&ines_header, dest_name);

  if (ines_header.ctrl1 & INES_TRAINER)
    {
      setext (dest_name, ".700");
      // don't write backups of parts, because one name is used
      if (q_fwrite (trainer_data, 0, 512, dest_name, "wb") == -1)
        fprintf (stderr, ucon64_msg[WRITE_ERROR], dest_name); // try to continue
      else
        fprintf (stdout, ucon64_msg[WROTE], dest_name);
    }

  if (prg_size > 0)
    {
      setext (dest_name, ".PRG");
      // don't write backups of parts, because one name is used
      if (q_fwrite (prg_data, 0, prg_size, dest_name, "wb") == -1)
        fprintf (stderr, ucon64_msg[WRITE_ERROR], dest_name); // try to continue
      else
        fprintf (stdout, ucon64_msg[WROTE], dest_name);
    }
  else
    printf ("WARNING: No PRG data in %s\n", ucon64.rom);

  if (chr_size > 0)
    {
      setext (dest_name, ".CHR");
      // don't write backups of parts, because one name is used
      if (q_fwrite (chr_data, 0, chr_size, dest_name, "wb") == -1)
        fprintf (stderr, ucon64_msg[WRITE_ERROR], dest_name); // try to continue
      else
        fprintf (stdout, ucon64_msg[WROTE], dest_name);
    }

  free (trainer_data);
  free (prg_data);
  free (chr_data);
  fclose(srcfile);
  return 0;
}


int
nes_n (st_rominfo_t *rominfo)
{
  if (type != UNIF)
    {
      fprintf (stderr, "ERROR: This option is only meaningful for UNIF files\n");
      return -1;
    }

  if (ucon64.file != NULL && strlen (ucon64.file) > 0)
    internal_name = ucon64.file;
  else
    internal_name = NULL;

  return nes_unif (rominfo);                    // will call nes_unif_unif()
}


int
nes_init (st_rominfo_t *rominfo)
{
  unsigned char magic[15], *rom_buffer;
  int result = -1, size, x, y, n, crc = 0;
  // currently 92 bytes is enough for ctrl_str, but extra space avoids
  //  introducing bugs when controller type text would be changed
  char buf[MAXBUFSIZE], ctrl_str[200], *str, *str_list[8];
  st_unif_chunk_t *unif_chunk, *unif_chunk2;

  internal_name = NULL;                         // reset this var, see nes_n()
  type = PASOFAMI;                              // reset type, see below

  q_fread (magic, 0, 15, ucon64.rom);
  if (memcmp (magic, INES_SIG_S, 4) == 0)
    {
      type = INES;
      result = 0;
    }
  else if (memcmp (magic, UNIF_SIG_S, 4) == 0)
    {
      type = UNIF;
      result = 0;
    }
  else if (memcmp (magic, FDS_SIG_S, 4) == 0)
    {
      type = FDS;
      result = 0;

      rominfo->buheader_start = FDS_HEADER_START;
      rominfo->buheader_len = FDS_HEADER_LEN;
      // we use ffe_header to save some space in the exe
      q_fread (&ffe_header, FDS_HEADER_START, FDS_HEADER_LEN, ucon64.rom);
      rominfo->buheader = &ffe_header;
    }
  else if (memcmp (magic, "\x01*NINTENDO-HVC*", 15) == 0) // "headerless" FDS/FAM file
    {
      if (ucon64.file_size % 65500 == 192)
        type = FAM;
      else
        type = FDS;
      result = 0;
    }

  if (type == PASOFAMI)                         // INES, UNIF, FDS and FAM are much
    {                                           //  more reliable than EXTCMP()s
      if (!EXTCMP (ucon64.rom, ".prm") ||
          !EXTCMP (ucon64.rom, ".700") ||
          !EXTCMP (ucon64.rom, ".prg") ||
          !EXTCMP (ucon64.rom, ".chr"))
        {
          type = PASOFAMI;
          result = 0;
        }
      else                                      // TODO: finding a reliable means
        {                                       //  for detecting FFE images
          x = q_fgetc (ucon64.rom, 0) * 8192;
          x += q_fgetc (ucon64.rom, 1) * 8192 << 8;
          if (ucon64.file_size - UNKNOWN_HEADER_LEN == x &&
              q_fgetc (ucon64.rom, 8) == 0xaa && q_fgetc (ucon64.rom, 9) == 0xbb)
            {
              type = FFE;
              result = 0;
            }
        }
    }
  if (ucon64.console == UCON64_NES)
    result = 0;

  switch (type)
    {
    case INES:
      rominfo->copier_usage = ines_usage;
      rominfo->buheader_start = INES_HEADER_START;
      rominfo->buheader_len = INES_HEADER_LEN;
      q_fread (&ines_header, INES_HEADER_START, INES_HEADER_LEN, ucon64.rom);
      rominfo->buheader = &ines_header;
      ucon64.split = 0;                         // iNES files are never split

      sprintf (buf, "Internal size: %.4f Mb\n",
        TOMBIT_F ((ines_header.prg_size << 14) + (ines_header.chr_size << 13)));
      strcat (rominfo->misc, buf);

      sprintf (buf, "Internal PRG size: %.4f Mb\n",     // ROM
        TOMBIT_F (ines_header.prg_size << 14));
      strcat (rominfo->misc, buf);

      sprintf (buf, "Internal CHR size: %.4f Mb\n",     // VROM
        TOMBIT_F (ines_header.chr_size << 13));
      strcat (rominfo->misc, buf);

      x = (ines_header.ctrl1 >> 4) | (ines_header.ctrl2 & 0xf0);
      if (ines_header.ctrl2 & 0xf)
        sprintf (buf, "Memory mapper (iNES): %d (%d)\n", x,
          x | ((ines_header.ctrl2 & 0xf) << 8));
      else
        sprintf (buf, "Memory mapper (iNES): %d\n", x);
      strcat (rominfo->misc, buf);

      if (ines_header.ctrl1 & INES_MIRROR)
        str = "vertical";
      else if (ines_header.ctrl1 & INES_4SCREEN)
        str = "four screens of VRAM";
      else
        str = "horizontal";
      sprintf (buf, "Mirroring: %s\n", str);
      strcat (rominfo->misc, buf);

      sprintf (buf, "Save RAM: %s\n", (ines_header.ctrl1 & INES_SRAM) ? "yes" : "no");
      strcat (rominfo->misc, buf);

      sprintf (buf, "512-byte trainer: %s\n",
                (ines_header.ctrl1 & INES_TRAINER) ? "yes" : "no");
      strcat (rominfo->misc, buf);

      sprintf (buf, "VS-System: %s", (ines_header.ctrl2 & 0x01) ? "yes" : "no");
      strcat (rominfo->misc, buf);
      break;
    case UNIF:
      rominfo->copier_usage = unif_usage;
      rominfo->buheader_start = UNIF_HEADER_START;
      rominfo->buheader_len = UNIF_HEADER_LEN;
      q_fread (&unif_header, UNIF_HEADER_START, UNIF_HEADER_LEN, ucon64.rom);
      rominfo->buheader = &unif_header;

      rom_size = ucon64.file_size - UNIF_HEADER_LEN;
      if ((rom_buffer = (unsigned char *) malloc (rom_size)) == NULL)
        {
          fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], rom_size);
          exit (1);
        }
      q_fread (rom_buffer, UNIF_HEADER_LEN, rom_size, ucon64.rom);
      ucon64.split = 0;                         // UNIF files are never split

#ifdef  WORDS_BIGENDIAN
      unif_header.revision = bswap_32 (unif_header.revision);
#endif
      sprintf (buf, "UNIF revision: %d\n", unif_header.revision);
      strcpy (rominfo->misc, buf);

      if ((unif_chunk = read_chunk (MAPR_ID, rom_buffer, 0)) != NULL)
        {
          sprintf (buf, "Board name: %s\n", (char *) unif_chunk->data);
          strcat (rominfo->misc, buf);
        }
      free (unif_chunk);
      if ((unif_chunk = read_chunk (READ_ID, rom_buffer, 0)) != NULL)
        {
          sprintf (buf, "Comment: %s\n", (char *) unif_chunk->data);
          strcat (rominfo->misc, buf);
        }
      free (unif_chunk);
#if     UNIF_REVISION > 7
      if ((unif_chunk = read_chunk (WRTR_ID, rom_buffer, 0)) != NULL)
        {
          char ucon64_name[] = "uCON64";
          strcat (rominfo->misc, "Processed by: ");
          y = 0;
          do
            {
              if (y)
                strcat (rominfo->misc, ", ");
              /*
                The format of the uCON64 WRTR chunk is:
                uCON64<marker><version string><marker><OS string>
                but other tools needn't use the same format. We can only be
                sure that the string starts with the tool name.
              */
              y = strlen (unif_chunk->data);
              x = 0;
              if (!strncmp (unif_chunk->data, ucon64_name, strlen (ucon64_name)))
                {
                  while (x < y)
                    {
                      if (((char *) unif_chunk->data)[x] == WRTR_MARKER)
                        ((char *) unif_chunk->data)[x] = ' ';
                      x++;
                    }
                }
              else
                {
                  while (x < y)
                    {
                      if (((char *) unif_chunk->data)[x] == WRTR_MARKER)
                        {
                          ((char *) unif_chunk->data)[x] = 0;
                          break;
                        }
                      x++;
                    }
                }
              strcat (rominfo->misc, unif_chunk->data);
              y = 1;
              free (unif_chunk);
            }
          while ((unif_chunk = read_chunk (WRTR_ID, rom_buffer, 1)) != NULL);
          strcat (rominfo->misc, "\n");
        }
#endif
      if ((unif_chunk = read_chunk (NAME_ID, rom_buffer, 0)) != NULL)
        {
          sprintf (buf, "Internal name: %s\n", (char *) unif_chunk->data);
          strcat (rominfo->misc, buf);
        }
      free (unif_chunk);
      if ((unif_chunk = read_chunk (TVCI_ID, rom_buffer, 0)) != NULL)
        {
          str_list[0] = "NTSC";
          str_list[1] = "PAL";
          str_list[2] = "NTSC/PAL";
          x = *((unsigned char *) unif_chunk->data);
          sprintf (buf, "Television standard: %s\n", x > 2 ? "unknown" : str_list[x]);
          strcat (rominfo->misc, buf);
        }
      free (unif_chunk);
      if ((unif_chunk = read_chunk (DINF_ID, rom_buffer, 0)) != NULL)
        {
          st_dumper_info_t *info = (st_dumper_info_t *) unif_chunk->data;
          sprintf (buf, "Dump info:\n"
                        "  dumper: %s\n"
                        "  date: %d-%d-%d\n"
                        "  agent: %s\n",
                        info->dumper_name,
                        info->day, info->month,
#ifdef  WORDS_BIGENDIAN
                        bswap_16 (info->year),
#else
                        info->year,
#endif
                        info->dumper_agent);
          strcat (rominfo->misc, buf);
        }
      free (unif_chunk);
      if ((unif_chunk = read_chunk (CTRL_ID, rom_buffer, 0)) != NULL)
        {
          str_list[0] = "regular joypad";
          str_list[1] = "zapper";
          str_list[2] = "R.O.B.";
          str_list[3] = "Arkanoid controller";
          str_list[4] = "power pad";
          str_list[5] = "four-score adapter";
          str_list[6] = "unknown";              // bit 6 and 7 are reserved
          str_list[7] = str_list[6];
          ctrl_str[0] = 0;

          x = *((unsigned char *) unif_chunk->data);
          y = 0;
          for (n = 0; n < 8; n++)
            if (x & (1 << n))
              {
                if (y)
                  strcat (ctrl_str, ", ");
                strcat (ctrl_str, str_list[n]);
                y = 1;
              }
          sprintf (buf, "Supported controllers: %s\n", ctrl_str);
          strcat (rominfo->misc, buf);
        }
      free (unif_chunk);

      size = 0;
      // PRG chunk info
      for (n = 0; n < 16; n++)
        {
          if ((unif_chunk = read_chunk (unif_prg_ids[n], rom_buffer, 0)) != NULL)
            {
              crc = mem_crc32 (unif_chunk->length, crc, unif_chunk->data);
              size += unif_chunk->length;
              if ((unif_chunk2 = read_chunk (unif_pck_ids[n], rom_buffer, 0)) == NULL)
                str = "not available";
              else
                {
                  x = mem_crc32 (unif_chunk->length, 0, unif_chunk->data);
#ifdef  WORDS_BIGENDIAN
                  x = bswap_32 (x);
#endif
                  str =
#ifdef  ANSI_COLOR
                    ucon64.ansi_color ?
                      ((x == *((int *) unif_chunk2->data)) ?
                        "\x1b[01;32mok\x1b[0m" : "\x1b[01;31mbad\x1b[0m")
                      :
                      ((x == *((int *) unif_chunk2->data)) ? "ok" : "bad");
#else
                      (x == *((int *) unif_chunk2->data)) ? "ok" : "bad";
#endif
                }
              sprintf (buf, "PRG%d: %.4f Mb, checksum %s\n",
                n, TOMBIT_F (unif_chunk->length), str);
              strcat (rominfo->misc, buf);
              free (unif_chunk2);
            }
          free (unif_chunk);
        }

      // CHR chunk info
      for (n = 0; n < 16; n++)
        {
          if ((unif_chunk = read_chunk (unif_chr_ids[n], rom_buffer, 0)) != NULL)
            {
              crc = mem_crc32 (unif_chunk->length, crc, unif_chunk->data);
              size += unif_chunk->length;
              if ((unif_chunk2 = read_chunk (unif_cck_ids[n], rom_buffer, 0)) == NULL)
                str = "not available";
              else
                {
                  x = mem_crc32 (unif_chunk->length, 0, unif_chunk->data);
#ifdef  WORDS_BIGENDIAN
                  x = bswap_32 (x);
#endif
                  str =
#ifdef  ANSI_COLOR
                    ucon64.ansi_color ?
                      ((x == *((int *) unif_chunk2->data)) ?
                        "\x1b[01;32mok\x1b[0m" : "\x1b[01;31mbad\x1b[0m")
                      :
                      ((x == *((int *) unif_chunk2->data)) ? "ok" : "bad");
#else
                      (x == *((int *) unif_chunk2->data)) ? "ok" : "bad";
#endif
                }
              sprintf (buf, "CHR%d: %.4f Mb, checksum %s\n",
                        n, TOMBIT_F (unif_chunk->length), str);
              strcat (rominfo->misc, buf);
              free (unif_chunk2);
            }
          free (unif_chunk);
        }
      ucon64.crc32 = crc;
      rominfo->data_size = size;

      // Don't introduce extra code just to make this line be printed above
      //  the previous two line types (PRG & CHR)
      sprintf (buf, "Size: %.4f Mb\n", TOMBIT_F (rominfo->data_size));
      strcat (rominfo->misc, buf);

      x = 0;
      if ((unif_chunk = read_chunk (BATR_ID, rom_buffer, 0)) != NULL)
        x = 1;
      sprintf (buf, "Save RAM: %s\n", x ? "yes" : "no");
      strcat (rominfo->misc, buf);
      free (unif_chunk);
      if ((unif_chunk = read_chunk (MIRR_ID, rom_buffer, 0)) != NULL)
        {
          str_list[0] = "horizontal (hard wired)";
          str_list[1] = "vertical (hard wired)";
          str_list[2] = "all pages from $2000 (hard wired)";
          str_list[3] = "all pages from $2400 (hard wired)";
          str_list[4] = "four screens of VRAM (hard wired)";
          str_list[5] = "controlled by mapper hardware";
          x = *((unsigned char *) unif_chunk->data);
          sprintf (buf, "Mirroring: %s\n", x > 5 ? "unknown" : str_list[x]);
          strcat (rominfo->misc, buf);
        }
      free (unif_chunk);
      x = 0;
      if ((unif_chunk = read_chunk (VROR_ID, rom_buffer, 0)) != NULL)
        x = 1;
      sprintf (buf, "VRAM override: %s", x ? "yes" : "no");
      strcat (rominfo->misc, buf);
      free (unif_chunk);

      free (rom_buffer);
      break;
    case PASOFAMI:
      /*
        Either a *.PRM header file, a 512-byte *.700 trainer file, a *.PRG
        ROM data file or a *.CHR VROM data file.
      */
      rominfo->copier_usage = pasofami_usage;
      rominfo->buheader_start = 0;
      strcpy (buf, ucon64.rom);
      setext (buf, ".PRM");
      if (access (buf, F_OK) == 0)
        {
          rominfo->buheader_len = q_fsize (buf);
          // we use ffe_header to save some space
          q_fread (&ffe_header, 0, rominfo->buheader_len, buf);
          rominfo->buheader = &ffe_header;
        }
      else
        rominfo->buheader_len = 0;

      /*
        Build a temporary iNES image in memory from the Pasofami files.
        In memory, because we want to be able to display info for Pasofami
        files on read-only filesystems WITHOUT messing with/finding temporary
        storage somewhere. We also want to calculate the CRC and it's handy to
        have the data in memory for that.
        Note that nes_j() wouldn't be much different if q_fcrc32() would be
        used. This function wouldn't be much different either.
      */
      x = nes_j (rominfo, &rom_buffer);
      rominfo->data_size = (ines_header.prg_size << 14) + (ines_header.chr_size << 13) +
                             ((ines_header.ctrl1 & INES_TRAINER) ? 512 : 0);
      if (x == 0)
        {                                       // use buf only if it could be allocated
          ucon64.crc32 = mem_crc32 (rominfo->data_size, 0, rom_buffer);
          free (rom_buffer);
        }

      sprintf (buf, "Size: %.4f Mb\n", TOMBIT_F (rominfo->data_size));
      strcat (rominfo->misc, buf);

      sprintf (buf, "PRG size: %.4f Mb\n",      // ROM, don't say internal,
        TOMBIT_F (ines_header.prg_size << 14)); //  because it's not
      strcat (rominfo->misc, buf);

      sprintf (buf, "CHR size: %.4f Mb\n",      // VROM
        TOMBIT_F (ines_header.chr_size << 13));
      strcat (rominfo->misc, buf);

      sprintf (buf, "Memory mapper (iNES): %d\n",
        (ines_header.ctrl1 >> 4) | (ines_header.ctrl2 & 0xf0));
      strcat (rominfo->misc, buf);

      sprintf (buf, "Mirroring: %s\n",
        (ines_header.ctrl1 & INES_MIRROR) ? "vertical" : "horizontal");
      strcat (rominfo->misc, buf);

      sprintf (buf, "Save RAM: %s\n",
        (ines_header.ctrl1 & INES_SRAM) ? "yes" : "no");
      strcat (rominfo->misc, buf);

      sprintf (buf, "512-byte trainer: %s",
        (ines_header.ctrl1 & INES_TRAINER) ? "yes" : "no");
      strcat (rominfo->misc, buf);
      break;
    case FFE:
      /*
        512-byte header
        512-byte trainer (optional)
        ROM data
        VROM data (optional)

        It makes no sense to make a temporary iNES image here. It makes sense
        for Pasofami, because there might be a .PRM file and because there is
        still other information about the image structure.
        FFE as we now know it (probably incomplete) is a plain stupid format,
        because there is no information about the image other than it's size
        and CRC.
      */
      rominfo->copier_usage = ffe_usage;
      rominfo->buheader_start = UNKNOWN_HEADER_START;
      rominfo->buheader_len = UNKNOWN_HEADER_LEN;
      q_fread (&ffe_header, UNKNOWN_HEADER_START, UNKNOWN_HEADER_LEN, ucon64.rom);
      rominfo->buheader = &ffe_header;
      break;
    case FDS:
      rominfo->copier_usage = fds_usage;
      rominfo->country = "Japan";
      strcat (rominfo->misc, "\n");
      nes_fdsl (rominfo, rominfo->misc);        // will also fill in rominfo->name
      break;
    case FAM:
      rominfo->copier_usage = fds_usage;
      rominfo->country = "Japan";

      // FAM files don't have a header. Instead they seem to have a 192 byte trailer.
      rominfo->buheader_start = ucon64.file_size - FAM_HEADER_LEN;
      rominfo->buheader_len = FAM_HEADER_LEN;

      // we use ffe_header to save some space
      q_fread (&ffe_header, rominfo->buheader_start, FAM_HEADER_LEN, ucon64.rom);
      rominfo->buheader = &ffe_header;
      strcat (rominfo->misc, "\n");
      nes_fdsl (rominfo, rominfo->misc);        // will also fill in rominfo->name
      break;
    }
  rominfo->console_usage = nes_usage;

  return result;
}


int
nes_fdsl (st_rominfo_t *rominfo, char *output_str)
/*
  Note that NES people prefer $ to signify hexadecimal numbers (not 0x).
  However we will print only addresses that way.
  This code is based on Marat Fayzullin's FDSLIST.
*/
{
  FILE *srcfile;
  unsigned char buffer[58], name[16], str_list_mem[6], *str_list[4],
                info_mem[MAXBUFSIZE], *info, line[80];
  int disk, n_disks, file, n_files, start, size, x, header_len = 0;

  if (output_str == NULL)
    {
      info = info_mem;
      info[0] = 0;
    }
  else
    info = output_str;

  if ((srcfile = fopen (ucon64.rom, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ucon64.rom);
      return -1;
    }

  n_disks = (ucon64.file_size - rominfo->buheader_len) / 65500;
  x = (ucon64.file_size - rominfo->buheader_len) % 65500;
  if (x)
    {
      sprintf (line, "WARNING: %d excessive bytes\n", x);
      strcat (info, line);
    }

  if (type == FDS)
    header_len = rominfo->buheader_len;
  else if (type == FAM)                         // if type == FAM rominfo->buheader_len
    header_len = 0;                             //  contains the length of the trailer
  for (disk = 0; disk < n_disks; disk++)
    {
      // go to the next disk
      fseek (srcfile, disk * 65500 + header_len, SEEK_SET);

      // read the disk header
      if (fread (buffer, 1, 58, srcfile) != 58)
        {
          fprintf (stderr, "ERROR: Can't read disk header\n");
          fclose (srcfile);
          return -1;
        }

      if (memcmp (buffer, "\x01*NINTENDO-HVC*", 15))
        {
          fprintf (stderr, "ERROR: Invalid disk header\n");
          fclose (srcfile);
          return -1;                            // should we return?
        }

      if (buffer[56] != 2)
        strcat (info, "WARNING: Invalid file number header\n");

      memcpy (name, buffer + 16, 4);
      name[4] = 0;
      if (disk == 0 && output_str != NULL)
        memcpy (rominfo->name, name, 4);
      n_files = buffer[57];
      sprintf (line, "Disk: '%-4s'  Side: %c  Files: %d  Maker: 0x%02x  Version: 0x%02x\n",
               name, (buffer[21] & 1) + 'A', n_files, buffer[15], buffer[20]);
      strcat (info, line);

      file = 0;
      while (file < n_files && fread (buffer, 1, 16, srcfile) == 16)
        {
          if (buffer[0] != 3)
            {
              sprintf (line, "WARNING: Invalid file header block ID (0x%02x)\n", buffer[0]);
              strcat (info, line);
            }

          // get name, data location, and size
          strncpy (name, buffer + 3, 8);
          name[8] = 0;
          start = buffer[11] + 256 * buffer[12];
          size = buffer[13] + 256 * buffer[14];

          x = fgetc (srcfile);
          if (x != 4)
            {
              sprintf (line, "WARNING: Invalid data block ID (0x%02x)\n", x);
              strcat (info, line);
            }

          str_list[0] = "Code";
          str_list[1] = "Tiles";
          str_list[2] = "Picture";
          if (buffer[15] > 2)
            {
              str_list[3] = str_list_mem;
              sprintf (str_list[3], "0x%02x?", buffer[15]);
              buffer[15] = 3;
            }
          /*
            Some FDS files contain control characters in their names. sprintf()
            won't print those character so we have to use mkprint().
          */
          sprintf (line, "%03d $%02x '%-8s' $%04x-$%04x [%s]\n",
                   buffer[1], buffer[2], mkprint (name, '-'), start, start + size - 1,
                   str_list[buffer[15]]);
          strcat (info, line);

          fseek (srcfile, size, SEEK_CUR);
          file++;
        }
      if (disk != n_disks - 1)
        strcat (info, "\n");                    // print newline between disk info blocks
    }

  if (output_str == NULL)
    puts (info);

  return 0;
}


int
nes_fds (st_rominfo_t *rominfo)
/*
  This function converts a Famicom Disk System disk image from .FAM format to
  .FDS format. It does almost the same as -strip apart from three checks
  whether the input file is a valid FAM file.
  The "algorithm" comes from Marat Fayzullin's FAM2FDS.
*/
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], *buffer;
  int n;

  if (type != FAM)
    {
      fprintf (stderr, "ERROR: %s is not a FAM file\n", ucon64.rom);
      return -1;
    }
  if ((buffer = (char *) malloc (65500)) == NULL)
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], 65500);
      return -1;
    }

  strcpy (dest_name, ucon64.rom);
  setext (dest_name, ".FDS");
  strcpy (src_name, ucon64.rom);
  handle_existing_file (dest_name, src_name);

  for (n = 0; n < 4; n++)
    {
      if (q_fread (buffer, n * 65500, 65500, src_name) != 65500)
        break;

      // check disk image for validity
      if (buffer[0] != 1 || buffer[56] != 2 || buffer[58] != 3)
        {
          fprintf (stderr, "ERROR: %s is not a valid FAM file\n", ucon64.rom);
          break;
        }
      // FAM2FDS also does the following:
      //  1 - check if the last chunk is one of a new game (use buffer[16] - buffer[19])
      //  2 - if not, check if buffer[21] (side bit) differs from least significant bit of n
      //  3 - if that is the case _print_ (but don't do anything else) "WRONG" else "OK"

      if (q_fwrite (buffer, n * 65500, 65500, dest_name, n ? "ab" : "wb") != 65500)
        break;
    }
  /*
    FAM2FDS prints an error message if n isn't 4 at this point (a break was
    executed). However, other information seems to indicate that FAM files can
    hold fewer than 4 disk images.
  */

  free (buffer);
  remove_temp_file ();
  fprintf (stdout, ucon64_msg[WROTE], dest_name);

  return 0;
}
