//
// "$Id"
//
// Font definitions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

// Two internal fltk data structures:
//
// Fl_Fontdesc: an entry into the fl_font() table.  This entry may contain
// several "fonts" according to the system, for instance under X all the
// sizes are different X fonts, but only one fl_font.
//
// Fl_XFont: a structure for an actual system font, with junk to help
// choose it and info on character sizes.  Each Fl_Font has a linked
// list of these.  These are created the first time each system font
// is used.

#ifndef FL_FONT_
#define FL_FONT_

class Fl_XFont {
public:
  Fl_XFont *next;	// linked list of sizes of this style
#ifndef WIN32
  XFontStruct* font;	// X font information, 0 for display list
  XCharStruct* per_char;// modified from XFontStruct to have 0x20-0xff in it
  XCharStruct* free_this; // pointer saved for delete[]
  Fl_XFont(const char*, int);
#else
  HFONT fid;
  int width[256];
  TEXTMETRIC metr;
  Fl_XFont(const char*, int, int);
#endif
  int number;		// which slot in FL's font table
  int minsize;		// smallest point size that should use this
  int maxsize;		// largest point size that should use this
#if HAVE_GL
  unsigned int listbase;// base of display list, 0 = none
#endif
  ~Fl_XFont();
};

extern Fl_XFont *fl_current_xfont;
extern Fl_XFont *fl_fixed_xfont;

struct Fl_Fontdesc {
  const char *name;
  Fl_XFont *first;	// linked list of sizes of this style
#ifndef WIN32
  char **xlist;		// matched X font names
  int n;		// size of xlist, negative = don't free xlist!
#endif
};

extern Fl_Fontdesc *fl_fonts; // the table

#ifndef WIN32
// functions for parsing X font names:
const char* fl_font_word(const char *p, int n);
char *fl_find_fontsize(char *name);
#endif

void fl_draw(const char *, int x, int y, int w, int h, Fl_Align,
	     void (*callthis)(const char *, int n, int x, int y));

#endif

//
// End of "$Id: Fl_Font.H,v 1.2 1998/10/19 20:45:45 mike Exp $".
//
