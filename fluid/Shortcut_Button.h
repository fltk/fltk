//
// Shortcut header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#ifndef _FLUID_SHORTCUT_BUTTON_H
#define _FLUID_SHORTCUT_BUTTON_H

#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>

class Shortcut_Button : public Fl_Button {
public:
  int svalue;
  int handle(int);
  void draw();
  Shortcut_Button(int X,int Y,int W,int H, const char* l = 0) :
    Fl_Button(X,Y,W,H,l) {svalue = 0;}
};

class Widget_Bin_Button : public Fl_Button {
public:
  int handle(int);
  Widget_Bin_Button(int X,int Y,int W,int H, const char* l = 0) :
  Fl_Button(X,Y,W,H,l) { }
};

class Widget_Bin_Window_Button : public Fl_Button {
public:
  int handle(int);
  Widget_Bin_Window_Button(int X,int Y,int W,int H, const char* l = 0) :
  Fl_Button(X,Y,W,H,l) { }
};


typedef int (Fluid_Coord_Callback)(class Fluid_Coord_Input const *, void*);

typedef struct Fluid_Coord_Input_Vars {
  const char *name_;
  Fluid_Coord_Callback *callback_;
} Fluid_Coord_Input_Vars;

class Fluid_Coord_Input : public Fl_Input {
  Fl_Callback *user_callback_;
  Fluid_Coord_Input_Vars *vars_;
  void *vars_user_data_;
  static void callback_handler_cb(Fluid_Coord_Input *This, void *v);
  void callback_handler(void *v);
  int eval_var(uchar *&s) const;
  int eval(uchar *&s, int prio) const;
  int eval(const char *s) const;
public:
  Fluid_Coord_Input(int x, int y, int w, int h, const char *l=0L);
  const char *text() const { return Fl_Input::value(); }
  void text(const char *v) { Fl_Input::value(v); }
  int value() const;
  void value(int v);
  void callback(Fl_Callback *cb) {
    user_callback_ = cb;
  }
  void variables(Fluid_Coord_Input_Vars *vars, void *user_data) {
    vars_ = vars;
    vars_user_data_ = user_data;
  }
};

#endif

