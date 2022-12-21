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
  virtual void update() FL_OVERRIDE;
  virtual void draw() FL_OVERRIDE;
  virtual void about(Fl_Callback *cb, void *data) FL_OVERRIDE;
  virtual int add(const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags) FL_OVERRIDE;
  virtual int add(const char* str) FL_OVERRIDE;
  virtual int insert(int index, const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags) FL_OVERRIDE;
  virtual void menu(const Fl_Menu_Item *m) FL_OVERRIDE;
  virtual void shortcut (int i, int s) FL_OVERRIDE;
  virtual void setonly (Fl_Menu_Item *item) FL_OVERRIDE;
  virtual void clear() FL_OVERRIDE;
  virtual int clear_submenu(int index) FL_OVERRIDE;
  virtual void remove(int index) FL_OVERRIDE;
  virtual void replace(int index, const char *name) FL_OVERRIDE;
  virtual void mode(int i, int fl) FL_OVERRIDE;
  virtual void create_window_menu() FL_OVERRIDE;
  int find_first_window();
  void new_window(Fl_Window *win);
  void remove_window(Fl_Window *win);
  void rename_window(Fl_Window *win);
  static Fl_MacOS_Sys_Menu_Bar_Driver* driver();
};


#endif /* Fl_MacOS_Sys_Menu_Bar_Driver_H */
