//
// "$Id$"
//
// implementation of Fl_Graphics_Driver class for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include "config_lib.h"
#include <FL/Fl_Graphics_Driver.H>
#include <FL/Fl_Image.H>
#include <FL/fl_draw.H>

FL_EXPORT Fl_Graphics_Driver *fl_graphics_driver; // the current driver of graphics operations

const Fl_Graphics_Driver::matrix Fl_Graphics_Driver::m0 = {1, 0, 0, 1, 0, 0};

/** Constructor */
Fl_Graphics_Driver::Fl_Graphics_Driver()
{
  font_ = 0;
  size_ = 0;
  sptr=0; rstackptr=0; 
  rstack[0] = NULL;
  fl_clip_state_number=0;
  m = m0; 
  fl_matrix = &m; 
  font_descriptor_ = NULL;
};

/** Return the graphics driver used when drawing to the platform's display */
Fl_Graphics_Driver &Fl_Graphics_Driver::default_driver()
{
  static Fl_Graphics_Driver *pMainDriver = Fl_Display_Device::display_device()->driver();
  return *pMainDriver;
}


/** see fl_text_extents() */
void Fl_Graphics_Driver::text_extents(const char*t, int n, int& dx, int& dy, int& w, int& h)
{
  w = (int)width(t, n);
  h = - height();
  dx = 0;
  dy = descent();
}

/** see fl_focus_rect() */
void Fl_Graphics_Driver::focus_rect(int x, int y, int w, int h)
{
  line_style(FL_DOT);
  rect(x, y, w, h);
  line_style(FL_SOLID);
}

/** Draws an Fl_Image scaled to width \p W & height \p H with top-left corner at \em X,Y
 \return zero when the graphics driver doesn't implement scaled drawing, non-zero if it does implement it.
 */
int Fl_Graphics_Driver::draw_scaled(Fl_Image *img, int X, int Y, int W, int H) {
  return 0;
}

/** see fl_copy_offscreen() */
void Fl_Graphics_Driver::copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy)
{
  fl_begin_offscreen(pixmap);
  uchar *img = fl_read_image(NULL, srcx, srcy, w, h, 0);
  fl_end_offscreen();
  fl_draw_image(img, x, y, w, h, 3, 0);
  delete[] img;
}

/** see fl_set_spot() */
void Fl_Graphics_Driver::set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win)
{
  // nothing to do, reimplement in driver if needed
}


/** see fl_reset_spot() */
void Fl_Graphics_Driver::reset_spot()
{
  // nothing to do, reimplement in driver if needed
}


/** Sets the value of the fl_gc global variable when it changes */
void Fl_Graphics_Driver::global_gc()
{
  // nothing to do, reimplement in driver if needed
}


/** see Fl::set_color(Fl_Color, unsigned) */
void Fl_Graphics_Driver::set_color(Fl_Color i, unsigned c)
{
  // nothing to do, reimplement in driver if needed
}


/** see Fl::free_color(Fl_Color, int) */
void Fl_Graphics_Driver::free_color(Fl_Color i, int overlay)
{
  // nothing to do, reimplement in driver if needed
}

/** Add a rectangle to an Fl_Region */
void Fl_Graphics_Driver::add_rectangle_to_region(Fl_Region r, int x, int y, int w, int h)
{
  // nothing to do, reimplement in driver if needed
}

/** Create a rectangular Fl_Region */
Fl_Region Fl_Graphics_Driver::XRectangleRegion(int x, int y, int w, int h)
{
  // nothing to do, reimplement in driver if needed
  return 0;
}

/** Delete an Fl_Region */
void Fl_Graphics_Driver::XDestroyRegion(Fl_Region r)
{
  // nothing to do, reimplement in driver if needed
}

/** Helper function for image drawing by platform-specific graphics drivers */
int Fl_Graphics_Driver::start_image(Fl_Image *img, int XP, int YP, int WP, int HP, int &cx, int &cy,
                     int &X, int &Y, int &W, int &H)
{
  // account for current clip region (faster on Irix):
  clip_box(XP,YP,WP,HP,X,Y,W,H);
  cx += X-XP; cy += Y-YP;
  // clip the box down to the size of image, quit if empty:
  if (cx < 0) {W += cx; X -= cx; cx = 0;}
  if (cx+W > img->w()) W = img->w()-cx;
  if (W <= 0) return 1;
  if (cy < 0) {H += cy; Y -= cy; cy = 0;}
  if (cy+H > img->h()) H = img->h()-cy;
  if (H <= 0) return 1;
  return 0;
}

//
// End of "$Id$".
//
