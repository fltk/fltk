//
// "$Id: Fl_Menu_.H,v 1.7.2.4 2001/01/22 15:13:37 easysw Exp $"
//
// Menu base class header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Menu__H
#define Fl_Menu__H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif
#include "Fl_Menu_Item.H"

class Fl_Menu_ : public Fl_Widget {

  Fl_Menu_Item *menu_;
  const Fl_Menu_Item *value_;

protected:

  uchar down_box_;
  uchar textfont_;
  uchar textsize_;
  uchar textcolor_;
  uchar alloc;

public:
  FL_EXPORT const Fl_Menu_Item* picked(const Fl_Menu_Item*);

  FL_EXPORT Fl_Menu_(int,int,int,int,const char * =0);
  FL_EXPORT ~Fl_Menu_();

  const Fl_Menu_Item* test_shortcut() {return picked(menu()->test_shortcut());}
  FL_EXPORT void global();

  const Fl_Menu_Item *menu() const {return menu_;}
  FL_EXPORT void menu(const Fl_Menu_Item *m);
  FL_EXPORT void copy(const Fl_Menu_Item *m, void* user_data = 0);
  FL_EXPORT int  add(const char*, int shortcut, Fl_Callback*, void* = 0, int = 0);
  int  add(const char* a, const char* b, Fl_Callback* c,
	  void* d = 0, int e = 0) {return add(a,fl_old_shortcut(b),c,d,e);}
  FL_EXPORT int  size() const ;
  FL_EXPORT void clear();
  FL_EXPORT int  add(const char *);
  FL_EXPORT void replace(int,const char *);
  FL_EXPORT void remove(int);
  void shortcut(int i, int s) {menu_[i].shortcut(s);}
  void mode(int i,int x) {menu_[i].flags = x;}
  int  mode(int i) const {return menu_[i].flags;}

  const Fl_Menu_Item *mvalue() const {return value_;}
  int value() const {return value_-menu_;}
  FL_EXPORT int value(const Fl_Menu_Item*);
  int value(int i) {return value(menu_+i);}
  const char *text() const {return value_ ? value_->text : 0;}
  const char *text(int i) const {return menu_[i].text;}

  Fl_Font textfont() const {return (Fl_Font)textfont_;}
  void textfont(uchar c) {textfont_=c;}
  uchar textsize() const {return textsize_;}
  void textsize(uchar c) {textsize_=c;}
  Fl_Color textcolor() const {return (Fl_Color)textcolor_;}
  void textcolor(uchar c) {textcolor_=c;}

  Fl_Boxtype down_box() const {return (Fl_Boxtype)down_box_;}
  void down_box(Fl_Boxtype b) {down_box_ = b;}

  // back compatability:
  Fl_Color down_color() const {return selection_color();}
  void down_color(uchar c) {selection_color(c);}
};

#endif

//
// End of "$Id: Fl_Menu_.H,v 1.7.2.4 2001/01/22 15:13:37 easysw Exp $".
//
