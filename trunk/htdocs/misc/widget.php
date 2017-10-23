<?php
/*
widget.php - HTML widget wrappers in PHP

Copyright (c) 2006 - 2007 NoisyB


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
if (!defined ("MISC_WIDGET_PHP"))
{
define ("MISC_WIDGET_PHP", 1);  
require_once ("misc.php");   // sprint_r()


// widget_init() flags
define ("WIDGET_CSS_OFF",   1);
define ("WIDGET_JS_OFF",    1<<1);
define ("WIDGET_DEBUG",     1<<2);

// widget_*() (widget-wise) flags
define ("WIDGET_RO",         1);    // widget is read-only (widget_textarea, ...)
define ("WIDGET_FOCUS",      1<<1); // document focus is on this widget (widget_text, widget_textarea, ...)
define ("WIDGET_SUBMIT",     1<<2); // widget does submit the whole form
define ("WIDGET_CHECKED",    1<<3); // widget is checked (widget_checkbox, widget_radio, ...)
define ("WIDGET_DISABLED",   1<<4); // widget is inactive 
//define ("WIDGET_VALIDATE",   1<<5); // validate value entered in widget

// widget_relate() flags
define ("WIDGET_RELATE_BOOKMARK",    1<<6);  // browser bookmark
define ("WIDGET_RELATE_STARTPAGE",   1<<7);  // use as start page
define ("WIDGET_RELATE_SEARCH",      1<<8);  // add search plugin to browser
define ("WIDGET_RELATE_LINKTOUS",    1<<9);  // link-to-us code for link sections of other sites
define ("WIDGET_RELATE_TELLAFRIEND", 1<<10); // send tell-a-friend email (smtp)
define ("WIDGET_RELATE_SBOOKMARKS",  1<<11); // social bookmarks
define ("WIDGET_RELATE_DIGGTHIS",    1<<12);
define ("WIDGET_RELATE_DONATE",      1<<13); // donate button (paypal, etc..)
define ("WIDGET_RELATE_RSSFEED",     1<<14); // generate RSS feed
define ("WIDGET_RELATE_ALL",         WIDGET_RELATE_BOOKMARK|
                                     WIDGET_RELATE_STARTPAGE|
                                     WIDGET_RELATE_SEARCH|
                                     WIDGET_RELATE_LINKTOUS|
                                     WIDGET_RELATE_TELLAFRIEND|
                                     WIDGET_RELATE_SBOOKMARKS|
                                     WIDGET_RELATE_DIGGTHIS|
                                     WIDGET_RELATE_DONATE|
                                     WIDGET_RELATE_RSSFEED);


class misc_widget
{
  var $focus = NULL;
  var $name = NULL;
  var $method = NULL;
  var $img_r = NULL,
      $img_bl = NULL,
      $img_b = NULL,
      $img_br = NULL;
  var $flags = 0;


function
widget_init ($flags)
{
  $config = new configure ();
  $p = "";

  $this->flags = $flags;

  if ($config->have_css && !($flags & WIDGET_CSS_OFF))
    {
/*
      $p .= "<link rel=\"stylesheet\" type=\"text/css\" media=\"all\" href=\"css/a.css\">\n";
      $p .= "<link rel=\"stylesheet\" type=\"text/css\" media=\"all\" href=\"css/img.css\">\n";
      $p .= "<link rel=\"stylesheet\" type=\"text/css\" media=\"all\" href=\"css/select.css\">\n";
      $p .= "<link rel=\"stylesheet\" type=\"text/css\" media=\"all\" href=\"css/box.css\">\n";
      $p .= "<link rel=\"stylesheet\" type=\"text/css\" media=\"all\" href=\"css/slider.css\">\n";
      $p .= "<link rel=\"stylesheet\" type=\"text/css\" media=\"all\" href=\"css/relate.css\">\n";
//    removes the lf behind </form>
      $p .= "<link rel=\"stylesheet\" type=\"text/css\" media=\"all\" href=\"css/start.css\">\n";
*/
    }

  if ($config->have_js && !($flags & WIDGET_JS_OFF))
    {
/*
      $p .= "<script type=\"text/javascript\"><!--\n"
           ."var js_ver = 1.0;\n"
           ."//--></script>\n"
           ."<script language=\"JavaScript1.1\"><!--\n"
           ."js_ver = 1.1;\n"
           ."//--></script>\n"
           ."<script language=\"JavaScript1.2\"><!--\n"
           ."js_ver = 1.2;\n"
           ."//--></script>\n"
           ."<script language=\"JavaScript1.3\"><!--\n"
           ."js_ver = 1.3;\n"
           ."//--></script>\n"
           ."<script language=\"JavaScript1.4\"><!--\n"
           ."js_ver = 1.4;\n"
           ."//--></script>\n"
           ."<script language=\"JavaScript1.5\"><!--\n"
           ."js_ver = 1.5;\n"
           ."//--></script>\n"
           ."<script language=\"JavaScript1.6\"><!--\n"
           ."js_ver = 1.6;\n"
           ."//--></script>\n"
           ."<script type=\"text/javascript\"><!--\n"
           .$config->get_js()
           ."//--></script>\n";

      $p .= "<script type=\"text/javascript\" src=\"js/event.js\"></script>\n";
      $p .= "<script type=\"text/javascript\" src=\"js/misc.js\"></script>\n";
      $p .= "<script type=\"text/javascript\" src=\"js/window.js\"></script>\n";
      $p .= "<script type=\"text/javascript\" src=\"js/panel.js\"></script>\n";
      $p .= "<script type=\"text/javascript\" src=\"js/slider.js\"></script>\n";
      $p .= "<script type=\"text/javascript\" src=\"js/relate.js\"></script>\n";
*/
    }

  echo $p;
}


function
widget_start ($name, $target, $method)
{
  $this->name = $name;
  $this->method = $method;

  return "<form class=\"widget_start\" name=\""
        .$name
        ."\" method=\""
        .$method
        ."\" action=\""
        .($target ? $target : $_SERVER['PHP_SELF'])
        ."\""
        .(!strcasecmp ($method, "POST") ? " enctype=\"multipart/form-data\"" : "")
        .">";
}


function
widget_end ()
{
  $p = "</form>"; // TODO: no line-feed after </form>

  if (isset ($this->focus))
    if (!is_null ($this->focus))
      $p .= "<script type=\"text/javascript\"><!--\n\n"
           ."  document."
           .$this->name
           ."."
           .$this->focus
           .".focus();\n\n"
           ."--></script>";

  if (!($this->flags & WIDGET_JS_OFF))
    $p .= "<script type=\"text/javascript\">\n"
         ."<!--\n"
         ."var sliderEl = document.getElementById ? document.getElementById(\"slider-1\") : null;\n"
         ."var inputEl = document."
         .$this->name
         ."[\"slider-input-1\"];\n"
         ."var s = new Slider(sliderEl, inputEl);\n"
         ."s.onchange = function ()\n"
         ."  {\n"
         ."window.status = \"Value: \" + s.getValue();\n"
         ."};\n"
         ."s.setValue(50);\n"
         ."//-->\n"
         ."</script>";

  $this->name = NULL;
  $this->focus = NULL;

  return $p;
}


function
widget_text ($name, $value, $tooltip, $size, $maxlength, $flags)
{
  if ($flags & WIDGET_FOCUS)
    $this->focus = $name;

  return "<input class=\"widget_text\" type=\"text\" name=\""
        .$name
        ."\" value=\""
        .$value
        ."\" size=\""
        .$size
        ."\" maxlength=\""
        .$maxlength
        ."\" title=\""
        .$tooltip
        ."\""
        .($flags & WIDGET_DISABLED ? " disabled" : "")
        .">";
}


function
widget_textarea ($name, $value, $tooltip, $cols, $rows, $flags)
{
  if ($flags & WIDGET_FOCUS)
    $this->focus = $name;

  return "<textarea class=\"widget_textarea\" name=\""
        .$name
        ."\" title=\""
        .$tooltip
        ."\" cols=\""
        .$cols
        ."\" rows=\""
        .$rows
        ."\""
        .($flags & WIDGET_RO ? " readonly=\"readonly\"" : "")
        .($flags & WIDGET_DISABLED ? " disabled" : "")
        .">"
        .$value
        ."</textarea>";
}


function
widget_hidden ($name, $value, $flags)
{
  if ($flags & WIDGET_FOCUS)
    $this->focus = $name;

  return "<input class=\"widget_hidden\" type=\"hidden\" name=\""
        .$name
        ."\" value=\""
        .$value
        ."\">";
}


function
widget_submit ($name, $label, $tooltip, $flags)
{
  if ($flags & WIDGET_FOCUS)
    $this->focus = $name;

  return "<input class=\"widget_submit\" type=\"submit\" name=\""
        .$name
        ."\" value=\""
        .$label
        ."\" title=\""
        .$tooltip
        ."\">";
}


function
widget_reset ($name, $label, $tooltip, $flags)
{
  if ($flags & WIDGET_FOCUS)
    $this->focus = $name;

  return "<input class=\"widget_reset\" type=\"reset\" name=\""
        .$name
        ."\" value=\""
        .$label
        ."\" title=\""
        .$tooltip
        ."\""
        .($flags & WIDGET_DISABLED ? " disabled" : "")
        .">";
}


function
widget_image ($name, $value, $img, $w, $h, $tooltip, $flags)
{
  if ($flags & WIDGET_FOCUS)
    $this->focus = $name;

  return "<button class=\"widget_image\" type=\"submit\""
        .($flags & WIDGET_DISABLED ? " disabled" : "")
        .">"
        ."<img src=\""
        .$img
        ."\""
        .($w != -1 ? " width=\"".$w."\"" : "")
        .($h != -1 ? " height=\"".$h."\"" : "")
        ." border=\"0\""
        ." alt=\""
        .$tooltip
        ."\">"
//        ."<span>"
//        .$tooltip
//        ."</span>"
        ."</button>";
}


function
widget_checkbox ($name, $tooltip, $flags)
{
  if ($flags & WIDGET_FOCUS)
    $this->focus = $name;

  return "<input class=\"widget_checkbox\" type=\"checkbox\""
        ." name=\""
        .$name
        ."\""
        .($flags & WIDGET_CHECKED ? " checked" : "")
        ." title=\""
        .$tooltip
        ."\""
        .($flags & WIDGET_DISABLED ? " disabled" : "")
        .">";
}


function
widget_radio ($name, $value_array, $label_array, $tooltip, $vertical, $flags)
{
  if ($flags & WIDGET_FOCUS)
    $this->focus = $name;

  $p = "";
  $i_max = sizeof ($value_array);
  for ($i = 0; $i < $i_max; $i++)
    $p .= "<input class=\"widget_radio\" type=\"radio\""
         .($flags & WIDGET_SUBMIT ? " onchange=\"this.form.submit();\"" : "")
         .(!$i ? " checked" : "")
         ." name=\""
         .$name
         ."\""
         ." title=\""
         .$tooltip
         ."\""
         ." value=\""
         .$value_array[$i]
         ."\""
         .($flags & WIDGET_DISABLED ? " disabled" : "")
         ."> "
         .$label_array[$i]
         .($vertical ? "<br>" : "");

  return $p;
}


/*
  In PHP versions earlier than 4.1.0, $HTTP_POST_FILES should be used instead
  of $_FILES. Use phpversion() for version information.

  $_FILES['userfile']['name']
    The original name of the file on the client machine. 
  $_FILES['userfile']['type']
    The mime type of the file, if the browser
    provided this information. An example would be "image/gif". This mime
    type is however not checked on the PHP side and therefore don't take its
    value for granted.
  $_FILES['userfile']['size']
    The size, in bytes, of the uploaded file. 
  $_FILES['userfile']['tmp_name']
    The temporary filename of the file in which the uploaded file was stored on the server. 
  $_FILES['userfile']['error']
    The error code associated with this file upload. This element was added in PHP 4.2.0 

  UPLOAD_ERR_OK          0; There is no error, the file uploaded with success. 
  UPLOAD_ERR_INI_SIZE    1; The uploaded file exceeds the upload_max_filesize directive in php.ini. 
  UPLOAD_ERR_FORM_SIZE   2; The uploaded file exceeds the MAX_FILE_SIZE directive that was specified in the HTML form. 
  UPLOAD_ERR_PARTIAL     3; The uploaded file was only partially uploaded. 
  UPLOAD_ERR_NO_FILE     4; No file was uploaded. 
  UPLOAD_ERR_NO_TMP_DIR  6; Missing a temporary folder. Introduced in PHP 4.3.10 and PHP 5.0.3. 
  UPLOAD_ERR_CANT_WRITE  7; Failed to write file to disk. Introduced in PHP 5.1.0. 
  UPLOAD_ERR_EXTENSION   8; File upload stopped by extension. Introduced in PHP 5.2.0. 

  related php.ini settings
    if (post_max_size > upload_max_filesize) in php.ini
      otherwise you will not be able to report the correct error in case of a
      too big upload ! Also check the max-execution-time (upload-time could be
      added to execution-time)

    if (post >post_max_size) in php.ini
      $_FILES and $_POST will return empty

  The data encoding type, enctype, MUST be specified as enctype="multipart/form-data"
  MAX_FILE_SIZE must precede the file input field
  Name of input element determines name in $_FILES array
*/
function
widget_upload ($name, $label, $tooltip, $upload_path, $max_file_size, $mime_type, $flags)
{
  if (strcasecmp ($this->method, "POST"))
    return "widget_upload() requires method=\"POST\" and enctype=\"multipart/form-data\" in the form tag\n";

  if ($flags & WIDGET_FOCUS)
    $this->focus = $name;

  $p = $this->widget_hidden ("MAX_FILE_SIZE", $max_file_size, 0)
      ."<input class=\"widget_upload\" type=\"file\""
      ." name=\""
      .$name
      ."\""
      ." title=\""
      .$tooltip
      ."\""
      .($max_file_size ? " maxlength=\"".$max_file_size."\"" : "")
      .($mime_type ? " accept=\"".$mime_type."\"" : "")
      .($flags & WIDGET_DISABLED ? " disabled" : "")
      .">"
      .$this->widget_submit ($name, $label, $tooltip, 0);

  if (!$_FILES)
    return $p;

/*
  $p .= "<pre>"
       .sprint_r ($_FILES) // debug
       ."</pre>";
*/

  if (move_uploaded_file ($_FILES[$name]["tmp_name"],
                          $upload_path
                         ."/"
                         .basename($_FILES[$name]["name"])) == FALSE)
    {
//    FALSE
    }

  $s = Array ();
  $s[UPLOAD_ERR_OK] =           "OK";
  $s[UPLOAD_ERR_INI_SIZE] =     "The uploaded file exceeds the upload_max_filesize directive ("
                               .ini_get ("upload_max_filesize")
                               .") in php.ini";
  $s[UPLOAD_ERR_FORM_SIZE] =    "The uploaded file exceeds the MAX_FILE_SIZE directive ("
                               .$max_file_size
                               .") that was specified in the HTML form";
  $s[UPLOAD_ERR_PARTIAL] =      "The uploaded file was only partially uploaded";
  $s[UPLOAD_ERR_NO_FILE] =      "No file was uploaded";
  $s[UPLOAD_ERR_NO_TMP_DIR] =   "Missing a temporary folder";
/*
  if (defined (UPLOAD_ERR_CANT_WRITE))
    $s[UPLOAD_ERR_CANT_WRITE] = "Failed to write file to disk";
  if (defined (UPLOAD_ERR_EXTENSION))
    $s[UPLOAD_ERR_EXTENSION] =  "File upload stopped by extension";
*/

  $e = $s[$_FILES[$name]["error"]];
  if (!$e)
    $e .= "Unknown File Error.";

  $p .= $e;

/*
  $p .= "\n\n\n<pre>"
       .sprint_r ($s) // debug
       ."</pre>";
*/
//  print_r ($_FILES); // debug

  return $p;
}
  

function
widget_password ($name, $tooltip, $flags)
{
  if ($flags & WIDGET_FOCUS)
    $this->focus = $name;

  return "<input class=\"widget_password\" type=\"password\""
        ." name=\""
        .$name
        ."\""
        ." title=\""
        .$tooltip
        ."\""
        .($flags & WIDGET_DISABLED ? " disabled" : "")
        .">";
}


function
widget_select ($img, $name, $img_array, $name_array, $value_array, $tooltip, $flags)
{
  if ($flags & WIDGET_FOCUS)
    $this->focus = $name;

  $p = "";

  if ($img)
    $p .= "<img class=\"widget_select\" src=\""
         .$img
         ."\" border=\"0\""
         .($tooltip ? " title=\"".$tooltip."\"" : "")
         .($tooltip ? " alt=\"".$tooltip."\"" : "")
         .">";

//  if (!($this->flags & WIDGET_CSS_OFF))
  if (0)
    {
      $p .= "<span>";
    
      $i_max = max (sizeof ($img_array), sizeof ($name_array), sizeof ($value_array));
      for ($i = 0; $i < $i_max; $i++)
        $p .= "<a href=\""
             .$value_array[$i]
             ."\">"
             .$name_array[$i]
             ."</a>";
    
      $p .= "</span>";
    }
  else
    {
      $p .= "<select"
           .($img ? " style=\"background-image:url('".$img."');\"" : "")
           .($flags & WIDGET_SUBMIT ? " onchange=\"this.form.submit();\"" : "")
           ." name=\""
           .$name
           ."\""
           .($tooltip ? " title=\"".$tooltip."\"" : "")
           .($flags & WIDGET_DISABLED ? " disabled" : "")
           .">";
    
      $p .= "<option selected"
//           ." style=\"background-image:url('".$img."');\""
           .">";
      $i_max = max (sizeof ($img_array), sizeof ($name_array), sizeof ($value_array));
      for ($i = 0; $i < $i_max; $i++)
        $p .= "<option"
//             .(!$i ? " selected" : "")
//             .(!$i ? " style=\"background-image:url('".$img."');\"" : "")
             ." value=\""
             .$value_array[$i]
             ."\">"
             .($img_array[$i] ? "<img src=\"".$img_array[$i]."\" border=\"0\">" : "")
             .$name_array[$i]
             ."</option>";
    
      $p .= "</select>";
    }

  return $p;
}


function
widget_select_int ($img, $name, $img_array, $name_array, $tooltip, $flags)
{
  $value_array = Array ();
  $i_max = max (sizeof ($name_array), sizeof ($value_array));

  for ($i = 0; $i < $i_max; $i++)
    $value_array[$i] = $i;

  return $this->widget_select ($img, $name, $img_array, $name_array, $value_array, $tooltip, $flags);
}


function
widget_a ($url, $target, $img, $w, $h, $label, $tooltip, $flags)
{
  if ($flags & WIDGET_FOCUS)
    $this->focus = $name;

  $p = "";
  if ($img)
    $p .= "<a class=\"widget_a_img\""
         ." href=\""
         .$url
         ."\""
         ." target=\""
         .$target
         ."\""
         .($flags & WIDGET_DISABLED ? " disabled" : "")
         .">"
         ."<img src=\""
         .$img
         ."\""
         .($w != -1 ? " width=\"".$w."\"" : "")
         .($h != -1 ? " height=\"".$h."\"" : "")
         ." border=\"0\""
         ." alt=\""
         .$tooltip
         ."\">"
         .($tooltip ? "<span>".$tooltip."</span>" : "")
         ."</a>";
  else if ($label)
    $p .= "<a class=\"widget_a_label\""
         ." href=\""
         .$url
         ."\""
         ." target=\""
         .$target
         ."\""
         .($flags & WIDGET_DISABLED ? " disabled" : "")
         ." title=\""
         .$tooltip
         ."\""
         .">"
         ."<nobr>"
         .$label
         ."</nobr>"
//         .($tooltip ? "<span>".$tooltip."</span>" : "")
         ."</a>";

  return $p;
}


function
widget_img ($name, $img, $w, $h, $border, $alt, $tooltip, $flags)
{
  if ($flags & WIDGET_FOCUS)
    $this->focus = $name;

  $p = "<img class=\"widget_img\" src=\""
      .$img
      ."\""
      ." name=\""
      .$name
      ."\""
      .($w != -1 ? " width=\"".$w."\"" : "")
      .($h != -1 ? " height=\"".$h."\"" : "")
      ." border=\""
      .$border
      ."\" alt=\""
      .$alt
      ."\""
      ." title=\""
      .$tooltip
      ."\">"
      .(!($this->flags & WIDGET_CSS_OFF) ? "<span>".$tooltip."</span>" : "");

  return $p;
}


function
widget_trans ($w, $h, $flags)
{
  if ($flags & WIDGET_FOCUS)
    $this->focus = $name;

  return "<img class=\"widget_trans\""
        ." src=\"images/widget_trans.png\""
        .($w != -1 ? " width=\"".$w."\"" : "")
        .($h != -1 ? " height=\"".$h."\"" : "")
        ." border=\"0\""
        ." alt=\"images/widget_trans.png not found\">";
}


function
widget_gauge ($percent, $flags)
{
  if ($flags & WIDGET_FOCUS)
    $this->focus = $name;

  return "<table class=\"widget_gauge\" border=\"0\" width=\"640\" cellspacing=\"0\" cellpadding=\"0\">"
        ."  <tr>"
        ."    <td width=\""
        .$percent
        ."%\" bgcolor=\"#00ff00\">&nbsp;</td>"
        ."    <td bgcolor=\"#ff0000\">&nbsp;</td>"
        ."  </tr>"
        ."</table>";
}


function
widget_panel ($url_array, $img_array, $w, $h, $tooltip)
{
?>
<script language="JavaScript">
<!--

//var test_array = new Array  (<?php

$p = "";
$i_max = sizeof ($img_array);  
for ($i = 0; $i < $i_max; $i++)
  {
    if ($i)
      $p .= ", ";
    $p .= "widget_panel_".$i;
  }

echo $p;
?>);

var img_w = <?php echo $w; ?>;
var img_h = <?php echo $h; ?>;
var img_n = <?php echo sizeof ($img_array); ?>;


function
js_panel_get_img_array ()
{
  var img = new Array (<?php

$p = "";
$i_max = sizeof ($img_array);
for ($i = 0; $i < $i_max; $i++)
  {
    if ($i)
      $p .= ", ";
    $p .= "widget_panel_".$i;
  }

echo $p;

?>);
  return img;
}

//-->
</script><?

  $p = "<table width=\"100%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n"
      ."<tr>\n"
      ."    <td height=\"10\" colspan=\"4\" onMouseOver=\"js_mouse_callback_func (js_panel_event_ignore);\">\n"
      ."    </td> \n"
      ."  </tr>\n"
      ."  <tr>\n"
      ."    <td width=\"10\" height=\"140\" valign=\"bottom\" onMouseOver=\"js_mouse_callback_func (js_panel_event_ignore);\">\n"
      ."    </td>\n"
      ."    <td width=\"14%\" valign=\"bottom\" onMouseOver=\"js_mouse_callback_func (js_panel_event);\">\n"
      ."    </td>\n"
      ."    <td width=\"86%\" valign=\"bottom\" onMouseOver=\"js_mouse_callback_func (js_panel_event);\">\n"
      ."<nobr>\n";

  $i_max = min (sizeof ($url_array), sizeof ($img_array));
  for ($i = 0; $i < $i_max; $i++)
    $p .= "<a href=\""
         .$url_array[$i]
         ."\" target=\"_blank\"><img name=\"widget_panel_"
         .$i
         ."\" src=\""
         .$img_array[$i]
         ."\" width=\""
         .$w
         ."\" height=\""
         .$h
         ."\" border=\"0\"></a>\n";

  $p .= "</nobr>\n"
       ."    </td>\n"
       ."    <td width=\"10\" valign=\"bottom\" onMouseOver=\"js_mouse_callback_func (js_panel_event_ignore);\">\n"
       ."    </td>\n"
       ."  </tr>\n"
       ."  <tr>\n"
       ."    <td height=\"10\" colspan=\"4\" onMouseOver=\"js_mouse_callback_func (js_panel_event_ignore);\">\n"
       ."    </td> \n"
       ."  </tr>\n"
       ."</table>\n";

  return $p;
}


function
widget_box_start ($img_tl, $img_t, $img_tr, $img_l, $img_r, $img_bl, $img_b, $img_br)
{
  $this->img_r  = $img_r;
  $this->img_bl = $img_bl;
  $this->img_b  = $img_b;
  $this->img_br = $img_br;

  return "<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n"
        ."  <tr>\n"
        ."    <td>\n"
        ."<img src=\""
        .$img_tl
        ."\">\n"
        ."    </td>\n"
        ."    <td class=\""
        .$img_t
        ."\">\n"
        ."    </td>\n"
        ."    <td class=\""
        .$img_tr
        ."\">\n"
        ."    </td>\n"
        ."  </tr>\n"
        ."  <tr>\n"
        ."    <td class=\""
        .$img_l
        ."\">\n"
        ."    </td>\n"
        ."    <td valign=\"top\" style=\"background-color:#fff\">\n";
}


function
widget_box_end ()
{
  return "    </td>\n"
        ."    <td class=\""
        .$this->img_r
        ."\">\n"
        ."    </td>\n"
        ."  </tr>\n"
        ."  <tr>\n"
        ."    <td class=\""
        .$this->img_bl
        ."\">\n"
        ."    </td>\n"
        ."    <td class=\""
        .$this->img_b
        ."\">\n"
        ."    </td>\n"
        ."    <td> \n"
        ."<img src=\""
        .$this->img_br
        ."\" border=\"0\">\n"
        ."    </td>\n"
        ."  </tr>\n"
        ."</table>\n";
}


function
widget_audio ($url, $start, $stream, $next_stream)
{
  return "<object"
//  classid="clsid:d27cdb6e-ae6d-11cf-96b8-444553540000" 
//  codebase="http://fpdownload.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=7,0,0,0" 
//  id="widget_audio"
//  movie="widget_audio.swf"
        ."><embed src=\""
        .dirname ($_SERVER['PHP_SELF'])
        ."/misc/misc_flash.swf?url="
        .$url
        ."&start="
        .$start
        ."&stream="
        .$stream
        ."&next="
        .$next_stream
        ."\""
        ." type=\"application/x-shockwave-flash\""
        ." width=\"1\""
        ." height=\"1\""
//  quality="high"
//  bgcolor="#ffffff"
//  loop="true"
//  align=""
//  allowScriptAccess="sameDomain"
//  swLiveConnect="true"
       ." pluginspace=\"http://www.macromedia.com/go/flashplayer/\""
       ."></embed></object>";
}


function
widget_slider ($name, $value, $tooltip, $vertical, $flags)
{

  return "<div class=\"slider\" id=\"slider-1\" tabIndex=\"1\" style=\"width:auto; margin:10px;\">"
        ."<input class=\"slider-input\" id=\"slider-input-1\"/>"
        ."</div>";
}


function
widget_tabs ($name, $value_array, $label_array, $tooltip, $vertical, $flags)
{
//  return $this->widget_radio ($name, $value_array, $label_array, $tooltip, $vertical, $flags);
  $p = "";
  if ($vertical)
    $p .= "<table border=\"0\"><tr>";

  $i_max = sizeof ($value_array);
  for ($i = 0; $i < $i_max; $i++)
    $p .= $this->widget_a ($value_array[$i], NULL, NULL, -1, -1, $label_array[$i], $tooltip, $flags)
         .($vertical ? "<br>" : " ");

  if ($vertical)
    $p .= "</tr></table>";

  return $p;
}


function
widget_tree ($name, $path, $mime_type, $flags)
{
  $p = "";
  $dir = opendir ($path);
  while (($file = readdir ($dir)) != false)
    {
      if (is_dir ($file))
        {
          $p .= "<img src=\"images/widget_tree_closed.png\" border=\"0\" alt=\"images/widget_tree_open.png\">"
                 .basename ($file);
        }
      else if (is_file ($file))
        {
          $stat = stat ($file);
          $p .= "<img src=\"images/widget_tree_file.png\" border=\"0\" alt=\"images/widget_tree_file.png\">"
               .basename ($file)
               .$stat['size'];
        }
      else // ?
        {
          $p .= "<img src=\"images/widget_tree_file.png\" border=\"0\" alt=\"images/widget_tree_file.png\">"
               .basename ($file);
        }

      $p .= "<br>\n";
    }
  closedir ($dir);

  return $p;
}




function
widget_relate ($title, $url, $rss_feed_url, $vertical, $flags)
{
  $p = "";
//  $p .= "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" style=\"background-color:#fff;\">\n"
//       ."<tr><td>\n";
  $p .= "<font size=\"-1\" face=\"arial,sans-serif\">\n";

/*
  // digg this button
  if ($flags & WIDGET_RELATE_DIGGTHIS)
    $p .= "<script><!--\n"
         ."digg_url = '"
         .$url
         ."';"
         ."//--></script>\n"
         ."<script type=\"text/javascript\" src=\"http://digg.com/api/diggthis.js\"></script>"
         .($vertical ? "<br>" : " ");
*/

  // donate
  if ($flags & WIDGET_RELATE_DONATE)
    $p .= "<img class=\"widget_relate_img\" src=\"images/widget_relate_paypal.png\" border=\"0\">"
         ."<a class=\"widget_relate_label\" href=\"http://paypal.com\">Donate</a>\n"
/*
<pre>* * *   D O N A T I O N S   A R E   A C C E P T E D   * * *</pre><br>
<br>
<img src="images/widget_relate_refrigator.jpg" border="0"><br>
<br>
Individuals and companies can now donate funds to support me and keep me from<br>
writing proprietary software.<br>
<br>
Thank You!<br>
<br>
<pre>* * *   D O N A T I O N S   A R E   A C C E P T E D   * * *</pre><br-->
search widget to include in other pages
*/
         .($vertical ? "<br>" : " ");

/*
  // link-to-us code for link sections of other sites
  if ($flags & WIDGET_RELATE_LINKTOUS)
    $p .= "<textarea title=\"Add this code to your blog or website\">Link to us</textarea>"
         .($vertical ? "<br>" : " ");
*/

  if ($flags & WIDGET_RELATE_TELLAFRIEND)
    $p .= "<img class=\"widget_relate_img\" src=\"images/widget_relate_tellafriend.png\" border=\"0\">"
         ."<a class=\"widget_relate_label\" href=\"mailto:?body="
         .$url
         ."&subject="
         .$title
         ."\""
         ." title=\"Send this link to your friends\">Tell a friend</a>"
         .($vertical ? "<br>" : " ");

  // add browser bookmark
  if ($flags & WIDGET_RELATE_BOOKMARK)
    $p .= "<img class=\"widget_relate_img\" src=\"images/widget_relate_star.png\" border=\"0\">"
         ."<a class=\"widget_relate_label\""
/*
         ." href=\"javascript:js_bookmark ('"
         .trim ($url)
         ."', '"
         .trim ($title)
         ."');\""
*/
         ." href=javascript:addToFavorites('fuck','you');"
         ." border=\"0\">Bookmark</a>\n"
         .($vertical ? "<br>" : " ");

  // use as startpage
  if ($flags & WIDGET_RELATE_STARTPAGE)
    $p .= "<img class=\"widget_relate_img\" src=\"images/widget_relate_home.png\" border=\"0\">"
         ."<a class=\"widget_relate_label\""
         ." href=\"http://\""
         ." onclick=\"this.style.behavior='url(#default#homepage)';this.setHomePage('http://torrent-finder.com');\""
         .">Make us your start page</a>"
         .($vertical ? "<br>" : " ");

  // add search plugin to browser
  if ($flags & WIDGET_RELATE_SEARCH)
    $p .= "<img class=\"widget_relate_img\" src=\"images/widget_relate_search.png\" border=\"0\">"
         ."<a class=\"widget_relate_label\""
         ." href=\"http://\""
//         ." href=\"javascript:js_bookmark('"
//         .$title
//         ."', '"
//         .$url
//         ."')\""
         ." border=\"0\">Add search</a>\n"
         .($vertical ? "<br>" : " ");

  // generate rss feed
  if ($flags & WIDGET_RELATE_RSSFEED)
    $p .= "<img class=\"widget_relate_img\" src=\"images/widget_relate_rss.png\" border=\"0\">"
         ."<a class=\"widget_relate_label\""
         ." href=\""
         .$rss_feed_url
         ."\""
         ." border=\"0\">RSS feed</a>\n";

  // social bookmarks
  if ($flags & WIDGET_RELATE_SBOOKMARKS)
    {
      $a = Array (
//        Array ("30 Day Tags",		"widget_relate_30_day_tags.png", NULL, NULL),
//        Array ("AddToAny",		"widget_relate_addtoany.png", NULL, NULL),
//        Array ("Ask",			"widget_relate_ask.png", NULL, NULL),
//        Array ("BM Access",		"widget_relate_bm_access.png", NULL, NULL),
        Array ("Backflip",		"widget_relate_backflip.png", "http://www.backflip.com/add_page_pop.ihtml?url=", "&title="),
//        Array ("BlinkBits",		"widget_relate_blinkbits.png", "http://www.blinkbits.com/bookmarklets/save.php?v=1&source_url=", "&title="),
        Array ("BlinkBits",		"widget_relate_blinkbits.png", "http://www.blinkbits.com/bookmarklets/save.php?v=1&source_image_url=&rss_feed_url=&rss_feed_url=&rss2member=&body=&source_url=", "&title="),
        Array ("Blinklist",		"widget_relate_blinklist.png", "http://www.blinklist.com/index.php?Action=Blink/addblink.php&Description=&Tag=&Url=", "&Title="),
//        Array ("Bloglines",		"widget_relate_bloglines.png", NULL, NULL),
        Array ("BlogMarks",		"widget_relate_blogmarks.png", "http://blogmarks.net/my/new.php?mini=1&simple=1&url=", "&content=&public-tags=&title="),
//        Array ("BlogMarks",		"widget_relate_blogmarks.png", "http://blogmarks.net/my/new.php?mini=1&simple=1&url=", "&title="),
        Array ("Blogmemes",		"widget_relate_blogmemes.png", "http://www.blogmemes.net/post.php?url=", "&title="),
//        Array ("Blue Dot",		"widget_relate_blue_dot.png", NULL, NULL),
        Array ("Buddymarks",		"widget_relate_buddymarks.png", "http://buddymarks.com/s_add_bookmark.php?bookmark_url=", "&bookmark_title="),
//        Array ("CiteULike",		"widget_relate_citeulike.png", NULL, NULL),
        Array ("Complore",		"widget_relate_complore.png", "http://complore.com/?q=node/add/flexinode-5&url=", "&title="),
//        Array ("Connotea",		"widget_relate_connotea.png", NULL, NULL),
        Array ("Del.icio.us",		"widget_relate_del.icio.us.png", "http://del.icio.us/post?v=2&url=", "&notes=&tags=&title="),
//        Array ("Del.icio.us",		"widget_relate_del.icio.us.png", "http://del.icio.us/post?v=2&url=", "&title="),
//        Array ("Del.icio.us",		"widget_relate_del.icio.us.png", "http://del.icio.us/post?url=", "&title="),
        Array ("De.lirio.us",		"widget_relate_de.lirio.us.png", "http://de.lirio.us/bookmarks/sbmtool?action=add&address=", "&title="),
        Array ("Digg",			"widget_relate_digg.png", "http://digg.com/submit?phase=2&url=", "&bodytext=&tags=&title="),
//        Array ("Digg",		"widget_relate_digg.png", "http://digg.com/submit?phase=2&url=", "&title="),
        Array ("Diigo",			"widget_relate_diigo.png", "http://www.diigo.com/post?url=", "&tag=&comments=&title="),
//        Array ("Dogear",		"widget_relate_dogear.png", NULL, NULL),
//        Array ("Dotnetkicks",		"widget_relate_dotnetkicks.png", "http://www.dotnetkicks.com/kick/?url=", "&title="),
//        Array ("Dude, Check This Out",	"widget_relate_dude_check_this_out.png", NULL, NULL),
//        Array ("Dzone",		"widget_relate_dzone.png", NULL, NULL),
//        Array ("Eigology",		"widget_relate_eigology.png", NULL, NULL),
        Array ("Fark",			"widget_relate_fark.png", "http://cgi.fark.com/cgi/fark/edit.pl?new_url=", "&title="),
//        Array ("Favoor",		"widget_relate_favoor.png", NULL, NULL),
//        Array ("FeedMeLinks",		"widget_relate_feedmelinks.png", NULL, NULL),
//        Array ("Feedmarker",		"widget_relate_feedmarker.png", NULL, NULL),
        Array ("Folkd",			"widget_relate_folkd.png", "http://www.folkd.com/submit/", NULL),
//        Array ("Freshmeat",		"widget_relate_freshmeat.png", NULL, NULL)
        Array ("Furl",			"widget_relate_furl.png", "http://www.furl.net/storeIt.jsp?u=", "&keywords=&t="),
//        Array ("Furl",		"widget_relate_furl.png", "http://www.furl.net/storeIt.jsp?u=", "&t="),
//        Array ("Furl",		"widget_relate_furl.png", "http://www.furl.net/store?s=f&to=0&u=", "&ti="),
//        Array ("Givealink",		"widget_relate_givealink.png", NULL, NULL),
        Array ("Google",		"widget_relate_google.png", "http://www.google.com/bookmarks/mark?op=add&hl=en&bkmk=", "&annotation=&labels=&title="),
//        Array ("Google",		"widget_relate_google.png", "http://www.google.com/bookmarks/mark?op=add&bkmk=", "&title="),
//        Array ("Humdigg",		"widget_relate_humdigg.png", NULL, NULL),
//        Array ("HLOM (Hyperlinkomatic)",		"widget_relate_hlom.png", NULL, NULL),
//        Array ("I89.us",		"widget_relate_i89.us.png", NULL, NULL),
        Array ("Icio",			"widget_relate_icio.png", "http://www.icio.de/add.php?url=", NULL),
//        Array ("Igooi",		"widget_relate_igooi.png", NULL, NULL),
//        Array ("Jots",		"widget_relate_jots.png", NULL, NULL),
//        Array ("Link Filter",		"widget_relate_link_filter.png", NULL, NULL),
//        Array ("Linkagogo",		"widget_relate_linkagogo.png", NULL, NULL),
        Array ("Linkarena",		"widget_relate_linkarena.png", "http://linkarena.com/bookmarks/addlink/?url=", "&desc=&tags=&title="),
//        Array ("Linkatopia",		"widget_relate_linkatopia.png", NULL, NULL),
//        Array ("Linklog",		"widget_relate_linklog.png", NULL, NULL),
//        Array ("Linkroll",		"widget_relate_linkroll.png", NULL, NULL),
//        Array ("Listable",		"widget_relate_listable.png", NULL, NULL),
//        Array ("Live",		"widget_relate_live.png", "https://favorites.live.com/quickadd.aspx?marklet=1&mkt=en-us&url=", "&title="),
//        Array ("Lookmarks",		"widget_relate_lookmarks.png", NULL, NULL),
        Array ("Ma.Gnolia",		"widget_relate_ma.gnolia.png", "http://ma.gnolia.com/bookmarklet/add?url=", "&description=&tags=&title="),
//        Array ("Ma.Gnolia",		"widget_relate_ma.gnolia.png", "http://ma.gnolia.com/bookmarklet/add?url=", "&title="),
//        Array ("Maple",		"widget_relate_maple.png", NULL, NULL),
//        Array ("MrWong",		"widget_relate_mrwong.png", NULL, NULL),
//        Array ("Mylinkvault",		"widget_relate_mylinkvault.png", NULL, NULL),
        Array ("Netscape",		"widget_relate_netscape.png", "http://www.netscape.com/submit/?U=", "&T="),
        Array ("NetVouz",		"widget_relate_netvouz.png", "http://netvouz.com/action/submitBookmark?url=", "&popup=yes&description=&tags=&title="),
//        Array ("NetVouz",		"widget_relate_netvouz.png", "http://netvouz.com/action/submitBookmark?url=", "&title="),
        Array ("Newsvine",		"widget_relate_newsvine.png", "http://www.newsvine.com/_tools/seed&save?u=", "&h="),
//        Array ("Newsvine",		"widget_relate_newsvine.png", "http://www.newsvine.com/_wine/save?popoff=1&u=", "&tags=&blurb="),
//        Array ("Nextaris",		"widget_relate_nextaris.png", NULL, NULL),
//        Array ("Nowpublic",		"widget_relate_nowpublic.png", NULL, NULL),
//        Array ("Oneview",		"widget_relate_oneview.png", "http://beta.oneview.de:80/quickadd/neu/addBookmark.jsf?URL=", "&title="),
//        Array ("Onlywire",		"widget_relate_onlywire.png", NULL, NULL),
//        Array ("Pligg",		"widget_relate_pligg.png", NULL, NULL),
//        Array ("Portachi",		"widget_relate_portachi.png", NULL, NULL),
//        Array ("Protopage",		"widget_relate_protopage.png", NULL, NULL),
        Array ("RawSugar",		"widget_relate_rawsugar.png", "http://www.rawsugar.com/pages/tagger.faces?turl=", "&tttl="),
        Array ("Reddit",		"widget_relate_reddit.png", "http://reddit.com/submit?url=", "&title="),
//        Array ("Rojo",		"widget_relate_rojo.png", NULL, NULL),
        Array ("Scuttle",		"widget_relate_scuttle.png", "http://www.scuttle.org/bookmarks.php/maxpower?action=add&address=", "&description="),
//        Array ("Searchles",		"widget_relate_searchles.png", NULL, NULL),
        Array ("Shadows",		"widget_relate_shadows.png", "http://www.shadows.com/features/tcr.htm?url=", "&title="),
//        Array ("Shadows",		"widget_relate_shadows.png", "http://www.shadows.com/bookmark/saveLink.rails?page=", "&title="),
//        Array ("Shoutwire",		"widget_relate_shoutwire.png", NULL, NULL),
        Array ("Simpy",			"widget_relate_simpy.png", "http://simpy.com/simpy/LinkAdd.do?href=", "&tags=&note=&title="),
//        Array ("Simpy",		"widget_relate_simpy.png", "http://simpy.com/simpy/LinkAdd.do?href=", "&title="),
        Array ("Slashdot",		"widget_relate_slashdot.png", "http://slashdot.org/bookmark.pl?url=", "&title="),
        Array ("Smarking",		"widget_relate_smarking.png", "http://smarking.com/editbookmark/?url=", "&tags=&description="),
//        Array ("Spurl",		"widget_relate_spurl.png", "http://www.spurl.net/spurl.php?url=", "&title="),
        Array ("Spurl",			"widget_relate_spurl.png", "http://www.spurl.net/spurl.php?v=3&tags=&url=", "&title="),
//        Array ("Spurl",		"widget_relate_.png", "http://www.spurl.net/spurl.php?v=3&url=", "&title="),
//        Array ("Squidoo",		"widget_relate_squidoo.png", NULL, NULL),
        Array ("StumbleUpon",		"widget_relate_stumbleupon.png", "http://www.stumbleupon.com/submit?url=", "&title="),
//        Array ("Tabmarks",		"widget_relate_tabmarks.png", NULL, NULL),
//        Array ("Taggle",		"widget_relate_taggle.png", NULL, NULL),
//        Array ("Tag Hop",		"widget_relate_taghop.png", NULL, NULL),
//        Array ("Taggly",		"widget_relate_taggly.png", NULL, NULL),
//        Array ("Tagtooga",		"widget_relate_tagtooga.png", NULL, NULL),
//        Array ("TailRank",		"widget_relate_tailrank.png", NULL, NULL),
        Array ("Technorati",		"widget_relate_technorati.png", "http://technorati.com/faves?tag=&add=", NULL),
//        Array ("Technorati",		"widget_relate_technorati.png", "http://technorati.com/faves?add=", "&title="),
//        Array ("Tutorialism",		"widget_relate_tutorialism.png", NULL, NULL),
//        Array ("Unalog",		"widget_relate_unalog.png", NULL, NULL),
//        Array ("Wapher",		"widget_relate_wapher.png", NULL, NULL),
        Array ("Webnews",		"widget_relate_webnews.png", "http://www.webnews.de/einstellen?url=", "&title="),
//        Array ("Whitesoap",		"widget_relate_whitesoap.png", NULL, NULL),
//        Array ("Wink",		"widget_relate_wink.png", NULL, NULL),
//        Array ("WireFan",		"widget_relate_wirefan.png", NULL, NULL),
        Array ("Wists",			"widget_relate_wists.png", "http://wists.com/r.php?c=&r=", "&title="),
//        Array ("Wists",		"widget_relate_wists.png", "http://www.wists.com/?action=add&url=", "&title="),
        Array ("Yahoo",			"widget_relate_yahoo.png", "http://myweb2.search.yahoo.com/myresults/bookmarklet?u=", "&d=&tag=&t="),
//        Array ("Yahoo",		"widget_relate_yahoo.png", "http://myweb2.search.yahoo.com/myresults/bookmarklet?u=", "&t="),
//        Array ("Yahoo",		"widget_relate_yahoo.png", "http://myweb.yahoo.com/myresults/bookmarklet?u=", "&t="),
        Array ("Yigg",			"widget_relate_yigg.png", "http://yigg.de/neu?exturl=", NULL),
//        Array ("Zumaa",		"widget_relate_zumaa.png", NULL, NULL),
//        Array ("Zurpy",		"widget_relate_zurpy.png", NULL, NULL),
      );

      $i_max = sizeof ($a);
      for ($i = 0; $i < $i_max; $i++)
        $p .= "<a class=\"widget_relate_img\" href=\""
             .$a[$i][2]
             .urlencode ($url)
             .($a[$i][3] ? $a[$i][3].urlencode ($title) : "")
             ."\" alt=\"Add to "
             .$a[$i][0]
             ."\" title=\"Add to "
             .$a[$i][0]
             ."\">"
             ."<img src=\"images/"
             .$a[$i][1]
             ."\" border=\"0\"></a>\n";
    }

  $p .= "</font>";
//  $p .= "</td></tr></table>\n";

  return $p; 
}


function
widget_pos_start ($x, $y)
{
}


function
widget_pos_end ()
{
}


function
widget_hidden_previous_request ()
{
// inserts all names and values of the previous request
// that are not used in this form as hidden names and values
}


function
widget_maps ()
{
// shows google maps by ip(geoip?), country, city, or long/lat
}


function
widget_wizard ($xml_file)
{
  $xml = simplexml_load_file ($xml_file);

  $p = "";
  for ($i = 0; $xml->widgets->widget[$i]; $i++)
    {
      $widget = $xml->widgets->widget[$i];

      if ($this->flags & WIDGET_DEBUG)
        {
          $p .= "<hr><pre><tt>"
               ."widget_"
               .$widget->type
               ."():\n\n"
               .sprint_r ($widget)
               ."</tt></pre>";
        }

      switch ($widget->type)
        {
          case "a":
            $p .= $this->widget_a ($widget->url, $widget->target, $widget->img, $widget->w, $widget->h, $widget->label, $widget->tooltip, $widget->flags);
            break;

          case "checkbox":
            $p .= $this->widget_checkbox ($widget->name, $widget->tooltip, $widget->flags);
            break;

          case "image":
            $p .= $this->widget_image ($widget->name, $widget->value, $widget->img, $widget->w, $widget->h, $widget->tooltip, $widget->flags);
            break;

          case "img":
            $p .= $this->widget_img ($widget->name, $widget->img, $widget->w, $widget->h, $widget->border, $widget->alt, $widget->tooltip, $widget->flags);
            break;

          case "password":
            $p .= $this->widget_password ($widget->name, $widget->tooltip, $widget->flags);
            break;

          case "radio":
            $p .= $this->widget_radio ($widget->name, $widget->values->value, $widget->labels->label, $widget->tooltip, $widget->vertical, $widget->flags);
            break;

          case "reset":
            $p .= $this->widget_reset ($widget->name, $widget->label, $widget->tooltip, $widget->flags);
            break;

          case "select":
            $p .= $this->widget_select ($widget->img, $widget->name, $widget->images->image, $widget->labels->label, $widget->values->value, $widget->tooltip, $widget->flags);
            break;

          case "submit":
            $p .= $this->widget_submit ($widget->name, $widget->label, $widget->tooltip, $widget->flags);
            break;

          case "textarea":
            $p .= $this->widget_textarea ($widget->name, $widget->value, $widget->tooltip, $widget->cols, $widget->rows, $widget->flags);
            break;

          case "trans":
            $p .= $this->widget_trans ($widget->w, $widget->h, $widget->flags);
            break;

          case "text":
            $p .= $this->widget_text ($widget->name, $widget->value, $widget->tooltip, $widget->size, $widget->maxlength, $widget->flags);
            break;

          default:
            break;
        }
    }

  return $p;
}


};

}

?>