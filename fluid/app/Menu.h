//
// Application Main Menu header for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_APP_MENU_H
#define FLUID_APP_MENU_H

#include <FL/Fl_Menu_Item.H>

extern void exit_cb(class Fl_Widget *,void *); // TODO: remove this
extern void toggle_widgetbin_cb(Fl_Widget *, void *);
extern void menu_file_save_cb(Fl_Widget *, void *arg);
extern void menu_file_open_history_cb(Fl_Widget *, void *v);
extern void align_widget_cb(Fl_Widget *, long);

#endif // FLUID_APP_MENU_H

