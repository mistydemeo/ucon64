<?php


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

  sql_set ($db, $p, 0);
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

  $stats = sql_query ($db, $p, 0);

  $p = "";

  if ($stats)
    for ($p = "", $i = 0; $stats[$i]; $i++)
      $p .= $stats[$i][0]
           ." "
           .$stats[$i][1]
           . " "
           .get_country_by_ip ($stats[$i][1])
           ."<br>";

  echo $p;
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
  if ($GLOBALS['misc_method'] == "GET")
    return $_GET[$name];
  else 
    return $_POST[$name];
}


function
html_head_tags ($icon, $title, $refresh, $charset,
                $use_dc, $dc_desc, $dc_keywords, $dc_identifier, $dc_lang, $dc_author)
{
  $p = "";

  if ($icon)
    $p .= "<link rel=\"icon\" href=\""
         .$icon
         ."\" type=\"image/png\">\n";

  if ($title)
    $p .= "<title>"
          .$title
          ."</title>\n";

  if ($refresh > 0)
    $p .= "<meta http-equiv=\"refresh\" content=\""
         .$refresh
         ."; URL="
         .$_SERVER['REQUEST_URI']
         ."\">\n";

  if ($charset)
    $p .= "<meta http-equiv=\"content-type\" content=\"text/html; charset="
         .$charset
         ."\">\n";
  else
    $p .= "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n";

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

       ."<meta name=\"title\" content=\""
       .($dc_desc ? $dc_desc : $title)
       ."\">\n"

       ."<meta name=\"creator\" content=\""
       .($dc_author ? $dc_author : "Admin")
       ."\">\n"

       ."<meta name=\"subject\" content=\""
       .($dc_desc ? $dc_desc : $title)
       ."\">\n"

       ."<meta name=\"description\" content=\""
       .($dc_desc ? $dc_desc : $title)
       ."\">\n"

       ."<meta name=\"publisher\" content=\""
       .($dc_author ? $dc_author : "Admin")
       ."\">\n"

//       ."<meta name=\"contributor\" content=\""
//       ."\">\n"

//       ."<meta name=\"date\" content=\""
//       ."\">\n"

       ."<meta name=\"type\" content=\"Software\">\n"

       ."<meta name=\"format\" content=\"text/html\">\n"

       ."<meta name=\"identifier\" content=\""
       .($dc_identifier ? $dc_identifier : "localhost")
       ."\">\n"

//       ."<meta name=\"source\" content=\""
//       ."\">\n"

       ."<meta name=\"language\" content=\""
       .($dc_lang ? $dc_lang : "en")
       ."\">\n"

//       ."<meta name=\"relation\" content=\""
//       ."\">\n"
//       ."<meta name=\"coverage\" content=\""
//       ."\">\n"
//       ."<meta name=\"rights\" content=\"GPL\">\n"
    ;

  echo $p;
}


?>
