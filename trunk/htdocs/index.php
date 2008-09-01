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

  echo html_head_tags ("images/icon.png",    // icon
                       "uCON64 - The backup tool and wonderful emulator's Swiss Army knife program",
                       "0",   // refresh
                       "utf-8", // charset
                       0,       // use dublin core
                       NULL,    // default dc-desc
                       "snes, magicom, magic drive, bung, game doctor, multi game hunter, wild card, dx2, pro fighter, "
                      ."smart bros, multi game doctor, dragon boy, gamestation, game master, game doctor, mini doctor, "
                      ."magic card, magic griffin, super magic drive, hacker, super disk, "
                      ."interceptor, mega disk interceptor, super disk, z64, v64, doctor v64, super "
                      ."twin, e-merger, v64jr, magic drive plus, super ufo, cd64, supercom partner, "
                      ."yoko, super charger, unimex duplikator, game station, Professor, saturn, "
                      ."neo-classic, neo, classic, gaming, java, games, online, n64, jaguar, atari, "
                      ."mp3, midi, music, files, mp3s, midis, nes, famicom, clone, multi-carts, "
                      ."multi carts, multi cart, mega drive, master system, snes cd, unit, history, "
                      ."story, saturn mod, modded, cart list, game list, full, hardware, connectors, "
                      ."modifications, pal, ntsc",
                       "http://ucon64.sourceforge.net",
                       "en",
                       "NoisyB");

  $w = new misc_widget;
//  $w->widget_init (WIDGET_CSS_A|WIDGET_CSS_IMG|WIDGET_CSS_RELATE, //|WIDGET_CSS_SELECT, // css flags
//                   0); // js flags
  $w->widget_init (WIDGET_CSS_A|WIDGET_CSS_IMG|WIDGET_CSS_RELATE|WIDGET_CSS_START, //|WIDGET_CSS_SELECT, // css flags
                   WIDGET_JS_RELATE); // js flags
?>
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
<?php

//require_once ("portal_logo.inc");
require_once ("index_news.inc");

?>
</center>
<br>
<br>
<img src="images/hr.png" width="100%" height="24" border="0">
<br>
<br>
<br>
<a name="ucon64"></a><center>
<?php

require_once ("index_ucon64.inc");

?>
</center>
<br>
<br>
<img src="images/hr.png" width="100%" height="24" border="0">
<br>
<br>
<br>
<a name="ucon64dat"></a><center>
<?php

require_once ("index_ucon64dat.inc");

?>
</center>
<br>
<br>
<img src="images/hr.png" width="100%" height="24" border="0">
<br>
<br>
<br>
<a name="ucon64gui"></a><center>
<?php

require_once ("index_ucon64gui.inc");

?>
</center>
<br>
<br>
<img src="images/hr.png" width="100%" height="24" border="0">
<br>
<br>
<br>
<a name="ucon64misc"></a><center>
<?php

require_once ("index_ucon64misc.inc");

?>
</center>
<br>
<br>
<img src="images/hr.png" width="100%" height="24" border="0">
<br>
<br>
<br>
<a name="links"></a><center>
<?php

require_once ("index_links.inc");

?>
</center>
<br>
<br>
<img src="images/hr.png" width="100%" height="24" border="0">
<br>
<br>
<br>
<center>
<?php

require_once ("index_bbs.inc");

?>
</center>
<br>
<br>
<center><img src="images/figures2.png" border="0"></center>
<font size="-1"><br>
<br>
<table border="0" bgcolor="#ffffff">
  <tr>   
    <td valign="top">
<?php

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

?></td></tr></table><br>
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
