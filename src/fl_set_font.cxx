//
// "$Id$"
//
// Font utilities for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

// Add a font to the internal table.
// Also see fl_set_fonts.cxx which adds all possible fonts.

#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Screen_Driver.H>
#include "flstring.h"
#include <stdlib.h>

struct Fl_Fontdesc;
extern FL_EXPORT Fl_Fontdesc *fl_fonts; // the table

static int table_size;
/**
  Changes a face.  The string pointer is simply stored,
  the string is not copied, so the string must be in static memory.
*/    
void Fl::set_font(Fl_Font fnum, const char* name) {
  Fl_Graphics_Driver &d = Fl_Graphics_Driver::default_driver();
  unsigned width = d.font_desc_size();
  if (!fl_fonts) fl_fonts = d.calc_fl_fonts();
  while (fnum >= table_size) {
    int i = table_size;
    if (!i) {	// don't realloc the built-in table
      table_size = 2*FL_FREE_FONT;
      i = FL_FREE_FONT;
      Fl_Fontdesc* t = (Fl_Fontdesc*)malloc(table_size*width);
      memcpy(t, fl_fonts, FL_FREE_FONT*width);
      fl_fonts = t;
    } else {
      table_size = 2*table_size;
      fl_fonts=(Fl_Fontdesc*)realloc(fl_fonts, table_size*width);
    }
    for (; i < table_size; i++) {
      memset((char*)fl_fonts + i * width, 0, width);
    }
  }
  d.font_name(fnum, name);
  d.font(-1, 0);
}

/** Copies one face to another. */
void Fl::set_font(Fl_Font fnum, Fl_Font from) {
  Fl::set_font(fnum, get_font(from));
}

/**
    Gets the string for this face.  This string is different for each
    face. Under X this value is passed to XListFonts to get all the sizes
    of this face.
*/
const char* Fl::get_font(Fl_Font fnum) {
  return Fl_Graphics_Driver::default_driver().font_name(fnum);
}

//
// End of "$Id$".
//
