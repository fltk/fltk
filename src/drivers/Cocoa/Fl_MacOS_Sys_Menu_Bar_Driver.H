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
  Fl_MacOS_Sys_Menu_Bar_Driver();
  virtual ~Fl_MacOS_Sys_Menu_Bar_Driver();
  void update() override;
  void draw() override;
  void about(Fl_Callback *cb, void *data) override;
  int add(const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags) override;
  int add(const char* str) override;
  int insert(int index, const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags) override;
  void menu(const Fl_Menu_Item *m) override;
  void shortcut (int i, int s) override;
  void setonly (Fl_Menu_Item *item) override;
  void clear() override;
  int clear_submenu(int index) override;
  void remove(int index) override;
  void replace(int index, const char *name) override;
  void mode(int i, int fl) override;
  void create_window_menu() override;
  int find_first_window();
  void new_window(Fl_Window *win);
  void remove_window(Fl_Window *win);
  void rename_window(Fl_Window *win);
  static Fl_MacOS_Sys_Menu_Bar_Driver* driver();
};


#endif /* Fl_MacOS_Sys_Menu_Bar_Driver_H */
