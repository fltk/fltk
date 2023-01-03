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
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//


#ifndef Fl_Quartz_Copy_Surface_Driver_H
#define Fl_Quartz_Copy_Surface_Driver_H

#include <FL/Fl_Copy_Surface.H>
#include <FL/platform.H>

class Fl_Quartz_Copy_Surface_Driver : public Fl_Copy_Surface_Driver {
  friend class Fl_Copy_Surface_Driver;
protected:
  CFMutableDataRef pdfdata;
  CGContextRef gc;
  static size_t MyPutBytes(void* info, const void* buffer, size_t count);
  Fl_Quartz_Copy_Surface_Driver(int w, int h);
  // implemented in Fl_cocoa.mm because uses Objective-c
  ~Fl_Quartz_Copy_Surface_Driver();
  void set_current() FL_OVERRIDE;
  void translate(int x, int y) FL_OVERRIDE;
  void untranslate() FL_OVERRIDE;
};

#endif /* Fl_Quartz_Copy_Surface_Driver_H */
