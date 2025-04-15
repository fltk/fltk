//
// Application Menu Bar widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 2025 by Bill Spitzak and others.
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

#ifndef FLUID_WIDGETS_APP_MENU_BAR_H
#define FLUID_WIDGETS_APP_MENU_BAR_H

//
// Include necessary headers...
//

#include <FL/Fl_Menu_Bar.H>

namespace fld {
namespace widget {

/**
 A text viewer with an additional highlighting color scheme.
 */
class App_Menu_Bar : public Fl_Menu_Bar {
public:
  App_Menu_Bar(int X, int Y, int W, int H, const char *L = nullptr);
  int handle(int event) override;
};

} // namespace widget
} // namespace fld

#endif // FLUID_WIDGETS_APP_MENU_BAR_H
