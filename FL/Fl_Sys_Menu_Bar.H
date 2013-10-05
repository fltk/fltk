//
// "$Id$"
//
// MacOS system menu bar header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

#ifndef Fl_Sys_Menu_Bar_H
#define Fl_Sys_Menu_Bar_H

#include "Fl_Menu_Bar.H"
#include "x.H"

#if defined(__APPLE__) || defined(FL_DOXYGEN)

/**
 @brief A class to create, modify and delete menus that appear on Mac OS X in the menu bar at the top of the screen.
 *
 * On other than Mac OS X platforms, Fl_Sys_Menu_Bar is a synonym of class Fl_Menu_Bar.
 *
 * You can configure a callback for the 'About' menu item to invoke your own code with fl_mac_set_about().
 *
 */
class FL_EXPORT Fl_Sys_Menu_Bar : public Fl_Menu_Bar {
protected:
  void draw();
public:
  /**
   @brief The constructor.
   *
   * On Mac OS X, all arguments are unused. On other platforms they are used as by Fl_Menu_Bar::Fl_Menu_Bar().
   */
  Fl_Sys_Menu_Bar(int x,int y,int w,int h,const char *l=0);
  const Fl_Menu_Item *menu() const {return Fl_Menu_::menu();}
  void menu(const Fl_Menu_Item *m);
  int add(const char* label, int shortcut, Fl_Callback*, void *user_data=0, int flags=0);
  int insert(int index, const char* label, int shortcut, Fl_Callback *cb, void *user_data=0, int flags=0);
  void remove(int n);
  void replace(int rank, const char *name);
#ifdef __APPLE__
  void update();
#else
  void update() {}
#endif
  void clear();
  int clear_submenu(int index);
};

#else

typedef Fl_Menu_Bar Fl_Sys_Menu_Bar;

#endif // defined(__APPLE__) || defined(FL_DOXYGEN)

#endif // Fl_Sys_Menu_Bar_H

//
// End of "$Id$".
//
