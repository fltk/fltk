//
// "$Id$"
//
// Definition of Apple Quartz graphics driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2016 by Bill Spitzak and others.
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


#ifndef Fl_Quartz_Copy_Surface_Driver_H
#define Fl_Quartz_Copy_Surface_Driver_H

#include <FL/Fl_Copy_Surface.H>

class Fl_Quartz_Copy_Surface_Driver : public Fl_Copy_Surface_Driver {
  friend class Fl_Copy_Surface_Driver;
protected:
  CFMutableDataRef pdfdata;
  CGContextRef gc;
  void prepare_copy_pdf_and_tiff(int w, int h);
  void init_PDF_context(int w, int h);
  static size_t MyPutBytes(void* info, const void* buffer, size_t count);
  Fl_Quartz_Copy_Surface_Driver(int w, int h);
  // implemented in Fl_cocoa.mm because uses Objective-c
  ~Fl_Quartz_Copy_Surface_Driver();
  void set_current();
  void translate(int x, int y);
  void untranslate();
  int w() {return width;}
  int h() {return height;}
  int printable_rect(int *w, int *h) {*w = width; *h = height; return 0;}
};

#endif /* Fl_Quartz_Copy_Surface_Driver_H */

//
// End of "$Id$".
//
