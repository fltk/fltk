//
// "$Id: Fl_Choice.cxx,v 1.10.2.5 2001/01/22 15:13:39 easysw Exp $"
//
// Choice widget for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Choice.H>
#include <FL/fl_draw.H>

// Emulates the Forms choice widget.  This is almost exactly the same
// as an Fl_Menu_Button.  The only difference is the appearance of the
// button: it draws the text of the current pick and a down-arrow.

extern char fl_draw_shortcut;

void Fl_Choice::draw() {
  draw_box();
  if (box() == FL_FLAT_BOX) return; // for XForms compatability
  int H = labelsize()/2+1;
  draw_box(FL_THIN_UP_BOX,x()+w()-3*H,y()+(h()-H)/2,2*H,H,color());
  if (mvalue()) {
    Fl_Menu_Item m = *mvalue();
    if (active_r()) m.activate(); else m.deactivate();
    int BW = Fl::box_dx(box());
    fl_clip(x(), y(), w()-3*H, h());
    fl_draw_shortcut = 2; // hack value to make '&' disappear
    m.draw(x()+BW, y(), w()-2*BW-3*H, h(), this);
    fl_draw_shortcut = 0;
    fl_pop_clip();
  }
  draw_label();
}

Fl_Choice::Fl_Choice(int x,int y,int w,int h, const char *l)
: Fl_Menu_(x,y,w,h,l) {
  align(FL_ALIGN_LEFT);
  when(FL_WHEN_RELEASE);
  textfont(FL_HELVETICA);
  down_box(FL_NO_BOX);
}

int Fl_Choice::value(int v) {
  if (!Fl_Menu_::value(v)) return 0;
  redraw();
  return 1;
}

int Fl_Choice::handle(int e) {
  if (!menu() || !menu()->text) return 0;
  const Fl_Menu_Item* v;
  switch (e) {
  case FL_PUSH:
    Fl::event_is_click(0);
  J1:
    v = menu()->pulldown(x(), y(), w(), h(), mvalue(), this);
    if (!v || v->submenu()) return 1;
    if (v != mvalue()) redraw();
    picked(v);
    return 1;
  case FL_SHORTCUT:
    if (Fl_Widget::test_shortcut()) goto J1;
    v = menu()->test_shortcut();
    if (!v) return 0;
    if (v != mvalue()) redraw();
    picked(v);
    return 1;
  default:
    return 0;
  }
}

//
// End of "$Id: Fl_Choice.cxx,v 1.10.2.5 2001/01/22 15:13:39 easysw Exp $".
//
