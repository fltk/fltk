//
// "$Id$"
//
// Screen/monitor bounding box API for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/Fl_Screen_Driver.H>
#include <config.h>


#ifndef FL_DOXYGEN
void Fl::call_screen_init()
{
  screen_driver()->init();
}
#endif

/** Returns the leftmost x coordinate of the main screen work area. */
int Fl::x()
{
  return screen_driver()->x();
}


/** Returns the topmost y coordinate of the main screen work area. */
int Fl::y()
{
  return screen_driver()->y();
}


/** Returns the width in pixels of the main screen work area. */
int Fl::w()
{
  return screen_driver()->w();
}


/** Returns the height in pixels of the main screen work area. */
int Fl::h()
{
  return screen_driver()->h();
}


/**
  Gets the number of available screens.
*/
int Fl::screen_count()
{
  return screen_driver()->screen_count();
}


/**
  Gets the bounding box of a screen
  that contains the specified screen position \p mx, \p my
  \param[out]  X,Y,W,H the corresponding screen bounding box
  \param[in] mx, my the absolute screen position
*/
void Fl::screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my)
{
  screen_driver()->screen_xywh(X, Y, W, H, mx, my);
}


/**
 Gets the bounding box of the work area of a screen
 that contains the specified screen position \p mx, \p my
 \param[out]  X,Y,W,H the work area bounding box
 \param[in] mx, my the absolute screen position
 */
void Fl::screen_work_area(int &X, int &Y, int &W, int &H, int mx, int my)
{
  screen_driver()->screen_work_area(X, Y, W, H, mx, my);
}


/**
 Gets the bounding box of the work area of the given screen.
 \param[out]  X,Y,W,H the work area bounding box
 \param[in] n the screen number (0 to Fl::screen_count() - 1)
 \see void screen_xywh(int &x, int &y, int &w, int &h, int mx, int my)
*/
void Fl::screen_work_area(int &X, int &Y, int &W, int &H, int n)
{
  screen_driver()->screen_work_area(X, Y, W, H, n);
}


/**
  Gets the screen bounding rect for the given screen.
  Under MSWindows, Mac OS X, and the Gnome desktop, screen #0 contains the menubar/taskbar
  \param[out]  X,Y,W,H the corresponding screen bounding box
  \param[in] n the screen number (0 to Fl::screen_count() - 1)
  \see void screen_xywh(int &x, int &y, int &w, int &h, int mx, int my)
*/
void Fl::screen_xywh(int &X, int &Y, int &W, int &H, int n)
{
  screen_driver()->screen_xywh(X, Y, W, H, n);
}


/**
  Gets the screen bounding rect for the screen
  which intersects the most with the rectangle
  defined by \p mx, \p my, \p mw, \p mh.
  \param[out]  X,Y,W,H the corresponding screen bounding box
  \param[in] mx, my, mw, mh the rectangle to search for intersection with
  \see void screen_xywh(int &X, int &Y, int &W, int &H, int n)
  */
void Fl::screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my, int mw, int mh)
{
  screen_driver()->screen_xywh(X, Y, W, H, mx, my, mw, mh);
}


/**
  Gets the screen number of a screen
  that contains the specified screen position \p x, \p y
  \param[in] x, y the absolute screen position
*/
int Fl::screen_num(int x, int y)
{
  return screen_driver()->screen_num(x, y);
}


/**
  Gets the screen number for the screen
  which intersects the most with the rectangle
  defined by \p x, \p y, \p w, \p h.
  \param[in] x, y, w, h the rectangle to search for intersection with
  */
int Fl::screen_num(int x, int y, int w, int h)
{
  return screen_driver()->screen_num(x, y, w, h);
}


/**
 Gets the screen resolution in dots-per-inch for the given screen.
 \param[out]  h, v  horizontal and vertical resolution
 \param[in]   n     the screen number (0 to Fl::screen_count() - 1)
 \see void screen_xywh(int &x, int &y, int &w, int &h, int mx, int my)
 */
void Fl::screen_dpi(float &h, float &v, int n)
{
  screen_driver()->screen_dpi(h, v, n);
}


/**
 Gets the bounding box of a screen that contains the mouse pointer.
 \param[out]  X,Y,W,H the corresponding screen bounding box
 \see void screen_xywh(int &x, int &y, int &w, int &h, int mx, int my)
 */
void Fl::screen_xywh(int &X, int &Y, int &W, int &H)
{
  int mx, my;
  int nscreen = Fl::screen_driver()->get_mouse(mx, my);
  Fl::screen_driver()->screen_xywh(X, Y, W, H, nscreen);
}


/**
 Gets the bounding box of the work area of the screen that contains the mouse pointer.
 \param[out]  X,Y,W,H the work area bounding box
 \see void screen_work_area(int &x, int &y, int &w, int &h, int mx, int my)
 */
void Fl::screen_work_area(int &X, int &Y, int &W, int &H)
{
  int mx, my;
  int nscreen = Fl::screen_driver()->get_mouse(mx, my);
  Fl::screen_driver()->screen_work_area(X, Y, W, H, nscreen);
}


//
// End of "$Id$".
//
