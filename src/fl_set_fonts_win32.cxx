//
// "$Id"
//
// WIN32 font utilities for the Fast Light Tool Kit (FLTK).
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

// This function fills in the fltk font table with all the fonts that
// are found on the X server.  It tries to place the fonts into families
// and to sort them so the first 4 in a family are normal, bold, italic,
// and bold italic.

#include <FL/Fl.H>
#include <FL/win32.H>
#include "Fl_Font.H"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

// turn a stored font name into a pretty name:
const char* Fl::get_font_name(Fl_Font fnum, int* ap) {
  const char* p = fl_fonts[fnum].name;
  if (!p || !*p) {if (ap) *ap = 0; return "";}
  if (ap) switch (*p) {
  case 'B': *ap = FL_BOLD; break;
  case 'I': *ap = FL_ITALIC; break;
  case 'P': *ap = FL_BOLD | FL_ITALIC; break;
  default:  *ap = 0; break;
  }
  return p+1;
}

static int fl_free_font = FL_FREE_FONT;

static int CALLBACK enumcb(ENUMLOGFONT FAR *lpelf,
  NEWTEXTMETRIC FAR *lpntm, int FontType, LPARAM p) {
  if (!p && lpelf->elfLogFont.lfCharSet != ANSI_CHARSET) return 1;
  char *n = (char*)(lpelf->elfFullName);
  for (int i=0; i<FL_FREE_FONT; i++) // skip if one of our built-in fonts
    if (!strcmp(Fl::get_font_name((Fl_Font)i),n)) return 1;
  char buffer[128];
  strcpy(buffer+1, n);
  buffer[0] = ' '; Fl::set_font((Fl_Font)(fl_free_font++), strdup(buffer));
  if (lpelf->elfLogFont.lfWeight <= 400)
    buffer[0] = 'B', Fl::set_font((Fl_Font)(fl_free_font++), strdup(buffer));
  buffer[0] = 'I'; Fl::set_font((Fl_Font)(fl_free_font++), strdup(buffer));
  if (lpelf->elfLogFont.lfWeight <= 400)
    buffer[0] = 'P', Fl::set_font((Fl_Font)(fl_free_font++), strdup(buffer));
  return 1;
}

Fl_Font Fl::set_fonts(const char* xstarname) {
  EnumFontFamilies(fl_gc, NULL, (FONTENUMPROC)enumcb, xstarname != 0);
  return (Fl_Font)fl_free_font;
}

int Fl::get_font_sizes(Fl_Font fnum, int*& sizep) {
  // pretend all fonts are scalable (most are and I don't know how
  // to tell anyways)
  static int array[1];
  sizep = array;
  return 1;
}

//
// End of "$Id: fl_set_fonts_win32.cxx,v 1.2 1998/10/19 20:46:45 mike Exp $".
//
