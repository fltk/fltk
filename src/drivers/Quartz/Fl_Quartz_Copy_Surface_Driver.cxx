//
// Copy-to-clipboard code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2019 by Bill Spitzak and others.
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

#include <FL/Fl_Copy_Surface.H>
#include <FL/platform.H>
#include "Fl_Quartz_Graphics_Driver.H"
#include "Fl_Quartz_Copy_Surface_Driver.H"
#include "../Cocoa/Fl_Cocoa_Window_Driver.H"


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
  float d = fl_graphics_driver->scale();
  CGRect bounds = CGRectMake(0, 0, w * d, h * d);
  gc = CGPDFContextCreate(myconsumer, &bounds, NULL);
  CGDataConsumerRelease(myconsumer);
  if (gc) {
    CGContextBeginPage(gc, &bounds);
    CGContextScaleCTM(gc, d, -d);
    CGContextTranslateCTM(gc, 0.5, -h + 0.5);
    CGContextSaveGState(gc);
  }
}

void Fl_Quartz_Copy_Surface_Driver::set_current() {
  driver()->gc(gc);
  fl_window = (FLWindow*)1;
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
