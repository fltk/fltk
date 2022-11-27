//
// fltk-admin for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022 by Bill Spitzak and others.
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

#ifndef _FLTK_ADMIN_H
#define _FLTK_ADMIN_H

#include <FL/Fl_Preferences.H>

typedef struct {
  const char *name;
  int deflt;
  const char *brief;
  const char *desc;
} Fl_Option_Data;

extern Fl_Option_Data g_option[];

extern void set_user_option(const char *name, int value);
extern int get_user_option(const char *name);
extern void clear_user_option(const char *name);

extern void set_system_option(const char *name, int value);
extern int get_system_option(const char *name);
extern void clear_system_option(const char *name);

#endif // _FLTK_ADMIN_H
