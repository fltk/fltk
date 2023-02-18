//
// Packing widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

// Based on code by Curtis Edwards
// Group that compresses all its children together and resizes to surround
// them on each redraw (only if box() is zero)
// Bugs: ?

#include <FL/Fl.H>
#include <FL/Fl_Pack.H>
#include <FL/fl_draw.H>

/**
  Creates a new Fl_Pack widget using the given position, size,
  and label string.

  The default boxtype is FL_NO_BOX.

  The default type() is Fl_Pack::VERTICAL.

  The destructor <I>also deletes all the children</I>. This allows a
  whole tree to be deleted at once, without having to keep a pointer to
  all the children in the user code. A kludge has been done so the
  Fl_Pack and all of its children can be automatic (local)
  variables, but you must declare the Fl_Pack <I>first</I>, so
  that it is destroyed last.

  \param[in] X,Y        X and Y coordinates (position)
  \param[in] W,H        width and height, respectively
  \param[in] L          label (optional)
*/
Fl_Pack::Fl_Pack(int X, int Y, int W, int H, const char *L)
: Fl_Group(X, Y, W, H, L) {
  resizable(0);
  spacing_ = 0;
  // type(VERTICAL); // already set like this
}

void Fl_Pack::draw() {
  int tx = x()+Fl::box_dx(box());
  int ty = y()+Fl::box_dy(box());
  int tw = w()-Fl::box_dw(box());
  int th = h()-Fl::box_dh(box());
  int rw, rh;
  int current_position = horizontal() ? tx : ty;
  int maximum_position = current_position;
  uchar d = damage();
  Fl_Widget*const* a = array();
  if (horizontal()) {
    rw = -spacing_;
    rh = th;

    for (int i = children(); i--;)
      if (child(i)->visible()) {
        if (child(i) != this->resizable()) rw += child(i)->w();
        rw += spacing_;
      }
  } else {
    rw = tw;
    rh = -spacing_;

    for (int i = children(); i--;)
      if (child(i)->visible()) {
        if (child(i) != this->resizable()) rh += child(i)->h();
        rh += spacing_;
      }
  }
  for (int i = children(); i--;) {
    Fl_Widget* o = *a++;
    if (o->visible()) {
      int X,Y,W,H;
      if (horizontal()) {
        X = current_position;
        W = o->w();
        Y = ty;
        H = th;
      } else {
        X = tx;
        W = tw;
        Y = current_position;
        H = o->h();
      }
      // Last child, if resizable, takes all remaining room
      if(i == 0 && o == this->resizable()) {
        if(horizontal())
          W = tw - rw;
        else
          H = th - rh;
      }
      if (spacing_ && current_position>maximum_position && box() &&
        (X != o->x() || Y != o->y() || d&FL_DAMAGE_ALL)) {
        fl_color(color());
        if (horizontal())
          fl_rectf(maximum_position, ty, spacing_, th);
        else
          fl_rectf(tx, maximum_position, tw, spacing_);
      }
      if (X != o->x() || Y != o->y() || W != o->w() || H != o->h()) {
        o->resize(X,Y,W,H);
        o->clear_damage(FL_DAMAGE_ALL);
      }
      if (d&FL_DAMAGE_ALL) {
        draw_child(*o);
        draw_outside_label(*o);
      } else update_child(*o);
      // child's draw() can change it's size, so use new size:
      current_position += (horizontal() ? o->w() : o->h());
      if (current_position > maximum_position)
        maximum_position = current_position;
      current_position += spacing_;
    }
  }

  if (horizontal()) {
    if (maximum_position < tx+tw && box()) {
      fl_color(color());
      fl_rectf(maximum_position, ty, tx+tw-maximum_position, th);
    }
    tw = maximum_position-tx;
  } else {
    if (maximum_position < ty+th && box()) {
      fl_color(color());
      fl_rectf(tx, maximum_position, tw, ty+th-maximum_position);
    }
    th = maximum_position-ty;
  }

  tw += Fl::box_dw(box()); if (tw <= 0) tw = 1;
  th += Fl::box_dh(box()); if (th <= 0) th = 1;
  if (tw != w() || th != h()) {
    Fl_Widget::resize(x(),y(),tw,th);
    Fl_Group *parent = this->parent();
    if (parent) parent->init_sizes();
    d = FL_DAMAGE_ALL;
  }
  if (d&FL_DAMAGE_ALL) {
    draw_box();
    draw_label();
  }
}

/** Override Fl_Group resize behavior.

  Resizing an Fl_Pack will not resize any of its children, but trigger a
  redraw, which in turn recalculates the dimensions of all children.

  \param[in] X, Y, W, H new position and size of the Fl_Pack widget
*/
void Fl_Pack::resize(int X, int Y, int W, int H) {
  Fl_Widget::resize(X, Y, W, H);
  redraw();
}
