//
// Code viewer widget for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_WIDGETS_CODE_VIEWER_H
#define FLUID_WIDGETS_CODE_VIEWER_H

// Syntax highlighting rewritten by erco@seriss.com 09/15/20.

//
// Include necessary headers...
//

#include "widgets/Code_Editor.h"

namespace fld {
namespace widget {

/**
 A widget derived from Code_Editor with highlighting for code blocks.

 This widget is used by the codeview system to show the design's
 source and header code. The secondary highlighting show the text
 part that corresponds to the selected widget(s).
 */
class Code_Viewer : public Code_Editor {
public:
  Code_Viewer(int X, int Y, int W, int H, const char *L = nullptr);

protected:
  void draw() override;

  /// Limit event handling to viewing, not editing
  int handle(int ev) override { return Fl_Text_Display::handle(ev); }
};

} // namespace widget
} // namespace fld

#endif // FLUID_WIDGETS_CODE_VIEWER_H
