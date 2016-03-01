//
// "$Id: Fl_Xlib_Copy_Surface.H 11241 2016-02-27 13:52:27Z manolo $"
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


#ifndef Fl_Xlib_Copy_Surface_h
#define Fl_Xlib_Copy_Surface_h

#include <FL/Fl_Copy_Surface.H>

#include "Fl_Translated_Xlib_Graphics_Driver.H"

class Fl_Copy_Surface::Helper : public Fl_Widget_Surface {
  friend class Fl_Copy_Surface;
private:
  int width;
  int height;
  Fl_Offscreen xid;
  Window oldwindow;
  Fl_Surface_Device *_ss;
  Helper(int w, int h);
  ~Helper();
  void set_current();
  void translate(int x, int y);
  void untranslate();
  int w() {return width;}
  int h() {return height;}
};

#endif /* Fl_Xlib_Copy_Surface_h */

//
// End of "$Id: Fl_Copy_Surface.H 11220 2016-02-26 12:51:47Z manolo $".
//
