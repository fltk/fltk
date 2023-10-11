//
// Menu button widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Rect.H>
#include <FL/fl_draw.H>


Fl_Menu_Button* Fl_Menu_Button::pressed_menu_button_ = NULL;


void Fl_Menu_Button::draw() {
  if (!box() || type()) return;

  // calculate position and size of virtual "arrow box" (choice button)

  int ah = h() - Fl::box_dh(box());
  int aw = ah > 20 ? 20 : ah; // limit width: don't waste space for button
  int ax = x() + w() - Fl::box_dx(box()) - aw;
  int ay = y() + (h() - ah) / 2;

  // the remaining space is used to draw the label

  draw_box(pressed_menu_button_ == this ? fl_down(box()) : box(), color());
  draw_label(x() + Fl::box_dx(box()), y(), w() - Fl::box_dw(box()) - aw, h());
  if (Fl::focus() == this) draw_focus();

  // draw the arrow (choice button)

  Fl_Color arrow_color = active_r() ? labelcolor() : fl_inactive(labelcolor());
  fl_draw_arrow(Fl_Rect(ax, ay, aw, ah), FL_ARROW_SINGLE, FL_ORIENT_DOWN, arrow_color);
}


/**
  Act exactly as though the user clicked the button or typed the
  shortcut key.  The menu appears, it waits for the user to pick an item,
  and if they pick one it sets value() and does the callback or
  sets changed() as described above.  The menu item is returned
  or NULL if the user dismisses the menu.

  \note Since FLTK 1.4.0 Fl_Menu_::menu_end() is called before the menu
    pops up to make sure the menu array is located in private storage.

  \see Fl_Menu_::menu_end()
*/
const Fl_Menu_Item* Fl_Menu_Button::popup() {
  menu_end();
  const Fl_Menu_Item* m;
  pressed_menu_button_ = this;
  redraw();
  Fl_Widget_Tracker mb(this);
  if (!box() || type()) {
    m = menu()->popup(Fl::event_x(), Fl::event_y(), label(), mvalue(), this);
  } else {
    m = menu()->pulldown(x(), y(), w(), h(), 0, this);
  }
  picked(m);
  pressed_menu_button_ = 0;
  if (mb.exists()) redraw();
  return m;
}

int Fl_Menu_Button::handle(int e) {
  if (!menu() || !menu()->text) return 0;
  switch (e) {
  case FL_ENTER: /* FALLTHROUGH */
  case FL_LEAVE:
    return (box() && !type()) ? 1 : 0;
  case FL_PUSH:
    if (!box()) {
      if (Fl::event_button() != 3) return 0;
    } else if (type()) {
      if (!(type() & (1 << (Fl::event_button()-1)))) return 0;
    }
    if (Fl::visible_focus()) Fl::focus(this);
    popup();
    return 1;
  case FL_KEYBOARD:
    if (!box()) return 0;
    if (Fl::event_key() == ' ' &&
        !(Fl::event_state() & (FL_SHIFT | FL_CTRL | FL_ALT | FL_META))) {
      popup();
      return 1;
    } else return 0;
  case FL_SHORTCUT:
    if (Fl_Widget::test_shortcut()) {popup(); return 1;}
    return test_shortcut() != 0;
  case FL_FOCUS: /* FALLTHROUGH */
  case FL_UNFOCUS:
    if (box() && Fl::visible_focus()) {
      redraw();
      return 1;
    }
  default:
    return 0;
  }
}

/**
  Creates a new Fl_Menu_Button widget using the given position,
  size, and label string. The default boxtype is FL_UP_BOX.
  <P>The constructor sets menu() to NULL.  See
  Fl_Menu_ for the methods to set or change the menu.
*/
Fl_Menu_Button::Fl_Menu_Button(int X,int Y,int W,int H,const char *l)
: Fl_Menu_(X,Y,W,H,l) {
  down_box(FL_NO_BOX);
}
