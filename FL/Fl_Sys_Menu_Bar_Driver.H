//
// "$Id$"
//
// system menu bar widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2017 by Bill Spitzak and others.
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

#ifndef Fl_Sys_Menu_Bar_Driver_H
#define Fl_Sys_Menu_Bar_Driver_H

#include <FL/Fl_Sys_Menu_Bar.H>

class Fl_Sys_Menu_Bar_Driver {
  friend class Fl_Sys_Menu_Bar;
protected:
  Fl_Sys_Menu_Bar *bar;
public:
  Fl_Sys_Menu_Bar_Driver();
  virtual ~Fl_Sys_Menu_Bar_Driver();
  virtual void update() {}
  virtual void draw() { bar->Fl_Menu_Bar::draw();}
  virtual void about(Fl_Callback *cb, void *data) {}
  virtual int add(const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags) {
    return bar->Fl_Menu_Bar::add(label, shortcut, cb, user_data, flags);
  }
  virtual int add(const char* str) { return bar->Fl_Menu_Bar::add(str); }
  virtual int insert(int index, const char* label, int shortcut, Fl_Callback *cb, void *user_data, int flags) {
    return bar->Fl_Menu_Bar::insert(index, label, shortcut, cb, user_data, flags);
  }
  virtual void menu(const Fl_Menu_Item *m) { bar->Fl_Menu_Bar::menu(m); }
  virtual void shortcut(int i, int s) { bar->Fl_Menu_Bar::shortcut(i, s); }
  virtual void setonly(Fl_Menu_Item *item) { bar->Fl_Menu_Bar::setonly(item); }
  virtual void clear() { bar->Fl_Menu_Bar::clear(); }
  virtual int clear_submenu(int index) { return bar->Fl_Menu_Bar::clear_submenu(index);}
  virtual void remove(int index) { bar->Fl_Menu_Bar::remove(index); }
  virtual void replace(int index, const char *name) { bar->Fl_Menu_Bar::replace(index, name); }
  virtual void mode(int i, int fl) { bar->Fl_Menu_Bar::mode(i, fl); }
  
  static Fl_Sys_Menu_Bar_Driver *driver_; // to be assigned with a unique object of this class or of a derived class
};


#endif // Fl_Sys_Menu_Bar_Driver_H

//
// End of "$Id$".
//
