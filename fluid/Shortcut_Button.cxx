//
// Widget type code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

#include "Shortcut_Button.h"

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

/** \class Shortcut_Button
 A button that allows the user to type a key combination to create shortcuts.
 After clicked once, the button catches the following keyboard events and
 records the pressed keys and all modifiers. It draws a text representation of
 the shortcut. The backspace key deletes the current shortcut.
 */

/**
 Draw the textual representation of the shortcut.
 */
void Shortcut_Button::draw() {
  if (value()) draw_box(FL_DOWN_BOX, (Fl_Color)9);
  else draw_box(FL_UP_BOX, FL_WHITE);
  fl_font(FL_HELVETICA,14); fl_color(FL_FOREGROUND_COLOR);
  if (g_project.use_FL_COMMAND && (svalue & (FL_CTRL|FL_META))) {
    char buf[1024];
    fl_snprintf(buf, 1023, "Command+%s", fl_shortcut_label(svalue&~(FL_CTRL|FL_META)));
    fl_draw(buf,x()+6,y(),w(),h(),FL_ALIGN_LEFT);
  } else {
    fl_draw(fl_shortcut_label(svalue),x()+6,y(),w(),h(),FL_ALIGN_LEFT);
  }
}

/**
 Handle keystrokes to catch the user's shortcut.
 */
int Shortcut_Button::handle(int e) {
  when(0); type(FL_TOGGLE_BUTTON);
  if (e == FL_KEYBOARD) {
    if (!value()) return 0;
    int v = Fl::event_text()[0];
    if ( (v > 32 && v < 0x7f) || (v > 0xa0 && v <= 0xff) ) {
      if (isupper(v)) {
        v = tolower(v);
        v |= FL_SHIFT;
      }
      v = v | (Fl::event_state()&(FL_META|FL_ALT|FL_CTRL));
    } else {
      v = (Fl::event_state()&(FL_META|FL_ALT|FL_CTRL|FL_SHIFT)) | Fl::event_key();
      if (v == FL_BackSpace && svalue) v = 0;
    }
    if (v != svalue) {svalue = v; set_changed(); redraw(); do_callback(); }
    return 1;
  } else if (e == FL_UNFOCUS) {
    int c = changed(); value(0); if (c) set_changed();
    return 1;
  } else if (e == FL_FOCUS) {
    return value();
  } else {
    int r = Fl_Button::handle(e);
    if (e == FL_RELEASE && value() && Fl::focus() != this) take_focus();
    return r;
  }
}

/** \class Widget_Bin_Button
 A button for the widget bin that allows the user to drag widgets into a window.
 Dragging and dropping a new widget makes it easy for the user to position
 a widget inside a window or group.
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
        // fake a buttton release
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
 This button is used by the widget bin to create new windows by drag'n'drop.
 The new window will be created wherever the user drops it on the desktop.
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
          drag_win = new Fl_Window(0, 0, 100, 100);
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
          if (new_type && new_type->is_window()) {
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
 An Input field for widget coordinates and sizes.
 This widget adds basic math capability to the text input field and a number
 of variables that can be used in the formula.
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
}

void Fluid_Coord_Input::callback_handler_cb(Fluid_Coord_Input *This, void *v) {
  This->callback_handler(v);
}

void Fluid_Coord_Input::callback_handler(void *v) {
  if (user_callback_)
    (*user_callback_)(this, v);
  // do *not* update the value to show the evaluated fomule here, because the
  // values of the variables have already updated after the user callback.
}

/**
 Get the value of a variable.
 Collects all conesecutive ASCII letters into a variable name, scans the
 Variable list for that name, and then calls the corresponding callback from
 the Variable array.
 \param s points to the first character of the variable name, must point after
    the last character of the variable name when returning.
 \return the integer value that wasf= found or calculated
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
 The interpreter understands unary plus and minus, basic integer math
 (+, -, *, /), brackets, and can handle a user defined list of variables
 by name. There is no error checking. We assume that the formula is
 entered correctly.
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
