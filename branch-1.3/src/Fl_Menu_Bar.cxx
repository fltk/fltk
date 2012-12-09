//
// "$Id$"
//
// Menu bar widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_draw.H>

void Fl_Menu_Bar::draw() {
  draw_box();
  if (!menu() || !menu()->text) return;
  const Fl_Menu_Item* m;
  int X = x()+6;
  for (m=menu()->first(); m->text; m = m->next()) {
    int W = m->measure(0,this) + 16;
    m->draw(X, y(), W, h(), this);
    X += W;
    if (m->flags & FL_MENU_DIVIDER) {
      int y1 = y() + Fl::box_dy(box());
      int y2 = y1 + h() - Fl::box_dh(box()) - 1;

      // Draw a vertical divider between menus...
      fl_color(FL_DARK3);
      fl_yxline(X - 6, y1, y2);
      fl_color(FL_LIGHT3);
      fl_yxline(X - 5, y1, y2);
    }
  }
}

int Fl_Menu_Bar::handle(int event) {
  const Fl_Menu_Item* v;
  if (menu() && menu()->text) switch (event) {
  case FL_ENTER:
  case FL_LEAVE:
    return 1;
  case FL_PUSH:
    v = 0;
  J1:
    v = menu()->pulldown(x(), y(), w(), h(), v, this, 0, 1);
    picked(v);
    return 1;
  case FL_SHORTCUT:
    if (visible_r()) {
      v = menu()->find_shortcut(0, true);
      if (v && v->submenu()) goto J1;
    }
    return test_shortcut() != 0;
  }
  return 0;
}


Fl_Menu_Bar::Fl_Menu_Bar(int X, int Y, int W, int H,const char *l)
: Fl_Menu_(X,Y,W,H,l) 
{
}

//
// End of "$Id$".
//
