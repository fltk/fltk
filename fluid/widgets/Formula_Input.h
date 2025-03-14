//
// Formula Int Input widget header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

#ifndef FLUID_WIDGETS_FORMULA_INPUT_H
#define FLUID_WIDGETS_FORMULA_INPUT_H

#include <FL/Fl_Input.H>

namespace fld {
namespace widget {

class Formula_Input;

// Callback signature for function returning the value of a variable.
typedef int (Fluid_Coord_Callback)(Formula_Input const *, void*);

// Entry for a list of variables available to an input field.
// Formula_Input::variables() expects an array of
// Formula_Input_Vars with the last entry's name_ set to nullptr.
typedef struct Formula_Input_Vars {
  const char *name_;
  Fluid_Coord_Callback *callback_;
} Formula_Input_Vars;

// A text input widget that understands simple math.
class Formula_Input : public Fl_Input
{
  Fl_Callback *user_callback_ { nullptr };
  Formula_Input_Vars *vars_ { nullptr };
  void *vars_user_data_ { nullptr };

  static void callback_handler_cb(Formula_Input *This, void *v);
  void callback_handler(void *v);
  int eval_var(uchar *&s) const;
  int eval(uchar *&s, int prio) const;
  int eval(const char *s) const;
  
public:
  Formula_Input(int x, int y, int w, int h, const char *l = nullptr);

  /** Return the text in the widget text field. */
  const char *text() const { return Fl_Input::value(); }
  
  /** Set the text in the text field */
  void text(const char *v) { Fl_Input::value(v); }
  
  int value() const;
  void value(int v);
  
  /** Set the general callback for this widget. */
  void callback(Fl_Callback *cb) { user_callback_ = cb; }

  void variables(fld::widget::Formula_Input_Vars *vars, void *user_data);
  int handle(int) override;
};

} // namespace widget
} // namespace fld

#endif // FLUID_WIDGETS_FORMULA_INPUT_H
