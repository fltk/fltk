// fl_set_font.C

// Add a font to the internal table.
// Also see fl_set_fonts.C which adds all possible fonts.

#include <config.h>
#include <FL/Fl.H>
#include <FL/x.H>
#include "Fl_Font.H"
#include <stdlib.h>
#include <string.h>

static int table_size;

void Fl::set_font(Fl_Font fnum, const char *name) {
  if (fnum >= table_size) {
    int i = table_size;
    if (!i) {	// don't realloc the built-in table
      table_size = 2*FL_FREE_FONT;
      i = FL_FREE_FONT;
      Fl_Fontdesc *t = (Fl_Fontdesc*)malloc(table_size*sizeof(Fl_Fontdesc));
      memcpy(t, fl_fonts, FL_FREE_FONT*sizeof(Fl_Fontdesc));
      fl_fonts = t;
    } else {
      table_size = 2*table_size;
      fl_fonts=(Fl_Fontdesc*)realloc(fl_fonts, table_size*sizeof(Fl_Fontdesc));
    }
    for (; i < table_size; i++) fl_fonts[i].name = 0;
  }
  Fl_Fontdesc *s = fl_fonts+fnum;
  if (s->name) {
    if (!strcmp(s->name, name)) {s->name = name; return;}
#ifndef WIN32
    if (s->xlist && s->n >= 0) XFreeFontNames(s->xlist);
#endif
    for (Fl_XFont *f = s->first; f;) {
#ifndef WIN32
      if (f == fl_fixed_xfont) break;
#endif
      Fl_XFont *n = f->next; delete f; f = n;
    }
    s->first = 0;
  }
  s->name = name;
#ifndef WIN32
  s->xlist = 0;
#endif
  s->first = 0;
}

const char *Fl::get_font(Fl_Font fnum) {return fl_fonts[fnum].name;}
