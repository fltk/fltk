//
// "$Id: Fl_Browser.H,v 1.8.2.7 2001/01/22 15:13:37 easysw Exp $"
//
// Browser header file for the Fast Light Tool Kit (FLTK).
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

// Forms-compatable browser.  Probably useful for other
// lists of textual data.  Notice that the line numbers
// start from 1, and 0 means "no line".

#ifndef Fl_Browser_H
#define Fl_Browser_H

#include "Fl_Browser_.H"

struct FL_BLINE;

class Fl_Browser : public Fl_Browser_ {

  FL_BLINE *first;		// the array of lines
  FL_BLINE *last;
  FL_BLINE *cache;
  int cacheline;		// line number of cache
  int lines;                	// Number of lines
  int full_height_;
  const int* column_widths_;
  char format_char_;		// alternative to @-sign
  char column_char_;		// alternative to tab

protected:

  // required routines for Fl_Browser_ subclass:
  FL_EXPORT void* item_first() const ;
  FL_EXPORT void* item_next(void*) const ;
  FL_EXPORT void* item_prev(void*) const ;
  FL_EXPORT int item_selected(void*) const ;
  FL_EXPORT void item_select(void*, int);
  FL_EXPORT int item_height(void*) const ;
  FL_EXPORT int item_width(void*) const ;
  FL_EXPORT void item_draw(void*, int, int, int, int) const ;
  FL_EXPORT int full_height() const ;
  FL_EXPORT int incr_height() const ;

  FL_EXPORT FL_BLINE* find_line(int) const ;
  FL_EXPORT FL_BLINE* _remove(int) ;
  FL_EXPORT void insert(int, FL_BLINE*);
  FL_EXPORT int lineno(void*) const ;

public:

  FL_EXPORT void remove(int);
  FL_EXPORT void add(const char*, void* = 0);
  FL_EXPORT void insert(int, const char*, void* = 0);
  FL_EXPORT void move(int to, int from);
  FL_EXPORT int  load(const char* filename);
  FL_EXPORT void clear();

  int size() const {return lines;}
  
  FL_EXPORT int topline() const ;
  enum Fl_Line_Position { TOP, BOTTOM, MIDDLE };
  FL_EXPORT void lineposition(int, Fl_Line_Position);
  void topline(int l) { lineposition(l, TOP); }
  void bottomline(int l) { lineposition(l, BOTTOM); }
  void middleline(int l) { lineposition(l, MIDDLE); }

  FL_EXPORT int select(int, int=1);
  FL_EXPORT int selected(int) const ;
  FL_EXPORT void show(int n);
  void show() {Fl_Widget::show();}
  FL_EXPORT void hide(int n);
  void hide() {Fl_Widget::hide();}
  FL_EXPORT int visible(int n) const ;

  FL_EXPORT int value() const ;
  void value(int v) {select(v);}
  FL_EXPORT const char* text(int) const ;
  FL_EXPORT void text(int, const char*);
  FL_EXPORT void* data(int) const ;
  FL_EXPORT void data(int, void* v);

  FL_EXPORT Fl_Browser(int, int, int, int, const char* = 0);
  ~Fl_Browser() { clear(); }

  char format_char() const {return format_char_;}
  void format_char(char c) {format_char_ = c;}
  char column_char() const {return column_char_;}
  void column_char(char c) {column_char_ = c;}
  const int* column_widths() const {return column_widths_;}
  void column_widths(const int* l) {column_widths_ = l;}

  int displayed(int n) const {return Fl_Browser_::displayed(find_line(n));}
  void make_visible(int n) {Fl_Browser_::display(find_line(n));}

  // for back compatability only:
  void replace(int a, const char* b) {text(a, b);}
  FL_EXPORT void display(int, int=1);
};

#endif

//
// End of "$Id: Fl_Browser.H,v 1.8.2.7 2001/01/22 15:13:37 easysw Exp $".
//
