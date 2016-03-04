//
// "$Id: Fl_GDI_Image_Surface.H 11241 2016-02-27 13:52:27Z manolo $"
//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
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

#if !defined(FL_GDI_IMAGE_SURFACE_H) && !defined(FL_DOXYGEN)
#define FL_GDI_IMAGE_SURFACE_H 1

#include <FL/Fl_Image_Surface.H>

class Fl_Image_Surface::Helper : public Fl_Widget_Surface {
  friend class Fl_Image_Surface;
public:
  Fl_Offscreen offscreen;
  int width;
  int height;
  Fl_Surface_Device *previous;
  Window pre_window;
  HDC _sgc;
  int _savedc;
  Helper(int w, int h, int high_res);
  ~Helper();
  void set_current();
  void translate(int x, int y);
  void untranslate();
  Fl_RGB_Image *image();
  void end_current();
  int printable_rect(int *w, int *h) {*w = width; *h = height; return 0;}
};

#endif // FL_GDI_IMAGE_SURFACE_H

//
// End of "$Id: Fl_GDI_Image_Surface.H 11220 2016-02-26 12:51:47Z manolo $".
//
