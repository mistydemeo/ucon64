<?php
//phpinfo();
require_once ("config.php");
require_once ("misc/misc.php");
require_once ("misc/widget.php");

  if ($use_gzip)
    ob_start ("ob_gzhandler"); // enable gzip

?>
<html>
<head>
<?php

  echo misc_head_tags ("images/icon.png");    // icon


  $p = "snes, magicom, magic drive, bung, game doctor, multi game hunter, wild card, dx2, pro fighter, "
      ."smart bros, multi game doctor, dragon boy, gamestation, game master, game doctor, mini doctor, "
      ."magic card, magic griffin, super magic drive, hacker, super disk, "
      ."interceptor, mega disk interceptor, super disk, z64, v64, doctor v64, super "
      ."twin, e-merger, v64jr, magic drive plus, super ufo, cd64, supercom partner, "
      ."yoko, super charger, unimex duplikator, game station, Professor, saturn, "
      ."neo-classic, neo, classic, gaming, java, games, online, n64, jaguar, atari, "
      ."mp3, midi, music, files, mp3s, midis, nes, famicom, clone, multi-carts, "
      ."multi carts, multi cart, mega drive, master system, snes cd, unit, history, "
      ."story, saturn mod, modded, cart list, game list, full, hardware, connectors, "
      ."modifications, pal, ntsc";

  echo '<meta name="Description" content="'.$p.'">';
  echo '<meta name="keywords" content="'.$p.'">';

?>
<title>uCON64 - The backup tool and wonderful emulator's Swiss Army knife program</title>
<link rel="stylesheet" media="all" href="index.css">
</head>
<body bgcolor="#5070d0" text="#000000" link="#0000ee" vlink="#0000ee" alink="#ffffff">
<span style="font-family: monospace;">
<br>
<br>
<br>
<center><img src="images/logo.png" width="418" height="121" border="0"></center><br>
<br>
<br>
<font size="-1">
<center><img src="images/figures1.png" border="0"></center>
<br>
<br>
<center>
<table border="0" bgcolor="#ffffff" width="610">
  <tr>
    <td valign="top">
<font size="-1">
<br>
<center><img src="images/new.png" border="0" width="200"></center>
<br>
<!--NEWS-->
20151231 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="#ucon64">uCON64 2.0.1 released</a><br>
20081010 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="#ucon64">The uCON64 CVS has been restored to the last working version</a><br>
20061110 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="#ucon64">The uCON64 CVS version is currently broken due to heavy<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;development</a><br>
<a name="flc">20060923 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="http://flc.berlios.de">flc has been moved to its own site</a><br>
20060914 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="#flc">flc 1.4.1 has been released</a><br>
20060412 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="#flc">flc 1.4.0 has been released</a><br>
20050312 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="#ucon64gui">uCON64 frontend 1.1 released (including a Mac OS X binary)</a><br>
20050126 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="#download">Amiga binaries of uCON64 2.0.0 available</a><br>
20050107 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="#ucon64gui">uCON64 frontend 1.0 released</a><br>
20050107 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="#ucon64">uCON64 2.0.0 released</a><br>
20041028 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="ucon64/hardware.html">updated hardware list (current CVS version of uCON64)</a><br>
20041028 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="#flc">unexpected release of flc 1.2.0</a><br>
20040319 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="#ucon64">uCON64 1.9.8-4 released</a><br>
<br>
    </td>
  </tr>
</table>
</center>
<br>
<br>
<img src="images/hr.png" width="100%" height="24" border="0">
<br>
<br>
<br>
<a name="ucon64"></a><center>
<table border="0" bgcolor="#ffffff" width="610">
  <tr>
    <td valign="top"><font size="-1">
<!-- keep them in one line or they'll look fucked up in mozilla -->
<a href="ucon64/readme.html">ReadME</a>|<a href="ucon64/faq.html#4">Install</a>|<a href="ucon64/hardware.html">Hardware</a>|<a href="ucon64/faq.html">FAQ</a>|<a href="ucon64/changes.html">Changes</a>|<a href="ucon64/license.html">License</a>|<a href="ucon64/developers.html#3">CVS</a>|<a href="mailto:ucon64-main@lists.sf.net">Bugs?</a>|<a href="mailto:ucon64-main@lists.sf.net">Contact</a>
<br>
<center>
<img src="images/black.png" height="10" width="590">
</center><font size="-1">
<img src="images/logo.png" widht="173" height="50"><br>
uCON64 is the backup tool and wonderful emulator's Swiss Army knife program<br>
It may be freely redistributed under the terms of the GNU Public License<br>
<br>
uCON64 is completely Open Source Software and has been ported to
<a href="#download">many platforms</a> using code from
<a href="ucon64/developers.html">various people</a><br>
<br>
Support for almost every video game system (Consoles, Handheld, and Arcade)
with very verbose ROM information is continuously added and updated<br>
<br>
Support for <a href="ucon64/hardware.html">many different backup units</a>
to backup/restore ROM(s) and SRAM(s) is also continuously added and updated
(if their manufacturers cooperate)<br>
<br>
Supports all common patch file formats like IPS (with RLE compression), APS,
BSL (Baseline Patch format), PPF (Playstation Patch File), and Game Genie<br>
<br>
Support for use and creation of <a href="http://www.romcenter.com">RomCenter</a>
Data files (DAT) for secure detection of bad dumps<br>
<br>
Has many options for every kind of ROM or SRAM handling, modification, and management
<br>
Supports gzip and zip<br>
<br>
Have a look at <a href="#ucon64misc">uCON64misc</a> for support files like
parport drivers or at <a href="#ucon64dat">uCON64dat</a> for DAT files
generated with/for uCON64<br>
<br>
<a href="ucon64/SWC-compatibility.txt">SWC-compatibility.txt</a>
A Super Wild Card compatibility list<br>
<br>
<!-- keep them in one line or they'll look fucked up in mozilla -->
Screenshots: <a href="images/ucon64_ss1.png">1</a>|<a href="images/ucon64_ss2.png">2</a>|<a href="images/ucon64_ss3.png">3</a>|<a href="images/ucon64_ss4.png">4</a>|<a href="images/ucon64_ss5.png">5</a>
<br>
<center>
<img src="images/black.png" height="10" width="590">
</center>
<a name="download"></a>
<img src="images/download.png"><br>
<b>CVS</b><br>
To get the latest uCON64 version from CVS read <a href="ucon64/developers.html#3">this</a><br>
<br>
<b>Sources</b><br><font size="-1">
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.1/ucon64-2.0.1-src.tar.gz/download">ucon64-2.0.1-src.tar.gz</a><br>
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.1/ucon64-2.0.1-src.zip/download">ucon64-2.0.1-src.zip</a><br>
<br>
<b>Binaries</b><br><font size="-1">
<br><img src="images/linux.png"><br>
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.1/ucon64-2.0.1-linux-x86_64-bin.tar.gz/download">ucon64-2.0.1-linux-x86_64-bin.tar.gz</a><br>
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.1/ucon64-2.0.1-usb-linux-x86_64-bin.tar.gz/download">ucon64-2.0.1-usb-linux-x86_64-bin.tar.gz</a> (with USB support, for USB version of F2A and Quickdev16)<br>
<br><img src="images/win32.png"> (MinGW)<br>
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.1/ucon64-2.0.1-win32-mingw-bin.zip/download">ucon64-2.0.1-win32-mingw-bin.zip</a><br>
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.1/ucon64-2.0.1-usb-win32-mingw-bin.zip/download">ucon64-2.0.1-usb-win32-mingw-bin.zip</a> (with USB support, for Quickdev16)<br>
<br><img src="images/win32.png"> (Visual C++)<br>
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.1/ucon64-2.0.1-win32-vc-bin.zip/download">ucon64-2.0.1-win32-vc-bin.zip</a><br>
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.1/ucon64-2.0.1-usb-win32-vc-bin.zip/download">ucon64-2.0.1-usb-win32-vc-bin.zip</a> (with USB support, for Quickdev16)<br>
<br><img src="images/cygwin-icon.png"> (Cygwin)<br>
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.1/ucon64-2.0.1-win32-cygwin-bin.zip/download">ucon64-2.0.1-win32-cygwin-bin.zip</a><br>
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.1/ucon64-2.0.1-usb-win32-cygwin-bin.zip/download">ucon64-2.0.1-usb-win32-cygwin-bin.zip</a> (with USB support, for Quickdev16)<br>
<br><img src="images/dos.png"><br>
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.1/ucon64-2.0.1-dos-bin.zip/download">ucon64-2.0.1-dos-bin.zip</a><br>
<br><img src="images/beos.png"><br>
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.0/ucon64-2.0.0-beos-bin.zip/download">ucon64-2.0.0-beos-bin.zip</a><br>
<br><img src="images/solaris.png"><br>
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.0/ucon64-2.0.0-solaris-bin.tar.gz/download">ucon64-2.0.0-solaris-bin.tar.gz</a><br>
<br><img src="images/freebsd.png"><br>
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.0/ucon64-2.0.0-freebsd-bin.tar.gz/download">ucon64-2.0.0-freebsd-bin.tar.gz</a><br>
<br><img src="images/openbsd.png"><br>
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.0/ucon64-2.0.0-openbsd-bin.tar.gz/download">ucon64-2.0.0-openbsd-bin.tar.gz</a><br>
<br><img src="images/macos.png"><br>
<a href="http://sourceforge.net/projects/ucon64/files/ucon64/ucon64-2.0.0/ucon64-2.0.0-macosx-bin.tar.gz/download">ucon64-2.0.0-macosx-bin.tar.gz</a><br>
<br><img src="images/amigaos.png"> (PPC & 68k)<br>
<a href="http://main.aminet.net/misc/emu/ucon64.lha">ucon64.lha</a><br>
    </td>
  </tr>
</table>
</center>
<br>
<br>
<img src="images/hr.png" width="100%" height="24" border="0">
<br>
<br>
<br>
<a name="ucon64dat"></a><center>
<table border="0" bgcolor="#ffffff" width="610">
  <tr>
    <td valign="top"><font size="-1">
<!-- keep them in one line or they'll look fucked up in mozilla -->
<a href="ucon64/faq.html">FAQ</a>|<a href="ucon64/developers.html#3">CVS</a>|<a href="mailto:ucon64-main@lists.sf.net">Bugs?</a>|<a href="mailto:ucon64-main@lists.sf.net">Contact</a>
<br>
<center>
<img src="images/black.png" height="10" width="590">
</center><font size="-1">
<img src="images/ucon64dat_small.png" border="0" height="50"><br>
<br>
Completely new, tested and working DAT files for all known ROMs generated with uCON64 (uCON64 option: --mkdat=DATFILE)<br>
You may read the <a href="ucon64/faq.html">FAQ</a> or just put them into ~/.ucon64/dat/ (might depend on which platform you use)<br>
They should also work with other ROM checking tools<br>
<br>
<center>
<img src="images/black.png" height="10" width="590">
</center>
<img src="images/download.png"><br><font size="-1">
<br>
<a href="http://prdownloads.sourceforge.net/ucon64/snes-even-better.zip?download">
  snes-even-better.zip</a> (by JohnDie and dbjh with many thanks to Cowering and Nach)<br>
<a href="http://prdownloads.sourceforge.net/ucon64/gen-20031115.zip?download">
  gen-20031115.zip</a> (by pauloB)<br>
<a href="http://prdownloads.sourceforge.net/ucon64/gg-20031115.zip?download">
  gg-20031115.zip</a> (by pauloB)<br>
<a href="http://prdownloads.sourceforge.net/ucon64/sms-20031115.zip?download">
  sms-20031115.zip</a> (by pauloB)<br>
<a href="http://prdownloads.sourceforge.net/ucon64/n64-20031115.zip?download">
  n64-20031115.zip</a> (by pauloB)<br>
<a href="http://prdownloads.sourceforge.net/ucon64/fds-20031116.zip?download">
  fds-20031116.zip</a> (by pauloB)<br>
<a href="http://prdownloads.sourceforge.net/ucon64/pce-20031116.zip?download">
  pce-20031116.zip</a> (by pauloB)<br>
<a href="http://prdownloads.sourceforge.net/ucon64/nes-20031208.zip?download">
  nes-20031208.zip</a> (by JohnDie)<br>
to be continued...<br>
<br>
For more DAT files see <a href="http://emulationrealm.net/rcdat.php#Cowering_GoodTools">http://emulationrealm.net/</a> or <a href="http://www.gbadat.altervista.org">http://www.gbadat.altervista.org</a><br>
    </td>
  </tr>
</table>
</center>
<br>
<br>
<img src="images/hr.png" width="100%" height="24" border="0">
<br>
<br>
<br>
<a name="ucon64gui"></a><center>
<table border="0" bgcolor="#ffffff" width="610">
  <tr>
    <td valign="top"><font size="-1">
<img src="images/logo_gui.png" border="0" height="50"><br>
You hate the commandline for being a commandline? No problem! Here are some
frontends for your favourite console tool and different platforms.<br>
Currently there exist FRONTENDS for Linux (Unix), Windows, BeOS and Java.<br>
<!-- keep them in one line or they'll look fucked up in mozilla -->
<br>
Screenshots: <a href="images/screenshot1.jpg">1</a>|<a href="images/screenshot2.jpg">2</a>|<a href="images/screenshot3.jpg">3</a>|<a href="images/screenshot4.jpg">4</a>|<a href="images/screenshot5.jpg">5</a>|<a href="images/screenshot6.jpg">6</a><br>
<br>
<center>
<img src="images/black.png" height="10" width="590">
</center>
<img src="images/download.png"><br>
<b>Sources</b><br><font size="-1">
<a href="http://prdownloads.sourceforge.net/ucon64/uf-FOX-1.1-src.tgz?download">uf-FOX-1.1-src.tgz</a><br>
<a href="http://prdownloads.sourceforge.net/ucon64/SeeUCONv1.0-src.zip?download">SeeUCONv1.0-src.zip</a> (does NOT work correctly with uCON64 2.0.0)<br>
<br>
<b>Binaries</b><br><font size="-1">
<img src="images/linux.png"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/uf-FOX-1.1-linux-bin.zip?download">uf-FOX-1.1-linux-bin.zip</a><br>
<br>
<img src="images/win32.png"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/uf-FOX-1.1-win32-bin.zip?download">uf-FOX-1.1-win32-bin.zip</a><br>
<br>
<img src="images/macos.png"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/uf-FOX-1.1-macosx-bin.zip?download">uf-FOX-1.1-macosx-bin.zip</a> (with thanks to Steve Paige)<br>
<br><img src="images/beos.png"><br>
can be found <a href="http://www.planetmir.de/html/softw.htm#ucon64">here</a> (thanks to Ove)<br>
<br><img src="images/java.png"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/SeeUCONv1.0.zip?download">SeeUCONv1.0.zip</a> (does NOT work correctly with uCON64 2.0.0)<br>
    </td>
  </tr>
</table>
</center>
<br>
<br>
<img src="images/hr.png" width="100%" height="24" border="0">
<br>
<br>
<br>
<a name="ucon64misc"></a><center>
<table border="0" bgcolor="#ffffff" width="610">
  <tr>
    <td><font size="-1">
<img src="images/ucon64misc.png" border="0" height="50"><font size="-1"><br>
<br>
<a href="ucon64misc/conn.html">conn.html</a> (html)<br>
<a href="http://prdownloads.sourceforge.net/ucon64/conn.pdf?download">conn.pdf</a> (pdf)<br>
Quick guide to the most Video Game Connectors and Cables<br>
<br>
<a href="http://prdownloads.sourceforge.net/ucon64/cxa1645.pdf?download">cxa1645.pdf</a><br>
CXA1645P/M is an RGB encoder used in many consoles and other video hardware<br>
These specifications were made for you if your console lacks S-Video or RGB
and IF you have some knowledge about electronics<br>
<br>
<a href="http://prdownloads.sourceforge.net/ucon64/faqs.zip?download">faqs.zip</a> (~1.1 MB)<br>
A loose collection of FAQ's about Hardware, modifications like PAL/NTSC
switches, or backup unit manuals<br>
<br>
<a href="http://prdownloads.sourceforge.net/ucon64/custom.zip?download">custom.zip</a><br>
Selfmade read-only backup unit for Super Nintendo and Sega Mega Drive with sources and PCB layouts (not tested)<br>
<!--br>
<a href="http://prdownloads.sourceforge.net/ucon64/warppipe-0.1.3-gpl-src.tar.gz?download">warppipe-0.1.3-gpl-src.tar.gz</a><br>
<a href="http://prdownloads.sourceforge.net/ucon64/warppipe-0.1.3-gpl-src.zip?download">warppipe-0.1.3-gpl-src.zip</a><br>
This is the release of the latest WarpPipe version with GPL license (as found
on Sourceforge CVS).<br>
This project is free for "adoption" since all original developers forked to a
closed source project with the same name but a different license.<br>
--><br>
<img src="images/dos.png"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/userport.zip?download">userport.zip</a><br>
<a href="http://prdownloads.sourceforge.net/ucon64/giveio.zip?download">giveio.zip</a><br>
I/O port driver for Windows NT, Windows 2000 or Windows XP<br>
Only necessary to access the parallel port<br>
<br>
<img src="images/dos.png"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/vgs.zip?download">vgs.zip</a><br>
Original sources of VGS the well-known transfer tool from JSI<br>
<br>
<img src="images/dos.png"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/dos32.zip?download">dos32.zip</a><br>
<a href="http://prdownloads.sourceforge.net/ucon64/dos4gw.zip?download">dos4gw.zip</a><br>
<a href="http://prdownloads.sourceforge.net/ucon64/csdpmi5b.zip?download">csdpmi5b.zip</a><br>
Some DOS extenders/DPMI hosts. uCON64 needs the files in csdpmi5b.zip under plain DOS (DOS without Windows)<br>
<br>
<img src="images/win32.png"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/port95nt.exe?download">port95nt.exe</a><br>
One of the three DLL I/O port drivers supported by the Windows versions of uCON64 (see the FAQ for links to the other two)<br>
Only necessary to access the parallel port<br>
<br>
<img src="images/win32.png"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/zlib.zip?download">zlib.zip</a><br>
zlib.dll<br>
<br>
<a href="http://www.cygwin.com"><img src="images/cygwin-icon.png" border="0"></a> (Cygwin)<br>
<a href="http://prdownloads.sourceforge.net/ucon64/cygwindll.zip?download">cygwindll.zip</a><br>
<a href="http://prdownloads.sourceforge.net/ucon64/cygzdll.zip?download">cygzdll.zip</a><br>
Needed by the uCON64 Cygwin port to work<br>
<br>
<img src="images/beos.png"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/ioport.zip?download">ioport.zip</a><br>
I/O port driver from <a href="http://www.infernal.currantbun.com/">Caz</a><br>
Only necessary to access the parallel port<br>
<br>
<img src="images/fal.png" width="60"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/preboot32.zip?download">preboot32.zip</a><br>
Loader for Flash Advance Linker (uCON64 options: --multi, --xfalmulti, --xfal)<br>
<br>
<a href="http://prdownloads.sourceforge.net/ucon64/mdpacku4.zip?download">mdpacku4.zip</a><br>
Loader for MD-PRO (uCON64 options: --multi, --xmd)<br>
<br>
<a href="http://prdownloads.sourceforge.net/ucon64/smsdbug3.zip?download">smsdbug3.zip</a><br>
Loader for SMS-PRO (uCON64 options: --multi, --xgg)<br>
<br>
<a href="http://prdownloads.sourceforge.net/ucon64/ggdbug3.zip?download">ggdbug3.zip</a><br>
Loader for GG-PRO (uCON64 options: --multi, --xgg)<br>
<br>
<a href="http://prdownloads.sourceforge.net/ucon64/pceboot.zip?download">pceboot.zip</a><br>
Loader for PCE-PRO (uCON64 options: --multi, --xpce)<br>
<br>
<a href="http://prdownloads.sourceforge.net/ucon64/sfbotx2.zip?download">sfbotx2.zip</a><br>
Loader for Super Flash (uCON64 options: --multi, --xsf)<br>
<br>
<a href="http://prdownloads.sourceforge.net/ucon64/xmcd-iso.zip?download">xmcd-iso.zip</a><br>
ISO's containing Genesis binaries to communicate with a Genesis via Mike Pavone's cable (uCON64 option: --xmcd)<br>
See <a href="http://www.retrodev.com">http://www.retrodev.com</a> for more information<br>
<br>
<img src="images/n64.png" width="30"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/sram_1bs.zip?download">sram_1bs.zip</a><br>
Memory Manager 1.0 beta<br>
By R. Bubba Magillicutty<br>
16 MBit Version (uCON64 options: --xv64, --xdjr)<br>
<br>
<img src="images/n64.png" width="30"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/bb-sram2.zip?download">bb-sram2.zip</a><br>
SRAM Manager 2.0 is a save game memory management utility for use with
Nintendo 64 memory paks and cartridges. (uCON64 options: --xv64, --xdjr)<br>
This utility is based on R. Bubba's original SRAM Manager 1.0b<br>
<br>
<img src="images/n64.png" width="30"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/nil-jrbu.zip?download">nil-jrbu.zip</a><br>
This allows the Doctor V64 Junior to backup 512M carts<br>
Send this program to your V64 Junior and then turn on the N64, it will then read
512M from the cart into its DRAM (even if the cart is smaller)<br>
Then you transfer the data from DRAM to your workstation<br>
<br>
<img src="images/n64.png" width="30"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/upsram11.zip?download">upsram11.zip</a><br>
LaC's N64 ROM that can be used to upload SRAM-savegames directly to the V64 (uCON64 options: --xv64, --xdjr)<br>
<br>
<img src="images/n64.png" width="30"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/ultrasmsv10.zip?download">ultrasmsv10.zip</a><br>
Sega Master System/Game Gear emulator for Nintendo 64 by Jos Kwanten<br>
(uCON64 option: --usms)<br>
<br>
<img src="images/gameboy.png" width="60"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/fcgb2.lzh?download">fcgb2.lzh</a><br>
NES emulator for GameBoy by Kami (uCON64 option: --n2gb)<br>
<br>
<img src="images/gameboy.png" width="60"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/gbpack13.zip?download">gbpack13.zip</a><br>
Menu program for multiple ROM files in one GB Card. Works with all versions of
Game Boy. Supports GB Card (64 Mbit)<br>
<br>
<img src="images/lynx.png" width="60"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/lynxit11.zip?download">lynxit11.zip</a><br>
Lynxit is a custom made backup unit for the Atari Lynx handheld<br>
(uCON64 option: --xlit)<br>
<br>
<img src="images/f2a.png" width="60"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/f2afilesv4.zip?download">f2afilesv4.zip</a><br>
Flash 2 Advance (Ultra) support files<br>
(uCON64 options: --xf2a, --xf2amulti, --xf2ac, --xf2as, --xf2ab)<br>
<br>
<img src="images/sc.png"><br>
<a href="http://prdownloads.sourceforge.net/ucon64/supercard_menu.zip?download">supercard_menu.zip</a><br>
Super Card (CF to GBA Adapter) support files<br>
(uCON64 option: --sc)<br>
    </td>
  </tr>
</table>
</center>
<br>
<br>
<img src="images/hr.png" width="100%" height="24" border="0">
<br>
<br>
<br>
<a name="links"></a><center>
<table border="0" bgcolor="#ffffff" width="610">
  <tr>
    <td valign="top"><font size="-1">
<img src="images/links_small.png" border="0"><br>
<br>
If you are using uCON64 you may also consider visiting one of these cool sites<br>
<br>
<a href="http://sourceforge.net"><IMG src="http://sourceforge.net/sflogo.php?group_id=12381" width="88" height="31" border="0" alt="SourceForge Logo"></a><br>
<a href="http://www.dcemulation.com/">DC Emulation</a><br>
<a href="http://petition.eurolinux.org/"><img src="images/noepat120.png" alt="Eurolinux Petition" border="0"></a><br>
<a href="http://ukcdr.org/issues/cd/quick/"><img border="0" width="262" height="50" src="images/badcd003.png" alt="Say NO to corrupt audio discs" /></a><br>
<a href="http://www.anime.org/~sakura/"><img src="images/banner2.png" border="0""></a><br>
<a href="http://www.eff.org"><img src="images/efflogo-general.png" border="0"></a><br>
<a href="http://www.eurasia.nu"><img src="images/eurasia_logo3.png" border="0"></a><br>
<a href="http://www.gamebase64.com/">GB64.COM - C64 Games, Database, Music, Emulation, Frontends, Reviews and Articles</a><br>
<a href="http://www.gamefaqs.com/">GameFAQs - Video Game FAQs, Cheats, Codes, Reviews, and Message Boards</a><br>
<a href="http://www.hwb.acc.umu.se/">HwB: The Hardware Book</a><br>
<a href="http://www.c64.com/">[www.c64.com]</a><br>
<a href="http://www.irtc.org/">The Internet Ray Tracing Competition</a><br>
<a href="http://ian-albert.com/misc/gamemaps.php">Video Game Maps - Ian-Albert.com</a><br>
<a href="http://c64.tin.at/">The C64 Adventure Game Solutions and Walkthrough Site</a><br>
<a href="http://speeddemosarchive.com">Speed Demos Archive</a><br>
<a href="http://www.c64-longplays.de.vu/">C64-Video-Longplays</a><br>
<a href="http://www.back2roots.org/">Back to the Roots - Amiga Culture Directory Project</a><br>
<a href="http://www.freeoldies.com/">Freeoldies, the abandonware search engine, oldies, vieux jeux</a><br>
<a href="http://www.kultboy.com/">ASM - Zeitschrift, Cover, Testberichte, Scans, Magazin, Infos.</a><br>
<a href="http://en.wikipedia.org/wiki/List_of_video_game_consoles">List of video game consoles - Wikipedia, the free encyclopedia</a><br>
<a href="http://en.wikipedia.org/wiki/Video_game_console">Video game console - Wikipedia, the free encyclopedia</a><br>
<a href="http://www.ukresistance.co.uk/2005/11/blue-sky-in-games-campaign-launched.html"><img src="images/blueskybanner2.png" border="0"></a><br>
<a href="http://www.gazunta.com/wwtg/wiki/pmwiki.php">What Was That Game</a><br>
<a href="http://www.pdroms.de">PDRoms - Your legal source for homebrew console and handheld productions</a><br>
<a href="http://www.detstar.com/">D e t s t a r . c o m - Detstar Gaming Network</a><br>
<a href="http://www.pbernert.com/">Pete's Domain</a><br>
<a href="http://www.robwebb.clara.co.uk/backup/">Robert Webb - Neo-classic Gaming and Collecting</a><br>
<a href="http://www.kultpower.de/">Kultpower.de - Die Powerplay und ASM Fan Site</a><br>
<a href="http://www.fcc.gov/oet/fccid/">OET -- FCC ID Number Search Page</a><br>
<a href="http://www.robwebb.clara.co.uk/backup/">Robert Webb - Neo-classic Gaming and Collecting</a><br>
<!--a href="http://www.front.com.tw">FFE</a-->
<!--a href="http://www.superufo.com/">SuperUFO</a-->
<!--a href="http://www.ntscco.com.hk">New Tai Sang CO</a-->
<!--a href="http://www.lan-kwei.com/">Lan Kwei Trading</a-->
<a href="http://www.tototek.com/">ToTotek</a><br>
<!--a href="http://www.lik-sang.com">Lik-Sang online</a-->
<!--a href="http://www.hkems.com/">Welcome To EMS Production Limited</a-->
<a href="http://www.gsarchives.net/index2.php">Game Sprite Archives</a><br>
<a href="http://chui.dcemu.co.uk/index.html">Chui's DC projects</a><br>
<a href="http://forthewiin.org">A Linux driver for the Nintendo Wii remote</a><br>
<a href="http://www.supercard.cn/eng/index.htm">Chinese Supercard Homepage</a><br>
<!--http://www.supermagi.com/
http://krypt.dyndns.org:81/dcfactory/utils.phtml
http://www.panelmonkey.org/
http://romhack.de/utilities/grafik.php
http://www.zophar.net/utilities/music.html
http://ftp.giga.or.at/pub/nih/ckmame/
http://www.neoflash.com/affiliates.php
-->
    </td>
  </tr>
</table>
</center>
<br>
<br>
<img src="images/hr.png" width="100%" height="24" border="0">
<br>
<br>
<br>
<center>
<table border="0" bgcolor="#ffffff" width="610">
  <tr>
    <td valign="top"><font size="-1">
Some <img src="images/old.png" border="0"> DOORS for a Commodore Amiga Bulletin Board System<br>
<!-- keep them in one line or they'll look fucked up in mozilla -->
Screenshots: <a href="images/belchblabla_ss.png">1</a>|<a href="images/belchcheck_ss.png">2</a>|<a href="images/belcheditor_ss1.png">3</a>|<a href="images/belcheditor_ss2.png">4</a>|<a href="images/belchview_ss.png">5</a>|<a href="images/belchwho_ss1.png">6</a>|<a href="images/belchwho_ss2.png">7</a>|<a href="images/belchwho_ss3.png">8</a>|<a href="images/dlcount_ss.png">9</a>|<a href="images/olm_ss.png">10</a>|<a href="images/quicknew_ss.png">11</a>
<br>
    </td>
  </tr>
</table>
</center>
<br>
<br>
<center><img src="images/figures2.png" border="0"></center>
<font size="-1"><br>
<br>
<!--table border="0" bgcolor="#ffffff">
  <tr>   
    <td valign="top">
<?php
/*
  // relations
    echo ""
        .$w->widget_relate ($relate_site_title_s, $relate_site_url_s, "./", 0,
//                            WIDGET_RELATE_BOOKMARK|
//                            WIDGET_RELATE_STARTPAGE|
//                            WIDGET_RELATE_SEARCH|
//                            WIDGET_RELATE_DONATE|
//                            WIDGET_RELATE_LINKTOUS|
                            WIDGET_RELATE_TELLAFRIEND|
                            WIDGET_RELATE_SBOOKMARKS|
//                            WIDGET_RELATE_RSSFEED|
                            0)
        ."<br>";
*/
?></td></tr></table--><br>
<a href="http://sourceforge.net"><IMG src="http://sourceforge.net/sflogo.php?group_id=12381" width="88" height="31" border="0" alt="SourceForge Logo"></a><br>
<br>
<br>
</span>
</body>
</html>
<?php

  if ($use_gzip)
    ob_end_flush ();

?>
