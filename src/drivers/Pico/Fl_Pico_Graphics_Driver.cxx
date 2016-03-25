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
  /*00*/0, /*01*/0, /*02*/0, /*03*/0,
  /*04*/0, /*05*/0, /*06*/0, /*07*/0,
  /*08*/0, /*09*/0, /*0A*/0, /*0B*/0,
  /*0C*/0, /*0D*/0, /*0E*/0, /*0F*/0,
  /*10*/0, /*11*/0, /*12*/0, /*13*/0,
  /*14*/0, /*15*/0, /*16*/0, /*17*/0,
  /*18*/0, /*19*/0, /*1A*/0, /*1B*/0,
  /*1C*/0, /*1D*/0, /*1E*/0, /*1F*/0,
  /* */0, /*!*/"\31\34\100\35\36", /*"*/"\31\22\100\51\42", /*#*/"\31\15\100\61\45\100\12\72\100\04\64",
  /*$*/"\62\51\11\02\13\53\64\55\15\04\100\30\36", /*%*/"\21\11\02\13\23\32\21\100\15\51\100\34\43\53\64\55\45\34", /*&*/"\63\45\15\04\13\52\41\21\12\65", /*'*/"\31\22",
  /*(*/"\51\32\23\24\35\56", /*)*/"\21\42\53\54\45\26", /***/"\31\33\15\100\33\55\100\02\33\62", /*+*/"\35\31\100\03\63",
  /*,*/"\35\45\36", /*-*/"\13\53", /*.*/"\35\36", /*/*/"\51\15",
  /*0*/"\21\12\14\25\55\64\62\51\21\100\24\52", /*1*/"\22\41\45", /*2*/"\12\21\51\62\53\24\15\65", /*3*/"\12\21\51\62\53\64\55\25\14\100\53\33",
  /*4*/"\55\51\04\64", /*5*/"\14\25\55\64\53\13\21\61", /*6*/"\62\51\21\12\14\25\55\64\53\13", /*7*/"\11\61\33\25",
  /*8*/"\12\21\51\62\53\64\55\25\14\23\12\100\23\53", /*9*/"\14\25\55\64\62\51\21\12\23\63", /*:*/"\32\33\100\35\36", /*;*/"\32\33\100\25\35\26",
  /*<*/"\62\13\64", /*=*/"\12\62\100\14\64", /*>*/"\12\63\14", /*?*/"\12\21\51\62\43\34\35\100\36\37",
  /*@*/"\56\16\05\02\11\51\62\64\55\35\24\23\32\52\63", /*A*/"\05\31\65\100\14\54", /*B*/"\11\51\62\53\64\55\15\11\100\13\53", /*C*/"\62\51\11\02\04\15\55\64",
  /*D*/"\11\51\62\64\55\15\11", /*E*/"\61\11\15\65\100\13\53", /*F*/"\61\11\15\100\13\53", /*G*/"\62\51\11\02\04\15\55\64\63\33",
  /*H*/"\11\15\100\61\65\100\13\63", /*I*/"\21\41\100\25\45\100\35\31", /*J*/"\51\54\45\15\04", /*K*/"\11\15\100\14\61\100\65\33",
  /*L*/"\11\15\65", /*M*/"\05\01\35\61\65", /*N*/"\05\01\65\61", /*O*/"\02\11\51\62\64\55\15\04\02",
  /*P*/"\15\11\51\62\53\13", /*Q*/"\02\11\51\62\64\55\15\04\02\100\65\34", /*R*/"\15\11\51\62\53\13\100\33\65", /*S*/"\62\51\11\02\13\53\64\55\15\04",
  /*T*/"\01\61\100\31\35", /*U*/"\61\64\55\15\04\01", /*V*/"\01\35\61", /*W*/"\01\15\31\55\61",
  /*X*/"\01\65\100\05\61", /*Y*/"\01\33\35\100\33\61", /*Z*/"\01\61\05\65", /*[*/"\51\31\36\56",
  /*\*/"\21\55", /*]*/"\21\41\46\26", /*^*/"\13\31\53", /*_*/"\06\76",
  /*`*/"\31\42", /*a*/"\22\52\63\65\100\63\23\14\25\55\64", /*b*/"\11\15\100\14\25\55\64\63\52\22\13", /*c*/"\63\52\22\13\14\25\55\64",
  /*d*/"\61\65\100\64\55\25\14\13\22\52\63", /*e*/"\64\63\52\22\13\14\25\55\100\64\14", /*f*/"\35\32\41\51\100\22\52", /*g*/"\62\65\56\26\100\63\52\22\13\14\25\55\64",
  /*h*/"\11\15\100\65\63\52\22\13", /*i*/"\31\32\100\33\100\23\33\35\100\25\45", /*j*/"\31\32\100\33\35\26\16", /*k*/"\11\15\100\14\62\100\33\65",
  /*l*/"\31\34\45\55", /*m*/"\05\02\100\03\12\22\33\35\100\33\42\52\63\65", /*n*/"\12\15\100\13\22\52\63\65", /*o*/"\22\13\14\25\55\64\63\52\22",
  /*p*/"\16\12\100\13\22\52\63\64\55\25\14", /*q*/"\62\66\100\63\52\22\13\14\25\55\64", /*r*/"\22\25\100\23\32\42\53", /*s*/"\63\52\22\13\64\55\25\14",
  /*t*/"\31\34\45\55\100\22\42", /*u*/"\12\14\25\55\64\62\100\64\65", /*v*/"\62\35\02", /*w*/"\02\15\32\55\62",
  /*x*/"\62\15\100\65\12", /*y*/"\12\45\62\100\45\36\16", /*z*/"\12\62\15\65", /*{*/"\51\41\32\33\24\35\36\47\57\100\14\24",
  /*|*/"\31\37", /*}*/"\21\31\42\43\54\64\100\54\45\46\37\27", /*~*/"\12\21\31\42\52\61", /*7F*/0
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
