//
// "$Id: fl_set_fonts_xft.cxx,v 1.1.2.1 2002/03/06 18:11:01 easysw Exp $"
//
// More font utilities for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2002 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <X11/Xft/Xft.h>

// This function fills in the fltk font table with all the fonts that
// are found on the X server.  It tries to place the fonts into families
// and to sort them so the first 4 in a family are normal, bold, italic,
// and bold italic.

// turn a stored font name into a pretty name:
const char* Fl::get_font_name(Fl_Font fnum, int* ap) {
  const char* p = fl_fonts[fnum].name;
  int type;
  switch (p[0]) {
  case 'B': type = FL_BOLD; break;
  case 'I': type = FL_ITALIC; break;
  case 'P': type = FL_BOLD | FL_ITALIC; break;
  default:  type = 0; break;
  }
  if (ap) {*ap = type; return p+1;}
  if (!type) {return p+1;}
  static char *buffer = new char[128];
  strcpy(buffer, p+1);
  if (type & FL_BOLD) strcat(buffer, " bold");
  if (type & FL_ITALIC) strcat(buffer, " italic");
  return buffer;
}

#if 0
extern "C" {
static int sort_function(const void *aa, const void *bb) {
  const char* name_a = (*(Fl_Fontdesc**)aa)->name;
  const char* name_b = (*(Fl_Fontdesc**)bb)->name;
  int ret = strcasecmp(name_a+1, name_b+1); if (ret) return ret;
  return name_a[0]-name_b[0]; // sort by attribute
}
}

static Fl_Fontdesc* make_a_font(char attrib, const char* name) {
  Fl_Fontdesc* newfont = new Fl_Fontdesc;
  char *n = new char[strlen(name)+2];
  n[0] = attrib;
  strcpy(n+1, name);
  newfont->name = n;
  newfont->first = 0;
  return newfont;
}
#endif // 0


Fl_Font Fl::set_fonts(const char* xstarname) {
  // TODO: implement this for Xft...
  return FL_FREE_FONT;
}


extern "C" {
static int int_sort(const void *aa, const void *bb) {
  return (*(int*)aa)-(*(int*)bb);
}
}

////////////////////////////////////////////////////////////////

// Return all the point sizes supported by this font:
// Suprisingly enough Xft works exactly like fltk does and returns
// the same list. Except there is no way to tell if the font is scalable.
int Fl::get_font_sizes(Fl_Font fnum, int*& sizep) {
  Fl_Fontdesc *s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // empty slot in table, use entry 0

  fl_open_display();
  XftFontSet* fs = XftListFonts(fl_display, fl_screen,
				XFT_FAMILY, XftTypeString, s->name+1, 0,
				XFT_PIXEL_SIZE, 0);
  static int* array = 0;
  static int array_size = 0;
  if (fs->nfont >= array_size) {
    delete[] array;
    array = new int[array_size = fs->nfont+1];
  }
  array[0] = 0; int j = 1; // claim all fonts are scalable
  for (int i = 0; i < fs->nfont; i++) {
    double v;
    if (XftPatternGetDouble(fs->fonts[i], XFT_PIXEL_SIZE, 0, &v) == XftResultMatch) {
      array[j++] = int(v);
    }
  }
  qsort(array+1, j-1, sizeof(int), int_sort);
  XftFontSetDestroy(fs);
  sizep = array;
  return j;
}

//
// End of "$Id: fl_set_fonts_xft.cxx,v 1.1.2.1 2002/03/06 18:11:01 easysw Exp $".
//
