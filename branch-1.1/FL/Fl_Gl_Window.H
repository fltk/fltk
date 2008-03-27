//
// "$Id$"
//
// OpenGL header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2005 by Bill Spitzak and others.
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
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//
#ifndef Fl_Gl_Window_H
#define Fl_Gl_Window_H

#include "Fl_Window.H"

#ifndef GLContext
typedef void* GLContext; // actually a GLXContext or HGLDC
#endif

class Fl_Gl_Choice; // structure to hold result of glXChooseVisual

class FL_EXPORT Fl_Gl_Window : public Fl_Window {

  int mode_;
  const int *alist;
  Fl_Gl_Choice *g;
  GLContext context_;
  char valid_f_;
  char damage1_; // damage() of back buffer
  virtual void draw_overlay();
  void init();

  void *overlay;
  void make_overlay();
  friend class _Fl_Gl_Overlay;

  static int can_do(int, const int *);
  int mode(int, const int *);

public:

  void show();
  void show(int a, char **b) {Fl_Window::show(a,b);}
  void flush();
  void hide();
  void resize(int,int,int,int);

  char valid() const {return valid_f_ & 1;}
  void valid(char v) {if (v) valid_f_ |= 1; else valid_f_ &= 0xfe;}
  void invalidate();

  char context_valid() const {return valid_f_ & 2;}
  void context_valid(char v) {if (v) valid_f_ |= 2; else valid_f_ &= 0xfd;}

  static int can_do(int m) {return can_do(m,0);}
  static int can_do(const int *m) {return can_do(0, m);}
  int can_do() {return can_do(mode_,alist);}
  Fl_Mode mode() const {return (Fl_Mode)mode_;}
  int mode(int a) {return mode(a,0);}
  int mode(const int *a) {return mode(0, a);}

  void* context() const {return context_;}
  void context(void*, int destroy_flag = 0);
  void make_current();
  void swap_buffers();
  void ortho();

  int can_do_overlay();
  void redraw_overlay();
  void hide_overlay();
  void make_overlay_current();

  ~Fl_Gl_Window();
  Fl_Gl_Window(int W, int H, const char *l=0) : Fl_Window(W,H,l) {init();}
  Fl_Gl_Window(int X, int Y, int W, int H, const char *l=0)
    : Fl_Window(X,Y,W,H,l) {init();}
};

#endif

//
// End of "$Id$".
//
