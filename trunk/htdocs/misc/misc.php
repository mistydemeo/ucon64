<?php


function
get_firefox ()
{
  //check user-agent and redirect ie users to http://www.mozilla.org/firefox
}


function
traffic ($db, $table_name)
{
  $p = "INSERT INTO `"
      .$table_name
      ."` (`time`,`ip`)"
      ." VALUES ('"
      .time(0)
      ."','"
      .$_SERVER['REMOTE_ADDR']
      ."');";

  sql_write ($db, $p, 0);
}


function
traffic_stats ($db, $table_name)
{
  $p = "SELECT `time`,`ip`"
      ." FROM `"
      .$table_name
      ."`"
      ." WHERE time > "
      .(time (0) - 86400)
      ." ORDER BY `time` DESC";

  $stats = sql_read ($db, $p, 0);

  $p = "";

  if ($stats)
    for ($p = "", $i = 0; $stats[$i]; $i++)
      $p .= $stats[$i][0]
           ." "
           .$stats[$i][1]
           . " "
           .get_country_by_ip ($stats[$i][1])
           ."<br>";

  return $p;
}


function
set_server_uriroot ($uriroot)
{
  $l = strlen ($uriroot);
  $GLOBALS['misc_uriroot'] = ($uriroot[0] != '/' ? "/" : "")
                       .($uriroot[$l - 1] == '/' ? substr ($uriroot, 0, $l - 1) : $uriroot);
}


function
get_server_uriroot ()
{
  return $GLOBALS['misc_uriroot'];
}


function
set_request_method_to_get ()
{
  $GLOBALS['misc_method'] = "GET";
}


function
set_request_method_to_post ()
{
  $GLOBALS['misc_method'] = "POST";
}


function
get_request_method ()
{
  return $GLOBALS['misc_method'];
}


function
get_request_value ($name)
{
  if ($GLOBALS['misc_method'] == "POST")
    return $_POST[$name];
  return $_GET[$name]; // default
}


function
html_head_tags ($icon, $title, $refresh, $charset,
                $use_dc, $dc_desc, $dc_keywords, $dc_identifier, $dc_lang, $dc_author)
{
  $p = "";

  if ($charset)
    $p .= "<meta http-equiv=\"content-type\" content=\"text/html; charset="
         .$charset
         ."\">\n";
  else
    $p .= "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n";

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

  return $p;
}


?>
