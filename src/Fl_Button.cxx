//
// "$Id: Fl_Button.cxx,v 1.4.2.6 2001/01/22 15:13:39 easysw Exp $"
//
// Button widget for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>

// There are a lot of subclasses, named Fl_*_Button.  Some of
// them are implemented by setting the type() value and testing it
// here.  This includes Fl_Radio_Button and Fl_Toggle_Button

int Fl_Button::value(int v) {
  v = v ? 1 : 0;
  oldval = v;
  clear_changed();
  if (value_ != v) {value_ = v; redraw(); return 1;}
  else return 0;
}

void Fl_Button::setonly() { // set this radio button on, turn others off
  value(1);
  Fl_Group* g = (Fl_Group*)parent();
  Fl_Widget*const* a = g->array();
  for (int i = g->children(); i--;) {
    Fl_Widget* o = *a++;
    if (o != this && o->type()==FL_RADIO_BUTTON) ((Fl_Button*)o)->value(0);
  }
}

void Fl_Button::draw() {
  if (type() == FL_HIDDEN_BUTTON) return;
  Fl_Color col = value() ? selection_color() : color();
//if (col == FL_GRAY && Fl::belowmouse()==this) col = FL_LIGHT1;
  draw_box(value() ? (down_box()?down_box():down(box())) : box(), col);
  draw_label();
}
int Fl_Button::handle(int event) {
  int newval;
  switch (event) {
  case FL_ENTER:
  case FL_LEAVE:
//  if ((value_?selection_color():color())==FL_GRAY) redraw();
    return 1;
  case FL_PUSH:
  case FL_DRAG:
    if (Fl::event_inside(this)) {
      if (type() == FL_RADIO_BUTTON) newval = 1;
      else newval = !oldval;
    } else
      newval = oldval;
    if (newval != value_) {
      value_ = newval;
      redraw();
      if (when() & FL_WHEN_CHANGED) do_callback();
    }
    return 1;
  case FL_RELEASE:
    if (value_ == oldval) {
      if (when() & FL_WHEN_NOT_CHANGED) do_callback();
      return 1;
    }
    if (type() == FL_RADIO_BUTTON)
      setonly();
    else if (type() == FL_TOGGLE_BUTTON)
      oldval = value_;
    else {
      value(oldval);
      if (when() & FL_WHEN_CHANGED) do_callback();
    }
    if (when() & FL_WHEN_RELEASE) do_callback(); else set_changed();
    return 1;
  case FL_SHORTCUT:
    if (!(shortcut() ?
	  Fl::test_shortcut(shortcut()) : test_shortcut())) return 0;
    if (type() == FL_RADIO_BUTTON && !value_) {
      setonly();
      if (when() & FL_WHEN_CHANGED) do_callback();
    } else if (type() == FL_TOGGLE_BUTTON) {
      value(!value());
      if (when() & FL_WHEN_CHANGED) do_callback();
    }
    if (when() & FL_WHEN_RELEASE) do_callback(); else set_changed();
    return 1;
  default:
    return 0;
  }
}

Fl_Button::Fl_Button(int x,int y,int w,int h, const char *l)
: Fl_Widget(x,y,w,h,l) {
  box(FL_UP_BOX);
  down_box(FL_NO_BOX);
  value_ = oldval = 0;
  shortcut_ = 0;
  set_flag(SHORTCUT_LABEL);
}

//
// End of "$Id: Fl_Button.cxx,v 1.4.2.6 2001/01/22 15:13:39 easysw Exp $".
//
