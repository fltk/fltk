//
// "$Id: Fl_Browser.cxx,v 1.9.2.12 2001/01/22 15:13:39 easysw Exp $"
//
// Browser widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Browser.H>
#include <FL/fl_draw.H>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// I modified this from the original Forms data to use a linked list
// so that the number of items in the browser and size of those items
// is unlimited.  The only problem is that the old browser used an
// index number to identify a line, and it is slow to convert from/to
// a pointer.  I use a cache of the last match to try to speed this
// up.

// Also added the ability to "hide" a line.  This set's it's height to
// zero, so the Fl_Browser_ cannot pick it.

#define SELECTED 1
#define NOTDISPLAYED 2

struct FL_BLINE {	// data is in a linked list of these
  FL_BLINE* prev;
  FL_BLINE* next;
  void* data;
  short length;		// sizeof(txt)-1, may be longer than string
  char flags;		// selected, displayed
  char txt[1];		// start of allocated array
};

void* Fl_Browser::item_first() const {return first;}

void* Fl_Browser::item_next(void* l) const {return ((FL_BLINE*)l)->next;}

void* Fl_Browser::item_prev(void* l) const {return ((FL_BLINE*)l)->prev;}

int Fl_Browser::item_selected(void* l) const {
  return ((FL_BLINE*)l)->flags&SELECTED;}

void Fl_Browser::item_select(void* l, int v) {
  if (v) ((FL_BLINE*)l)->flags |= SELECTED;
  else ((FL_BLINE*)l)->flags &= ~SELECTED;
}

FL_BLINE* Fl_Browser::find_line(int line) const {
  int n; FL_BLINE* l;
  if (line == cacheline) return cache;
  if (cacheline && line > (cacheline/2) && line < ((cacheline+lines)/2)) {
    n = cacheline; l = cache;
  } else if (line <= (lines/2)) {
    n = 1; l = first;
  } else {
    n = lines; l = last;
  }
  for (; n < line && l; n++) l = l->next;
  for (; n > line && l; n--) l = l->prev;
  ((Fl_Browser*)this)->cacheline = line;
  ((Fl_Browser*)this)->cache = l;
  return l;
}

int Fl_Browser::lineno(void* v) const {
  FL_BLINE* l = (FL_BLINE*)v;
  if (!l) return 0;
  if (l == cache) return cacheline;
  if (l == first) return 1;
  if (l == last) return lines;
  if (!cache) {
    ((Fl_Browser*)this)->cache = first;
    ((Fl_Browser*)this)->cacheline = 1;
  }
  // assumme it is near cache, search both directions:
  FL_BLINE* b = cache->prev;
  int bnum = cacheline-1;
  FL_BLINE* f = cache->next;
  int fnum = cacheline+1;
  int n = 0;
  for (;;) {
    if (b == l) {n = bnum; break;}
    if (f == l) {n = fnum; break;}
    if (b) {b = b->prev; bnum--;}
    if (f) {f = f->next; fnum++;}
  }
  ((Fl_Browser*)this)->cache = l;
  ((Fl_Browser*)this)->cacheline = n;
  return n;
}

FL_BLINE* Fl_Browser::_remove(int line) {
  FL_BLINE* ttt = find_line(line);
  deleting(ttt);

  cacheline = line-1;
  cache = ttt->prev;
  if (ttt->prev) ttt->prev->next = ttt->next;
  else first = ttt->next;
  if (ttt->next) ttt->next->prev = ttt->prev;
  else last = ttt->prev;

  lines--;
  full_height_ -= item_height(ttt);
  return(ttt);
}

void Fl_Browser::remove(int line) {
  if (line < 1 || line > lines) return;
  free(_remove(line));
}

void Fl_Browser::insert(int line, FL_BLINE* t) {
  if (!first) {
    t->prev = t->next = 0;
    first = last = t;
  } else if (line <= 1) {
    inserting(first, t);
    t->prev = 0;
    t->next = first;
    t->next->prev = t;
    first = t;
  } else if (line > lines) {
    t->prev = last;
    t->prev->next = t;
    t->next = 0;
    last = t;
  } else {
    FL_BLINE* n = find_line(line);
    inserting(n, t);
    t->next = n;
    t->prev = n->prev;
    t->prev->next = t;
    n->prev = t;
  }
  cacheline = line;
  cache = t;
  lines++;
  full_height_ += item_height(t);
  redraw_line(t);
}

void Fl_Browser::insert(int line, const char* newtext, void* data) {
  int l = strlen(newtext);
  FL_BLINE* t = (FL_BLINE*)malloc(sizeof(FL_BLINE)+l);
  t->length = l;
  t->flags = 0;
  strcpy(t->txt, newtext);
  t->data = data;
  insert(line, t);
}

void Fl_Browser::move(int to, int from) {
  if (from < 1 || from > lines) return;
  insert(to, _remove(from));
}

void Fl_Browser::text(int line, const char* newtext) {
  if (line < 1 || line > lines) return;
  FL_BLINE* t = find_line(line);
  int l = strlen(newtext);
  if (l > t->length) {
    FL_BLINE* n = (FL_BLINE*)malloc(sizeof(FL_BLINE)+l);
    replacing(t, n);
    cache = n;
    n->data = t->data;
    n->length = l;
    n->flags = t->flags;
    n->prev = t->prev;
    if (n->prev) n->prev->next = n; else first = n;
    n->next = t->next;
    if (n->next) n->next->prev = n; else last = n;
    free(t);
    t = n;
  }
  strcpy(t->txt, newtext);
  redraw_line(t);
}

void Fl_Browser::data(int line, void* data) {
  if (line < 1 || line > lines) return;
  find_line(line)->data = data;
}

int Fl_Browser::item_height(void* lv) const {
  FL_BLINE* l = (FL_BLINE*)lv;
  if (l->flags & NOTDISPLAYED) return 0;

  int hmax = 2; // use 2 to insure we don't return a zero!

  if (!l->txt[0]) {
    // For blank lines set the height to exactly 1 line!
    fl_font(textfont(), textsize());
    int h = fl_height();
    if (h > hmax) hmax = h;
  }
  else {
    // do each column separately as they may all set different fonts:
    for (char* str = l->txt; *str; str++) {
      Fl_Font font = textfont(); // default font
      int size = textsize(); // default size
      while (*str==format_char()) {
	str++;
	switch (*str++) {
	case 'l': case 'L': size = 24; break;
	case 'm': case 'M': size = 18; break;
	case 's': size = 11; break;
	case 'b': font = (Fl_Font)(font|FL_BOLD); break;
	case 'i': font = (Fl_Font)(font|FL_ITALIC); break;
	case 'f': case 't': font = FL_COURIER; break;
	case 'B':
	case 'C': strtol(str, &str, 10); break;// skip a color number
	case 'F': font = (Fl_Font)strtol(str,&str,10); break;
	case 'S': size = strtol(str,&str,10); break;
	case 0: case '@': str--;
	case '.': goto END_FORMAT;
	}
      }
      END_FORMAT:
      char* ptr = str;
      for(;*str && (*str!=column_char()); str++) ;
      if (ptr < str) {
	fl_font(font, size); int h = fl_height();
	if (h > hmax) hmax = h;
      }
      if (!*str) str --;
    }
  }

  return hmax; // previous version returned hmax+2!
}

int Fl_Browser::item_width(void* v) const {
  char* str = ((FL_BLINE*)v)->txt;
  const int* i = column_widths();
  int w = 0;

  while (*i) { // add up all tab-seperated fields
    w += *i++;
    char* e;
    for (e = str; *e && *e != column_char(); e++);
    if (!*e) return 0; // last one occupied by text
    str = e+1;
  }

  // OK, we gotta parse the string and find the string width...
  int size = textsize();
  Fl_Font font = textfont();
  int done = 0;

  // MRS - might this cause problems on some platforms - order of operations?

  while (*str == format_char_ && *++str && *str != format_char_) {
    switch (*str++) {
    case 'l': case 'L': size = 24; break;
    case 'm': case 'M': size = 18; break;
    case 's': size = 11; break;
    case 'b': font = (Fl_Font)(font|FL_BOLD); break;
    case 'i': font = (Fl_Font)(font|FL_ITALIC); break;
    case 'f': case 't': font = FL_COURIER; break;
    case 'B':
    case 'C': strtol(str, &str, 10); break;// skip a color number
    case 'F': font = (Fl_Font)strtol(str, &str, 10); break;
    case 'S': size = strtol(str, &str, 10); break;
    case '.':
      done = 1;
    case '@':
      str--;
      done = 1;
    }

    if (done)
      break;
  }

  fl_font(font, size);
  return w + int(fl_width(str)) + 6;
}

int Fl_Browser::full_height() const {
  return full_height_;
}

int Fl_Browser::incr_height() const {
  return textsize()+2;
}

void Fl_Browser::item_draw(void* v, int x, int y, int w, int h) const {
  char* str = ((FL_BLINE*)v)->txt;
  const int* i = column_widths();

  while (w > 6) {	// do each tab-seperated field
    int w1 = w;	// width for this field
    char* e = 0; // pointer to end of field or null if none
    if (*i) { // find end of field and temporarily replace with 0
      for (e = str; *e && *e != column_char(); e++);
      if (*e) {*e = 0; w1 = *i++;} else e = 0;
    }
    int size = textsize();
    Fl_Font font = textfont();
    Fl_Color lcol = textcolor();
    Fl_Align align = FL_ALIGN_LEFT;
    // check for all the @-lines recognized by XForms:
    while (*str == format_char() && *++str && *str != format_char()) {
      switch (*str++) {
      case 'l': case 'L': size = 24; break;
      case 'm': case 'M': size = 18; break;
      case 's': size = 11; break;
      case 'b': font = (Fl_Font)(font|FL_BOLD); break;
      case 'i': font = (Fl_Font)(font|FL_ITALIC); break;
      case 'f': case 't': font = FL_COURIER; break;
      case 'c': align = FL_ALIGN_CENTER; break;
      case 'r': align = FL_ALIGN_RIGHT; break;
      case 'B': 
	if (!(((FL_BLINE*)v)->flags & SELECTED)) {
	  fl_color((Fl_Color)strtol(str, &str, 10));
	  fl_rectf(x, y, w1, h);
	} else strtol(str, &str, 10);
        break;
      case 'C':
	lcol = (Fl_Color)strtol(str, &str, 10);
	break;
      case 'F':
	font = (Fl_Font)strtol(str, &str, 10);
	break;
      case 'N':
	lcol = FL_INACTIVE_COLOR;
	break;
      case 'S':
	size = strtol(str, &str, 10);
	break;
      case '-':
	fl_color(FL_DARK3);
	fl_line(x+3, y+h/2, x+w1-3, y+h/2);
	fl_color(FL_LIGHT3);
	fl_line(x+3, y+h/2+1, x+w1-3, y+h/2+1);
	break;
      case 'u':
      case '_':
	fl_color(lcol);
	fl_line(x+3, y+h-1, x+w1-3, y+h-1);
	break;
      case '.':
	goto BREAK;
      case '@':
	str--; goto BREAK;
      }
    }
  BREAK:
    fl_font(font, size);
    if (((FL_BLINE*)v)->flags & SELECTED)
      lcol = contrast(lcol, selection_color());
    if (!active_r()) lcol = inactive(lcol);
    fl_color(lcol);
    fl_draw(str, x+3, y, w1-6, h, e ? Fl_Align(align|FL_ALIGN_CLIP) : align);
    if (!e) break; // no more fields...
    *e = column_char(); // put the seperator back
    x += w1;
    w -= w1;
    str = e+1;
  }
}

static const int no_columns[1] = {0};

Fl_Browser::Fl_Browser(int x, int y, int w, int h, const char*l)
  : Fl_Browser_(x, y, w, h, l) {
  column_widths_ = no_columns;
  lines = 0;
  full_height_ = 0;
  cacheline = 0;
  format_char_ = '@';
  column_char_ = '\t';
  first = last = cache = 0;
}

void Fl_Browser::lineposition(int line, Fl_Line_Position pos) {
  if (line<1) line = 1;
  if (line>lines) line = lines;
  int p = 0;

  FL_BLINE* l;
  for (l=first; l && line>1; l = l->next) {
    line--; p += item_height(l);
  }
  if (l && (pos == BOTTOM)) p += item_height (l);

  int final = p, X, Y, W, H;
  bbox(X, Y, W, H);

  switch(pos) {
    case TOP: break;
    case BOTTOM: final -= H; break;
    case MIDDLE: final -= H/2; break;
  }
  
  if (final > (full_height() - H)) final = full_height() -H;
  position(final);
}

int Fl_Browser::topline() const {
  return lineno(top());
}

void Fl_Browser::clear() {
  for (FL_BLINE* l = first; l;) {
    FL_BLINE* h = l->next;
    free(l);
    l = h;
  }
  full_height_ = 0;
  first = 0;
  lines = 0;
  new_list();
}

void Fl_Browser::add(const char* newtext, void* data) {
  insert(lines+1, newtext, data);
  //Fl_Browser_::display(last);
}

const char* Fl_Browser::text(int line) const {
  if (line < 1 || line > lines) return 0;
  return find_line(line)->txt;
}

void* Fl_Browser::data(int line) const {
  if (line < 1 || line > lines) return 0;
  return find_line(line)->data;
}

int Fl_Browser::select(int line, int value) {
  if (line < 1 || line > lines) return 0;
  return Fl_Browser_::select(find_line(line), value);
}

int Fl_Browser::selected(int line) const {
  if (line < 1 || line > lines) return 0;
  return find_line(line)->flags & SELECTED;
}

void Fl_Browser::show(int line) {
  FL_BLINE* t = find_line(line);
  if (t->flags & NOTDISPLAYED) {
    t->flags &= ~NOTDISPLAYED;
    full_height_ += item_height(t);
    if (Fl_Browser_::displayed(t)) redraw_lines();
  }
}

void Fl_Browser::hide(int line) {
  FL_BLINE* t = find_line(line);
  if (!(t->flags & NOTDISPLAYED)) {
    full_height_ -= item_height(t);
    t->flags |= NOTDISPLAYED;
    if (Fl_Browser_::displayed(t)) redraw_lines();
  }
}

void Fl_Browser::display(int line, int value) {
  if (line < 1 || line > lines) return;
  if (value) show(line); else hide(line);
}

int Fl_Browser::visible(int line) const {
  if (line < 1 || line > lines) return 0;
  return !(find_line(line)->flags&NOTDISPLAYED);
}

int Fl_Browser::value() const {
  return lineno(selection());
}

//
// End of "$Id: Fl_Browser.cxx,v 1.9.2.12 2001/01/22 15:13:39 easysw Exp $".
//
