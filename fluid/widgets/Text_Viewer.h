//
// Text Viewer widget for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_WIDGETS_TEXT_VIEWER_H
#define FLUID_WIDGETS_TEXT_VIEWER_H

//
// Include necessary headers...
//

#include <FL/Fl_Text_Display.H>

namespace fld {
namespace widget {

/**
 A text viewer with an additional highlighting color scheme.
 */
class Text_Viewer : public Fl_Text_Display {
public:
  Text_Viewer(int X, int Y, int W, int H, const char *L = nullptr);
  ~Text_Viewer();
  void draw() override;

  /// access to protected member get_absolute_top_line_number()
  int top_line() { return get_absolute_top_line_number(); }
};

} // namespace widget
} // namespace fld

#endif // FLUID_WIDGETS_TEXT_VIEWER_H
