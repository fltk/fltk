//
// "$Id"
//
// OpenGL header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998 by Bill Spitzak and others.
//
// You must include this instead of GL/gl.h to get the Microsoft
// APIENTRY stuff included (from <windows.h>) prior to the OpenGL
// header files.
//
// This file also provides "missing" OpenGL functions, and
// gl_start() and gl_finish() to allow OpenGL to be used in any window
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#ifndef gl_draw_H
#define gl_draw_H

#include "Enumerations.H" // for color names
#ifdef WIN32
# include <windows.h>
#endif
#include <GL/gl.h>

void gl_start();
void gl_finish();

void gl_color(Fl_Color);
inline void gl_color(int c) {gl_color((Fl_Color)c);} // back compatability

void gl_rect(int x,int y,int w,int h);
inline void gl_rectf(int x,int y,int w,int h) {glRecti(x,y,x+w,y+h);}

void gl_font(int fontid, int size);
int  gl_height();
int  gl_descent();
double gl_width(const char *);
double gl_width(const char *, int n);
double gl_width(uchar);

void gl_draw(const char*);
void gl_draw(const char*, int n);
void gl_draw(const char*, int x, int y);
void gl_draw(const char*, int n, int x, int y);
void gl_draw(const char*, int x, int y, int w, int h, Fl_Align);
void gl_measure(const char*, int& x, int& y);

void gl_draw_image(const uchar *, int x,int y,int w,int h, int d=3, int ld=0);

#endif

//
// End of "$Id: gl.h,v 1.2 1998/10/19 21:39:25 mike Exp $".
//
