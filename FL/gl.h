// gl.H
// Fltk gl drawing functions.
// You must include this instead of GL/gl.h to get around a stupid
// fuck up by our good friends at Microsloth.
// This file also provides "missing" OpenGL functions, and
// gl_start() and gl_finish() to allow OpenGL to be used in any window

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
