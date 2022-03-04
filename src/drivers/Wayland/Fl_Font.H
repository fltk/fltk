//
// Font definitions for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021 by Bill Spitzak and others.
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

#ifndef FL_FONT_
#define FL_FONT_

#include <config.h>
#include "Fl_Wayland_Graphics_Driver.H"

class Fl_Wayland_Font_Descriptor : public Fl_Font_Descriptor {
public:
  Fl_Wayland_Font_Descriptor(const char* fontname, Fl_Fontsize size);
  FL_EXPORT ~Fl_Wayland_Font_Descriptor();
  PangoFontDescription *fontref;
  int **width; // array of arrays of character widths
};

extern FL_EXPORT Fl_Fontdesc *fl_fonts; // the table

#endif // FL_FONT_
