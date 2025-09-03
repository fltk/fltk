//
// Choice widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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
#include <FL/Fl_Choice.H>
#include <FL/fl_draw.H>
#include "flstring.h"

// Emulates the Forms choice widget.  This is almost exactly the same
// as an Fl_Menu_Button.  The only difference is the appearance of the
// button: it draws the text of the current pick and a down-arrow.
// The exact layout and the type of arrow can vary by FLTK scheme.

// FIXME: all such variations should be implemented in the "scheme code",
// hopefully in a future class derived from a base class Fl_Scheme or similar.
// Albrecht

void Fl_Choice::draw() {
  Fl_Boxtype btype = Fl::scheme() ? FL_UP_BOX           // non-default uses up box
                                  : FL_DOWN_BOX;        // default scheme uses down box
  int dx = Fl::box_dx(btype);
  int dy = Fl::box_dy(btype);

  // Arrow area
  int H = h() - 2 * dy;
  int W = 20;
  int X = x() + w() - W - dx;
  int Y = y() + dy;

  Fl_Rect ab(X, Y, W, H); // arrow box
  int active = active_r();
  Fl_Color arrow_color = active ? labelcolor() : fl_inactive(labelcolor());
  Fl_Color box_color = color();

  // From "original" code: modify the box color *only* for the default scheme.
  // This is weird (why?). I believe we should either make sure that the text
  // color contrasts well when the text is rendered *or* we should do this for
  // *all* schemes. Anyway, adapting the old code... (Albrecht)

  if (!Fl::scheme()) {            // default scheme only, see comment above
    if (fl_contrast(textcolor(), FL_BACKGROUND2_COLOR) == textcolor())
      box_color = FL_BACKGROUND2_COLOR;
    else
      box_color = fl_lighter(color());
  }

  // Draw the widget box

  draw_box(btype, box_color);

  // Arrow box or horizontal divider line, depending on the current scheme

  // Scheme:            Box or divider line
  // ----------------------------------------
  // Default (None):    Arrow box (FL_UP_BOX)
  // gtk+, gleam, oxy:  Divider line
  // else:              Nothing (!)

  if (Fl::scheme()) {
    if (Fl::is_scheme("gtk+") ||
        Fl::is_scheme("gleam") ||
        Fl::is_scheme("oxy")) {
      // draw the divider
      int x1 = x() + w() - W - 2 * dx;
      int y1 = y() + dy;
      int y2 = y() + h() - dy;

      fl_color(fl_darker(color()));
      fl_yxline(x1, y1, y2);

      fl_color(fl_lighter(color()));
      fl_yxline(x1 + 1, y1, y2);
    }
  } else { // Default scheme ("None")
    // Draw arrow box
    draw_box(FL_UP_BOX, X, Y, W, H, color());
    ab.inset(FL_UP_BOX);
  }

  // Draw choice arrow(s)
  fl_draw_arrow(ab, FL_ARROW_CHOICE, FL_ORIENT_NONE, arrow_color);

  W += 2 * dx;

  // Draw menu item's label
  if (mvalue()) {
    Fl_Menu_Item m = *mvalue();
    if (active) m.activate(); else m.deactivate();

    // Clip
    int xx = x() + dx, yy = y() + dy + 1, ww = w() - W, hh = H - 2;
    fl_push_clip(xx, yy, ww, hh);

    if (Fl::scheme()) {
      Fl_Label l;
      l.value = m.text;
      l.image = 0;
      l.deimage = 0;
      l.type = m.labeltype_;
      l.font = m.labelsize_ || m.labelfont_ ? m.labelfont_ : textfont();
      l.size = m.labelsize_ ? m.labelsize_ : textsize();
      l.color= m.labelcolor_ ? m.labelcolor_ : textcolor();
      l.h_margin_ = l.v_margin_ = l.spacing = 0;
      if (!m.active()) l.color = fl_inactive((Fl_Color)l.color);
      fl_draw_shortcut = 2; // hack value to make '&' disappear
      l.draw(xx+3, yy, ww>6 ? ww-6 : 0, hh, FL_ALIGN_LEFT);
      fl_draw_shortcut = 0;
      if ( Fl::focus() == this ) draw_focus(box(), xx, yy, ww, hh);
    }
    else {
      fl_draw_shortcut = 2; // hack value to make '&' disappear
      m.draw(xx, yy, ww, hh, this, Fl::focus() == this);
      fl_draw_shortcut = 0;
    }

    fl_pop_clip();
  }

  // Widget's label
  draw_label();
}

/**
  Create a new Fl_Choice widget using the given position, size and label string.
  The default boxtype is \c FL_UP_BOX.

  The constructor sets menu() to NULL.
  See Fl_Menu_ for the methods to set or change the menu.

  \param[in] X, Y, W, H position and size of the widget
  \param[in] L widget label, default is no label
 */
Fl_Choice::Fl_Choice(int X, int Y, int W, int H, const char *L)
: Fl_Menu_(X,Y,W,H,L) {
  align(FL_ALIGN_LEFT);
  when(FL_WHEN_RELEASE);
  textfont(FL_HELVETICA);
  box(FL_UP_BOX);
  down_box(FL_BORDER_BOX);
}

/**
  Sets the currently selected value using a pointer to menu item.
  Changing the selected value causes a redraw().
  \param[in] v pointer to menu item in the menu item array.
  \returns non-zero if the new value is different to the old one.
 */
int Fl_Choice::value(const Fl_Menu_Item *v) {
  if (!Fl_Menu_::value(v)) return 0;
  redraw();
  return 1;
}

/**
  Sets the currently selected value using the index into the menu item array.
  Changing the selected value causes a redraw().
  \param[in] v index of value in the menu item array.
  \returns non-zero if the new value is different to the old one.
 */
int Fl_Choice::value(int v) {
  if (v == -1) return value((const Fl_Menu_Item *)0);
  if (v < 0 || v >= (size() - 1)) return 0;
  if (!Fl_Menu_::value(v)) return 0;
  redraw();
  return 1;
}

int Fl_Choice::handle(int e) {
  if (!menu() || !menu()->text) return 0;
  const Fl_Menu_Item* v;
  Fl_Widget_Tracker wp(this);
  switch (e) {
  case FL_ENTER:
  case FL_LEAVE:
    return 1;

  case FL_KEYBOARD:
    if (Fl::event_key() != ' ' ||
        (Fl::event_state() & (FL_SHIFT | FL_CTRL | FL_ALT | FL_META))) return 0;
  case FL_PUSH:
    if (Fl::visible_focus()) Fl::focus(this);
  J1:
    if (Fl::scheme()
        || fl_contrast(textcolor(), FL_BACKGROUND2_COLOR) != textcolor()) {
      v = menu()->pulldown(x(), y(), w(), h(), mvalue(), this);
      if (wp.deleted()) return 1;
    } else {
      // In order to preserve the old look-n-feel of "white" menus,
      // temporarily override the color() of this widget...
      Fl_Color c = color();
      color(FL_BACKGROUND2_COLOR);
      v = menu()->pulldown(x(), y(), w(), h(), mvalue(), this);
      if (wp.deleted()) return 1;
      color(c);
    }
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
  case FL_FOCUS:
  case FL_UNFOCUS:
    if (Fl::visible_focus()) {
      redraw();
      return 1;
    } else return 0;
  default:
    return 0;
  }
}
