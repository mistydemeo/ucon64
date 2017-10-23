<?php
//header ("Content-type: image/png");
/*
draw.php - simple wrapper for drawing
  
Copyright (c) 2007 NoisyB
                            
                            
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
  
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
  
You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
if (!defined ("MISC_DRAW_PHP"))
{
define ("MISC_DRAW_PHP", 1);  


/*
  draw_open()
  draw_close()

  draw_read()           read from display to work surface
  draw_read_from_mem()  read picture from memory to work surface
  draw_read_from_file() read picture from a file to work surface
                              uses cache for speed

  draw_get_w()          return current width of work surface (NOT the selection)
  draw_get_h()          return current height of work surface (NOT the selection)
  draw_get_html_color() return the color of a pixel of the work surface
                          as html string

  draw_ctrl_select()    set selection on work surface 
  draw_ctrl_select_all() select the whole work surface (default)

  draw_ctrl_copy()      copy selection to copy surface with id
  draw_ctrl_paste()     paste copy surface with id to work surface
  draw_ctrl_resize()    resize the selection
  draw_ctrl_canvas()    canvas the work surface
                          work with positive and negative values for x, y, w, h
  draw_ctrl_scale()     scale the selection
  draw_ctrl_flip()      flip selection; Axis: 0 == x, 1 == y, 2 == both
  draw_ctrl_rotate()    rotate selection
  draw_ctrl_invert()    invert colors of selection

  draw_write()          write work surface to display
  draw_write_in_tiles() tiles the picture into 16*16(=256) rectangles
                          the "drawing" is done by the c argument
                          (could be used for fonts)
  draw_write_to_mem()   write work surface to mem
  draw_write_to_file()  write work surface to file (png)
  draw_write_to_SDL()   return work surface as SDL_Surface
  draw_write_pixel()    write pixel to display
  draw_write_rect()     write rectangle to display
  draw_write_box()      write box to display
  draw_write_ellipse()  write filled ellipse to display

  draw_sync()           update screen after one or more draw_write()
*/


class misc_draw
{
var $work = NULL; // work surface
var $s = NULL;    // draw/sync surface 
var $w = NULL;    // draw/sync surface width
var $h = NULL;    // draw/sync surface height
var $background = NULL;
var $antialias = 1;


function
draw_s_to_color ($html_color)
{
  $col = hexdec ($html_color);

  return imagecolorallocatealpha ($this->s,
                                  ($col & 0xff0000) >> 16,
                                  ($col & 0xff00) >> 8,
                                  ($col & 0xff), 0);
}


function
draw_open ($w, $h)
{
  $this->w = $w;
  $this->h = $h;
  $this->s = imagecreatetruecolor ($w, $h) or die ("draw_open() failed");

  imagealphablending($this->s, TRUE);

  // transparent background
//  $this->background = $this->draw_s_to_color ("#ffffff");
//  $this->background = imagecolorallocatealpha ($this->s, 0xff, 0xff, 0xff, 127);
//  imagefilledrectangle ($this->s, 0, 0, $w, $h, $this->background);
}


function
draw_close ()
{
  if ($this->work)
    imagedestroy ($this->work);
//  if ($this->s)
//    imagedestroy ($this->s);
}


// read to work surface
/*
function
draw_read ()
{
}


function
draw_read_from_mem (const unsigned char *data, int data_len)
{
}
*/


function
draw_read_from_file ($fname)
{
  if ($this->work)
    imagedestroy ($this->work);

  if (get_suffix ($fname) == ".jpg")
    {
      $tmp = imagecreatefromjpeg ($fname);
    }
  else if (get_suffix ($fname) == ".png")
    {
      $tmp = imagecreatefrompng ($fname);
      imagealphablending($tmp, TRUE);
    }
  else exit;

  if (!imageistruecolor ($tmp))
    {
      $w = imagesx ($tmp);
      $h = imagesy ($tmp);

      $this->work = imagecreatetruecolor ($w, $h); // turn it into truecolor

      imagealphablending($this->work, TRUE);

      imagecopy ($this->work, $tmp, 0, 0, 0, 0, $w, $h);
    }
  else $this->work = $tmp;
}


// work surface stats
function
draw_get_w ()
{
  return imagesx ($this->work);
}


function
draw_get_h ()
{
  return imagesy ($this->work);
}


function
draw_get_html_color ($x, $y)
{
  return sprintf ("#%06x", imagecolorat ($this->work, $x, $y));
}


/*
// select work area (default: draw_ctrl_select_all())
function
draw_ctrl_select (int x, int y, int w, int h)
{
}


function
draw_ctrl_select_all ()
{
}


//function
draw_ctrl_select_invert ()
{
}


// apply functions to selection
function
draw_ctrl_copy (int id)
{
}


function
draw_ctrl_paste (int id, int x, int y)
{
}
*/


function
draw_ctrl_resize ($w_new, $h_new, $quality)
{
  if (imagesx ($this->work) == $w_new &&
      imagesy ($this->work) == $h_new)
    return;

  $tmp = imagecreatetruecolor ($w_new, $h_new);
  imagealphablending($tmp, FALSE);
  imagesavealpha($tmp, TRUE);
  if (!($quality))
    imagecopyresized ($tmp, $this->work, 0, 0, 0, 0, $w_new, $h_new, imagesx ($this->work), imagesy ($this->work));
  else
    imagecopyresampled ($tmp, $this->work, 0, 0, 0, 0, $w_new, $h_new, imagesx ($this->work), imagesy ($this->work));

  imagedestroy ($this->work);

  $this->work = $tmp;
}


function
draw_ctrl_scale ($percent, $quality)
{
  draw_ctrl_resize ((int) imagesx ($this->work) * $percent, (int) imagesy ($this->work) * $percent, $quality);
}


/*
function
draw_ctrl_flip (int axis)
{
}
*/


function
draw_ctrl_rotate ($ankle)
{
  imagerotate ($this->work, $angle, $this->draw_s_to_color ("#000000"));
}


/*
function
draw_ctrl_invert ()
{
}


function
draw_ctrl_brightness (float percent_rel)
{
}
*/


function
draw_ctrl_greyscale ()
{
  $w = imagesx ($this->work);
  $h = imagesy ($this->work);

  for ($x = 0; $x < $w; $x++)
    for ($y = 0; $y < $h; $y++)
    {
      $color = imagecolorat($this->work, $x, $y);
      $grey = (int)(($color[red] + $color[green] + $color[blue]) / 3);
      ImageColorSet($this->work, $x + $y, $grey, $grey, $grey);
    }
/*
Method #2
This little function does it for you:

$image_id = imageCreateFromJPEG($image);
for($a=0;$a<imagecolorstotal ($image_id);$a++)
{
$color = ImageColorsForIndex($image_id,$i);
$R=.299 * ($color['red'])+ .587 * ($color['green'])+ .114 * ($color['blue']);
$G=.299 * ($color['red'])+ .587 * ($color['green'])+ .114 * ($color['blue']);
$B=.299 * ($color['red'])+ .587 * ($color['green'])+ .114 * ($color['blue']);
ImageColorSet($image_id, $a, $R, $G, $B);
}
imageJPEG($image_id,"$image");
*/
}


/*
function
draw_ctrl_canvas (int x_rel, int y_rel, int w_rel, int h_rel)
{
}


function
draw_ctrl_shift (int pixels_rel)
{
}


function
draw_ctrl_indexed (int colors)
{
}
*/


// write from work surface to display
function
draw_write ($x, $y)
{
  if (!$this->work)
    return;

  if (!$this->s)
    {
      $this->s = imagecreatetruecolor ($this->w, $this->h);
      imagealphablending($this->s, TRUE);
    }

  imagecopy ($this->s, $this->work, $x, $y, 0, 0, min (imagesx ($this->s), imagesx ($this->work)),
                                                  min (imagesy ($this->s), imagesy ($this->work)));
}


function
draw_write_in_tiles ($x, $y, $tiles)
{
  if (!$this->work)
    return;

  $w = imagesx ($this->work) / 16;
  $h = imagesy ($this->work) / 16;

  $i_max = strlen ($tiles);
  for ($i = 0; $i < $i_max; $i++)
    {
      $value = 0 + ord ($tiles[$i]);
      ImageCopy ($this->s, $this->work, $x + $i * $w, $y, $w * ($value % 16), $h * (int)($value / 16), $w, $h);
    } 
}

/*
function
draw_write_to_mem (char *data, int *data_len)
{
}
*/

function
draw_write_to_file ($fname)
{
  imagealphablending($this->s,FALSE);
  imagesavealpha($this->s,TRUE);
  imagepng ($this->s, $fname);
}

// write primitives directly to display
function
draw_write_pixel ($x, $y, $html_color)
{
  imagesetpixel ($this->s, $x, $y, $this->draw_s_to_color ($html_color));
}


function
draw_write_rect ($x, $y, $w, $h, $html_color)
{
  imagerectangle ($this->s, $x, $y, $x + $w, $y + $h, $this->draw_s_to_color ($html_color));
}


function
draw_write_box ($x, $y, $w, $h, $html_color)
{
  imagefilledrectangle ($this->s, $x, $y, $x + $w, $y + $h, $this->draw_s_to_color ($html_color));
}


function
draw_write_ellipse ($x, $y, $w, $h, $html_color)
{
  imageellipse ($this->s, $x, $y, $x + $w, $y + $h, $this->draw_s_to_color ($html_color));
}


// sync display
function
draw_sync_png ()
{
//  imagetruecolortopalette ($this->s, 1, 256); // 256 is alright
  imagealphablending($this->s,FALSE);
  imagesavealpha($this->s,TRUE);
  imagepng ($this->s);
}


function
draw_sync_jpg ()
{
//  imagetruecolortopalette ($this->work, 1, 256); // 256 is alright
//  imagealphablending($this->s,FALSE);
//  imagesavealpha($this->s,TRUE);
  imagejpeg ($this->s);
}


};
}

?>