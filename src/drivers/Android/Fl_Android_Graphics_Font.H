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

#ifndef FL_ANDROID_GRAPHICS_FONT_H
#define FL_ANDROID_GRAPHICS_FONT_H


#include "Fl_Android_Graphics_Driver.H"
#include "stb_truetype.h"


class Fl_Android_Font_Descriptor : public Fl_Font_Descriptor
{
private:
  stbtt_fontinfo pFont;
  Fl_Font pFontIndex;
  uint8_t *pFileBuffer;

public:
  Fl_Android_Font_Descriptor(const char* fontname, Fl_Font fnum, Fl_Fontsize size);
  uint8_t *get_bitmap(uint32_t c, int *w, int *h, int *dx, int *dy);
  void free_bitmap(uint8_t *bitmap);
  float get_advance(uint32_t c);
};



#if 0

fl_fonts array is managed by core?!

  /** Some platforms may need to implement this to support fonts */
virtual Fl_Fontdesc* calc_fl_fonts(void) {return NULL;}

 typedef int Fl_Fontsize;
 class Fl_Font_Descriptor;
 struct Fl_Fontdesc;

 /* This class is not part of FLTK's public API.
 Platforms usually define a derived class called Fl_XXX_Font_Descriptor
 containing extra platform-specific data/functions.
 This is a class for an actual system font, with junk to
 help choose it and info on character sizes.  Each Fl_Fontdesc has a
 linked list of these.  These are created the first time each system
 font/size combination is used.
 */
class Fl_Font_Descriptor {
public:
  /** linked list for this Fl_Fontdesc */
  Fl_Font_Descriptor *next;
  Fl_Fontsize size; /**< font size */
  Fl_Font_Descriptor(const char* fontname, Fl_Fontsize size);
  FL_EXPORT ~Fl_Font_Descriptor() {}
  short ascent, descent, q_width;
  unsigned int listbase;// base of display list, 0 = none
};

// This struct is not part of FLTK's public API.
struct Fl_Fontdesc {
  const char *name;
  char fontname[128];  // "Pretty" font name
  Fl_Font_Descriptor *first;  // linked list of sizes of this style
};

#endif

#if 0
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

  #endif


#endif // FL_ANDROID_GRAPHICS_FONT_H

//
// End of "$Id$".
//
