//
// "$Id: Fl_Browser.H,v 1.5 1998/11/05 16:04:37 mike Exp $"
//
// Browser header file for the Fast Light Tool Kit (FLTK).
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

// Forms-compatable browser.  Probably useful for other
// lists of textual data.  Notice that the line numbers
// start from 1, and 0 means "no line".

#ifndef Fl_Browser_H
#define Fl_Browser_H

#include "Fl_Browser_.H"

struct FL_BLINE;

class Fl_Browser : public Fl_Browser_ {

  // required routines for Fl_Browser_ subclass:
  void* item_first() const ;
  void* item_next(void*) const ;
  void* item_prev(void*) const ;
  int item_selected(void*) const ;
  void item_select(void*, int);
  int item_height(void*) const ;
  int item_width(void*) const ;
  void item_draw(void*, int, int, int, int) const ;
  int full_height() const ;
  int incr_height() const ;

  FL_BLINE *first;		// the array of lines
  FL_BLINE *last;
  FL_BLINE *cache;
  int cacheline;		// line number of cache
  int lines;                	// Number of lines
  int full_height_;
  const int* column_widths_;
  char format_char_;		// alternative to @-sign
  char column_char_;		// alternative to tab
  FL_BLINE* find_line(int) const ;
  FL_BLINE* _remove(int) ;
  void insert(int, FL_BLINE*);
  int lineno(void*) const ;

public:

  void remove(int);
  void add(const char*, void* = 0);
  void insert(int, const char*, void* = 0);
  void move(int to, int from);
  int  load(const char* filename);
  void clear();

  int size() const {return lines;}
  int topline() const ;
  void topline(int);

  int select(int, int=1);
  int selected(int) const ;
  void show(int n);
  void hide(int n);
  void hide() {Fl_Widget::hide();}
  int visible(int n) const ;

  int value() const ;
  void value(int v) {select(v);}
  const char* text(int) const ;
  void text(int, const char*);
  void* data(int) const ;
  void data(int, void* v);

  Fl_Browser(int, int, int, int, const char* = 0);

  char format_char() const {return format_char_;}
  void format_char(char c) {format_char_ = c;}
  char column_char() const {return column_char_;}
  void column_char(char c) {column_char_ = c;}
  const int* column_widths() const {return column_widths_;}
  void column_widths(const int* l) {column_widths_ = l;}

  int displayed(int n) const {return Fl_Browser_::displayed(find_line(n));}

  // for back compatability only:
  void replace(int a, const char* b) {text(a, b);}
  void display(int, int=1);
};

#endif

//
// End of "$Id: Fl_Browser.H,v 1.5 1998/11/05 16:04:37 mike Exp $".
//
