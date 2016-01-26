//
// "$Id$"
//
// Color functions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

/**
  \file fl_color.cxx
  \brief Color handling
*/

// Implementation of fl_color(i), fl_color(r,g,b).

#include <config.h>
#include "config_lib.h"


// -----------------------------------------------------------------------------


#ifdef FL_CFG_GFX_QUARTZ

# include "cfg_gfx/quartz_color.cxx"

#endif


// -----------------------------------------------------------------------------


#ifdef FL_CFG_GFX_GDI

# include "cfg_gfx/gdi_color.cxx"

#endif


// -----------------------------------------------------------------------------


#ifdef FL_CFG_GFX_XLIB

# include "drivers/Xlib/Fl_Xlib_Graphics_Driver_color.cxx"

#endif


// -----------------------------------------------------------------------------

/** \addtogroup  fl_attributes
 @{ */

/**
 Returns the RGB value(s) for the given FLTK color index.

 This form returns the RGB values packed in a 32-bit unsigned
 integer with the red value in the upper 8 bits, the green value
 in the next 8 bits, and the blue value in bits 8-15.  The lower
 8 bits will always be 0.
 */
unsigned Fl::get_color(Fl_Color i) {
  if (i & 0xffffff00) return (i);
  else return fl_cmap[i];
}
/**
 Sets an entry in the fl_color index table.  You can set it to
 any 8-bit RGB color.  The color is not allocated until fl_color(i)
 is used.
 */
void Fl::set_color(Fl_Color i, uchar red, uchar green, uchar blue) {
  Fl::set_color((Fl_Color)(i & 255),
                ((unsigned)red<<24)+((unsigned)green<<16)+((unsigned)blue<<8));
}
/**
 Returns the RGB value(s) for the given FLTK color index.

 This form returns the red, green, and blue values
 separately in referenced variables.

 See also unsigned get_color(Fl_Color c)
 */
void Fl::get_color(Fl_Color i, uchar &red, uchar &green, uchar &blue) {
  unsigned c;

  if (i & 0xffffff00) c = (unsigned)i;
  else c = fl_cmap[i];

  red   = uchar(c>>24);
  green = uchar(c>>16);
  blue  = uchar(c>>8);
}

/**
 Returns the weighted average color between the two given colors.
 The red, green and blue values are averages using the following formula:
 \code
 color = color1 * weight  + color2 * (1 - weight)
 \endcode
 Thus, a \p weight value of 1.0 will return the first color, while a
 value of 0.0 will return the second color.
 \param[in] color1, color2 boundary colors
 \param[in] weight weighting factor
 */
Fl_Color fl_color_average(Fl_Color color1, Fl_Color color2, float weight) {
  unsigned rgb1;
  unsigned rgb2;
  uchar r, g, b;

  if (color1 & 0xffffff00) rgb1 = color1;
  else rgb1 = fl_cmap[color1 & 255];

  if (color2 & 0xffffff00) rgb2 = color2;
  else rgb2 = fl_cmap[color2 & 255];

  r = (uchar)(((uchar)(rgb1>>24))*weight + ((uchar)(rgb2>>24))*(1-weight));
  g = (uchar)(((uchar)(rgb1>>16))*weight + ((uchar)(rgb2>>16))*(1-weight));
  b = (uchar)(((uchar)(rgb1>>8))*weight + ((uchar)(rgb2>>8))*(1-weight));

  return fl_rgb_color(r, g, b);
}

/**
 Returns the inactive, dimmed version of the given color
 */
Fl_Color fl_inactive(Fl_Color c) {
  return fl_color_average(c, FL_GRAY, .33f);
}

/**
 Returns a color that contrasts with the background color.
 This will be the foreground color if it contrasts sufficiently with the
 background color. Otherwise, returns \p FL_WHITE or \p FL_BLACK depending
 on which color provides the best contrast.
 \param[in] fg,bg foreground and background colors
 \return contrasting color
 */
Fl_Color fl_contrast(Fl_Color fg, Fl_Color bg) {
  unsigned c1, c2;	// RGB colors
  int l1, l2;		// Luminosities


  // Get the RGB values for each color...
  if (fg & 0xffffff00) c1 = (unsigned)fg;
  else c1 = fl_cmap[fg];

  if (bg & 0xffffff00) c2 = (unsigned)bg;
  else c2 = fl_cmap[bg];

  // Compute the luminosity...
  l1 = ((c1 >> 24) * 30 + ((c1 >> 16) & 255) * 59 + ((c1 >> 8) & 255) * 11) / 100;
  l2 = ((c2 >> 24) * 30 + ((c2 >> 16) & 255) * 59 + ((c2 >> 8) & 255) * 11) / 100;

  // Compare and return the contrasting color...
  if ((l1 - l2) > 99) return fg;
  else if ((l2 - l1) > 99) return fg;
  else if (l2 > 127) return FL_BLACK;
  else return FL_WHITE;
}
/**
 @}
 */

//
// End of "$Id$".
//
