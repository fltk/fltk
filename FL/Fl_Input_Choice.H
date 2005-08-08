//
// "$Id$"
//
// An input/chooser widget.
//            ______________  ____
//           |              || __ |
//           | input area   || \/ |
//           |______________||____|
//
// Copyright 1998-2005 by Bill Spitzak and others.
// Copyright 2004 by Greg Ercolano.
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
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#ifndef Fl_Input_Choice_H
#define Fl_Input_Choice_H

#include <FL/Fl.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/fl_draw.H>

class Fl_Input_Choice : public Fl_Group {
  // Private class to handle slightly 'special' behavior of menu button
  class InputMenuButton : public Fl_Menu_Button {
    void draw() {
      draw_box(FL_UP_BOX, color());
      fl_color(active_r() ? labelcolor() : fl_inactive(labelcolor()));
      int xc = x()+w()/2, yc=y()+h()/2;
      fl_polygon(xc-5,yc-3,xc+5,yc-3,xc,yc+3);
      if (Fl::focus() == this) draw_focus();
    }
  public:
    InputMenuButton(int x,int y,int w,int h,const char*l=0) : 
	Fl_Menu_Button(x,y,w,h,l) { box(FL_UP_BOX); }
  };

  Fl_Input *inp_;
  InputMenuButton *menu_;

  static void menu_cb(Fl_Widget*, void *data) { 
    Fl_Input_Choice *o=(Fl_Input_Choice *)data;
    o->inp_->value(o->menu_->text());
    o->do_callback();
  }

  static void inp_cb(Fl_Widget*, void *data) { 
    Fl_Input_Choice *o=(Fl_Input_Choice *)data;
    o->do_callback();
  }

  // Custom resize behavior -- input stretches, menu button doesn't
  inline int inp_x() { return(x() + Fl::box_dx(box())); }
  inline int inp_y() { return(y() + Fl::box_dy(box())); }
  inline int inp_w() { return(w() - Fl::box_dw(box()) - 20); }
  inline int inp_h() { return(h() - Fl::box_dh(box())); }

  inline int menu_x() { return(x() + w() - 20 - Fl::box_dx(box())); }
  inline int menu_y() { return(y() + Fl::box_dy(box())); }
  inline int menu_w() { return(20); }
  inline int menu_h() { return(h() - Fl::box_dh(box())); }

public:
  Fl_Input_Choice (int x,int y,int w,int h,const char*l=0) : Fl_Group(x,y,w,h,l) {
    Fl_Group::box(FL_DOWN_BOX);
    align(FL_ALIGN_LEFT);				// default like Fl_Input
    inp_ = new Fl_Input(inp_x(), inp_y(),
			inp_w(), inp_h());
    inp_->callback(inp_cb, (void*)this);
    inp_->box(FL_FLAT_BOX);		// cosmetic
    menu_ = new InputMenuButton(menu_x(), menu_y(),
				menu_w(), menu_h());
    menu_->callback(menu_cb, (void*)this);
    menu_->box(FL_FLAT_BOX);				// cosmetic
    end();
  }
  void add(const char *s) {
    menu_->add(s);
  }
  void clear() {
    menu_->clear();
  }
  Fl_Boxtype down_box() const {
    return (menu_->down_box());
  }
  void down_box(Fl_Boxtype b) {
    menu_->down_box(b);
  }
  const Fl_Menu_Item *menu() {
    return (menu_->menu());
  }
  void menu(const Fl_Menu_Item *m) {
    menu_->menu(m);
  }
  void resize(int X, int Y, int W, int H) {
    Fl_Group::resize(X,Y,W,H);
    inp_->resize(inp_x(), inp_y(), inp_w(), inp_h());
    menu_->resize(menu_x(), menu_y(), menu_w(), menu_h());
  }
  Fl_Color textcolor() const {
    return (inp_->textcolor());
  }
  void textcolor(Fl_Color c) {
    inp_->textcolor(c);
  }
  uchar textfont() const {
    return (inp_->textfont());
  }
  void textfont(uchar f) {
    inp_->textfont(f);
  }
  uchar textsize() const {
    return (inp_->textsize());
  }
  void textsize(uchar s) {
    inp_->textsize(s);
  }
  const char* value() const {
    return (inp_->value());
  }
  void value(const char *val) {
    inp_->value(val);
  }
  void value(int val) {
    menu_->value(val);
    inp_->value(menu_->text(val));
  }
  Fl_Menu_Button *menubutton() { return menu_; }
  Fl_Input *input() { return inp_; }
};

#endif // !Fl_Input_Choice_H

//
// End of "$Id$".
//
