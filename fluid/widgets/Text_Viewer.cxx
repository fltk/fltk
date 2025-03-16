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

//
// Include necessary headers...
//

#include "widgets/Text_Viewer.h"

using namespace fld;
using namespace fld::widget;

/**
 Create a fld::widget::Text_Viewer widget.
 \param[in] X, Y, W, H position and size of the widget
 \param[in] L optional label
 */
Text_Viewer::Text_Viewer(int X, int Y, int W, int H, const char *L)
: Fl_Text_Display(X, Y, W, H, L)
{
  buffer(new Fl_Text_Buffer);
}

/**
 Avoid memory leaks.
 */
Text_Viewer::~Text_Viewer() {
  Fl_Text_Buffer *buf = mBuffer;
  buffer(nullptr);
  delete buf;
}

/**
 Tricking Fl_Text_Display into using bearable colors for this specific task.
 */
void Text_Viewer::draw()
{
  Fl_Color c = Fl::get_color(FL_SELECTION_COLOR);
  Fl::set_color(FL_SELECTION_COLOR, fl_color_average(FL_BACKGROUND_COLOR, FL_FOREGROUND_COLOR, 0.9f));
  Fl_Text_Display::draw();
  Fl::set_color(FL_SELECTION_COLOR, c);
}


