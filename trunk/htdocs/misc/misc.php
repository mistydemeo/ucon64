<?php
/*
misc.php - miscellaneous functions

Copyright (c) 2006 NoisyB


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


function
window_open ()
{
/*
require_once ("misc/widget.php");


  $w = new misc_widget;
  $w->widget_init (0, // css flags
                   WIDGET_JS_WINDOW); // js flags
?>
<a href="javascript:js_window_open ('ripalot.php',
                                    'mywindow',
                                    'width=450,'
                                   +'height=450,'
                                   +'resizable=no,'
                                   +'scrollbars=no,'
                                   +'toolbar=no,'
                                   +'location=no,'
                                   +'directories=no,'
                                   +'status=no,'
                                   +'menubar=no,'
                                   +'copyhistory=no');">Start</a>
<?php
*/
}


function
islocalhost ()
{
  return $_SERVER['REMOTE_ADDR'] == $_SERVER['SERVER_ADDR'];
}


function
isip ($ip)
{
  // $ip can also be a list of ip's
  return stristr ($ip, $_SERVER['REMOTE_ADDR']);
}


function
get_suffix ($filename)
// get_suffix() never returns NULL
{
  $p = basename ($filename);
  if (!$p)
    $p = filename;

  $s = strchr ($p, '.');
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
  strcpy (get_suffix ($filename), $suffix);

  return $filename;
}


/*
  getfile()           runs callback_func with the realpath() of file/dir as string
                        flags:
  0                           pass all files/dirs with their realpath()
  GETFILE_FILES_ONLY     pass only files with their realpath()
  GETFILE_RECURSIVE      pass all files/dirs with their realpath()'s recursively
  GETFILE_RECURSIVE_ONCE like GETFILE_FILE_RECURSIVE, but only one level deep
  (GETFILE_FILES_ONLY|GETFILE_RECURSIVE)
                           pass only files with their realpath()'s recursively

  callback_func()       getfile() expects the callback_func to return the following
                          values:
                          0 == ok, 1 == skip the rest/break, -1 == failure/break
*/
//define ("GETFILE_FILES_ONLY",     1);
//define ("GETFILE_RECURSIVE",      1<<1);
//define ("GETFILE_RECURSIVE_ONCE", 1<<2);
function
getfile ($path_array, $callback_func, $flags)
{
  $result = 0;
  $i_max = sizeof ($path_array);

  for ($i = 0; $i < $i_max; $i++)
    {
      $dir = opendir ($path_array[$i]);

      if ($dir)
        {
          while (($file = readdir ($dir)) != false)
            if (strcmp ($file, "..") != 0 &&
                strcmp ($file, ".") != 0)
              {
                $result = callback_func ($file);
                if ($result == 1)
                  {
                    closedir ($dir);
                    return 0;
                  }
                if ($result == -1)
                  {
                    closedir (dir);
                    return -1;
                  }
              }
          closedir ($dir);
        }
    }

  return 0;
}


if (!function_exists('sprint_r'))
{
function 
sprint_r ($var)
{
  ob_start();

  print_r ($var);

  $ret = ob_get_contents();

  ob_end_clean();

  return $ret;
}
}


function
force_mozilla ()
{
  if (!stristr ($_SERVER['HTTP_USER_AGENT'], "moz"))
    {
/*
      echo "<script type=\"text/javascript\"><!--\n"
          ."location.href=\"http://www.mozilla.com/firefox/\"\n"
          ."//--></script>\n";
*/
      echo "<meta http-equiv=\"refresh\" content=\"1; URL=http://www.mozilla.com/firefox/\">";

/*
?>
<span style="font-family: arial,sans-serif;">
<table border="0" cellpadding="0" cellspacing="0" width="80%" height="100">
  <tr>
    <td border="0" cellpadding="0" cellspacing="0" bgcolor="#ffff80" align="center">
<font size="-1" face="arial" color="#000000">Your browser is not supported here. Redirecting...</font>
    </td>
  </tr>
</table>
</span>
<?php
*/
      exit ();
    }
}


function
misc_exec ($cmdline)
{
  $a = array();

//  exec ("bash -c \"".$cmdline."\"", $a, $res);
  exec ($cmdline, $a, $res);

  $p = $res."\n";

  $i_max = sizeof ($a);
  for ($i = 0; $i < $i_max; $i++)
    $p .= $a[$i]."\n";

  return $p;
}


function
get_request_value ($name)
{
  if (isset ($_POST[$name]))
    return $_POST[$name];

  if (isset ($_GET[$name]))
    return $_GET[$name];

  return NULL;
}


function
html_head_tags ($icon, $title, $refresh, $charset,
                $use_dc, $dc_desc, $dc_keywords, $dc_identifier, $dc_lang, $dc_author)
{
  $p = "";

  if ($charset)
    $p .= "<meta http-equiv=\"Content-Type\" content=\"text/html; charset="
         .$charset
         ."\">\n";
  else
    $p .= "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n";

  if ($refresh > 0)
    $p .= "<meta http-equiv=\"refresh\" content=\""
         .$refresh
         ."; URL="
         .$_SERVER['REQUEST_URI']
         ."\">\n";

  if ($icon)
    $p .= "<link rel=\"icon\" href=\""
         .$icon
         ."\" type=\"image/png\">\n";

  if ($title)
    $p .= "<title>"
          .$title
          ."</title>\n";

  if (!$use_dc)
    {
      echo $p;
      return;
    }

  $p .= "<meta name=\"description\" content=\""
       .($dc_desc ? $dc_desc : $title)
       ."\">\n"

       ."<meta name=\"author\" content=\""
       .($dc_author ? $dc_author : "Admin")
       ."\">\n"

       ."<meta name=\"keywords\" content=\""
       .($dc_keywords ? $dc_keywords : "html, php")
       ."\">\n"

       ."<meta name=\"robots\" content=\"follow\">\n"

       ."<!-- Dublin Core -->\n"
       ."<meta name=\"DC.Title\" content=\""
       .($dc_desc ? $dc_desc : $title)
       ."\">\n"

       ."<meta name=\"DC.Creator\" content=\""
       .($dc_author ? $dc_author : "Admin")
       ."\">\n"

       ."<meta name=\"DC.Subject\" content=\""
       .($dc_desc ? $dc_desc : $title)
       ."\">\n"

       ."<meta name=\"DC.Description\" content=\""
       .($dc_desc ? $dc_desc : $title)
       ."\">\n"

       ."<meta name=\"DC.Publisher\" content=\""
       .($dc_author ? $dc_author : "Admin")
       ."\">\n"

//       ."<meta name=\"DC.Contributor\" content=\""
//       ."\">\n"

//       ."<meta name=\"DC.Date\" content=\""
//       ."\">\n"

       ."<meta name=\"DC.Type\" content=\"Software\">\n"

       ."<meta name=\"DC.Format\" content=\"text/html\">\n"

       ."<meta name=\"DC.Identifier\" content=\""
       .($dc_identifier ? $dc_identifier : "localhost")
       ."\">\n"

//       ."<meta name=\"DC.Source\" content=\""
//       ."\">\n"

       ."<meta name=\"DC.Language\" content=\""
       .($dc_lang ? $dc_lang : "en")
       ."\">\n"

//       ."<meta name=\"DC.Relation\" content=\""
//       ."\">\n"
//       ."<meta name=\"DC.Coverage\" content=\""
//       ."\">\n"
//       ."<meta name=\"DC.Rights\" content=\"GPL\">\n"
    ;



/*
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head profile="http://geotags.com/geo">
    
    <meta name="description" content="Trapping keyboard events with Javascript -- in a cross-browser way [Sniptools]">
    <meta name="keywords" content="Javascript keyboard events, keypress, javascript, keyCode, which, repeat, keydown event, Sniptools">
    <meta name="author" content="Shashank Tripathi">
    <meta name="revisit-after" content="1 week">
    <meta name="robots" content="index,all">
    <meta name="revisit-after" content="7 days">
    <meta name="author" content="Shashank Tripathi">
    <meta name="generator" content="Homesite 5.0&nbsp; | &nbsp;  Dreamweaver 6 beta&nbsp; | &nbsp; TopStyle 3&nbsp; | &nbsp; Notepad&nbsp; | &nbsp; Adobe PS 7.0">
    <meta name="resource-type" content="Public">
    <meta name="classification" content="Internet Services">
    <meta name="MSSmartTagsPreventParsing" content="TRUE">
    <meta name="robots" content="ALL">
    <meta name="distribution" content="Global">
    <meta name="rating" content="Safe For Kids">
    <meta name="language" content="English">
    <meta name="doc-type" content="Public">
    <meta name="doc-class" content="Living Document">
    <meta name="doc-rights" content="Copywritten Work">
    <meta name="distribution" content="Global">

    <meta http-equiv="imagetoolbar" content="no">
    <meta http-equiv="reply-to" content="editor@NOSPAM.sniptools.com">
    <meta http-equiv="MSThemeCompatible" content="Yes">
    <meta http-equiv="Content-Language" content="en">
    <meta http-equiv="Expires" content="Mon, 24 Sep 1976 12:43:30 IST">
<?php
*/

  return $p;
}


/*
  misc_proxy()
    performs many different tasks (see below)
    can be used to include other html pages inline

  $translate_func
    a (optional) callback function that translates foreign text in html
*/
define ("PROXY_SHOW_HEADER",    1);     // insert the header as comment into the html
//define ("PROXY_REQ_EDITOR",     1<<1);  // edit GET/POST requests/urls (shows all targets in a list)
//define ("PROXY_MAKEPDF",        1<<2);  // turn the whole html page into a pdf
define ("PROXY_PASS_FORMS",     1<<3);  // pass only form tags
define ("PROXY_PASS_LINKS",     1<<4);  // pass only the http links
//define ("PROXY_FILTER_COOKIES", 1<<5);  // remove cookies
//define ("PROXY_FILTER_JS",      1<<6);  // remove JavaScript
//define ("PROXY_FILTER_FLASH",   1<<7);  // remove Flash movies
//define ("PROXY_FILTER_CSS",     1<<8);  // remove CSS
//define ("PROXY_FILTER_ADS",     1<<9); // remove ads (content that comes from a different server)
define ("PROXY_FILTER_HTML",    1<<10); // remove all html tags
/*
  A dereferer is a means to strip the details of the referring website from a
  link request so that the target website cannot identify the page which was
  clicked on to originate a request.
*/
//define ("PROXY_DEREFERER",      1<<12);
/*
  PROXY_PUSH_*
    shows only the CAPTCHA dialog of a (news) site
    and a title, url and description prompt (depending on the target)
*/
define ("PROXY_PUSH_DIGG",      1<<13);
define ("PROXY_PUSH_SLASHDOT",  1<<14); // no CAPTCHA
define ("PROXY_PUSH_DELICIOUS", 1<<15);

function
misc_proxy ($url, $translate_func, $flags)
{
//  $res_keys = $http_response_header; // deprecated
  $res = apache_response_headers ();
  $res_keys = array_keys ($res);
  $req = apache_request_headers ();
  $req_keys = array_keys ($req);

  if (($fp = fopen ($url, "rb")) == false)
    return -1;

  $p = "";
  $i_max = sizeof ($res_keys);
/*
  for ($i = 1; $i < $i_max; $i++)
    {
      if (!strncasecmp ($res[$res_keys[$i]], "Content-Type: ", 14))
        {
          if ($res[$res_keys[$i]] == "Content-Type: audio/mpeg" ||
              $res[$res_keys[$i]] == "Content-Type: application/octet-stream")
            $p .= "Content-Disposition: attachment; filename=".$file;
        }
      else
        $p .= $res[$res_keys[$i]];
      $p .= $res_keys[$i];
    }

  header ($p);
*/

  if ($flags & PROXY_SHOW_HEADER)
    {
      $p = "<!--\n";
      $j_max = sizeof ($req_keys);
      for ($j = 0; $j < $j_max; $j++)
        $p .= $req_keys[$j]
             .": "
             .$req[$req_keys[$j]]
             ."\n";

      $p .= "\n\n\n\n";

      for ($i = 0; $i < $i_max; $i++)
        $p .= $res_keys[$i]
             .": "
             .$res[$res_keys[$i]]
             ."\n";
      $p .= "\n//-->";

      echo $p;
    }

  if ($translate_func ||
      $flags & PROXY_PASS_FORMS ||
      $flags & PROXY_PASS_LINKS)
    {
      while (($p = fgets ($fp)))
        echo $translate_func ($p);
    }
  else
    fpassthru ($fp);

  fclose ($fp);

  return 0;
}


function
rsstool_table_insert ($db, $url, $title, $desc, $site, $dl_url, $date, $dl_date)
{
  $p = sprintf ("INSERT INTO `rsstool_table` ("
      ." `rsstool_url`, `rsstool_url_md5`, `rsstool_url_crc32`,"
      ." `rsstool_dl_url`, `rsstool_dl_url_md5`, `rsstool_dl_url_crc32`,"
      ." `rsstool_title`, `rsstool_title_md5`, `rsstool_title_crc32`,"
      ." `rsstool_site`, `rsstool_desc`, `rsstool_date`, `rsstool_dl_date`) VALUES ('"
      .$db->sql_stresc ($url)
      ."', '"
      .$db->sql_stresc (md5 ($url))
      ."', %u, '"
      .$db->sql_stresc ($dl_url)
      ."', '"
      .$db->sql_stresc (md5 ($dl_url))
      ."', %u, '"
      .$db->sql_stresc ($title)
      ."', '"
      .$db->sql_stresc (md5 ($title))
      ."', %u, '"
      .$db->sql_stresc ($site)
      ."', '"
      .$db->sql_stresc ($desc)
      ."', '"
      .$db->sql_stresc ($date)
      ."', '"
      .$db->sql_stresc ($dl_date)
      ."');", $db->sql_stresc (crc32 ($url)),
              $db->sql_stresc (crc32 ($dl_url)),
              $db->sql_stresc (crc32 ($title)));

  $db->sql_write ($p, 1);
}


?>