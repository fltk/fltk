//
// "$Id$"
//
// Standard X11 font selection code for the Fast Light Tool Kit (FLTK).
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

/*
 This module implements a lowest-common-denominator font for OpenGL.
 It will always work, even if the main graphics library does not support
 rendering text into a texture buffer.
 
 The font is limited to a single face and ASCII characters. It is drawn using
 lines which makes it arbitrarily scalable. I am trying to keep font data really
 compact.
 */


#include <config.h>
#include "../../config_lib.h"
#include "Fl_OpenGL_Graphics_Driver.H"
#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>


// FIXME: check out FreeGlut:
// FIXME: implement font-to-RGBA in the main graphics driver

#if 1

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



double Fl_OpenGL_Graphics_Driver::width(const char *str, int n) {
  return size_*n*0.5;
}

int Fl_OpenGL_Graphics_Driver::descent() {
  return (int)(size_ - size_*0.8);
}

int Fl_OpenGL_Graphics_Driver::height() {
  return (int)(size_);
}

void Fl_OpenGL_Graphics_Driver::text_extents(const char *str, int n, int& dx, int& dy, int& w, int& h)
{
  dx = 0;
  dy = descent();
  w = (int)width(str, n);
  h = size_;
}

void Fl_OpenGL_Graphics_Driver::draw(const char *str, int n, int x, int y)
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
            glEnd();
            glBegin(GL_POINTS); glVertex2f(px, py); glEnd();
            rendering = 0;
          }
          break;
        } else if (cmd>63) {
          if (cmd=='\100' && rendering) {
            glEnd();
            glBegin(GL_POINTS); glVertex2f(px, py); glEnd();
            rendering = 0;
          }
        } else {
          if (!rendering) { glBegin(GL_LINE_STRIP); rendering = 1; }
          int vx = (cmd & '\70')>>3;
          int vy = (cmd & '\07');
          px = (int)(0.5+x+vx*size_*0.5/8.0);
          py = (int)(0.5+y+vy*size_/8.0-0.8*size_);
          glVertex2f(px, py);
        }
      }
    }
    x += size_*0.5;
  }
}

#elif 0

/*
extern FL_EXPORT Fl_Glut_StrokeFont glutStrokeRoman;
extern FL_EXPORT Fl_Glut_StrokeFont glutStrokeMonoRoman;
#  define GLUT_STROKE_ROMAN             (&glutStrokeRoman)
#  define GLUT_STROKE_MONO_ROMAN        (&glutStrokeMonoRoman)

FL_EXPORT void glutStrokeCharacter(void *font, int character);
FL_EXPORT GLfloat glutStrokeHeight(void *font);
FL_EXPORT int glutStrokeLength(void *font, const unsigned char *string);
FL_EXPORT void glutStrokeString(void *font, const unsigned char *string);
FL_EXPORT int glutStrokeWidth(void *font, int character);
*/

#else

void Fl_OpenGL_Graphics_Driver::draw(const char* str, int n, int x, int y) {
  gl_draw(str, n, x, y);
}

#endif


//
// End of "$Id$".
//
