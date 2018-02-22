<?php
/*
misc.php - miscellaneous functions

Copyright (c) 2006 - 2009 NoisyB


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
if (!defined ('MISC_MISC_PHP'))
{
define ('MISC_MISC_PHP', 1);
//error_reporting(E_ALL | E_STRICT);


function
tor_get_contents ($url, $tor_proxy_host = '127.0.0.1', $tor_proxy_port = 9050, $timeout = 300)
{
  $sock = curl_init();
  if ($sock == FALSE)
    {
      echo 'CURL support missing';
      return NULL;
    }

  curl_setopt ($sock, CURLOPT_PROXY, $tor_proxy_host.':'.$tor_proxy_port);
  curl_setopt ($sock, CURLOPT_URL, $url);
  curl_setopt ($sock, CURLOPT_HEADER, 1);
  $agent = random_user_agent ();
  curl_setopt ($sock, CURLOPT_USERAGENT, $agent);
  curl_setopt ($sock, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt ($sock, CURLOPT_FOLLOWLOCATION, 1);
  curl_setopt ($sock, CURLOPT_TIMEOUT, $timeout);
  curl_setopt ($sock, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);

  $response = curl_exec ($sock);
  if ($response == FALSE)
    {
      echo 'TOR not running';
      return NULL;
    }

  // DEBUG
//  echo '<pre><tt>';
//  print_r ($response);

  $info = curl_getinfo ($sock);

  // DEBUG
//  echo '<pre><tt>';
//  print_r ($info);

  curl_close ($sock);

  return $response;
}


function
misc_url_exists ($url)
{
  if (file_get_contents ($url, FALSE, NULL, 0, 0) === false)
    return false;
  return true;
}


function
misc_get_browser_config ()
{
  // get the settings of the browser and stores them in cookie
  if (isset ($_GET['config']))
    {
      $a = array('js' => $_GET['js'],
//                 'flash' => $_GET['flash'], 
                 'w' => $_GET['w'], 
                 'h' => $_GET['h'],
        );

      // DEBUG
//      echo '<pre><tt>';
//      print_r ($a);

      // TODO: store in cookie

      return $a;
    }


  $p = '';

  // send javascript probe to browser and a refresh
  $p .= '<script type="text/javascript">'."\n"
//       .'var w = window.width;'."\n"
//       .'var h = window.height;'."\n"
       .'var w = screen.width;'."\n"
       .'var h = screen.height;'."\n"
       .'if (self.innerWidth != undefined)'."\n"
       .'  {'."\n"
       .'    w = self.innerWidth; h = self.innerHeight;'."\n"
       .'  }'."\n"
       .'else'."\n"
       .'  {'."\n"
       .'    var d = document.documentElement;'."\n"
       .'    if (d)'."\n"
       .'      {'."\n"
       .'        w = d.clientWidth; h = d.clientHeight;'."\n"
       .'      }'."\n"
       .'  }'."\n"
//       .'document.write (w+" "+h);'."\n"
       .'document.write (\'<meta http-equiv="refresh" content="0,URL=?config=1'
       .'&js=1'
//       .'&flash=0'
       .'&w=\'+w+\''
       .'&h=\'+h+\''
       .'">\');'."\n"
       .'</script>'
       .'<noscript>'
       .'<meta http-equiv="refresh" content="0,URL=?config=1&js=0">'
       .'</noscript>'
;

  echo $p;

/*
$flash = '
function detectingFLASH() {
  var browser = navigator.userAgent.toLowerCase();
  flashVersion = 0;	
	// NS3+, Opera3+, IE5+ Mac
	if ( navigator.plugins != null && navigator.plugins.length > 0 ) {
		var flashPlugin = navigator.plugins['Shockwave Flash'];
		if ( typeof flashPlugin == 'object' ) { 
			if ( flashPlugin.description.indexOf('7.') != -1 ) flashVersion = 7;
			else if ( flashPlugin.description.indexOf('6.') != -1 ) flashVersion = 6;
			else if ( flashPlugin.description.indexOf('5.') != -1 ) flashVersion = 5;
			else if ( flashPlugin.description.indexOf('4.') != -1 ) flashVersion = 4;
			else if ( flashPlugin.description.indexOf('3.') != -1 ) flashVersion = 3;
		}
	} // IE4+ Win32 (VBscript)
	else if ( browser.indexOf("msie") != -1 && parseInt(navigator.appVersion) >= 4 && browser.indexOf("win")!= -1 && browser.indexOf("16bit")== -1 ) {
	  document.write('<scr' + 'ipt language="VBScript"\> \n');
		document.write('on error resume next \n');
		document.write('DIM obFlash \n');
		document.write('SET obFlash = CreateObject("ShockwaveFlash.ShockwaveFlash.7") \n');
		document.write('IF IsObject(obFlash) THEN \n');
		document.write('flashVersion = 7 \n');
		document.write('ELSE SET obFlash = CreateObject("ShockwaveFlash.ShockwaveFlash.6") END IF \n');
		document.write('IF flashVersion < 7 and IsObject(obFlash) THEN \n');
		document.write('flashVersion = 6 \n');
		document.write('ELSE SET obFlash = CreateObject("ShockwaveFlash.ShockwaveFlash.5") END IF \n');
		document.write('IF flashVersion < 6 and IsObject(obFlash) THEN \n');
		document.write('flashVersion = 5 \n');
		document.write('ELSE SET obFlash = CreateObject("ShockwaveFlash.ShockwaveFlash.4") END IF \n');
		document.write('IF flashVersion < 5 and IsObject(obFlash) THEN \n');
		document.write('flashVersion = 4 \n');
		document.write('ELSE SET obFlash = CreateObject("ShockwaveFlash.ShockwaveFlash.3") END IF \n');
		document.write('IF flashVersion < 4 and IsObject(obFlash) THEN \n');
		document.write('flashVersion = 3 \n');
		document.write('END IF');
	  document.write('</scr' + 'ipt\> \n');
  } // no Flash
  else {
	flashVersion = -1;
  }
return flashVersion;
';
*/
}


function
misc_explode_tag ($html_tag)
{
  // returns 2d array with tag name and attributes and their values
  $s = strpos ($html_tag, '<') + 1;
  $l = strrpos ($html_tag, '>') - $s;
  $p = substr ($html_tag, $s, $l);
  $p = trim ($p);
  // '=      "' to '="'
  $p = str_replace (array ('= "','=  "'), '="', $p);
  $p = str_replace (array ('= "','=  "'), '="', $p);
  $p = str_replace (array ('= "','=  "'), '="', $p);
  // '      ="' to '="'
  $p = str_replace (array (' ="','  ="'), '="', $p);
  $p = str_replace (array (' ="','  ="'), '="', $p);
  $p = str_replace (array (' ="','  ="'), '="', $p);

  // DEBUG
//  echo $p;

  $a = explode (' ', $p);
  $a = array_merge (array_unique ($a));

  // DEBUG
//  echo '<pre><tt>';
//  print_r ($a);

  $tag = array ();
  $count = 0;
  for ($i = 0; isset ($a[$i]); $i++)
    if (strpos ($a[$i], '=')) // is attribute
      {
        $aa = explode ('=', $a[$i], 2);

        $tag[strtolower (trim ($aa[0]))] = '"'.trim (trim ($aa[1]), '"').'"'; // trim first spaces then quotes
      }
    else // attribute without value (e.g. tag name)
      $tag[strtolower (trim ($a[$i]))] = NULL;

  // DEBUG
//  echo '<pre><tt>';
//  print_r ($tag);

  return $tag;
}


function
enforce_gre ()
{
  $p = '';

  // enforce gecko render engine (used by firefox, safari, etc.)
  if (!stristr ($_SERVER['HTTP_USER_AGENT'], "moz"))
    {
      $url = 'http://www.firefox.com';

      // redirect using js
//      $p .= '<script type="text/javascript"><!--'."\n"
//           .'location.href="'.$url.'"'."\n"
//           .'//--></script>'."\n";

      $refresh = 1;
      $p .= '<meta name="refresh" content="refresh: '.$refresh.'; url="'.$url.'">';

      $p .= '<span style="font-family: arial,sans-serif;">'
           .'<table border="0" cellpadding="0" cellspacing="0" width="80%" height="100">'
           .'<tr>'
           .'<td border="0" cellpadding="0" cellspacing="0" bgcolor="#ffff80" align="center">'
           .'<font size="-1" face="arial" color="#000000">Your browser is not supported here. Redirecting...</font>'
           .'</td>'
           .'</tr>'
           .'</table>'
           .'</span>';

      echo $p;

      exit;
    }
}


function
misc_implode_tag ($tag)
{
  // DEBUG
//  echo '<pre><tt>';
//  print_r ($tag);

  $p = '';

  $p .= '<';

  $a = array_keys ($tag);
  for ($i = 0; $a[$i]; $i++)
    {
      if ($i > 0)
        $p .= ' ';

      $p .= $a[$i];

      if ($tag[$a[$i]])
        $p .= '='.$tag[$a[$i]];
    }

  $p .= '>';

  return $p;
}


function
misc_gettag ($html_tag, $attr = array(), $use_existing_attr = false)
{
  $tag = misc_explode_tag ($html_tag);

  if (!$use_existing_attr)
    {
      // BUT keep the attributes without value (e.g. tag name)
      $attr_keys = array_keys ($tag);
      for ($i = 0; $attr_keys[$i]; $i++)
        if ($tag[$attr_keys[$i]] == NULL)
          $a[$attr_keys[$i]] = $tag[$attr_keys[$i]];
      $tag = $a;
    }

  if (!$attr)
    return misc_implode_tag ($tag);

//  $tag = array_replace ($tag, $attr);
  $attr_keys = array_keys ($attr);
  for ($i = 0; $attr_keys[$i]; $i++)
    if ($attr[$attr_keys[$i]] != NULL)
      $tag[$attr_keys[$i]] = '"'.trim ($attr[$attr_keys[$i]], '"').'"';

  return misc_implode_tag ($tag);
}


$ctype__ = array(32,32,32,32,32,32,32,32,32,40,40,40,40,40,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
  -120,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,4,4,4,4,4,4,4,4,4,4,16,16,16,16,16,16,
  16,65,65,65,65,65,65,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,16,16,16,16,16,
  16,66,66,66,66,66,66,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,16,16,16,16,32,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
function isalnum ($c){ global $ctype__; return ((($ctype__[( ord($c) )]&(01 | 02 | 04 )) != 0)?1:0);}
function isalpha ($c){ global $ctype__; return ((($ctype__[( ord($c) )]&(01 | 02 )) != 0)?1:0);}
function isascii ($c){ global $ctype__; return (((( ord($c) )<=0177) != 0)?1:0);}
function iscntrl ($c){ global $ctype__; return ((($ctype__[( ord($c) )]& 040 ) != 0)?1:0);}
function isdigit ($c){ global $ctype__; return ((($ctype__[( ord($c) )]& 04 ) != 0)?1:0);}
function isgraph ($c){ global $ctype__; return ((($ctype__[( ord($c) )]&(020 | 01 | 02 | 04 )) != 0)?1:0);}
function islower ($c){ global $ctype__; return ((($ctype__[( ord($c) )]& 02 ) != 0)?1:0);}
function isprint ($c){ global $ctype__; return ((($ctype__[( ord($c) )]&(020 | 01 | 02 | 04 | 0200 )) != 0)?1:0);}
function ispunct ($c){ global $ctype__; return ((($ctype__[( ord($c) )]& 020 ) != 0)?1:0);}
function isspace ($c){ global $ctype__; return ((($ctype__[( ord($c) )]& 010 ) != 0)?1:0);}
function isupper ($c){ global $ctype__; return ((($ctype__[( ord($c) )]& 01 ) != 0)?1:0);}
function isxdigit ($c){ global $ctype__; return ((($ctype__[( ord($c) )]&(0100 | 04 )) != 0)?1:0);}


function
get_ip ($address)
{
  // if it isn't a valid IP assume it is a hostname
  $preg = '#^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}'
         .'(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$#';
  if (!preg_match($preg, $address))
    {
      $result = gethostbyname ($address);

      // not a valid host nor IP
      if ($result === $address)
        $result = false;
    }
  else
    $result = $address;
        
  return $result;
}


function
check_udp ($address, $port)
{
  $fp = fsockopen('udp://'.$address, $port, $errno, $errstr, 1); 
  if (!$fp)
    return 0;

  socket_set_timeout ($fp, 2);
  if (!fwrite($fp,'\x00'))
    return 0;

  $t1 = time();
  $res = fread($fp, 1);
  $t2 = time();
  fclose ($fp);

  if ($t2 - $t1 > 1)
    return 0; 

  if (!($res))
    return 0;

  return 1;
}


function
scandir4 ($path, $sort)
{
  $i = 0;
  $a = array ();

  $dir = opendir ($path);
  while (($a[$i] = readdir ($dir)) !== false)
    $i++;
  closedir ($dir);

  if ($sort)
    array_multisort ($a, SORT_DESC);
  else
    sort ($a);

  return $a;
}


function
misc_download ($url, $path)
{
  if (!($img = file_get_contents ($url)))
    return;

  if (!($out = fopen ($path, 'wb')))
    return;
 
  fwrite ($out, $img);
  fclose ($out);
}


function
time_ms ()
// returns milliseconds since midnight
{
  $tv = gettimeofday ();

  if ($tv)
    {
      $t = $tv['usec'] / 1000;
      $t += ($tv['sec'] % 86400) * 1000;
    }

  return $t;
}


function
time_count ($t_date)
{
  static $t_now = 0;
  static $t_count = 0;

  if ($t_now == 0)
    $t_now = $t_count = time ();

  $p = NULL;

  while ($t_count > $t_date)
    {
      $t_calc = $t_now - $t_count;

      if ($t_calc < 3600)
        {
          $t_count -= 300;
          if ($t_calc)
            if ($t_count < $t_date)
            $p .= sprintf ('%s', $t_calc / 60 .' minutes');
        }
      else if ($t_calc < 86400)
        {
          $t_count -= 3600;
          if ($t_count < $t_date)
            $p .= sprintf ('%s', $t_calc / 3600 .' hours');
        }
      else
        {
          $t_count -= 86400;
          if ($t_count < $t_date)
            $p .= sprintf ('%s', $t_calc / 86400 .' days');
        }
    }

  return $p;
}


function
islocalhost ()
{
  return $_SERVER['REMOTE_ADDR'] == $_SERVER['SERVER_ADDR'];
}


function
misc_get_keywords_alnum ($s, $keyword_size = 3)
{
  if (strlen (trim ($s)) < $keyword_size)
    return false;

  for ($i = 0; $s[$i]; $i++)
    if (!isalnum ($s[$i]) && $s[$i] != '_' && $s[$i] != '.')
      return false;

  return true;
}


function
misc_get_keywords_alpha ($s, $keyword_size = 3)
{
  if (strlen (trim ($s)) < $keyword_size)
    return false;

  for ($i = 0; $s[$i]; $i++)
    if (!isalpha ($s[$i]) && $s[$i] != '_' && $s[$i] != '.')
      return false;

  return true;
}


function
misc_get_keywords ($s, $flag = 0) // default = isalnum
{
  $s = str_replace (array ('. ', ',', ';', '!', '?', '"'), ' ', $s);
  $s = str_replace (array ('  ', '  ', '  ', '  ', '  '), ' ', $s);

  for ($i = 0; $s[$i]; $i++)
    if (ispunct ($s[$i]) && $s[$i] != '_' && $s[$i] != '.')
      $s[$i] = ' ';

  $a = explode (' ', strtolower ($s));
  for ($i = 0; isset ($a[$i]); $i++)
    $a[$i] = trim ($a[$i], ' .');
  // TODO: more sensitivity instead of array_filter()
  $a = array_filter ($a, (!$flag ? 'misc_get_keywords_alnum' : 'misc_get_keywords_alpha'));
  $a = array_merge (array_unique ($a));

  // DEBUG
//  echo '<pre><tt>';
//  print_r ($a);

  $s = implode (' ', $a);
  $s = trim ($s);

  return $s;
}


function
misc_get_keywords_html ($s, $flag = 0) // default = isalnum
{
  // so one keyword does not get glued to another because of strip_tags()
  $s = str_replace ('>', '> ', $s);
  $s = str_replace (array ('  ', '  ', '  ', '  ', '  '), ' ', $s);
  $s = strip_tags ($s);

  return misc_get_keywords ($s, $flag);
}


function
echo_gzip ($p)
{
  ob_start ('ob_gzhandler');
  echo $p;
  ob_end_flush ();
}


function
get_suffix ($filename)
// get_suffix() never returns NULL
{
  $p = basename ($filename);
  if (!$p)
    $p = $filename;

  $s = strrchr ($p, '.');
  if (!$s)
    $s = strchr ($p, 0);
  if ($s == $p)
    $s = strchr ($p, 0);

  return $s;
}


function
set_suffix ($filename, $suffix)
{
  // always use set_suffix() and NEVER the code below
  return str_replace (get_suffix ($filename), $suffix, $filename);
}


function
str_shorten ($s, $limit)
{
  // Make sure a small or negative limit doesn't cause a negative length for substr().
  if ($limit < 3)
    $limit = 3;

  // Now truncate the string if it is over the limit.
  if (strlen ($s) > $limit)
    return substr($s, 0, $limit - 3).'..';
  else
    return $s;
}


/*
function
short_name ($s, $limit)
{
  return str_shorten ($s, $limit);
}
*/


function
in_tag ($s)
{
  // are we inside a tag?
  return strpos ($s, '>') < strpos ($s, '<');
}


function
is_url ($s)
{
  // checks if string is a url
  $is_url = 0;

  if (strlen ($s) > 4 &&
      isalpha ($s[0]) &&
      !strstr ($s, '..') &&
      substr_count ($s, '.') == 2 &&
      (substr ($s, -4, 1) == '.' || substr ($s, -3, 1) == '.'))
    $is_url = 1;

  return $is_url;
}


function
parse_links ($s)
{
  // turn plain text urls into links
//  return preg_replace ('/\\b(https?|ftp|file):\/\/[-A-Z0-9+&@#\/%?=~_|!:,.;]*[-A-Z0-9+&@#\/%=~_|]/i', "<a href=\"#\" onclick=\"open_url('\\0')\";return false;>\\0</a>", $s);
//  return ereg_replace("[[:alpha:]]+://[^<>[:space:]]+[[:alnum:]/]","<a href=\"\\0\">\\0</a>", $s);
//  $s = eregi_replace("((([ftp://])|(http(s?)://))((:alnum:|[-\%\.\?\=\#\_\:\&\/\~\+\@\,\;])*))","<a href = '\\0' target='_blank'>\\0</a>", $s);
//  $s = eregi_replace("(([^/])www\.|(^www\.))((:alnum:|[-\%\.\?\=\#\_\:\&\/\~\+\@\,\;])*)", "\\2<a href = 'http://www.\\4'>www.\\4</a>", $s);

  $a = explode (' ', strip_tags ($s));
  $a = array_merge (array_unique ($a)); // remove dupes

  // find eventual urls
  $a_size = sizeof ($a);
  for ($i = 0; $i < $a_size; $i++)
    if (is_url ($a[$i]))
      {
        if (stristr ($a[$i], 'http://'))
          $s = str_replace ($a[$i], '<a href="'.$a[$i].'">'.$a[$i].'</a>', $s);
        else
          $s = str_replace ($a[$i], '<a href="http://'.$a[$i].'">'.$a[$i].'</a>', $s);
      }

  return $s;
}


if (!function_exists('sys_get_temp_dir'))
{
function
sys_get_temp_dir ()
{
  if ($temp = getenv ('TMP'))
    return $temp;
  if ($temp = getenv ('TEMP'))
    return $temp;
  if ($temp = getenv ('TMPDIR'))
    return $temp;
  $temp = tempnam (__FILE__, '');
  if (file_exists ($temp))
    {
      unlink ($temp);
      return dirname ($temp);
    }
  return null;
}
}


if (!function_exists ('sprint_r'))
{
function 
sprint_r ($var)
{
  ob_start ();

  print_r ($var);

  $ret = ob_get_contents ();

  ob_end_clean ();

  return $ret;
}
}


// turn any variable into XML string
function
var_xml ($v)
{
  ob_start ();

  print_r ($v);
//  var_dump ($v);

  $p = ob_get_contents ();

  ob_end_clean ();

  $p = str_replace (array (' => ', 'SimpleXMLElement', 'Object'), '', $p);
  $p = str_replace (array (']Array'), ']', $p);
//  $p = str_replace (array ('('."\n"), '', $p);
//  $p = str_replace ('[', '<', $p);
//  $p = str_replace (']', '>', $p);
  $p = '<?xml version="1.0" encoding="UTF-8"?>'.$p;

  return $p;
}


function
ftp_search ($search)
{
  $search = str_replace (' ', '+', $search);
//  $query = 'intitle:("Index.of.*"|"Index.von.*") +("'.$search.'") -inurl:(htm|php|html|py|asp|pdf) -inurl:inurl -inurl:in
  $query = 'intitle:("Index.of.*") +("'.$search.'") -inurl:(htm|php|html|py|asp|pdf) -inurl:inurl -inurl:index';
  $url = 'http://www.google.com/search?q='.urlencode ($query);

  return $url;
}


function
video_search ($search)
{
  $search = str_replace (' ', '+', $search);
//  $query = 'intitle:("Index.of.*"|"Index.von.*") +("'.$search.'") -inurl:(htm|php|html|py|asp|pdf) -inurl:inurl -inurl:in
  $query = 'intitle:("Index.of.*") +("'.$search.'") -inurl:(htm|php|html|py|asp|pdf) -inurl:inurl -inurl:index';
  $url = 'http://www.google.com/search?q='.urlencode ($query);

  return $url;
// youtube
//    <feed>http://video.google.com/videosearch?q=quakeworld&amp;so=1&amp;output=rss&amp;num=1000</feed>
//    <feed>http://gdata.youtube.com/feeds/api/videos?vq=quake1&amp;max-results=50</feed>
}


function
wikipedia_search ($search)
{
  $url = 'http://en.wikipedia.org/w/index.php?title=Special%3ASearch&redirs=0&search='.$search.'&fulltext=Search&ns0=1';
  return $url;
}


function
lyrics_search ($search)
{
  $url = 'http://www.google.com/search?ie=UTF-8&oe=utf-8&q='.$search;
  return $url;
}


function
image_search ($search)
{
  $url = 'http://images.google.com/search?ie=UTF-8&oe=utf-8&q='.$search;             
  return $url;
}


// gaming
function
walkthrough_search ($search)
{
  $url = 'http://images.google.com/search?ie=UTF-8&oe=utf-8&q=walkthrough+'.$search;
  return $url;
}


// gaming
function
cheat_search ($search)
{
  $url = 'http://images.google.com/search?ie=UTF-8&oe=utf-8&q=cheat+'.$search;
  return $url;
}


/*
function
vname (&$var, $scope=false, $prefix='unique', $suffix='value')
{
  if ($scope)
    $vals = $scope;
  else
    $vals = $GLOBALS;

  $old = $var;
  $var = $new = $prefix.rand().$suffix;
  $vname = FALSE;

  foreach ($vals as $key => $val)
    if($val === $new)
      $vname = $key;

  $var = $old;

  return $vname;
}
*/


function
misc_exec ($cmdline, $debug = 0)
{
  if ($debug)
    echo 'cmdline: '.$cmdline."\n"
        .'escaped: '.escapeshellcmd ($cmdline).' (not used)'."\n"
;
  if ($debug == 2)
    return '';

  $a = array();

  exec ($cmdline, $a, $res);

  $p = '';
  if ($debug)
    $p = $res."\n";

  $i_max = count ($a);
  for ($i = 0; $i < $i_max; $i++)
    $p .= $a[$i]."\n";

  return $p;
}


function
get_cookie ($name)
{
//  if (isset ($_REQUEST[$name]))
//    return $_REQUEST[$name];

  if (isset ($_COOKIE[$name]))
    return $_COOKIE[$name];

  return NULL;
}


function
get_request_value ($name)
{
//  global $_POST;
//  global $_GET;

  if (isset ($_POST[$name]))
    return $_POST[$name];

  if (isset ($_GET[$name]))
    return $_GET[$name];

  return NULL;
}


function
http_build_query2 ($args = array(), $use_existing_arguments = false)
{
  if ($use_existing_arguments)
    $a = array_merge ($_GET, $args); // $args overwrites $_GET
  else
    $a = $args;

  if (!sizeof ($a))
    return '';

  return http_build_query ($a);
} 


function
misc_getlink ($args = array(), $use_existing_arguments = false)
{
  return '?'.http_build_query2 ($args, $use_existing_arguments);
}


function
misc_seo_description ($html_body)
{
  // generate meta tag from the body
  $p = strip_tags ($html_body);
  $p = str_replace (array ('&nbsp;', '&gt;', '&lt;', "\n"), ' ', $p);
  $p = misc_get_keywords ($p, 1);
  return '<meta name="Description" content="'.$p.'">'
        .'<meta name="keywords" content="'.$p.'">';
}


function
misc_head_tags ($icon, $refresh = 0, $charset = 'UTF-8')
{
  $p = '';

  $p .= '<meta http-equiv="Content-Type" content="text/html;charset='.$charset.'">';
//  $p .= '<meta name="Content-Type" content="text/html; charset='.$charset.'">';

  if ($refresh > 0)
//    header ('location:'.$_SERVER['REQUEST_URI']);
    header ('refresh: '.$refresh.'; url='.$_SERVER['REQUEST_URI']);
//    $p .= '<meta http-equiv="refresh" content="'.$refresh.'; URL='.$_SERVER['REQUEST_URI'].'">';

/*
    <meta http-equiv="imagetoolbar" content="no">
    <meta http-equiv="reply-to" content="editor@NOSPAM.sniptools.com">
    <meta http-equiv="MSThemeCompatible" content="Yes">
    <meta http-equiv="Content-Language" content="en">
    <meta http-equiv="Expires" content="Mon, 24 Sep 1976 12:43:30 IST">
*/

  if ($icon)
    $p .= '<link rel="icon" href="'.$icon.'" type="image/png">';

  return $p;
}


/*
  split html into an array with content separated by tags
    returns an array

  $a['start']      // [...<head>]
  $a['head']       // <head>[...]</head>
  $a['between']    // [</head>...<body>]
  $a['body']       // <body>[...]</body>
  $a['end']        // [</body>...]EOF
*/
function
split_html_content ($html)
{
  $p = strtolower ($html);

  // split in head, body (and the rest)
  if (strpos ($p, '<head'))
    {
      $head_start = strpos ($p, '<head');
      $head_start += strpos (substr ($p, $head_start), '>') + 1;
      $head_len = strpos ($p, '</head>') - $head_start;
    }
  else
    {
      $head_start = 0;
      $head_len = 0;
    }

  if (strpos ($p, '<body'))
    {
      $body_start = strpos ($p, '<body');
      $body_start += strpos (substr ($p, $body_start), '>') + 1;
      $body_len = strpos ($p, '</body>') - $body_start;
    }
  else
    {
      $body_start = 0;
      $body_len = strlen ($html);
    }

  $start = substr ($html, 0, $head_start);
  $head = substr ($html, $head_start, $head_len);
  $between = substr ($html, $head_start + $head_len, $body_start - ($head_start + $head_len));

  $body = substr ($html, $body_start, $body_len);

  $end = substr ($html, $body_start + $body_len);

  $a = array ();

  $a['start'] = $start;
  $a['head'] = $head;
  $a['between'] = $between;
  $a['body'] = $body;
  $a['end'] = $end;

  // DEBUG
//print_r ($a);

  return $a;
}


define ('ALLOW_DEF', ''
       .'<a></a>'
       .'<br>'

       .'<font></font>'
       .'<form></form>'

       .'<img>'
       .'<input>'

       .'<option></option>'

       .'<pre></pre>'

       .'<select></select>'

       .'<table></table>'
       .'<td></td>'
       .'<textarea></textarea>'
       .'<tr></tr>'
       .'<tt></tt>'
);


function
embed_other_page ($url, $form_action = '', $form_method = 'GET', $allow = ALLOW_DEF)
{
  $html = '';

  $html = file_get_contents ($url);
  // extract head (JS, CSS, etc.) and body
  $a = split_html_content ($html);
  $html = ''.$a['body'];

  // normalize, remove unwanted tags
  $html = strip_tags ($html, $allow);

  // rewrite form action and method
  // TODO: repeat
  $html_tag = substr ($html, strpos ($html, '<form '));
  $html_tag = substr ($html_tag, 0, strpos ($html_tag, '>') + 1);
  $p = misc_gettag ($html_tag, array ('action' => $form_action, 'method' => $form_method), false);
  $html = str_ireplace ($html_tag, $p, $html);

  if ($form_method == 'POST')
    {
      // replace GET links with POST forms if necessary
      // TODO: repeat
      $html_tag = substr ($html, strpos ($html, '<a '));
      $html_tag = substr ($html_tag, 0, strpos ($html_tag, '</a>') + 1);

      // TODO: transform a link into a button in a post form
//      $p = a_href_to_post_form ($html_tag);
      $html = str_ireplace ($html_tag, $p, $html);
    }

  // HACK: fix absolute links again                     
//  $html = str_ireplace ($url.'http://', 'http://', $html);

  return $html;
}


function
embed_other_page_js ($url)
{
  $p = '';

  $p .= '<script type="text/javascript">
';

//  $p .= 'document.include = function (url) { document.write (\'<script type="text/javascript" src="\'+url+\'"></scr\'+\'ipt>\'); }';

  $p .= '// new prototype defintion
document.include = function (url)
{
  if (typeof (url) == \'undefined\')
    return false;

  var p, rnd;
  if (document.all) // IE: create an ActiveX Object instance
    p = new ActiveXObject(\'Microsoft.XMLHTTP\');
  else // mozilla: create an instance of XMLHttpRequest
    p = new XMLHttpRequest ();

  // prevent browsers from caching the included page by appending a random number
  rnd = Math.random ().toString ().substring (2);
  url = url.indexOf (\'?\') > -1 ? url+\'&rnd=\'+rnd : url+\'?rnd=\'+rnd;

  // open the url and write out the response
  p.open (\'GET\', url, false);
  p.send (null);

  document.write (p.responseText);
}
</script>
<script>
document.include (\''.$url.'\');
</script>';

  return $p;
}


function
random_user_agent ()
{
  $ua = array('Mozilla','Opera','Microsoft Internet Explorer','ia_archiver');   
  $op = array('Windows','Windows XP','Linux','Windows NT','Windows 2000','OSX');
  $agent  = $ua[rand(0,3)].'/'.rand(1,8).'.'.rand(0,9).' ('.$op[rand(0,5)].' '.rand(1,7).'.'.rand(0,9).'; en-US;)';
  return $agent;
}


function
misc_youtube_download_single ($video_id, $use_tor = 0, $debug = 0)
{
  // normalize
  if (strpos ($video_id, '?v='))
    $video_id = substr ($video_id, strpos ($video_id, '?v=') + 3);
  if (strpos ($video_id, '&'))
    $video_id = substr ($video_id, 0, strpos ($video_id, '&'));

  // DEBUG
//  echo $video_id;

  $url = 'http://www.youtube.com/get_video_info?&video_id='.$video_id;

/*
  if (misc_url_exists ($url) === true)
    {
      $h = get_headers ($url);

      // DEBUG
//      print_r ($h);

      return $h[19];
    }
*/

  if ($use_tor)
    $page = tor_get_contents ($url);
  else
    $page = file_get_contents ($url);
  $a = array ();
  parse_str ($page, $a);

  if ($debug == 1)
    {
      echo '<pre><tt>';
      print_r ($a);
      echo '</tt></pre>';
    }

  $b = explode (',', $a['fmt_url_map']);

  if ($debug == 1)
    {
      echo '<pre><tt>';
      print_r ($b);
      echo '</tt></pre>';
    }

  $a = array_merge ($a, $b);

  $url = urldecode ($b[0]);
  $url = substr ($url, strrpos ($url, 'http://'));
  $a['video_url'] = $url;

  return $a;
}


function
misc_youtube_download ($video_id, $use_tor = 0, $debug = 0)
{
  $a = array ();

  if (strstr ($video_id, '?v='))
    $a[0] = misc_youtube_download_single ($video_id, $debug);
  else if (strstr ($video_id, 'http://')) // RSS feed
    {
      if ($use_tor)
        {
          $xml = tor_get_contents ($video_id);
          $b = simplexml_load_string ($xml);
        }
      else
        $b = simplexml_load_file ($video_id);

      if ($debug == 1)
        {
          echo '<pre><tt>';
          print_r ($b);
          echo '</tt></pre>';
        }

      for ($i = 0; isset ($b->channel->item[$i]); $i++)
         $a[$i] = misc_youtube_download_single ($b->channel->item[$i]->link, $use_tor, $debug);
    }
  else
    $a[0] = misc_youtube_download_single ($video_id, $use_tor, $debug);

  return $a;
}


}


?>