//
// "$Id: Fl_Gl_Window.H,v 1.7 1999/02/16 21:59:38 mike Exp $"
//
// OpenGL header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
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
#ifndef Fl_Gl_Window_H
#define Fl_Gl_Window_H

#include "Fl_Window.H"

class Fl_Gl_Choice; // structure to hold result of glXChooseVisual

class Fl_Gl_Window : public Fl_Window {

  int mode_;
  const int *alist;
  Fl_Gl_Choice *g;
  void * context; // actually a GLXContext
  char valid_;
  char damage1_; // damage() of back buffer
  virtual FL_EXPORT void draw_overlay();
  FL_EXPORT void init();

  void *overlay;
  FL_EXPORT void make_overlay();
  friend class _Fl_Gl_Overlay;

  static FL_EXPORT int can_do(int, const int *);
  FL_EXPORT int mode(int, const int *);

public:

  FL_EXPORT void show();
  void show(int a, char **b) {Fl_Window::show(a,b);}
  FL_EXPORT void flush();
  FL_EXPORT void hide();
  FL_EXPORT void resize(int,int,int,int);

  char valid() const {return valid_;}
  void valid(char i) {valid_ = i;}
  FL_EXPORT void invalidate();

  static int can_do(int i) {return can_do(i,0);}
  static int can_do(const int *i) {return can_do(0, i);}
  int can_do() {return can_do(mode_,alist);}
  Fl_Mode mode() const {return (Fl_Mode)mode_;}
  int mode(int a) {return mode(a,0);}
  int mode(const int *a) {return mode(0, a);}

  FL_EXPORT int can_do_overlay();
  FL_EXPORT void redraw_overlay();
  FL_EXPORT void hide_overlay();

  FL_EXPORT void make_current();
  FL_EXPORT void make_overlay_current();
  FL_EXPORT void swap_buffers();
  FL_EXPORT void ortho();

  FL_EXPORT ~Fl_Gl_Window();
  Fl_Gl_Window(int W, int H, const char *l=0) : Fl_Window(W,H,l) {init();}
  Fl_Gl_Window(int X, int Y, int W, int H, const char *l=0)
    : Fl_Window(X,Y,W,H,l) {init();}
};

#endif

//
// End of "$Id: Fl_Gl_Window.H,v 1.7 1999/02/16 21:59:38 mike Exp $".
//
