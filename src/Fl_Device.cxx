//
// "$Id$"
//
// implementation of Fl_Device class for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include <FL/Fl_Device.H>
//#include <FL/fl_draw.H>
#include <FL/Fl_Image.H>

/** \brief Draws an Fl_Pixmap object to the device. 
 *
 Specifies a bounding box for the image, with the origin (upper left-hand corner) of 
 the image offset by the cx and cy arguments.
 */
void Fl_Device::draw(Fl_Pixmap *pxm,int XP, int YP, int WP, int HP, int cx, int cy)
{
  pxm->draw(XP, YP, WP, HP, cx, cy);
}

/** \brief Draws an Fl_Bitmap object to the device. 
 *
 Specifies a bounding box for the image, with the origin (upper left-hand corner) of 
 the image offset by the cx and cy arguments.
 */
void Fl_Device::draw(Fl_Bitmap *bm,int XP, int YP, int WP, int HP, int cx, int cy)
{
  bm->draw(XP, YP, WP, HP, cx, cy);
}

/** \brief Draws an Fl_RGB_Image object to the device. 
 *
 Specifies a bounding box for the image, with the origin (upper left-hand corner) of 
 the image offset by the cx and cy arguments.
 */
void Fl_Device::draw(Fl_RGB_Image *rgb,int XP, int YP, int WP, int HP, int cx, int cy)
{
  rgb->draw(XP, YP, WP, HP, cx, cy);
}

/**
 @brief Sets this device (display, printer, local file) as the target of future graphics calls.
 *
 @return  The current target device of graphics calls.
 */
Fl_Device *Fl_Device::set_current(void)
{
  Fl_Device *current = fl_device;
  fl_device = this;
  return current;
}

/**
 @brief    Returns the current target device of graphics calls.
 */
Fl_Device *Fl_Device::current(void)
{
  return fl_device;
}

//
// End of "$Id$".
//
