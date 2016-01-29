//
// "$Id$"
//
// implementation of class Fl_Gl_Device_Plugin for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2014 by Bill Spitzak and others.
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


// FIXME: fill with code!
#if 0

#include <config.h>
#include "config_lib.h"
#include <FL/Fl_Printer.H>
#include <FL/Fl_Gl_Window.H>
#include "Fl_Gl_Choice.H"
#include <FL/Fl_RGB_Image.H>
#include "FL/Fl.H"

#if defined(WIN32) || defined(__APPLE__)
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement code to read OpenGL renderings into RGB maps"
#else
#endif


// ------ this should be in a separate file! -----------------------------------
#ifdef FL_CFG_GFX_OPENGL

#include <FL/Fl_Device.H>
#include <FL/gl.h>

#include "drivers/OpenGL/Fl_OpenGL_Graphics_Driver.h"

Fl_OpenGL_Display_Device *Fl_OpenGL_Display_Device::display_device() {
  static Fl_OpenGL_Display_Device *display = new Fl_OpenGL_Display_Device(new Fl_OpenGL_Graphics_Driver());
  return display;
};

Fl_OpenGL_Display_Device::Fl_OpenGL_Display_Device(Fl_OpenGL_Graphics_Driver *graphics_driver)
: Fl_Surface_Device(graphics_driver)
{
}

const char *Fl_OpenGL_Display_Device::class_id = "Fl_OpenGL_Display_Device";

#endif
// ------ end of separate file! ------------------------------------------------

#include "drivers/OpenGL/Fl_OpenGL_Graphics_Driver_arci.cxx"
#include "drivers/OpenGL/Fl_OpenGL_Graphics_Driver_color.cxx"
#include "drivers/OpenGL/Fl_OpenGL_Graphics_Driver_font.cxx"
#include "drivers/OpenGL/Fl_OpenGL_Graphics_Driver_image.cxx"
#include "drivers/OpenGL/Fl_OpenGL_Graphics_Driver_line_style.cxx"
#include "drivers/OpenGL/Fl_OpenGL_Graphics_Driver_rect.cxx"
#include "drivers/OpenGL/Fl_OpenGL_Graphics_Driver_vertex.cxx"


#if defined(__APPLE__)
uchar *convert_BGRA_to_RGB(uchar *baseAddress, int w, int h, int mByteWidth)
{
  uchar *newimg = new uchar[3*w*h];
  uchar *to = newimg;
  for (int i = 0; i < h; i++) {
    uchar *from = baseAddress + i * mByteWidth;
    for (int j = 0; j < w; j++, from += 4) {
#if __ppc__
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
#endif

static Fl_RGB_Image* capture_gl_rectangle(Fl_Gl_Window *glw, int x, int y, int w, int h)
/* captures a rectangle of a Fl_Gl_Window window, and returns it as a RGB image
 stored from bottom to top.
 */
{
#if defined(__APPLE__)
  const int bytesperpixel = 4;
  int factor = glw->pixels_per_unit();
  if (factor > 1) {
    w *= factor; h *= factor; x *= factor; y *= factor;
  }
#else
  const int bytesperpixel = 3;
#endif
  glw->flush(); // forces a GL redraw, necessary for the glpuzzle demo
  // Read OpenGL context pixels directly.
  // For extra safety, save & restore OpenGL states that are changed
  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
  glPixelStorei(GL_PACK_ALIGNMENT, 4); /* Force 4-byte alignment */
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
  // Read a block of pixels from the frame buffer
  int mByteWidth = w * bytesperpixel;
  mByteWidth = (mByteWidth + 3) & ~3;    // Align to 4 bytes
  uchar *baseAddress = new uchar[mByteWidth * h];
  glReadPixels(x, glw->pixel_h() - (y+h), w, h,
#if defined(__APPLE__)
               GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
#else
               GL_RGB, GL_UNSIGNED_BYTE,
#endif
               baseAddress);
  glPopClientAttrib();
#if defined(__APPLE__)
  baseAddress = convert_BGRA_to_RGB(baseAddress, w, h, mByteWidth);
  mByteWidth = 3 * w;
#endif
  Fl_RGB_Image *img = new Fl_RGB_Image(baseAddress, w, h, 3, mByteWidth);
  img->alloc_array = 1;
  return img;
}

#ifdef __APPLE__
static void imgProviderReleaseData (void *info, const void *data, size_t size)
{
  delete (Fl_RGB_Image *)info;
}
#endif

/**
 This class will make sure that OpenGL printing/screen capture is available if fltk_gl
 was linked to the program
 */
class Fl_Gl_Device_Plugin : public Fl_Device_Plugin {
public:
  Fl_Gl_Device_Plugin() : Fl_Device_Plugin(name()) { }
  virtual const char *name() { return "opengl.device.fltk.org"; }
  virtual int print(Fl_Widget *w, int x, int y, int height /*useless*/) {
    Fl_Gl_Window *glw = w->as_gl_window();
    if (!glw) return 0;
    Fl_RGB_Image *img = capture_gl_rectangle(glw, 0, 0, glw->w(), glw->h());
#ifdef __APPLE__
    if (Fl_Surface_Device::surface()->driver()->has_feature(Fl_Graphics_Driver::NATIVE)) {
      // convert the image to CGImage, and draw it at full res (useful on retina display)
      CGColorSpaceRef cSpace = CGColorSpaceCreateDeviceRGB();
      CGDataProviderRef provider = CGDataProviderCreateWithData(img, img->array, img->ld() * img->h(), imgProviderReleaseData);
      CGImageRef cgimg = CGImageCreate(img->w(), img->h(), 8, 24, img->ld(), cSpace,
                                     (CGBitmapInfo)(kCGImageAlphaNone),
                                     provider, NULL, false, kCGRenderingIntentDefault);
      CGColorSpaceRelease(cSpace);
      CGDataProviderRelease(provider);
      CGContextDrawImage(fl_gc, CGRectMake(0, 0, glw->w(), glw->h()), cgimg);
      CFRelease(cgimg);
      return 1;
    } else if (img->w() > glw->w()) {
      Fl_RGB_Image *img2 = (Fl_RGB_Image*)img->copy(glw->w(), glw->h());
      delete img;
      img = img2;
    }
#endif
    int ld = img->ld() ? img->ld() : img->w() * img->d();
    fl_draw_image(img->array + (img->h() - 1) * ld, x, y , img->w(), img->h(), 3, - ld);
    delete img;
    return 1;
  }
  virtual Fl_RGB_Image* rectangle_capture(Fl_Widget *widget, int x, int y, int w, int h) {
    Fl_Gl_Window *glw = widget->as_gl_window();
    if (!glw) return NULL;
    return capture_gl_rectangle(glw, x, y, w, h);
  }
};

static Fl_Gl_Device_Plugin Gl_Device_Plugin;

// The purpose of this variable, used in Fl_Gl_Window.cxx, is only to force this file to be loaded
// whenever Fl_Gl_Window.cxx is loaded, that is, whenever fltk_gl is.
FL_EXPORT int fl_gl_load_plugin = 0;

#endif

//
// End of "$Id$".
//
