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

#if USE_XFT
typedef struct _XftFont XftFont;
#else
#  include "../../Xutf8.h"
#endif // USE_XFT

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
#  if USE_XFT
  XftFont* font;
  //const char* encoding;
  int angle;
  FL_EXPORT Fl_Font_Descriptor(const char* xfontname, Fl_Fontsize size, int angle);
#  else
  XUtf8FontStruct* font;	// X UTF-8 font information
  FL_EXPORT Fl_Font_Descriptor(const char* xfontname);
#  endif
#  if HAVE_GL
  unsigned int listbase;// base of display list, 0 = none
  char glok[64];
#  endif // HAVE_GL

  FL_EXPORT ~Fl_Font_Descriptor();

#endif // FL_DOXYGEN
};

//extern FL_EXPORT Fl_Font_Descriptor *fl_fontsize; // the currently selected one

struct Fl_Fontdesc {
  const char *name;
  char fontname[128];	// "Pretty" font name
  Fl_Font_Descriptor *first;	// linked list of sizes of this style
  char **xlist;		// matched X font names
  int n;		// size of xlist, negative = don't free xlist!
};

extern FL_EXPORT Fl_Fontdesc *fl_fonts; // the table

#endif

//
// End of "$Id: Fl_Font.H 11506 2016-04-02 11:58:13Z manolo $".
//
