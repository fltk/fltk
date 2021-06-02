//
// implementation of class Fl_OpenGL_Display_Device for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2017 by Bill Spitzak and others.
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

#include <config.h>
#include <FL/Fl_Gl_Window.H>
#include "../../Fl_Gl_Window_Driver.H"
#include <FL/Fl_Image.H>
#include "../../Fl_Screen_Driver.H"
#include "../../Fl_Window_Driver.H"
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

Fl_RGB_Image* Fl_OpenGL_Display_Device::capture_gl_rectangle(Fl_Gl_Window* glw, int x, int y, int w, int h)
{
  return Fl_Gl_Window_Driver::driver(glw)->capture_gl_rectangle(x, y, w, h);
}

/* Captures a rectangle of a Fl_Gl_Window and returns it as a RGB image.
 This is the platform-independent version. Some platforms may re-implement it.
 */
Fl_RGB_Image* Fl_Gl_Window_Driver::capture_gl_rectangle(int x, int y, int w, int h)
{
  Fl_Gl_Window *glw = pWindow;
  glw->flush(); // forces a GL redraw, necessary for the glpuzzle demo
  // Read OpenGL context pixels directly.
  // For extra safety, save & restore OpenGL states that are changed
  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
  glPixelStorei(GL_PACK_ALIGNMENT, 4); /* Force 4-byte alignment */
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
  //
  float s = glw->pixels_per_unit();
  if (s != 1) {
    x = int(x * s); y = int(y * s); w = int(w * s); h = int(h * s);
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
