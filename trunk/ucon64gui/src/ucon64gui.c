/*
ucon64gui.c - a GUI for ucon64

written by 2002 NoisyB (noisyb@gmx.net)
           

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
#include "ucon64gui.h"

#include "snes/snes.h"

#include "backup/swc.h"

#include "xpm/trans_1x3.xpm"
#include "xpm/icon_16x16.xpm"
#include "xpm/icon.xpm"
#include "xpm/open.xpm"

void
ucon64_bottom (void)
{

  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0);
  html2gui_br ();
/*  html2gui_textarea (ucon64gui.ucon64_output, 80, 30);

  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0);
  html2gui_br ();
  html2gui_textarea (ucon64gui.cmd, 80, 1);
  html2gui_br ();
*/
  html2gui_img (icon_16x16_xpm, 48, 48, 0);

  html2gui_ ("uCON64gui "
#ifdef __GTK_GUI__
             "(GTK) "
#endif
             "0.1.0 2002 by NoisyB ");
  html2gui_a ("http://ucon64.sf.net", "_blank");

}


void
ucon64_system (void)
{
//  FILE *fh;
  char buf[MAXBUFSIZE];
  char buf2[MAXBUFSIZE];

  switch (ucon64gui.console)
    {
    case ucon64_SNES:
      strcat (ucon64gui.cmd, " -snes");
      break;

    default:
      break;
    }

  sprintf(buf2,"xterm -title \"%s\" -e",ucon64gui.cmd);

  sprintf(buf,"%s %s",buf2,ucon64gui.cmd);

//  html2gui_html_end();
  system (buf);
//  html2gui_html(640,400,0);


/*
  if (!(fh = popen (buf, "r")))
    {
      strcpy (ucon64gui.ucon64_output, "");
      return;
    }

  while (fgets (buf, sizeof buf, fh) != NULL)
    {
      strcat (ucon64gui.ucon64_output, buf);
    }
  pclose (fh);
*/
}

void
ucon64_rom (void)
{
  html2gui_input_file ("Select ROM", ucon64gui.rom);
}

void
ucon64_file (void)
{
//_text nutzen?
  html2gui_input_file ("Select ROM", ucon64gui.file);
}

void
ucon64_info (void)
{
  strcpy (ucon64gui.rom, html2gui_filename);
  sprintf (ucon64gui.cmd, "ucon64 \"%s\"", ucon64gui.rom);
  ucon64_system ();
}

void
ucon64_ls (void)
{
  strcpy (ucon64gui.cmd, "ucon64 -ls .");
  ucon64_system ();
}

void
ucon64_e (void)
{
  strcpy (ucon64gui.rom, html2gui_filename);
  sprintf (ucon64gui.cmd, "ucon64 -e \"%s\"", ucon64gui.rom);
  ucon64_system ();
}


void
ucon64_root (void)
{

  ucon64gui.console = ucon64_UNKNOWN;

//<html>
  html2gui_html (640, 400, 0);

  html2gui_title ("uCON64gui", icon_xpm);


  html2gui_input_submit (ucon64_rom, "Open ROM", "Open ROM", 100, 50,
                         open_xpm);
  html2gui_input_submit (ucon64_e, "Emulate", "Run ROM in an emulator", 100,
                         50, NULL);

  html2gui_img (trans_1x3_xpm, 0, 0, 0);
  html2gui_br ();

  html2gui_ ("Miscellaneous options");
  html2gui_br ();
  html2gui_input_submit (ucon64_info, "Show info",
                         "Click here to see information about ROM", 10, 10,
                         NULL);

/*
  -e            emulate/run ROM (see $HOME/.ucon64rc for more)
  -crc          show CRC32 value of ROM
  -crchd        show CRC32 value of ROM (regarding to +512 Bytes header)
  -dbs          search ROM database (all entries) by CRC32; $ROM=0xCRC32
  -db           ROM database statistics (# of entries)
  -dbv          view ROM database (all entries)
  -ls           generate ROM list for all ROMs; $ROM=DIRECTORY
  -lsv          like -ls but more verbose; $ROM=DIRECTORY
  -rl           rename all files in DIRECTORY to lowercase; $ROM=DIRECTORY
  -ru           rename all files in DIRECTORY to uppercase; $ROM=DIRECTORY
  -hex          show ROM as hexdump; use "ucon64 -hex $ROM|less"
  -find         find string in ROM; $FILE=STRING ('?'==wildcard for ONE char!)
  -c            compare ROMs for differencies; $FILE=OTHER_ROM
  -cs           compare ROMs for similarities; $FILE=OTHER_ROM
  -swap         swap/(de)interleave ALL Bytes in ROM (1234<->2143)
  -ispad        check if ROM is padded
  -pad          pad ROM to full Mb
  -padhd        pad ROM to full Mb (regarding to +512 Bytes header)
  -stp          strip first 512 Bytes (possible header) from ROM
  -ins          insert 512 Bytes (0x00) before ROM
  -strip        strip Bytes from end of ROM; $FILE=VALUE
  -b		apply Baseline/BSL patch (<=x.x); $FILE=PATCHFILE
  -i		apply IPS patch (<=1.2); $FILE=PATCHFILE
  -mki		create IPS patch; $FILE=CHANGED_ROM
  -a		apply APS patch (<=1.2); $FILE=PATCHFILE
  -mka		create APS patch; $FILE=CHANGED_ROM
  -na		change APS description; $ROM=PATCHFILE $FILE=DESCRIPTION
  -ppf		apply PPF patch (<=2.0); $ROM=RAW_IMAGE $FILE=PATCHFILE
  -mkppf	create PPF patch; $ROM=RAW_IMAGE $FILE=CHANGED_IMAGE
  -nppf		change PPF description; $ROM=PATCHFILE $FILE=DESCRIPTION
  -idppf	change PPF FILE_ID.DIZ (2.0); $ROM=PATCHFILE $FILE=FILE_ID.DIZ
*/
  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0);
//  html2gui_hr ();
  html2gui_ ("Console specific options");
  html2gui_br ();
  html2gui_input_submit (ucon64_snes, "Super Nintendo",
                         "Options for Super Nintendo", 10, 10, NULL);

  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0);
//  html2gui_hr ();
  html2gui_ ("Backup unit specific options");
  html2gui_br ();
  html2gui_input_submit (ucon64_swc, "Super Wild Card",
                         "Options for Super Wild Card", 10, 10, NULL);

  ucon64_bottom ();

//</html>
}

int
main (int argc, char *argv[])
{
  html2gui_start (argc, argv);

  ucon64_root ();

  html2gui_end ();

  return (0);
}


/*
Dreamcast
1998 SEGA http://www.sega.com
  -dc           force recognition; NEEDED
  *		show info (default); ONLY $ROM=ISO_IMAGE
TODO:  -iso     convert DiscJuggler3/CDI IMAGE to MODE2/2336; $ROM=CDI_IMAGE
  -ppf		apply PPF patch (<=2.0); $ROM=RAW_IMAGE $FILE=PATCHFILE
  -mkppf	create PPF patch; $ROM=RAW_IMAGE $FILE=CHANGED_IMAGE
  -nppf		change PPF description; $ROM=PATCHFILE $FILE=DESCRIPTION
  -idppf	change PPF FILE_ID.DIZ (2.0); $ROM=PATCHFILE $FILE=FILE_ID.DIZ
CD-ReWriter
http://cdrdao.sourceforge.net/ (recommended burn engine)
  -xcdrw        read/write ISO/RAW/BIN IMAGE; $ROM=CD_IMAGE $FILE=TRACK_MODE
                reads automatically when $ROM does not exist
                $FILE="MODE1"          (2048 Bytes; standart ISO9660)
                $FILE="MODE1_RAW"      (2352 Bytes)
                $FILE="MODE2"          (2336 Bytes)
                $FILE="MODE2_FORM1"    (2048 Bytes)
                $FILE="MODE2_FORM2"    (2324 Bytes)
                $FILE="MODE2_FORM_MIX" (2336 Bytes)
                $FILE="MODE2_RAW"      (2352 Bytes; default)
  -mktoc        generate TOC file for cdrdao; $ROM=CD_IMAGE $FILE=TRACK_MODE

Playstation (One)/Playstation 2 (CD only)
1994/(2000) Sony http://www.playstation.com
  -psx		force recognition; NEEDED
  *		show info (default); ONLY $ROM=RAW_IMAGE
  -ppf		apply PPF patch (<=2.0); $ROM=RAW_IMAGE $FILE=PATCHFILE
  -mkppf	create PPF patch; $ROM=RAW_IMAGE $FILE=CHANGED_IMAGE
  -nppf		change PPF description; $ROM=PATCHFILE $FILE=DESCRIPTION
  -idppf	change PPF FILE_ID.DIZ (2.0); $ROM=PATCHFILE $FILE=FILE_ID.DIZ
CD-ReWriter
http://cdrdao.sourceforge.net/ (recommended burn engine)
  -xcdrw        read/write ISO/RAW/BIN IMAGE; $ROM=CD_IMAGE $FILE=TRACK_MODE
                reads automatically when $ROM does not exist
                $FILE="MODE1"          (2048 Bytes; standart ISO9660)
                $FILE="MODE1_RAW"      (2352 Bytes)
                $FILE="MODE2"          (2336 Bytes)
                $FILE="MODE2_FORM1"    (2048 Bytes)
                $FILE="MODE2_FORM2"    (2324 Bytes)
                $FILE="MODE2_FORM_MIX" (2336 Bytes)
                $FILE="MODE2_RAW"      (2352 Bytes; default)
  -mktoc        generate TOC file for cdrdao; $ROM=CD_IMAGE $FILE=TRACK_MODE
DexDrive
InterAct http://www.dexdrive.de
TODO:  -xdex    send/receive SRAM to/from DexDrive; $FILE=PORT
		receives automatically when $ROM(=SRAM) does not exist

Playstation 2
2000 Sony http://www.playstation.com
Saturn
1994 SEGA http://www.sega.com
Real3DO
1993
CD32
1993 Commodore
CD-i
1991
  -ps2, -sat, -3do, -cd32, -cdi
                force recognition; NEEDED
  *             show info (default); ONLY $ROM=RAW_IMAGE
  -iso          convert RAW/BIN to ISO9660; $ROM=RAW_IMAGE
  -ppf		apply PPF patch (<=2.0); $ROM=RAW_IMAGE $FILE=PATCHFILE
  -mkppf	create PPF patch; $ROM=RAW_IMAGE $FILE=CHANGED_IMAGE
  -nppf		change PPF description; $ROM=PATCHFILE $FILE=DESCRIPTION
  -idppf	change PPF FILE_ID.DIZ (2.0); $ROM=PATCHFILE $FILE=FILE_ID.DIZ
CD-ReWriter
http://cdrdao.sourceforge.net/ (recommended burn engine)
  -xcdrw        read/write ISO/RAW/BIN IMAGE; $ROM=CD_IMAGE $FILE=TRACK_MODE
                reads automatically when $ROM does not exist
                $FILE="MODE1"          (2048 Bytes; standart ISO9660)
                $FILE="MODE1_RAW"      (2352 Bytes)
                $FILE="MODE2"          (2336 Bytes)
                $FILE="MODE2_FORM1"    (2048 Bytes)
                $FILE="MODE2_FORM2"    (2324 Bytes)
                $FILE="MODE2_FORM_MIX" (2336 Bytes)
                $FILE="MODE2_RAW"      (2352 Bytes; default)
  -mktoc        generate TOC file for cdrdao; $ROM=CD_IMAGE $FILE=TRACK_MODE

GameBoy Advance
2001 Nintendo http://www.nintendo.com
  -gba          force recognition
  -hd           force ROM has header (+512 Bytes)
  -nhd          force ROM has no header
  *             show info (default)
  -n            change ROM name; $FILE=NEWNAME
  -logo         restore ROM logo character data 0x04-0x9F
  -sram         patch ROM for SRAM saving
  -crp          slow down Flash Advance Linker access for ROM (crash patch);
                $FILE=WAIT_TIME
                $FILE=0  (default in most crash patches)
                $FILE=4  (faster than 0, slower than 8)
                $FILE=8  (faster than 4, slower than 28)
                $FILE=12 (slowest cartridge access speed)
                $FILE=16 (faster than 28, but slower than 20)
                $FILE=20 (default in most original carts)
                $FILE=24 (fastest cartridge access speed)
                $FILE=28 (faster than 8 but slower than 16)
  -strip        strip Bytes from end of ROM (use -ispad before); $FILE=VALUE
  -multi        make multirom for Flash Advance Linker; file with loader must
                be specified first, then all the ROMs, multirom to create last
  -multi2       same as -multi but truncate multirom size to 128Mb
  -i		apply IPS patch (<=1.2); $FILE=PATCHFILE
  -mki		create IPS patch; $FILE=CHANGED_ROM
Flash Advance Linker
2001 Visoly http://www.visoly.com
  -xfal         send/receive ROM to/from Flash Advance Linker; $FILE=PORT
                receives automatically when $ROM does not exist
  -xfalc<n>     specify chip size in mbits of ROM in Flash Advance Linker when
                receiving. n can be 8,16,32,64,128 or 256. default is -xfalc32
  -xfals        send/receive SRAM to/from Flash Advance Linker; $FILE=PORT
                receives automatically when $ROM(=SRAM) does not exist
  -xfalb<n>     send/receive SRAM to/from Flash Advance Linker bank n
                $FILE=PORT; receives automatically when SRAM does not exist

                You only need to specify PORT if uCON64 doesn't detect the
                (right) parallel port. If that is the case give a hardware
                address, for example:
                ucon64 -xfal "0087 - Mario Kart Super Circuit (U).gba" 0x378

Nintendo 64
1996 Nintendo http://www.nintendo.com
  -n64          force recognition
  -swp          force ROM is swapped (2143)
  -nswp         force ROM is not swapped (1234)
  *             show info (default)
  -n            change ROM name; $FILE=NEWNAME
  -v64          convert to Doctor V64 (and compatibles/swapped)
  -z64          convert to Z64 (Zip Drive/not swapped)
  -bot          add/extract boot code to/from ROM; $FILE=BOOTCODE
                extracts automatically when $FILE does not exist (4032 Bytes)
  -sram         LAC's Makesram; $ROM=(LAC's SRAM ROM image) $FILE=SRAMFILE
                the SRAMFILE must have a size of 512 Bytes
  -usms         Jos Kwanten's ultraSMS (Sega Master System/GameGear Emulator);
                $ROM=(Jos Kwanten's ultraSMS ROM image) $FILE=SMSROM(<=4Mb)
  -chk          fix ROM checksum
NOTE: supports only 6101 and 6102 boot codes
  -p            pad ROM to full Mb
  -a		apply APS patch (<=1.2); $FILE=PATCHFILE
  -mka		create APS patch; $FILE=CHANGED_ROM
  -na		change APS description; $ROM=PATCHFILE $FILE=DESCRIPTION
  -i		apply IPS patch (<=1.2); $FILE=PATCHFILE
  -mki		create IPS patch; $FILE=CHANGED_ROM
Doctor V64
19XX Bung Enterprises Ltd http://www.bung.com.hk
  -xv64		send/receive ROM to/from Doctor V64; $FILE=PORT
		receives automatically when $ROM does not exist
Doctor64 Jr
19XX Bung Enterprises Ltd http://www.bung.com.hk
TEST:  -xdjr    send/receive ROM to/from Doctor64 Jr; $FILE=PORT
  		receives automatically when $ROM does not exist
CD64
19XX UFO http://www.cd64.com
TODO:  -xcd64	send/receive ROM to/from CD64; $FILE=PORT
		receives automatically when $ROM does not exist
DexDrive
InterAct http://www.dexdrive.de
TODO:  -xdex    send/receive SRAM to/from DexDrive; $FILE=PORT
		receives automatically when $ROM(=SRAM) does not exist


Neo Geo Pocket/Neo Geo Pocket Color
1998/1999 SNK http://www.neogeo.co.jp
  -ngp		force recognition
  *		show info (default)
Flash Pocket Linker
http://www.mrflash.com
TODO:  -xfpl    send/receive ROM to/from Flash Pocket Linker; $FILE=PORT
		receives automatically when $ROM does not exist

Neo Geo/Neo Geo CD(Z)/MVS
1990/1994 SNK http://www.neogeo.co.jp
  -ng		force recognition
  -ns		force ROM is not splitted
  *		show info (default)
  -bios		convert NeoCd Bios to work with NeoCD emulator; $ROM=BIOS
		http://www.illusion-city.com/neo/
  -sam		convert SAM/M.A.M.E. sound to WAV; $ROM=SAMFILE
CD-ReWriter
http://cdrdao.sourceforge.net/ (recommended burn engine)
  -xcdrw        read/write ISO/RAW/BIN IMAGE; $ROM=CD_IMAGE $FILE=TRACK_MODE
                reads automatically when $ROM does not exist
                $FILE="MODE1"          (2048 Bytes; standart ISO9660)
                $FILE="MODE1_RAW"      (2352 Bytes)
                $FILE="MODE2"          (2336 Bytes)
                $FILE="MODE2_FORM1"    (2048 Bytes)
                $FILE="MODE2_FORM2"    (2324 Bytes)
                $FILE="MODE2_FORM_MIX" (2336 Bytes)
                $FILE="MODE2_RAW"      (2352 Bytes; default)
  -mktoc        generate TOC file for cdrdao; $ROM=CD_IMAGE $FILE=TRACK_MODE

Genesis/Sega Mega Drive/Sega CD/32X/Nomad
1989/19XX/19XX SEGA http://www.sega.com
  -gen          force recognition
  -hd           force ROM has SMD header (+512 Bytes)
  -nhd          force ROM has no SMD header (MGD2/MGH/RAW)
  -ns           force ROM is not splitted
  *             show info (default)
  -smd          convert to Super Magic Drive/SMD
  -smds         convert Emulator (*.srm) SRAM to Super Magic Drive/SMD
                $ROM=SRAM
  -stp          convert SRAM from backup unit for use with an Emulator
NOTE: -stp just strips the first 512 bytes
  -mgd          convert to Multi GameMGD2/MGH/RAW
  -j            join splitted ROM
  -n            change foreign ROM name; $FILE=NEWNAME
  -n2           change japanese ROM name; $FILE=NEWNAME
  -s            split ROM into 4Mb parts (for backup unit(s) with fdd)
  -p            pad ROM to full Mb
  -chk          fix ROM checksum
  -1991         fix old third party ROMs to work with consoles build after
                October 1991 by inserting "(C) SEGA" and "(C)SEGA"
  -gge          encode GameGenie code; $ROM=AAAAAA:VVVV
  -ggd          decode GameGenie code; $ROM=XXXX-XXXX
  -b		apply Baseline/BSL patch (<=x.x); $FILE=PATCHFILE
  -i		apply IPS patch (<=1.2); $FILE=PATCHFILE
  -mki		create IPS patch; $FILE=CHANGED_ROM
Super Com Pro (HK)/Super Magic Drive/SMD
19XX Front Far East/FFE http://www.front.com.tw
TEST:  -xsmd    send/receive ROM to/from Super Magic Drive/SMD; $FILE=PORT
                receives automatically when $ROM does not exist
TEST:  -xsmds   send/receive SRAM to/from Super Magic Drive/SMD; $FILE=PORT
                receives automatically when $ROM(=SRAM) does not exist
Multi Game Doctor(2)/Multi Game Hunter/MGH/RAW
19XX Bung Enterprises Ltd http://www.bung.com.hk
?Makko Toys Co., Ltd.?
TODO:  -xmgd    send/receive ROM to/from Multi GameMGD2/MGH/RAW; $FILE=PORT
		receives automatically when $ROM does not exist
CD-ReWriter
http://cdrdao.sourceforge.net/ (recommended burn engine)
  -xcdrw        read/write ISO/RAW/BIN IMAGE; $ROM=CD_IMAGE $FILE=TRACK_MODE
                reads automatically when $ROM does not exist
                $FILE="MODE1"          (2048 Bytes; standart ISO9660)
                $FILE="MODE1_RAW"      (2352 Bytes)
                $FILE="MODE2"          (2336 Bytes)
                $FILE="MODE2_FORM1"    (2048 Bytes)
                $FILE="MODE2_FORM2"    (2324 Bytes)
                $FILE="MODE2_FORM_MIX" (2336 Bytes)
                $FILE="MODE2_RAW"      (2352 Bytes; default)
  -mktoc        generate TOC file for cdrdao; $ROM=CD_IMAGE $FILE=TRACK_MODE

GameBoy/(Super GB)/GB Pocket/Color GB/(GB Advance)
1989/1994/1996/1998/2001 Nintendo http://www.nintendo.com
  -gb		force recognition
  -hd		force ROM has SSC header (+512 Bytes)
  -nhd		force ROM has no SSC header
  *		show info (default)
  -n		change ROM name; $FILE=NEWNAME
  -mgd		convert to Multi GameMGD2/RAW
  -ssc		convert to Super Smart Card/SSC (+512 Bytes)
  -sgb		convert from GB Xchanger/GB/GBC to Super Backup Card/GX/GBX
  -gbx		convert from Super Backup Card/GX/GBX to GB Xchanger/GB/GBC
  -n2gb		convert for use with Kami's FC Emulator (NES Emulator);
		$ROM=NES_ROM $FILE=FC.GB (the Emulator)
		m-kami@da2.so-net.ne.jp www.playoffline.com
  -chk		fix ROM checksum
  -gge		encode GameGenie code; $ROM=AAAA:VV or $ROM=AAAA:VV:CC
  -ggd		decode GameGenie code; $ROM=XXX-XXX or $ROM=XXX-XXX-XXX
  -gg		apply GameGenie code (permanent);
		$FILE=XXX-XXX or $FILE=XXX-XXX-XXX
  -b		apply Baseline/BSL patch (<=x.x); $FILE=PATCHFILE
  -i		apply IPS patch (<=1.2); $FILE=PATCHFILE
  -mki		create IPS patch; $FILE=CHANGED_ROM
GameBoy Xchanger
  -xgbx         send/receive ROM to/from GB Xchanger; $FILE=PORT
                receives automatically when $ROM does not exist
  -xgbxs        send/receive SRAM to/from GB Xchanger; $FILE=PORT
                receives automatically when $ROM(=SRAM) does not exist
  -xgbxb<n>     send/receive 64kbits SRAM to/from GB Xchanger bank n
                $FILE=PORT; receives automatically when $ROM does not exist

                You only need to specify PORT if uCON64 doesn't detect the
                (right) parallel port. If that is the case give a hardware
                address, for example:
                ucon64 -xgbx "Pokemon (Green).gb" 0x378

Panther(32bit prototype)/Jaguar64/Jaguar64 CD
1989 Flare2/1993 Atari/1995 Atari
  -jag		force recognition
  -hd		force ROM has header (+512 Bytes)
  -nhd		force ROM has no header
  *		show info (default)
CD-ReWriter
http://cdrdao.sourceforge.net/ (recommended burn engine)
  -xcdrw        read/write ISO/RAW/BIN IMAGE; $ROM=CD_IMAGE $FILE=TRACK_MODE
                reads automatically when $ROM does not exist
                $FILE="MODE1"          (2048 Bytes; standart ISO9660)
                $FILE="MODE1_RAW"      (2352 Bytes)
                $FILE="MODE2"          (2336 Bytes)
                $FILE="MODE2_FORM1"    (2048 Bytes)
                $FILE="MODE2_FORM2"    (2324 Bytes)
                $FILE="MODE2_FORM_MIX" (2336 Bytes)
                $FILE="MODE2_RAW"      (2352 Bytes; default)
  -mktoc        generate TOC file for cdrdao; $ROM=CD_IMAGE $FILE=TRACK_MODE

Handy(prototype)/Lynx/Lynx II
1987 Epyx/1989 Atari/1991 Atari
  -lynx		force recognition
  -hd		force ROM has LNX header (+64 Bytes)
  -nhd		force ROM has no LNX header
  *		show info (default)
TODO:  -o2lyx        convert *.O file to LYX format (for developers)
  -lyx		convert to LYX/RAW (strip 64 Bytes LNX header)
  -lnx		convert to LNX (uses default values for the header);
		adjust the LNX header with the following options
  -n		change ROM name (LNX only); $FILE=NEWNAME
  -nrot		set no rotation (LNX only)
  -rotl		set rotation left (LNX only)
  -rotr		set rotation right (LNX only)
  -b0		change Bank0 kBytes size (LNX only); $FILE={0,64,128,256,512}
  -b1		change Bank1 kBytes size (LNX only); $FILE={0,64,128,256,512}

PC-Engine (CD Unit/Core Grafx(II)/Shuttle/GT/LT/Super CDROM/DUO(-R(X)))
Super Grafx/Turbo (Grafx(16)/CD/DUO/Express)
1987/19XX/19XX NEC
  -pce		force recognition
  -hd		force ROM has SMG header (+512 Bytes)
  -nhd		force ROM has no SMG header (MGD2/RAW)
  *		show info (default)
  -smg		convert to Super Magic Griffin/SMG
  -mgd		convert to Multi Game DoctorMGD2/RAW
Multi Game Doctor(2)/Multi Game Hunter/MGH/RAW
19XX Bung Enterprises Ltd http://www.bung.com.hk
?Makko Toys Co., Ltd.?
TODO:  -xmgd    send/receive ROM to/from Multi GameMGD2/MGH/RAW; $FILE=PORT
		receives automatically when $ROM does not exist
CD-ReWriter
http://cdrdao.sourceforge.net/ (recommended burn engine)
  -xcdrw        read/write ISO/RAW/BIN IMAGE; $ROM=CD_IMAGE $FILE=TRACK_MODE
                reads automatically when $ROM does not exist
                $FILE="MODE1"          (2048 Bytes; standart ISO9660)
                $FILE="MODE1_RAW"      (2352 Bytes)
                $FILE="MODE2"          (2336 Bytes)
                $FILE="MODE2_FORM1"    (2048 Bytes)
                $FILE="MODE2_FORM2"    (2324 Bytes)
                $FILE="MODE2_FORM_MIX" (2336 Bytes)
                $FILE="MODE2_RAW"      (2352 Bytes; default)
  -mktoc        generate TOC file for cdrdao; $ROM=CD_IMAGE $FILE=TRACK_MODE

Sega Master System(II/III)/GameGear (Handheld)
1986/19XX SEGA http://www.sega.com
  -sms		force recognition
  -hd		force ROM has header (+512 Bytes)
  -nhd		force ROM has no header (MGD2/MGH/RAW)
  *		show info (default)
  -mgd		convert to Multi GameMGD2/MGH/RAW
  -smd		convert to Super Magic Drive/SMD (+512 Bytes)
  -smds         convert Emulator (*.srm) SRAM to Super Magic Drive/SMD
  -gge		encode GameGenie code; $ROM=AAAA:VV or $ROM=AAAA:VV:CC
  -ggd		decode GameGenie code; $ROM=XXX-XXX or $ROM=XXX-XXX-XXX
  -gg		apply GameGenie code (permanent); $FILE=CODE-CODE
Super Com Pro (HK)/Super Magic Drive/SMD
19XX Front Far East/FFE http://www.front.com.tw
TEST:  -xsmd    send/receive ROM to/from Super Magic Drive/SMD; $FILE=PORT
                receives automatically when $ROM does not exist
TEST:  -xsmds   send/receive SRAM to/from Super Magic Drive/SMD; $FILE=PORT
                receives automatically when $ROM(=SRAM) does not exist

Nintendo Entertainment System/NES
1983 Nintendo http://www.nintendo.com
  -nes          force recognition
  *             show info/contents (default)
  -hd           force ROM has FFE header (+512 Bytes)
  -nhd          force ROM has no FFE header
  -ns           force ROM is not splitted
  -ffe          convert to FFE (+512 Bytes)
  -ines         convert to iNES(Emu)/RAW
  -ineshd       extract iNES header from ROM (16 Bytes)
TODO:  -unif    convert to UNIF format/UNF
  -j            join Pasofami/PRM/PRG/CHR/700/splitted ROM
  -gge          encode GameGenie code; $ROM=AAAA:VV or $ROM=AAAA:VV:CC
  -ggd          decode GameGenie code; $ROM=XXXXXX or $ROM=XXXXXXXX
  -gg           apply GameGenie code (permanent);
                $FILE=XXXXXX or $FILE=XXXXXXXX

WonderSwan/WonderSwan Color
19XX/19XX Bandai
  -swan		force recognition
  *             show info (default)
TODO:  -chk     fix ROM checksum

Sega System 16(A/B)/Sega System 18/dual 68000
1987/19XX/19XX SEGA http://www.sega.com
Atari VCS 2600(aka Stella)/Atari 5200 SuperSystem/Atari CX7800/Atari 2600 Jr
1977/1982/1984/1986 Atari
ColecoVision
1982
Nintendo Virtual Boy
19XX Nintendo http://www.nintendo.com
Vectrex
1982
Intellivision
1979 Mattel
  -s16, -ata, -coleco, -vboy, -vec, -intelli
                force recognition
  -hd           force ROM has header (+512 Bytes)
  -nhd          force ROM has no header
  *             show info (default)

Database: 15217 known ROMs in ucon64_db.c (+0)

TIP: ucon64 -help -snes (would show only Super Nintendo related help)
     ucon64 -help|less (to see everything in less)
     give the force recognition option a try if something went wrong

Report problems/ideas/fixes to noisyb@gmx.net or go to http://ucon64.sf.net


*/
