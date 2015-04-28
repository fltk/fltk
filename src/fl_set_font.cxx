//
// "$Id$"
//
// Font utilities for the Fast Light Tool Kit (FLTK).
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

// Add a font to the internal table.
// Also see fl_set_fonts.cxx which adds all possible fonts.

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/fl_draw.H>
#include "flstring.h"
#include "Fl_Font.H"
#include <stdlib.h>

static int table_size;
/**
  Changes a face.  The string pointer is simply stored,
  the string is not copied, so the string must be in static memory.
*/    
void Fl::set_font(Fl_Font fnum, const char* name) {
#ifdef __APPLE__
  if (!fl_fonts) fl_fonts = Fl_X::calc_fl_fonts();
#endif
  while (fnum >= table_size) {
    int i = table_size;
    if (!i) {	// don't realloc the built-in table
      table_size = 2*FL_FREE_FONT;
      i = FL_FREE_FONT;
      Fl_Fontdesc* t = (Fl_Fontdesc*)malloc(table_size*sizeof(Fl_Fontdesc));
      memcpy(t, fl_fonts, FL_FREE_FONT*sizeof(Fl_Fontdesc));
      fl_fonts = t;
    } else {
      table_size = 2*table_size;
      fl_fonts=(Fl_Fontdesc*)realloc(fl_fonts, table_size*sizeof(Fl_Fontdesc));
    }
    for (; i < table_size; i++) {
      fl_fonts[i].fontname[0] = 0;
      fl_fonts[i].name = 0;
#if !defined(WIN32) && !defined(__APPLE__)
      fl_fonts[i].xlist = 0;
      fl_fonts[i].n = 0;
#endif // !WIN32 && !__APPLE__
    }
  }
  Fl_Fontdesc* s = fl_fonts+fnum;
  if (s->name) {
    if (!strcmp(s->name, name)) {s->name = name; return;}
#if !defined(WIN32) && !defined(__APPLE__)
    if (s->xlist && s->n >= 0) XFreeFontNames(s->xlist);
#endif
    for (Fl_Font_Descriptor* f = s->first; f;) {
      Fl_Font_Descriptor* n = f->next; delete f; f = n;
    }
    s->first = 0;
  }
  s->name = name;
  s->fontname[0] = 0;
#if !defined(WIN32) && !defined(__APPLE__)
  s->xlist = 0;
#endif
  s->first = 0;
  Fl_Display_Device::display_device()->driver()->font(-1, 0);
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
#ifdef __APPLE__
  if (!fl_fonts) fl_fonts = Fl_X::calc_fl_fonts();
#endif
  return fl_fonts[fnum].name;
}

//
// End of "$Id$".
//
