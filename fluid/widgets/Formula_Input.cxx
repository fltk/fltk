//
// Formula Int Input widget code for the Fast Light Tool Kit (FLTK).
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

#include "widgets/Formula_Input.h"

#include <FL/fl_string_functions.h>
#include "../src/flstring.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

using namespace fld;
using namespace fld::widget;

/** \class fld::widget::Formula_Input
 The Formula_Input widget is an input field for entering widget coordinates
 and sizes. It includes basic math capabilities and allows the use of variables
 in formulas. This widget is useful for specifying precise positions and
 dimensions for widgets in a graphical user interface.
 */

/**
 Create an input field.
 */
Formula_Input::Formula_Input(int x, int y, int w, int h, const char *l)
: Fl_Input(x, y, w, h, l)
{
  Fl_Input::callback((Fl_Callback*)callback_handler_cb);
  text("0");
}

void Formula_Input::callback_handler_cb(Formula_Input *This, void *v) {
  This->callback_handler(v);
}

void Formula_Input::callback_handler(void *v) {
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
int Formula_Input::eval_var(uchar *&s) const {
  if (!vars_)
    return 0;
  // find the end of the variable name
  uchar *v = s;
  while (isalpha(*s)) s++;
  int n = (int)(s-v);
  // find the variable in the list
  for (Formula_Input_Vars *vars = vars_; vars->name_; vars++) {
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
int Formula_Input::eval(uchar *&s, int prio) const {
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

 The Formula_Input widget includes a formula interpreter that allows you
 to evaluate a string containing a mathematical formula and obtain the result
 as an integer. The interpreter supports unary plus and minus, basic integer
 math operations (such as addition, subtraction, multiplication, and division),
 and brackets. It also allows you to define a list of variables by name and use
 them in the formula. The interpreter does not perform error checking, so it is
 assumed that the formula is entered correctly.

 \param s formula as a C string
 \return the calculated value
 */
int Formula_Input::eval(const char *s) const
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
int Formula_Input::value() const {
  return eval(text());
}

/**
 Set the field to an integer value, replacing previous texts.
 */
void Formula_Input::value(int v) {
  char buf[32];
  fl_snprintf(buf, sizeof(buf), "%d", v);
  text(buf);
}

/**
 Allow vertical mouse dragging and mouse wheel to interactively change the value.
 */
int Formula_Input::handle(int event) {
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

/** Set the list of the available variables
 \param vars array of variables, last entry `has name_` set to `nullptr`
 \param user_data is forwarded to the Variable callback */
void Formula_Input::variables(Formula_Input_Vars *vars, void *user_data) {
  vars_ = vars;
  vars_user_data_ = user_data;
}
