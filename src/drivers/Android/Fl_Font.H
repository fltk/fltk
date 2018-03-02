//
// "$Id$"
//
// Font definitions for the Fast Light Tool Kit (FLTK).
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

// Two internal fltk data structures:
//
// Fl_Fontdesc: an entry into the fl_font() table.  There is one of these
// for each fltk font number.
//
#ifndef FL_FONT_
#define FL_FONT_

#include <config.h>

#if 0

class Fl_GDI_Font_Descriptor : public Fl_Font_Descriptor {
public:
  HFONT fid;
  int *width[64];
  TEXTMETRIC metr;
  int angle;
  FL_EXPORT Fl_GDI_Font_Descriptor(const char* fontname, Fl_Fontsize size);
#  if HAVE_GL
  char glok[64];
#  endif // HAVE_GL
  FL_EXPORT ~Fl_GDI_Font_Descriptor();
};

#endif

extern FL_EXPORT Fl_Fontdesc *fl_fonts; // the table

#endif

//
// End of "$Id$".
//
