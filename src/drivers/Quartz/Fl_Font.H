//
// "$Id: Fl_Font.H 11506 2016-04-02 11:58:13Z manolo $"
//
// Font definitions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2011 by Bill Spitzak and others.
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
#include <ApplicationServices/ApplicationServices.h>

/**
   This a structure for an actual system font, with junk to
   help choose it and info on character sizes.  Each Fl_Fontdesc has a
   linked list of these.  These are created the first time each system
   font/size combination is used.
*/
class Fl_Font_Descriptor {
public:
  /** linked list for this Fl_Fontdesc */
  Fl_Font_Descriptor *next;	
  Fl_Fontsize size; /**< font size */
#ifndef FL_DOXYGEN // don't bother with platorm dependant details in the doc. 
  Fl_Font_Descriptor(const char* fontname, Fl_Fontsize size);
  ATSUTextLayout layout;
#     if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
  CTFontRef fontref;
  // the unicode span is divided in 512 blocks of 128 characters
  float *width[512]; // array of arrays of character widths
#     endif
  ATSUStyle style;
  short ascent, descent, q_width;
#  if HAVE_GL
  unsigned int listbase;// base of display list, 0 = none
#  endif // HAVE_GL

  FL_EXPORT ~Fl_Font_Descriptor();

#endif // FL_DOXYGEN
};

//extern FL_EXPORT Fl_Font_Descriptor *fl_fontsize; // the currently selected one

struct Fl_Fontdesc {
  const char *name;
  char fontname[128];	// "Pretty" font name
  Fl_Font_Descriptor *first;	// linked list of sizes of this style
};

extern FL_EXPORT Fl_Fontdesc *fl_fonts; // the table

#endif

//
// End of "$Id: Fl_Font.H 11506 2016-04-02 11:58:13Z manolo $".
//
