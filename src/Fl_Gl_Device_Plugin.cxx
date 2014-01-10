//
// "$Id$"
//
// implementation of class Fl_Gl_Device_Plugin for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010 by Bill Spitzak and others.
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

#include <config.h>
#include <FL/Fl_Printer.H>
#include <FL/Fl_Gl_Window.H>
#include "Fl_Gl_Choice.H"
#include "FL/Fl.H"
#ifndef __APPLE__
#include "FL/fl_draw.H"
#endif

#if defined(__APPLE__)
static void imgProviderReleaseData (void *info, const void *data, size_t size)
{
  free((void *)data);
}
#endif

static void print_gl_window(Fl_Gl_Window *glw, int x, int y, int height)
{
#if defined(__APPLE__)
  const int bytesperpixel = 4;
#else
  const int bytesperpixel = 3;
#endif
  glw->flush(); // forces a GL redraw necessary for the glpuzzle demo
  // Read OpenGL context pixels directly.
  // For extra safety, save & restore OpenGL states that are changed
  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
  glPixelStorei(GL_PACK_ALIGNMENT, 4); /* Force 4-byte alignment */
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
  // Read a block of pixels from the frame buffer
  int mByteWidth = glw->w() * bytesperpixel;                
  mByteWidth = (mByteWidth + 3) & ~3;    // Align to 4 bytes
  uchar *baseAddress = (uchar*)malloc(mByteWidth * glw->h());
  glReadPixels(0, 0, glw->w(), glw->h(), 
#if defined(__APPLE__)
	       GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
#else
	       GL_RGB, GL_UNSIGNED_BYTE,
#endif
	       baseAddress);
  glPopClientAttrib();
#if defined(__APPLE__)
// kCGBitmapByteOrder32Host and CGBitmapInfo are supposed to arrive with 10.4
// but some 10.4 don't have kCGBitmapByteOrder32Host, so we play a little #define game
#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_4
#define kCGBitmapByteOrder32Host 0
#define CGBitmapInfo CGImageAlphaInfo
#elif ! defined(kCGBitmapByteOrder32Host)
#ifdef __BIG_ENDIAN__
#define kCGBitmapByteOrder32Host (4 << 12)
#else    /* Little endian. */
#define kCGBitmapByteOrder32Host (2 << 12)
#endif
#endif
  CGColorSpaceRef cSpace = CGColorSpaceCreateDeviceRGB();
  CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, baseAddress, mByteWidth * glw->h(), imgProviderReleaseData);
  CGImageRef image = CGImageCreate(glw->w(), glw->h(), 8, 8*bytesperpixel, mByteWidth, cSpace, 
				   (CGBitmapInfo)(kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Host), 
				   provider, NULL, false, kCGRenderingIntentDefault);
  if(image == NULL) return;
  CGContextSaveGState(fl_gc);
  CGContextTranslateCTM(fl_gc, 0, height);
  CGContextScaleCTM(fl_gc, 1.0f, -1.0f);
  CGRect rect = { { x, height - y - glw->h() }, { glw->w(), glw->h() } };
  Fl_X::q_begin_image(rect, 0, 0, glw->w(), glw->h());
  CGContextDrawImage(fl_gc, rect, image);
  Fl_X::q_end_image();
  CGContextRestoreGState(fl_gc);
  CGImageRelease(image);
  CGColorSpaceRelease(cSpace);
  CGDataProviderRelease(provider);  
#else
  fl_draw_image(baseAddress + (glw->h() - 1) * mByteWidth, x, y , glw->w(), glw->h(), bytesperpixel, - mByteWidth);
  free(baseAddress);
#endif // __APPLE__
}

/**
 This class will make sure that OpenGL printing is available if fltk_gl
 was linked to the program.
 */
class Fl_Gl_Device_Plugin : public Fl_Device_Plugin {
public:
  Fl_Gl_Device_Plugin() : Fl_Device_Plugin(name()) { }
  virtual const char *name() { return "opengl.device.fltk.org"; }
  virtual int print(Fl_Widget *w, int x, int y, int height) {
    Fl_Gl_Window *glw = w->as_gl_window();
    if (!glw) return 0;
    print_gl_window(glw, x, y, height);
    return 1; 
  }
};

static Fl_Gl_Device_Plugin Gl_Device_Plugin;

// The purpose of this variable, used in Fl_Gl_Window.cxx, is only to force this file to be loaded
// whenever Fl_Gl_Window.cxx is loaded, that is, whenever fltk_gl is.
FL_EXPORT int fl_gl_load_plugin = 0;

//
// End of "$Id$".
//
