//
// Definition of class Fl_MacOS_Sys_Menu_Bar_Driver for the Fast Light Tool Kit (FLTK).
//
// Copyright 2017 by Bill Spitzak and others.
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

#ifndef Fl_MacOS_Sys_Menu_Bar_Driver_H
#define Fl_MacOS_Sys_Menu_Bar_Driver_H

#include "../../Fl_Sys_Menu_Bar_Driver.H"

class Fl_MacOS_Sys_Menu_Bar_Driver : public Fl_Sys_Menu_Bar_Driver {
public:
  Fl_Menu_Item *window_menu_items;
  int first_window_menu_item;
  Fl_MacOS_Sys_Menu_Bar_Driver();
  virtual ~Fl_MacOS_Sys_Menu_Bar_Driver();
  void update() FL_OVERRIDE;
  void draw() FL_OVERRIDE;
  void about(Fl_Callback *cb, void *data) FL_OVERRIDE;
  int add(const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags) FL_OVERRIDE;
  int add(const char* str) FL_OVERRIDE;
  int insert(int index, const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags) FL_OVERRIDE;
  void menu(const Fl_Menu_Item *m) FL_OVERRIDE;
  void shortcut (int i, int s) FL_OVERRIDE;
  void setonly (Fl_Menu_Item *item) FL_OVERRIDE;
  void clear() FL_OVERRIDE;
  int clear_submenu(int index) FL_OVERRIDE;
  void remove(int index) FL_OVERRIDE;
  void replace(int index, const char *name) FL_OVERRIDE;
  void mode(int i, int fl) FL_OVERRIDE;
  void create_window_menu() FL_OVERRIDE;
  void new_window(Fl_Window *win);
  void remove_window(Fl_Window *win);
  void rename_window(Fl_Window *win);
  static Fl_MacOS_Sys_Menu_Bar_Driver* driver();
  void play_menu(const Fl_Menu_Item *) FL_OVERRIDE;
};


#endif /* Fl_MacOS_Sys_Menu_Bar_Driver_H */
