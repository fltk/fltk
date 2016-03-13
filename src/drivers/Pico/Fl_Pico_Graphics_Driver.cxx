//
// "$Id: Fl_Pico_Graphics_Driver.cxx 11241 2016-02-27 13:52:27Z manolo $"
//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
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


#include "../../config_lib.h"
#include "Fl_Pico_Graphics_Driver.h"
#include <FL/fl_draw.H>


void fl_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b) {
  fl_color(r,g,b);
  fl_rectf(x,y,w,h);
}


void Fl_Pico_Graphics_Driver::point(int x, int y)
{
  // implement in derived class
}

void Fl_Pico_Graphics_Driver::rect(int x, int y, int w, int h)
{
  line(x, y, x+w-1, y);
  line(x+w-1, y, x+w-1, y+h-1);
  line(x+w-1, y+h-1, x, y+h-1);
  line(x, y+h-1, x, y);
}

void Fl_Pico_Graphics_Driver::rectf(int x, int y, int w, int h)
{
  // implement in derived class
}

void Fl_Pico_Graphics_Driver::line(int x, int y, int x1, int y1)
{
  // implement in derived class
}

void Fl_Pico_Graphics_Driver::line(int x, int y, int x1, int y1, int x2, int y2)
{
  line(x, y, x1, y1);
  line(x1, y1, x2, y2);
}

void Fl_Pico_Graphics_Driver::xyline(int x, int y, int x1)
{
  line(x, y, x1, y);
}

void Fl_Pico_Graphics_Driver::xyline(int x, int y, int x1, int y2)
{
  line(x, y, x1, y, x1, y2);
}

void Fl_Pico_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3)
{
  line(x, y, x1, y, x1, y2);
  line(x1, y2, x3, y2);
}

void Fl_Pico_Graphics_Driver::yxline(int x, int y, int y1)
{
  line(x, y, x, y1);
}

void Fl_Pico_Graphics_Driver::yxline(int x, int y, int y1, int x2)
{
  line(x, y, x, y1, x2, y1);
}

void Fl_Pico_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3)
{
  line(x, y, x, y1, x2, y1);
  line(x2, y1, x2, y3);
}

void Fl_Pico_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2)
{
  line(x0, y0, x1, y1);
  line(x1, y1, x2, y2);
  line(x2, y2, x0, y0);
}

void Fl_Pico_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
{
  line(x0, y0, x1, y1);
  line(x1, y1, x2, y2);
  line(x2, y2, x3, y3);
  line(x3, y3, x0, y0);
}

void Fl_Pico_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2)
{
}

void Fl_Pico_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
{
}

void Fl_Pico_Graphics_Driver::push_clip(int x, int y, int w, int h)
{
}

int Fl_Pico_Graphics_Driver::clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H)
{
  return 0;
}

int Fl_Pico_Graphics_Driver::not_clipped(int x, int y, int w, int h)
{
  return 1;
}

void Fl_Pico_Graphics_Driver::push_no_clip()
{
}

void Fl_Pico_Graphics_Driver::pop_clip()
{
}

void Fl_Pico_Graphics_Driver::begin_complex_polygon()
{
}

void Fl_Pico_Graphics_Driver::transformed_vertex(double xf, double yf)
{
}

static double px, py; // FIXME: aaaah!

void Fl_Pico_Graphics_Driver::vertex(double x, double y)
{
  if (n>0) {
    if (what==LINE) {
      line(px, py, x, y);
    }
  }
  n++;
  px = x; py = y;
}

void Fl_Pico_Graphics_Driver::end_points()
{
}

void Fl_Pico_Graphics_Driver::end_line()
{
  // nothing to do
}

void Fl_Pico_Graphics_Driver::end_loop()
{
}

void Fl_Pico_Graphics_Driver::end_polygon()
{
}

void Fl_Pico_Graphics_Driver::end_complex_polygon()
{
}

void Fl_Pico_Graphics_Driver::gap()
{
}

void Fl_Pico_Graphics_Driver::circle(double x, double y, double r)
{
}

void Fl_Pico_Graphics_Driver::arc(int x, int y, int w, int h, double a1, double a2)
{
}

void Fl_Pico_Graphics_Driver::pie(int x, int y, int w, int h, double a1, double a2)
{
}

void Fl_Pico_Graphics_Driver::line_style(int style, int width, char* dashes)
{
}

void Fl_Pico_Graphics_Driver::color(uchar r, uchar g, uchar b)
{
}

Fl_Bitmask Fl_Pico_Graphics_Driver::create_bitmask(int w, int h, const uchar *array)
{
  return 0;
}

void Fl_Pico_Graphics_Driver::delete_bitmask(Fl_Bitmask bm)
{
}



/*
  |01234567|
 -+--------+
 0|        |____
 1|++++++++|font
 2|++++++++|
 3|++++++++|
 4|++++++++|
 5|++++++++|____
 6|        |descent
 7|        |
 -+--------+
 */


static const char *font_data[128] = {
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,

  /* */0, /*!*/"\31\34\100\35\36", 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  /*0*/"\62\51\21\12\14\25\55\64\62\100\52\61",
  /*1*/"\22\31\35\100\25\45",
  0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,

  0,
  /*A*/"\15\31\55\100\23\43",
  /*B*/"\43\54\45\15\11\41\52\43\13",
  /*C*/"\62\51\21\12\14\25\55\64",
  /*D*/"\11\51\62\64\55\15\11",
  /*E*/"\61\11\15\65\100\13\43",
  /*F*/"\61\11\15\100\13\43",
  /*G*/"\62\51\21\12\14\25\55\64\100\65\63\33",
  /*H*/"\11\15\100\61\65\100\13\63",
  /*I*/"\21\41\100\31\35\100\25\45", 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, /*T*/"\11\71\100\41\45", 0, 0, /*W*/"\01\15\33\55\61",
  /*X*/"\15\51\100\11\55", 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, /*d*/"\64\55\55\25\14\13\22\52\63\100\61\65", /*e*/"\55\25\14\13\22\52\63\64\14", 0, 0,
  0, /*i*/"\31\32\100\23\33\35\100\25\45", 0, 0, /*l*/"\31\34\45", 0, /*n*/"\15\12\100\13\22\42\53\55", /*o*/"\55\25\14\13\22\52\63\64\55",
  /*p*/"\17\12\42\53\54\45\15", 0, /*r*/"\25\22\100\23\32\52", /*s*/"\62\22\13\64\55\15", /*t*/"\41\44\55\65\100\22\62", 0, 0, 0,
  /*x*/"\15\52\100\12\55", 0, 0, 0, 0, 0, 0, 0,
};


double Fl_Pico_Graphics_Driver::width(const char *str, int n) {
  return size_*n*0.5;
}

int Fl_Pico_Graphics_Driver::descent() {
  return (int)(size_ - size_*0.8);
}

int Fl_Pico_Graphics_Driver::height() {
  return (int)(size_);
}

void Fl_Pico_Graphics_Driver::text_extents(const char *str, int n, int& dx, int& dy, int& w, int& h)
{
  dx = 0;
  dy = descent();
  w = (int)width(str, n);
  h = size_;
}

void Fl_Pico_Graphics_Driver::draw(const char *str, int n, int x, int y)
{
  int i;
  for (i=0; i<n; i++) {
    char c = str[i] & 0x7f;
    const char *fd = font_data[(int)c];
    if (fd) {
      char rendering = 0;
      float px=0.0f, py=0.0f;
      for (;;) {
        char cmd = *fd++;
        if (cmd==0) {
          if (rendering) {
            end_line();
            rendering = 0;
          }
          break;
        } else if (cmd>63) {
          if (cmd=='\100' && rendering) {
            end_line();
            rendering = 0;
          }
        } else {
          if (!rendering) { begin_line(); rendering = 1; }
          int vx = (cmd & '\70')>>3;
          int vy = (cmd & '\07');
          px = (int)(0.5+x+vx*size_*0.5/8.0);
          py = (int)(0.5+y+vy*size_/8.0-0.8*size_);
          vertex(px, py);
        }
      }
    }
    x += size_*0.5;
  }
}



//
// End of "$Id: Fl_Pico_Graphics_Driver.cxx 11241 2016-02-27 13:52:27Z manolo $".
//
