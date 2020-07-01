//
// Standard dialog header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2020 by Bill Spitzak and others.
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

/** \file fl_ask.H
 API for common dialogs.
 */

#ifndef fl_ask_H
#  define fl_ask_H

#  include "Enumerations.H"
class Fl_Widget;

/** Different system beeps available.
 \sa fl_beep(int) */
enum Fl_Beep {
  FL_BEEP_DEFAULT = 0,  ///< Default beep.
  FL_BEEP_MESSAGE,  ///< Message beep.
  FL_BEEP_ERROR,  ///< Error beep.
  FL_BEEP_QUESTION,  ///< Question beep.
  FL_BEEP_PASSWORD,  ///< Password beep.
  FL_BEEP_NOTIFICATION  ///< Notification beep.
};

#  ifdef __GNUC__
/* the GNUC-specific attribute appearing below in prototypes with a variable list of arguments
 helps detection of mismatches between format string and argument list at compilation time */
#    define __fl_attr(x) __attribute__ (x)
#  else
#    define __fl_attr(x)
#  endif // __GNUC__

FL_EXPORT void fl_beep(int type = FL_BEEP_DEFAULT);
FL_EXPORT void fl_message(const char *,...) __fl_attr((__format__ (__printf__, 1, 2)));
FL_EXPORT void fl_alert(const char *,...) __fl_attr((__format__ (__printf__, 1, 2)));
// fl_ask() is deprecated since it uses "Yes" and "No" for the buttons,
// which does not conform to the current FLTK Human Interface Guidelines.
// Use fl_choice() instead with the appropriate verbs instead.
FL_EXPORT int fl_ask(const char *,...) __fl_attr((__format__ (__printf__, 1, 2), __deprecated__));
FL_EXPORT int fl_choice(const char *q,const char *b0,const char *b1,const char *b2,...) __fl_attr((__format__ (__printf__, 1, 5)));
FL_EXPORT const char *fl_input(const char *label, const char *deflt = 0, ...) __fl_attr((__format__ (__printf__, 1, 3)));
FL_EXPORT const char *fl_password(const char *label, const char *deflt = 0, ...) __fl_attr((__format__ (__printf__, 1, 3)));

FL_EXPORT Fl_Widget *fl_message_icon();
extern FL_EXPORT Fl_Font fl_message_font_;
extern FL_EXPORT Fl_Fontsize fl_message_size_;
inline void fl_message_font(Fl_Font f, Fl_Fontsize s) {
  fl_message_font_ = f; fl_message_size_ = s;}

FL_EXPORT void fl_message_hotspot(int enable);
FL_EXPORT int fl_message_hotspot(void);

FL_EXPORT void fl_message_position(const int x, const int y, const int center = 0);
FL_EXPORT void fl_message_position(Fl_Widget *widget);
FL_EXPORT int fl_message_position(int *x = 0, int *y = 0);

/** \see fl_message_position(Fl_Widget *widget). */
inline void fl_message_position(Fl_Widget &widget) {
  fl_message_position(&widget);
}

FL_EXPORT void fl_message_title(const char *title);
FL_EXPORT void fl_message_title_default(const char *title);

// pointers you can use to change FLTK to another language:
extern FL_EXPORT const char* fl_no;
extern FL_EXPORT const char* fl_yes;
extern FL_EXPORT const char* fl_ok;
extern FL_EXPORT const char* fl_cancel;
extern FL_EXPORT const char* fl_close;
#endif // !fl_ask_H
