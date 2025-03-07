//
// Shortcut header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

// Adding drag and drop for dragging widgets into windows.
class Widget_Bin_Button : public Fl_Button {
public:
  int handle(int) FL_OVERRIDE;
  Widget_Bin_Button(int X,int Y,int W,int H, const char* l = 0) :
  Fl_Button(X,Y,W,H,l) { }
};

// Adding drag and drop functionality to drag window prototypes onto the desktop.
class Widget_Bin_Window_Button : public Fl_Button {
public:
  int handle(int) FL_OVERRIDE;
  Widget_Bin_Window_Button(int X,int Y,int W,int H, const char* l = 0) :
  Fl_Button(X,Y,W,H,l) { }
};

// Callback signature for function returning the value of a variable.
typedef int (Fluid_Coord_Callback)(class Fluid_Coord_Input const *, void*);

// Entry for a list of variables available to an input field.
// Fluid_Coord_Input::variables() expects an array of Fluid_Coord_Input_Vars
// with the last entry's name_ set to NULL.
typedef struct Fluid_Coord_Input_Vars {
  const char *name_;
  Fluid_Coord_Callback *callback_;
} Fluid_Coord_Input_Vars;

// A text input widget that understands simple math.
class Fluid_Coord_Input : public Fl_Input
{
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

  /** Return the text in the widget text field. */
  const char *text() const { return Fl_Input::value(); }

  /** Set the text in the text field */
  void text(const char *v) { Fl_Input::value(v); }

  int value() const;
  void value(int v);

  /** Set the general callback for this widget. */
  void callback(Fl_Callback *cb) {
    user_callback_ = cb;
  }

  /** Set the list of the available variables
   \param vars array of variables, last entry `has name_` set to `NULL`
   \param user_data is forwarded to the Variable callback */
  void variables(Fluid_Coord_Input_Vars *vars, void *user_data) {
    vars_ = vars;
    vars_user_data_ = user_data;
  }

  int handle(int) FL_OVERRIDE;
};

#endif
