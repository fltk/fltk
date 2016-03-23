//
// "$Id$"
//
// A base class for platform specific window handling code
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

/** \file Fl_Window_Driver.H
 \brief declaration of classe Fl_Window_Driver.
*/

#ifndef FL_WINDOW_DRIVER_H
#define FL_WINDOW_DRIVER_H

#include <FL/Fl_Export.H>
#include <stdlib.h>


class Fl_Window;
class Fl_X;
class Fl_Image;
class Fl_Shared_Image;
class Fl_RGB_Image;

/**
 \brief A base class for platform specific window handling code.
  */
class FL_EXPORT Fl_Window_Driver
{
  friend class Fl_Window;
  friend class Fl_X;

protected:
  Fl_Window *pWindow;
  struct shape_data_type;
  shape_data_type *shape_data_; ///< non-null means the window has a non-rectangular shape

public:
  Fl_Window_Driver(Fl_Window *);
  virtual ~Fl_Window_Driver();
  static Fl_Window_Driver *newWindowDriver(Fl_Window *);

  // --- accessors to private window data
  int minw();
  int minh();
  int maxw();
  int maxh();
  int fullscreen_screen_top();
  int fullscreen_screen_bottom();
  int fullscreen_screen_left();
  int fullscreen_screen_right();
  unsigned char size_range_set();
  void force_position(int c);
  void x(int X);
  void y(int Y);

  // --- window data
  virtual int decorated_w() = 0;
  virtual int decorated_h() = 0;

  // --- window management
  virtual void take_focus();
  virtual void flush_double();
  virtual void flush_overlay();
  virtual void flush_single();
  virtual void draw_begin();
  virtual void draw_end();
  void draw();
  virtual void make_current();
  virtual void label(const char *name, const char *mininame);

  virtual Fl_X *makeWindow() { /* FIXME: move Fl_X::make(Fl_Window*) here for OSX, MSWin, and X11 */ return 0; }
  virtual void wait_for_expose() {} // TODO: check
  virtual void destroy_double_buffer(); // TODO: check
  virtual void show();
  virtual void show_menu();
  virtual void resize(int X,int Y,int W,int H) {}
  virtual void hide() {}
  int hide_common();
  virtual void map() {}
  virtual void unmap() {}
  virtual void fullscreen_on() {}
  virtual void fullscreen_off(int X, int Y, int W, int H) {}

  // --- window shape stuff
  void shape_pixmap_(Fl_Image* pixmap); // TODO: check
  virtual void shape(const Fl_Image* img) {} // TODO: check
  virtual void shape_alpha_(Fl_Image* img, int offset) {} // TODO: check

  // --- window icon stuff
  virtual void icons(const Fl_RGB_Image *icons[], int count) {} // TODO: check
  virtual const void *icon() const {return NULL;} // TODO: check
  virtual void icon(const void * ic) {} // TODO: check
  virtual void free_icons() {} // TODO: check

  // --- window printing helper
  virtual void capture_titlebar_and_borders(Fl_Shared_Image*& top, Fl_Shared_Image*& left, Fl_Shared_Image*& bottom, Fl_Shared_Image*& right); // TODO: check
};


#endif // FL_WINDOW_DRIVER_H

//
// End of "$Id$".
//
