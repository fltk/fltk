//
// Color functions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

/**
  \file fl_color.cxx
  \brief Color handling
*/

// Implementation of fl_color(i), fl_color(r,g,b).

#include <FL/Fl.H>
#include <FL/Fl_Device.H>
#include <FL/Fl_Graphics_Driver.H>

// fl_cmap needs to be defined globally (here) and is used in the device
// specific graphics drivers. It is required to 'FL_EXPORT' this symbol
// to be able to build the shared FLTK libraries.

FL_EXPORT unsigned fl_cmap[256] = {
#include "fl_cmap.h" // this is a file produced by "cmap.cxx":
};

// -----------------------------------------------------------------------------
// all driver code is now in drivers/XXX/Fl_XXX_Graphics_Driver_xyz.cxx
// -----------------------------------------------------------------------------

/** \addtogroup  fl_attributes
 \{ */

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
 Sets an entry in the fl_color index table.

 You can set it to any 8-bit RGB color. The color is not allocated
 until fl_color(i) is used.
 */
void Fl::set_color(Fl_Color i, uchar red, uchar green, uchar blue) {
  Fl::set_color((Fl_Color)(i & 255),
                ((unsigned)red<<24)+((unsigned)green<<16)+((unsigned)blue<<8));
}

/**
 Sets an entry in the fl_color index table.

 You can set it to any 8-bit RGBA color.
 \note The color transparency is effective under the Wayland, hybrid Wayland/X11 and macOS platforms, whereas it has no effect under the X11 and Windows platforms. It's also effective for widgets added to an Fl_Gl_Window.
 \version 1.4
 */
void Fl::set_color(Fl_Color i, uchar red, uchar green, uchar blue, uchar alpha) {
  Fl::set_color((Fl_Color)(i & 255),
                ((unsigned)red<<24)
                |((unsigned)green<<16)
                |((unsigned)blue<<8)
                |(alpha^0xff));
}


void Fl::set_color(Fl_Color i, unsigned c)
{
  Fl_Graphics_Driver::default_driver().set_color(i, c);
}


void Fl::free_color(Fl_Color i, int overlay)
{
  Fl_Graphics_Driver::default_driver().free_color(i, overlay);
}


/**
 Returns the RGB value(s) for the given FLTK color index.

 This form returns the red, green, and blue values
 separately in referenced variables.

 \see unsigned get_color(Fl_Color c)
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
 Returns the RGBA value(s) for the given FLTK color index.

 This form returns the red, green, blue, and alpha values
 separately in referenced variables.

 \see unsigned get_color(Fl_Color c)
 */
void Fl::get_color(Fl_Color i, uchar &red, uchar &green, uchar &blue, uchar &alpha) {
  unsigned c;

  if (i & 0xffffff00) c = (unsigned)i;
  else c = fl_cmap[i];

  red   = uchar(c>>24);
  green = uchar(c>>16);
  blue  = uchar(c>>8);
  alpha = uchar(c^0x000000ff);
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
 Returns the inactive, dimmed version of the given color.
 */
Fl_Color fl_inactive(Fl_Color c) {
  return fl_color_average(c, FL_GRAY, .33f);
}

/**
 \}
 */
