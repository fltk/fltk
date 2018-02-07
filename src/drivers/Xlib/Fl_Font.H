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

#if USE_XFT
typedef struct _XftFont XftFont;
#else
#  include "../../Xutf8.h"
#endif // USE_XFT

class Fl_Xlib_Font_Descriptor : public Fl_Font_Descriptor {
public:
#  if USE_XFT
#    if USE_PANGO
        int descent_;
        int height_;
#    else
        XftFont* font;
#    endif
  int angle;
  FL_EXPORT Fl_Xlib_Font_Descriptor(const char* xfontname, Fl_Fontsize size, int angle);
#  else
  XUtf8FontStruct* font;	// X UTF-8 font information
  FL_EXPORT Fl_Xlib_Font_Descriptor(const char* xfontname);
#  endif
#  if HAVE_GL
  char glok[64];
#  endif // HAVE_GL
  FL_EXPORT ~Fl_Xlib_Font_Descriptor();
};

//extern FL_EXPORT Fl_Font_Descriptor *fl_fontsize; // the currently selected one

#if !USE_XFT
struct Fl_Xlib_Fontdesc { // replaces Fl_Fontdesc
  const char *name;
  char fontname[128];	// "Pretty" font name
  Fl_Font_Descriptor *first;	// linked list of sizes of this style
  char **xlist;		// matched X font names
  int n;		// size of xlist, negative = don't free xlist!
};
#endif

extern FL_EXPORT Fl_Fontdesc *fl_fonts; // the table

#endif

//
// End of "$Id$".
//
