//
// "$Id$"
//
// implementation of class Fl_Gl_Device_Plugin for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2017 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#include "../../config_lib.h"
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Image.H>
#include <FL/gl.h>
#include <string.h>

#include "Fl_OpenGL_Graphics_Driver.H"
#include "Fl_OpenGL_Display_Device.H"

// TODO: much of Fl_Gl_Choice should probably go here

Fl_OpenGL_Display_Device *Fl_OpenGL_Display_Device::display_device() {
  static Fl_OpenGL_Display_Device *display = new Fl_OpenGL_Display_Device(new Fl_OpenGL_Graphics_Driver());
  return display;
};

Fl_OpenGL_Display_Device::Fl_OpenGL_Display_Device(Fl_OpenGL_Graphics_Driver *graphics_driver)
: Fl_Surface_Device(graphics_driver)
{
}

#ifdef FL_CFG_GFX_QUARTZ
#include <FL/Fl_Gl_Window_Driver.H>

// convert BGRA to RGB and also exchange top and bottom
static uchar *convert_BGRA_to_RGB(uchar *baseAddress, int w, int h, int mByteWidth)
{
  uchar *newimg = new uchar[3*w*h];
  uchar *to = newimg;
  for (int i = h-1; i >= 0; i--) {
    uchar *from = baseAddress + i * mByteWidth;
    for (int j = 0; j < w; j++, from += 4) {
#if defined(__ppc__) && __ppc__
      memcpy(to, from + 1, 3);
      to += 3;
#else
      *(to++) = *(from+2);
      *(to++) = *(from+1);
      *(to++) = *from;
#endif
    }
  }
  delete[] baseAddress;
  return newimg;
}

Fl_RGB_Image* Fl_OpenGL_Display_Device::capture_gl_rectangle(Fl_Gl_Window* glw, int x, int y, int w, int h)
{
  float factor = glw->pixels_per_unit();
  if (factor != 1) {
    w *= factor; h *= factor; x *= factor; y *= factor;
  }
  Fl_Cocoa_Gl_Window_Driver *driver = (Fl_Cocoa_Gl_Window_Driver*)glw->gl_driver();
  driver->flush_context(); // to capture also the overlay and for directGL demo
  // Read OpenGL context pixels directly.
  // For extra safety, save & restore OpenGL states that are changed
  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
  glPixelStorei(GL_PACK_ALIGNMENT, 4); /* Force 4-byte alignment */
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
  // Read a block of pixels from the frame buffer
  int mByteWidth = w * 4;
  mByteWidth = (mByteWidth + 3) & ~3;    // Align to 4 bytes
  uchar *baseAddress = new uchar[mByteWidth * h];
  glReadPixels(x, glw->pixel_h() - (y+h), w, h,
               GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, baseAddress);
  glPopClientAttrib();
  baseAddress = convert_BGRA_to_RGB(baseAddress, w, h, mByteWidth);
  Fl_RGB_Image *img = new Fl_RGB_Image(baseAddress, w, h, 3, 3 * w);
  img->alloc_array = 1;
  driver->flush_context();
  return img;
}

#else

#include "../../Fl_Screen_Driver.H"
#include "../../Fl_Window_Driver.H"
Fl_RGB_Image* Fl_OpenGL_Display_Device::capture_gl_rectangle(Fl_Gl_Window *glw, int x, int y, int w, int h)
/* captures a rectangle of a Fl_Gl_Window window, and returns it as a RGB image
 */
{
  glw->flush(); // forces a GL redraw, necessary for the glpuzzle demo
  // Read OpenGL context pixels directly.
  // For extra safety, save & restore OpenGL states that are changed
  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
  glPixelStorei(GL_PACK_ALIGNMENT, 4); /* Force 4-byte alignment */
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
  //
  int ns = Fl_Window_Driver::driver(glw)->screen_num();
  float s = Fl::screen_driver()->scale(ns);
  if (s != 1) {
    x *= s; y *= s; w *= s; h *= s;
  }
  // Read a block of pixels from the frame buffer
  int mByteWidth = w * 3;
  mByteWidth = (mByteWidth + 3) & ~3;    // Align to 4 bytes
  uchar *baseAddress = new uchar[mByteWidth * h];
  glReadPixels(x, glw->pixel_h() - (y+h), w, h,
               GL_RGB, GL_UNSIGNED_BYTE,
               baseAddress);
  glPopClientAttrib();
  // GL gives a bottom-to-top image, convert it to top-to-bottom
  uchar *tmp = new uchar[mByteWidth];
  uchar *p = baseAddress ;
  uchar *q = baseAddress + (h-1)*mByteWidth;
  for (int i = 0; i < h/2; i++, p += mByteWidth, q -= mByteWidth) {
    memcpy(tmp, p, mByteWidth);
    memcpy(p, q, mByteWidth);
    memcpy(q, tmp, mByteWidth);
  }
  delete[] tmp;
  
  Fl_RGB_Image *img = new Fl_RGB_Image(baseAddress, w, h, 3, mByteWidth);
  img->alloc_array = 1;
  return img;
}

#endif
//
// End of "$Id$".
//
