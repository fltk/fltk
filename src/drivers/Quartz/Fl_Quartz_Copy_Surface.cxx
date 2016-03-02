//
// "$Id: Fl_Quartz_Copy_Surface.cxx 11241 2016-02-27 13:52:27Z manolo $"
//
// Copy-to-clipboard code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#include "config_lib.h"

#ifdef FL_CFG_GFX_QUARTZ
#include "Fl_Quartz_Copy_Surface.H"
#include "Fl_Quartz_Graphics_Driver.H"
#endif

Fl_Copy_Surface::Helper::Helper(int w, int h) : Fl_Widget_Surface(NULL), width(w), height(h) {
  driver(new Fl_Quartz_Graphics_Driver);
  prepare_copy_pdf_and_tiff(w, h);
}

Fl_Copy_Surface::Helper::~Helper() {
  // that code is implemented in Fl_cocoa.mm because it uses some Objective-c
  Fl_X::complete_copy_pdf_and_tiff(gc, pdfdata);
}

void Fl_Copy_Surface::Helper::set_current() {
  driver()->gc(gc);
  fl_window = (Window)1;
  Fl_Surface_Device::set_current();
}

size_t Fl_Copy_Surface::Helper::MyPutBytes(void* info, const void* buffer, size_t count)
{
  CFDataAppendBytes ((CFMutableDataRef) info, (const UInt8 *)buffer, count);
  return count;
}

void Fl_Copy_Surface::Helper::init_PDF_context(int w, int h)
{
  CGRect bounds = CGRectMake(0, 0, w, h );
  pdfdata = CFDataCreateMutable(NULL, 0);
  CGDataConsumerRef myconsumer;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1040
  if (&CGDataConsumerCreateWithCFData != NULL) {
    myconsumer = CGDataConsumerCreateWithCFData(pdfdata); // 10.4
  }
  else
#endif
  {
    static CGDataConsumerCallbacks callbacks = { Fl_Copy_Surface::Helper::MyPutBytes, NULL };
    myconsumer = CGDataConsumerCreate ((void*) pdfdata, &callbacks);
  }
  gc = CGPDFContextCreate (myconsumer, &bounds, NULL);
  CGDataConsumerRelease (myconsumer);
}

void Fl_Copy_Surface::Helper::prepare_copy_pdf_and_tiff(int w, int h)
{
  init_PDF_context(w, h);
  if (gc == NULL) return;
  CGRect bounds = CGRectMake(0, 0, w, h );
  CGContextBeginPage (gc, &bounds);
  CGContextTranslateCTM(gc, 0, h);
  CGContextScaleCTM(gc, 1.0f, -1.0f);
  CGContextSaveGState(gc);
}

void Fl_Copy_Surface::Helper::translate(int x, int y) {
  CGContextRestoreGState(gc);
  CGContextSaveGState(gc);
  CGContextTranslateCTM(gc, x, y);
  CGContextSaveGState(gc);
}

void Fl_Copy_Surface::Helper::untranslate() {
  CGContextRestoreGState(gc);
}

//
// End of "$Id: Fl_Copy_Surface.H 11220 2016-02-26 12:51:47Z manolo $".
//
