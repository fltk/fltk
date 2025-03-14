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

// Syntax highlighting rewritten by erco@seriss.com 09/15/20.

//
// Include necessary headers...
//

#include "widgets/Code_Viewer.h"

using namespace fld;
using namespace fld::widget;

/**
 Create a fld::widget::Code_Viewer widget.
 \param[in] X, Y, W, H position and size of the widget
 \param[in] L optional label
 */
Code_Viewer::Code_Viewer(int X, int Y, int W, int H, const char *L)
: Code_Editor(X, Y, W, H, L)
{
  default_key_function(kf_ignore);
  remove_all_key_bindings(&key_bindings);
  cursor_style(CARET_CURSOR);
}

/**
 Tricking Fl_Text_Display into using bearable colors for this specific task.
 */
void Code_Viewer::draw()
{
  Fl_Color c = Fl::get_color(FL_SELECTION_COLOR);
  Fl::set_color(FL_SELECTION_COLOR, fl_color_average(FL_BACKGROUND_COLOR, FL_FOREGROUND_COLOR, 0.9f));
  Code_Editor::draw();
  Fl::set_color(FL_SELECTION_COLOR, c);
}


