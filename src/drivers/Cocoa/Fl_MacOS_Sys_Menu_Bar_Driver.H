//
// "$Id$"
//
// system menu bar widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 2017 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#ifndef Fl_MacOS_Sys_Menu_Bar_Driver_H
#define Fl_MacOS_Sys_Menu_Bar_Driver_H

#include <FL/Fl_Sys_Menu_Bar_Driver.H>

class Fl_MacOS_Sys_Menu_Bar_Driver : public Fl_Sys_Menu_Bar_Driver {
public:
  Fl_MacOS_Sys_Menu_Bar_Driver();
  virtual ~Fl_MacOS_Sys_Menu_Bar_Driver();
  virtual void update();
  virtual void draw() {}
  virtual void about(Fl_Callback *cb, void *data);
  virtual int add(const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags);
  virtual int add(const char* str);
  virtual int insert(int index, const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags);
  virtual void menu(const Fl_Menu_Item *m);
  virtual void shortcut (int i, int s);
  virtual void setonly (Fl_Menu_Item *item);
  virtual void clear();
  virtual int clear_submenu(int index);
  virtual void remove(int index);
  virtual void replace(int index, const char *name);
  virtual void mode(int i, int fl);
  static Fl_MacOS_Sys_Menu_Bar_Driver* new_driver();
};


#endif /* Fl_MacOS_Sys_Menu_Bar_Driver_H */

//
// End of "$Id$".
//
