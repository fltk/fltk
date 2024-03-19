//
// Widget type code for the Fast Light Tool Kit (FLTK).
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

#include "custom_widgets.h"

#include "fluid.h"
#include "Fl_Window_Type.h"
#include "factory.h"
#include "widget_panel.h"
#include "widget_browser.h"

#include <FL/platform.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Menu_.H>
#include <FL/fl_string_functions.h>
#include "../src/flstring.h"

/** \class Widget_Bin_Button
 The Widget_Bin_Button button is a button that can be used in the widget bin to
 allow the user to drag and drop widgets into a window or group. This feature
 makes it easy for the user to position a widget at a specific location within
 the window or group.
 */

/**
 Convert mouse dragging into a drag and drop event.
 */
int Widget_Bin_Button::handle(int inEvent)
{
  int ret = 0;
  switch (inEvent) {
    case FL_PUSH:
      Fl_Button::handle(inEvent);
      return 1; // make sure that we get drag events
    case FL_DRAG:
      ret = Fl_Button::handle(inEvent);
      if (!user_data())
        return ret;
      if (!Fl::event_is_click()) { // make it a dnd event
        // fake a drag outside of the widget
        Fl::e_x = x()-1;
        Fl_Button::handle(inEvent);
        // fake a button release
        Fl_Button::handle(FL_RELEASE);
        // make it into a dnd event
        const char *type_name = (const char*)user_data();
        Fl_Type::current_dnd = Fl_Type::current;
        Fl::copy(type_name, (int)strlen(type_name)+1, 0);
        Fl::dnd();
        return 1;
      }
      return ret;
  }
  return Fl_Button::handle(inEvent);
}

/** \class Widget_Bin_Window_Button
 The Widget_Bin_Window_Button button is used in the widget bin to create new
 windows by dragging and dropping. When the button is dragged and dropped onto
 the desktop, a new window will be created at the drop location.
 */

/**
 Convert mouse dragging into a drag and drop event.
 */
int Widget_Bin_Window_Button::handle(int inEvent)
{
  static Fl_Window *drag_win = NULL;
  int ret = 0;
  switch (inEvent) {
    case FL_PUSH:
      Fl_Button::handle(inEvent);
      return 1; // make sure that we get drag events
    case FL_DRAG:
      ret = Fl_Button::handle(inEvent);
      if (!user_data())
        return ret;
      if (!Fl::event_is_click()) {
        if (!drag_win) {
          drag_win = new Fl_Window(0, 0, 480, 320);
          drag_win->border(0);
          drag_win->set_non_modal();
        }
        if (drag_win) {
          drag_win->position(Fl::event_x_root()+1, Fl::event_y_root()+1);
          drag_win->show();
        }
        // Does not work outside window: fl_cursor(FL_CURSOR_HAND);
      }
      return ret;
    case FL_RELEASE:
      if (drag_win) {
        Fl::delete_widget(drag_win);
        drag_win = NULL;
        // create a new window here
        Fl_Type *prototype = typename_to_prototype((char*)user_data());
        if (prototype) {
          Fl_Type *new_type = add_new_widget_from_user(prototype, kAddAfterCurrent);
          if (new_type && new_type->is_a(ID_Window)) {
            Fl_Window_Type *new_window = (Fl_Window_Type*)new_type;
            Fl_Window *w = (Fl_Window *)new_window->o;
            w->position(Fl::event_x_root(), Fl::event_y_root());
          }
        }
        widget_browser->display(Fl_Type::current);
        widget_browser->rebuild();
      }
      return Fl_Button::handle(inEvent);
  }
  return Fl_Button::handle(inEvent);
}

/** \class Fluid_Coord_Input
 The Fluid_Coord_Input widget is an input field for entering widget coordinates
 and sizes. It includes basic math capabilities and allows the use of variables
 in formulas. This widget is useful for specifying precise positions and
 dimensions for widgets in a graphical user interface.
 */

/**
 Create an input field.
 */
Fluid_Coord_Input::Fluid_Coord_Input(int x, int y, int w, int h, const char *l) :
Fl_Input(x, y, w, h, l),
user_callback_(0L),
vars_(0L),
vars_user_data_(0L)
{
  Fl_Input::callback((Fl_Callback*)callback_handler_cb);
  text("0");
}

void Fluid_Coord_Input::callback_handler_cb(Fluid_Coord_Input *This, void *v) {
  This->callback_handler(v);
}

void Fluid_Coord_Input::callback_handler(void *v) {
  if (user_callback_)
    (*user_callback_)(this, v);
  // do *not* update the value to show the evaluated formula here, because the
  // values of the variables have already updated after the user callback.
}

/**
 \brief Get the value of a variable.
 Collects all consecutive ASCII letters into a variable name, scans the
 Variable list for that name, and then calls the corresponding callback from
 the Variable array.
 \param s points to the first character of the variable name, must point after
    the last character of the variable name when returning.
 \return the integer value that was found or calculated
 */
int Fluid_Coord_Input::eval_var(uchar *&s) const {
  if (!vars_)
    return 0;
  // find the end of the variable name
  uchar *v = s;
  while (isalpha(*s)) s++;
  int n = (int)(s-v);
  // find the variable in the list
  for (Fluid_Coord_Input_Vars *vars = vars_; vars->name_; vars++) {
    if (strncmp((char*)v, vars->name_, n)==0 && vars->name_[n]==0)
      return vars->callback_(this, vars_user_data_);
  }
  return 0;
}

/**
 Evaluate a formula into an integer, recursive part.
 \param s remaining text in this formula, must return a pointer to the next
    character that will be interpreted.
 \param prio priority of current operation
 \return the value so far
 */
int Fluid_Coord_Input::eval(uchar *&s, int prio) const {
  int v = 0, sgn = 1;
  uchar c = *s++;

  // check for end of text
  if (c==0) { s--; return sgn*v; }

  // check for unary operator
  if (c=='-') { sgn = -1; c = *s++; }
  else if (c=='+') { sgn = 1; c = *s++; }

  // read value, variable, or bracketed term
  if (c==0) {
    s--; return sgn*v;
  } else if (c>='0' && c<='9') {
    // numeric value
    while (c>='0' && c<='9') {
      v = v*10 + (c-'0');
      c = *s++;
    }
  } else if (isalpha(c)) {
    v = eval_var(--s);
    c = *s++;
  } else if (c=='(') {
    // opening bracket
    v = eval(s, 5);
  } else {
    return sgn*v; // syntax error
  }
  if (sgn==-1) v = -v;

  // Now evaluate all following binary operators
  for (;;) {
    if (c==0) {
      s--;
      return v;
    } else if (c=='+' || c=='-') {
      if (prio<=4) { s--; return v; }
      if (c=='+') { v += eval(s, 4); }
      else if (c=='-') { v -= eval(s, 4); }
    } else if (c=='*' || c=='/') {
      if (prio<=3) { s--; return v; }
      if (c=='*') { v *= eval(s, 3); }
      else if (c=='/') {
        int x = eval(s, 3);
        if (x!=0) // if x is zero, don't divide
          v /= x;
      }
    } else if (c==')') {
      return v;
    } else {
      return v; // syntax error
    }
    c = *s++;
  }
  return v;
}

/**
 Evaluate a formula into an integer.

 The Fluid_Coord_Input widget includes a formula interpreter that allows you
 to evaluate a string containing a mathematical formula and obtain the result
 as an integer. The interpreter supports unary plus and minus, basic integer
 math operations (such as addition, subtraction, multiplication, and division),
 and brackets. It also allows you to define a list of variables by name and use
 them in the formula. The interpreter does not perform error checking, so it is
 assumed that the formula is entered correctly.

 \param s formula as a C string
 \return the calculated value
 */
int Fluid_Coord_Input::eval(const char *s) const
{
  // duplicate the text, so we can modify it
  uchar *buf = (uchar*)fl_strdup(s);
  uchar *src = buf, *dst = buf;
  // remove all whitespace to make the parser easier
  for (;;) {
    uchar c = *src++;
    if (c==' ' || c=='\t') continue;
    *dst++ = c;
    if (c==0) break;
  }
  src = buf;
  // now jump into the recursion
  int ret = eval(src, 5);
  ::free(buf);
  return ret;
}

/**
 Evaluate the formula and return the result.
 */
int Fluid_Coord_Input::value() const {
  return eval(text());
}

/**
 Set the field to an integer value, replacing previous texts.
 */
void Fluid_Coord_Input::value(int v) {
  char buf[32];
  fl_snprintf(buf, sizeof(buf), "%d", v);
  text(buf);
}

/**
 Allow vertical mouse dragging and mouse wheel to interactively change the value.
 */
int Fluid_Coord_Input::handle(int event) {
  switch (event) {
    case FL_MOUSEWHEEL:
      if (Fl::event_dy()) {
        value( value() - Fl::event_dy() );
        set_changed();
        do_callback(FL_REASON_CHANGED);
      }
      return 1;
  }
  return Fl_Input::handle(event);
}
