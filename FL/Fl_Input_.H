//
// "$Id: Fl_Input_.H,v 1.6.2.4 2001/01/22 15:13:37 easysw Exp $"
//
// Input base class header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Input__H
#define Fl_Input__H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

#define FL_NORMAL_INPUT		0
#define FL_FLOAT_INPUT		1
#define FL_INT_INPUT		2
#define FL_HIDDEN_INPUT		3
#define FL_MULTILINE_INPUT	4
#define FL_SECRET_INPUT		5

class Fl_Input_ : public Fl_Widget {

  const char* value_;
  char* buffer;

  int size_;
  int bufsize;
  int position_;
  int mark_;
  int xscroll_, yscroll_;
  int mu_p;
  int maximum_size_;

  uchar textfont_;
  uchar textsize_;
  uchar textcolor_;
  uchar cursor_color_;
  uchar erase_cursor_only;

  FL_EXPORT const char* expand(const char*, char*) const;
  FL_EXPORT double expandpos(const char*, const char*, const char*, int*) const;
  FL_EXPORT void minimal_update(int, int);
  FL_EXPORT void minimal_update(int p);
  FL_EXPORT void put_in_buffer(int newsize);

  FL_EXPORT void setfont() const;

protected:

  FL_EXPORT int word_start(int i) const;
  FL_EXPORT int word_end(int i) const;
  FL_EXPORT int line_start(int i) const;
  FL_EXPORT int line_end(int i) const;
  FL_EXPORT void drawtext(int, int, int, int);
  FL_EXPORT int up_down_position(int, int keepmark=0);
  FL_EXPORT void handle_mouse(int, int, int, int, int keepmark=0);
  FL_EXPORT int handletext(int e, int, int, int, int);
  FL_EXPORT void maybe_do_callback();
  int xscroll() const {return xscroll_;}
  int yscroll() const {return yscroll_;}

public:

  FL_EXPORT void resize(int, int, int, int);

  FL_EXPORT Fl_Input_(int, int, int, int, const char* = 0);
  FL_EXPORT ~Fl_Input_();

  FL_EXPORT int value(const char*);
  FL_EXPORT int value(const char*, int);
  FL_EXPORT int static_value(const char*);
  FL_EXPORT int static_value(const char*, int);
  const char* value() const {return value_;}
  char index(int i) const {return value_[i];}
  int size() const {return size_;}
  int maximum_size() const {return maximum_size_;}
  void maximum_size(int m) {maximum_size_ = m;}

  int position() const {return position_;}
  int mark() const {return mark_;}
  FL_EXPORT int position(int p, int m);
  int position(int p) {return position(p, p);}
  int mark(int m) {return position(position(), m);}
  FL_EXPORT int replace(int, int, const char*, int=0);
  int cut() {return replace(position(), mark(), 0);}
  int cut(int n) {return replace(position(), position()+n, 0);}
  int cut(int a, int b) {return replace(a, b, 0);}
  int insert(const char* t, int l=0){return replace(position_, mark_, t, l);}
  FL_EXPORT int copy();
  FL_EXPORT int undo();
  FL_EXPORT int copy_cuts();

  Fl_Font textfont() const {return (Fl_Font)textfont_;}
  void textfont(uchar s) {textfont_ = s;}
  uchar textsize() const {return textsize_;}
  void textsize(uchar s) {textsize_ = s;}
  Fl_Color textcolor() const {return (Fl_Color)textcolor_;}
  void textcolor(uchar n) {textcolor_ = n;}
  Fl_Color cursor_color() const {return (Fl_Color)cursor_color_;}
  void cursor_color(uchar n) {cursor_color_ = n;}
};

#endif 

//
// End of "$Id: Fl_Input_.H,v 1.6.2.4 2001/01/22 15:13:37 easysw Exp $".
//
