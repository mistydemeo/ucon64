<?php


function
get_country_by_ip ($ip)
{
include ("misc/geoip/geoip.inc");
  geoip_load_shared_mem ("php/geoip/GeoIP.dat");

  $gi = geoip_open ("php/geoip/GeoIP.dat", GEOIP_SHARED_MEMORY);

  $country = geoip_country_name_by_addr ($gi, $ip);

  geoip_close ($gi);

  return $country;
}


function
get_country ()
{
  return get_country_by_ip ($_SERVER['REMOTE_ADDR']);
}


function
traffic ($db, $table_name)
{
  $p = sprintf ("INSERT INTO `%s` (`time`,`ip`)", $table_name);
  $p .= sprintf (" VALUES ('%ld','%s');", time(0), $_SERVER['REMOTE_ADDR']);

  sql_set ($db, $p, 0);
}


function
traffic_stats ($db, $table_name)
{
  $p = "SELECT `time`,`ip`";
  $p .= sprintf (" FROM `%s`", $table_name);
  $p .= sprintf (" WHERE time > 0", $category);
  $p .= " ORDER BY `time` DESC";

  $stats = sql_query ($db, $p, 0);

  if ($stats)
    for ($i = 0; $stats[$i]; $i++)
      printf ("%ld %s %s<br>", $stats[$i][0], $stats[$i][1], get_country_by_ip ($stats[$i][1]));
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
  if ($icon)
    printf ("<link rel=\"icon\" href=\"%s\" type=\"image/png\">\n", $icon);

  if ($title)
    printf ("<title>%s</title>\n", $title);

  if ($refresh > 0)
    printf("<meta http-equiv=\"refresh\" content=\"%d; URL=%s\">\n", $refresh, $_SERVER['REQUEST_URI']);

  if ($charset)
    printf ("<meta http-equiv=\"content-type\" content=\"text/html; charset=%s\">\n", $charset);
  else
    printf ("<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n");

  if (!$use_dc)
    return;

  printf (
    "<meta name=\"description\" content=\"%s\">\n"
   ."<meta name=\"author\" content=\"%s\">\n"
   ."<meta name=\"keywords\" content=\"%s\">\n"
   ."<meta name=\"robots\" content=\"follow\">\n"
   ."<meta name=\"title\" content=\"%s\">\n"
   ."<meta name=\"creator\" content=\"%s\">\n"
   ."<meta name=\"subject\" content=\"%s\">\n"
   ."<meta name=\"description\" content=\"%s\">\n"
   ."<meta name=\"publisher\" content=\"%s\">\n"
//   ."<meta name=\"contributor\" content=\"%s\">\n"
//   ."<meta name=\"date\" content=\"%s\">\n"
//   ."<meta name=\"type\" content=\"Software\">\n"
   ."<meta name=\"format\" content=\"text/html\">\n"
   ."<meta name=\"identifier\" content=\"%s\">\n"
//   ."<meta name=\"source\" content=\"%s\">\n"
   ."<meta name=\"language\" content=\"%s\">\n"
//   ."<meta name=\"relation\" content=\"%s\">\n"
//   ."<meta name=\"coverage\" content=\"%s\">\n"
//   ."<meta name=\"rights\" content=\"GPL\">\n"
,
      $dc_desc ? $dc_desc : $title,
      $dc_author ? $dc_author : "Admin",
      $dc_keywords ? $dc_keywords : "html, php",
      $dc_desc ? $dc_desc : $title,
      $dc_author ? $dc_author : "Admin",
      $dc_desc ? $dc_desc : $title,
      $dc_desc ? $dc_desc : $title,
      $dc_author ? $dc_author : "Admin",
      $dc_identifier ? $dc_identifier : "localhost",
      $dc_lang ? $dc_lang : "en"
  );
}


?>
