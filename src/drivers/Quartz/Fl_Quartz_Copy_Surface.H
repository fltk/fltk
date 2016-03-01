//
// "$Id: Fl_Quartz_Copy_Surface.H 11241 2016-02-27 13:52:27Z manolo $"
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

#ifndef FL_QUARTZ_COPY_SURFACE_H
#define FL_QUARTZ_COPY_SURFACE_H 1

#include <FL/Fl_Copy_Surface.H>

#include <ApplicationServices/ApplicationServices.h>

class Fl_Copy_Surface::Helper : public Fl_Widget_Surface {
  friend class Fl_Copy_Surface;
private:
  int width;
  int height;
  CFMutableDataRef pdfdata;
  CGContextRef gc;
  void prepare_copy_pdf_and_tiff(int w, int h);
  void init_PDF_context(int w, int h);
  static size_t MyPutBytes(void* info, const void* buffer, size_t count);
  Helper(int w, int h);
  ~Helper();
  void set_current();
  void translate(int x, int y);
  void untranslate();
  int w() {return width;}
  int h() {return height;}
};

#endif // FL_QUARTZ_COPY_SURFACE_H

//
// End of "$Id: Fl_Copy_Surface.H 11220 2016-02-26 12:51:47Z manolo $".
//
