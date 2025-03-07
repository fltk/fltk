//
// Widget Bin Button header file for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_WIDGETS_BIN_BUTTON_H
#define FLUID_WIDGETS_BIN_BUTTON_H

#include <FL/Fl_Button.H>

namespace fld {
namespace widget {

// Adding drag and drop for dragging widgets into windows.
class Bin_Button : public Fl_Button {
public:
  int handle(int) override;
  Bin_Button(int X,int Y,int W,int H, const char* l = nullptr) :
  Fl_Button(X,Y,W,H,l) { }
};

// Adding drag and drop functionality to drag window prototypes onto the desktop.
class Bin_Window_Button : public Fl_Button {
public:
  int handle(int) override;
  Bin_Window_Button(int X,int Y,int W,int H, const char* l = nullptr) :
  Fl_Button(X,Y,W,H,l) { }
};

} // namespace widget
} // namespace fld

#endif // FLUID_WIDGETS_BIN_BUTTON_H
