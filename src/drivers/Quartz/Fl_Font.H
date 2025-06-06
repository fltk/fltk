//
// Font definitions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

// Two internal fltk data structures:
//
// Fl_Fontdesc: an entry into the fl_font() table.  There is one of these
// for each fltk font number.
//
#ifndef FL_FONT_
#define FL_FONT_

#include <config.h>
#include "Fl_Quartz_Graphics_Driver.H"
#include "../../Fl_Scalable_Graphics_Driver.H" // Fl_Font_Descriptor
#include <ApplicationServices/ApplicationServices.h>

class Fl_Quartz_Font_Descriptor : public Fl_Font_Descriptor {
public:
  Fl_Quartz_Font_Descriptor(const char* fontname, Fl_Fontsize size);
  virtual FL_EXPORT ~Fl_Quartz_Font_Descriptor();
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
  CTFontRef fontref;
  // the unicode span is divided in 512 blocks of 128 characters
  float *width[512]; // array of arrays of character widths
# endif
# if HAS_ATSU
  ATSUTextLayout layout;
  ATSUStyle style;
# endif
  short q_width;
};

extern FL_EXPORT Fl_Fontdesc *fl_fonts; // the table

#endif
