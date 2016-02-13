//
// "$Id$"
//
// All screen related calls in a driver style class.
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
#include <FL/Fl_Screen_Driver.H>
#include <FL/Fl.H>


char Fl_Screen_Driver::bg_set = 0;
char Fl_Screen_Driver::bg2_set = 0;
char Fl_Screen_Driver::fg_set = 0;


Fl_Screen_Driver::Fl_Screen_Driver() :
num_screens(-1)
{
}


void Fl_Screen_Driver::display(const char *) {
  // blank
}


int Fl_Screen_Driver::visual(int) {
  // blank
  return 1;
}


void Fl_Screen_Driver::screen_xywh(int &X, int &Y, int &W, int &H)
{
  int x, y;
  Fl::get_mouse(x, y);
  screen_xywh(X, Y, W, H, x, y);
}


void Fl_Screen_Driver::screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my)
{
  screen_xywh(X, Y, W, H, screen_num(mx, my));
}


void Fl_Screen_Driver::screen_work_area(int &X, int &Y, int &W, int &H)
{
  int x, y;
  Fl::get_mouse(x, y);
  screen_work_area(X, Y, W, H, x, y);
}


void Fl_Screen_Driver::screen_work_area(int &X, int &Y, int &W, int &H, int mx, int my)
{
  screen_work_area(X, Y, W, H, screen_num(mx, my));
}


int Fl_Screen_Driver::screen_count()
{
  if (num_screens < 0)
    init();
  return num_screens ? num_screens : 1;
}


void Fl_Screen_Driver::screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my, int mw, int mh)
{
  screen_xywh(X, Y, W, H, screen_num(mx, my, mw, mh));
}


int Fl_Screen_Driver::screen_num(int x, int y)
{
  int screen = 0;
  if (num_screens < 0) init();

  for (int i = 0; i < num_screens; i ++) {
    int sx, sy, sw, sh;
    Fl::screen_xywh(sx, sy, sw, sh, i);
    if ((x >= sx) && (x < (sx+sw)) && (y >= sy) && (y < (sy+sh))) {
      screen = i;
      break;
    }
  }
  return screen;
}


// Return the number of pixels common to the two rectangular areas
static inline float fl_intersection(int x1, int y1, int w1, int h1,
                                    int x2, int y2, int w2, int h2)
{
  if(x1+w1 < x2 || x2+w2 < x1 || y1+h1 < y2 || y2+h2 < y1)
    return 0.;
  int int_left = x1 > x2 ? x1 : x2;
  int int_right = x1+w1 > x2+w2 ? x2+w2 : x1+w1;
  int int_top = y1 > y2 ? y1 : y2;
  int int_bottom = y1+h1 > y2+h2 ? y2+h2 : y1+h1;
  return (float)(int_right - int_left) * (int_bottom - int_top);
}


int Fl_Screen_Driver::screen_num(int x, int y, int w, int h)
{
  int best_screen = 0;
  float best_intersection = 0.;
  for (int i = 0; i < num_screens; i++) {
    int sx = 0, sy = 0, sw = 0, sh = 0;
    screen_xywh(sx, sy, sw, sh, i);
    float sintersection = fl_intersection(x, y, w, h, sx, sy, sw, sh);
    if (sintersection > best_intersection) {
      best_screen = i;
      best_intersection = sintersection;
    }
  }
  return best_screen;
}


const char *Fl_Screen_Driver::get_system_scheme()
{
  return 0L;
}


//
// End of "$Id$".
//
