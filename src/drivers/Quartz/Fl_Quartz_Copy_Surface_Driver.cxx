//
// "$Id$"
//
// Copy-to-clipboard code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2019 by Bill Spitzak and others.
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

#include "../../config_lib.h"

#ifdef FL_CFG_GFX_QUARTZ
#include <FL/Fl_Copy_Surface.H>
#include <FL/platform.H>
#include "Fl_Quartz_Graphics_Driver.H"
#include "Fl_Quartz_Copy_Surface_Driver.H"
#include "../Cocoa/Fl_Cocoa_Window_Driver.H"

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

Fl_Copy_Surface_Driver *Fl_Copy_Surface_Driver::newCopySurfaceDriver(int w, int h)
{
  return new Fl_Quartz_Copy_Surface_Driver(w, h);
}

/**
 \}
 \endcond
 */

Fl_Quartz_Copy_Surface_Driver::Fl_Quartz_Copy_Surface_Driver(int w, int h) : Fl_Copy_Surface_Driver(w, h) {
  driver(new Fl_Quartz_Printer_Graphics_Driver);
  pdfdata = CFDataCreateMutable(NULL, 0);
  CGDataConsumerRef myconsumer;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1040
  if (&CGDataConsumerCreateWithCFData != NULL) {
    myconsumer = CGDataConsumerCreateWithCFData(pdfdata); // 10.4
  }
  else
#endif
  {
    static CGDataConsumerCallbacks callbacks = { Fl_Quartz_Copy_Surface_Driver::MyPutBytes, NULL };
    myconsumer = CGDataConsumerCreate((void*) pdfdata, &callbacks);
  }
  CGRect bounds = CGRectMake(0, 0, w, h );
  gc = CGPDFContextCreate(myconsumer, &bounds, NULL);
  CGDataConsumerRelease(myconsumer);
  if (gc) {
    CGContextBeginPage(gc, &bounds);
    CGContextTranslateCTM(gc, 0.5, h-0.5);
    CGContextScaleCTM(gc, 1.0f, -1.0f);
    CGContextSaveGState(gc);
  }
}

void Fl_Quartz_Copy_Surface_Driver::set_current() {
  driver()->gc(gc);
  fl_window = (Window)1;
  Fl_Surface_Device::set_current();
}

size_t Fl_Quartz_Copy_Surface_Driver::MyPutBytes(void* info, const void* buffer, size_t count)
{
  CFDataAppendBytes ((CFMutableDataRef) info, (const UInt8 *)buffer, count);
  return count;
}

void Fl_Quartz_Copy_Surface_Driver::translate(int x, int y) {
  CGContextRestoreGState(gc);
  CGContextSaveGState(gc);
  CGContextTranslateCTM(gc, x, y);
  CGContextSaveGState(gc);
}

void Fl_Quartz_Copy_Surface_Driver::untranslate() {
  CGContextRestoreGState(gc);
}

void Fl_Quartz_Copy_Surface_Driver::draw_decorated_window(Fl_Window *win, int x_offset, int y_offset) {
  CALayer *layer = Fl_Cocoa_Window_Driver::driver(win)->get_titlebar_layer();
  if (!layer) {
    return Fl_Widget_Surface::draw_decorated_window(win, x_offset, y_offset);
  }
  CGContextSaveGState(gc);
  int bt = win->decorated_h() - win->h();
  CGContextTranslateCTM(gc, x_offset - 0.5, y_offset + bt - 0.5);
  float s = Fl::screen_scale(win->screen_num());
  CGContextScaleCTM(gc, 1/s, s >= 1 ? -1/s : -1);
  Fl_Cocoa_Window_Driver::draw_layer_to_context(layer, gc, win->w() * s, bt*s);
  CGContextRestoreGState(gc);  
  draw(win, x_offset, y_offset + bt); // print the window inner part
}

#endif // FL_CFG_GFX_QUARTZ

//
// End of "$Id$".
//
