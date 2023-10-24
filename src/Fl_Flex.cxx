//
// Fl_Flex widget implementation for the Fast Light Tool Kit (FLTK).
//
// Copyright 2020 by Karsten Pedersen
// Copyright 2022-2023 by Bill Spitzak and others.
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

#include <FL/Fl_Flex.H>
#include <stdlib.h>       // malloc, free, ...

/**
  Construct a new Fl_Flex widget with the given position, size, and label.

  You can set \p type(Fl_Flex::HORIZONTAL) or \p type(Fl_Flex::VERTICAL).
  The default is \p type(Fl_Flex::VERTICAL).

  Alternate constructors let you specify the layout as Fl_Flex::HORIZONTAL or
  Fl_Flex::VERTICAL directly. Fl_Flex::ROW is an alias of Fl_Flex::HORIZONTAL
  and Fl_Flex::COLUMN is an alias of Fl_Flex::VERTICAL.

  \param[in]  X,Y   position
  \param[in]  W,H   size (width and height)
  \param[in]  L     label (optional)

  \see Fl_Flex::Fl_Flex(int direction)
  \see Fl_Flex::Fl_Flex(int w, int h, int direction)
  \see Fl_Flex::Fl_Flex(int x, int y, int w, int h, int direction)
  \see Fl_Flex::Fl_Flex(int x, int y, int w, int h, const char *L)
*/
Fl_Flex::Fl_Flex(int X, int Y, int W, int H, const char *L)
  : Fl_Group(X, Y, W, H, L) {
  init();
}

// special Fl_Flex constructors w/o label (backwards compatible with original Fl_Flex widget)

/**
  Construct a new Fl_Flex widget specifying its layout.

  Use Fl_Flex::HORIZONTAL (aka Fl_Flex::ROW) or Fl_Flex::VERTICAL
  (aka Fl_Flex::COLUMN) as the \p direction argument.

  This constructor sets the position and size to (0, 0, 0, 0) which is suitable
  for nested Fl_Flex widgets. Use one of the other constructors to set the
  desired position and size as well.

  \param[in]  direction  horizontal (row) or vertical (column) layout

  \see Fl_Flex::Fl_Flex(int w, int h, int direction)
  \see Fl_Flex::Fl_Flex(int x, int y, int w, int h, int direction)
  \see Fl_Flex::Fl_Flex(int x, int y, int w, int h, const char *L)
*/
Fl_Flex::Fl_Flex(int direction)
  : Fl_Group(0, 0, 0, 0, 0) {
  init(direction);
}

/**
  Construct a new Fl_Flex widget specifying its layout and size.

  Use Fl_Flex::HORIZONTAL (aka Fl_Flex::ROW) or Fl_Flex::VERTICAL
  (aka Fl_Flex::COLUMN) as the \p direction argument.

  This constructor sets the position to (x = 0, y = 0) which is suitable
  for nested Fl_Flex widgets. Use one of the other constructors to set the
  desired position as well.

  \param[in]  w,h        widget size
  \param[in]  direction  horizontal (row) or vertical (column) layout

  \see Fl_Flex::Fl_Flex(int direction)
  \see Fl_Flex::Fl_Flex(int x, int y, int w, int h, int direction)
  \see Fl_Flex::Fl_Flex(int x, int y, int w, int h, const char *L)

*/
Fl_Flex::Fl_Flex(int w, int h, int direction)
  : Fl_Group(0, 0, w, h, 0) {
  init(direction);
}

/**
  Construct a new Fl_Flex widget specifying its layout, position, and size.

  Use Fl_Flex::HORIZONTAL (aka Fl_Flex::ROW) or Fl_Flex::VERTICAL
  (aka Fl_Flex::COLUMN) as the \p direction argument.

  This constructor sets the position and size of the widget which is suitable
  for top level Fl_Flex widgets but does not set a widget label.
  Use Fl_Widget::label() to set one if desired.

  \param[in]  x,y   widget position
  \param[in]  w,h   widget size
  \param[in]  direction   horizontal (row) or vertical (column) layout

  \see Fl_Flex::Fl_Flex(int direction)
  \see Fl_Flex::Fl_Flex(int w, int h, int direction)
  \see Fl_Flex::Fl_Flex(int x, int y, int w, int h, const char *L)

*/

Fl_Flex::Fl_Flex(int x, int y, int w, int h, int direction)
  : Fl_Group(x, y, w, h, 0) {
  init(direction);
}

void Fl_Flex::init(int t) {
  margin_left_      = 0;      // default margin size
  margin_top_       = 0;      // default margin size
  margin_right_     = 0;      // default margin size
  margin_bottom_    = 0;      // default margin size
  gap_              = 0;      // default gap size
  fixed_size_size_  = 0;      // number of fixed size widgets
  fixed_size_alloc_ = 0;      // allocated size of array of fixed size widgets
  fixed_size_       = NULL;   // array of fixed size widgets
  need_layout_      = false;  // no need to calculate layout yet

  type(HORIZONTAL);
  if (t == VERTICAL)
    type(VERTICAL);
}

Fl_Flex::~Fl_Flex() {
  if (fixed_size_)
    free(fixed_size_);
}

/*
 Fl_Group calls this method when a child widget is about to be removed.
 Make sure that the widget is also removed from our fixed list.
 */
void Fl_Flex::on_remove(int index) {
  fixed(child(index), 0);
  need_layout(1);
}

/**
  Draw the widget.

  This will finally calculate the layout of the widget and of all its
  children if necessary and draw the widget.

  Some changes of included children may require a new layout to be
  calculated. If this is the case the user may need to call layout()
  to make sure everything is calculated properly.

  \see layout()
*/
void Fl_Flex::draw() {
  if (need_layout())
    layout();
  Fl_Group::draw();
}

/**
  Resize the container and calculate all child positions and sizes.

  \param[in]  x,y   position
  \param[in]  w,h   width and height
*/
void Fl_Flex::resize(int x, int y, int w, int h) {
  Fl_Widget::resize(x, y, w, h);
  layout();
} // resize()


/**
  Calculates the layout of the widget and redraws it.

  If you change widgets in the Fl_Flex container you should call this method
  to force recalculation of child widget sizes and positions. This can be
  useful (necessary) if you hide(), show(), add() or remove() children.

  Call this method if you need to recalculate widget positions for usage in
  an algorithm that places widgets at certain positions or when you need to
  display (show) or hide one or more children depending on the current layout
  (for instance a side bar).

  This method also calls redraw() on the Fl_Flex widget.
*/
void Fl_Flex::layout() {

  const int nc = children();

  int dx = Fl::box_dx(box());
  int dy = Fl::box_dy(box());
  int dw = Fl::box_dw(box());
  int dh = Fl::box_dh(box());

  // Calculate total space minus gaps
  int gaps = nc > 1 ? nc - 1 : 0;
  int hori = horizontal();
  int space = hori ? (w() - dw - margin_left_ - margin_right_)
                   : (h() - dh - margin_top_ - margin_bottom_);

  // set x and y (start) position, calculate widget sizes
  int xp = x() + dx + margin_left_;
  int yp = y() + dy + margin_top_;
  int hh = h() - dh - margin_top_ - margin_bottom_; // if horizontal: constant height of widgets
  int vw = w() - dw - margin_left_ - margin_right_; // if vertical:   constant width of widgets

  int fw = nc;    // number of flexible widgets

  // Precalculate remaining space that can be distributed

  for (int i = 0; i < nc; i++) {
    Fl_Widget *c = child(i);
    if (c->visible()) {
      if (fixed(c)) {
        space -= (hori ? c->w() : c->h());
        fw--;
      }
    } else { // hidden widget
      fw--;
      gaps--;
    }
  }

  if (gaps > 0)
    space -= gaps * gap_;

  // Set children to shared width/height of remaining space

  int sp = 0;     // width or height of flexible widget
  int rem = 0;    // remainder (to be distributed evenly)
  if (fw > 0) {
    sp = space / fw;
    rem = space % fw;
    if (rem)      // adjust space for first 'rem' widgets
      sp++;
  }

  for (int i = 0; i < nc; i++) {
    Fl_Widget *c = child(i);
    if (!c->visible())
      continue;

    if (hori) {
      if (fixed(c)) {
        c->resize(xp, yp, c->w(), hh);
      } else {
        c->resize(xp, yp, sp, hh);
        if (--rem == 0) sp--;
      }
      xp += c->w() + gap_;
    } else {
      if (fixed(c)) {
        c->resize(xp, yp, vw, c->h());
      } else {
        c->resize(xp, yp, vw, sp);
        if (--rem == 0) sp--;
      }
      yp += c->h() + gap_;
    }
  }

  need_layout(0); // layout done, no need to do it again when drawing
  redraw();
}

/**
  Ends automatic child addition and resizes all children.

  This marks the Fl_Flex widget as changed (need_layout(1)) which forces the
  widget to calculate its layout depending on all children and whether
  they have been assigned fix sizes or not right before it is drawn.

  \see need_layout(int)
  \see draw()
*/
void Fl_Flex::end() {
  Fl_Group::end();
  need_layout(1);
}

/**
  Set the horizontal or vertical size of a child widget.

  This sets either the width or height of a child widget, depending on the
  type() of the Fl_Flex container (Fl_Flex::HORIZONTAL or Fl_Flex::VERTICAL).
  The other dimension is set to the full width or height of the Fl_Flex widget
  minus border and margin sizes.

  This can be used to set a fixed widget width or height of children
  of Fl_Flex so they are not resized dynamically.

  If \p size is 0 (zero) or negative the widget size is reset to flexible size.

  \param[in]  child widget to be affected
  \param[in]  size  width (Fl_Flex::HORIZONTAL) or height (Fl_Flex::VERTICAL)
*/
void Fl_Flex::fixed(Fl_Widget *child, int size) {
  if (size <= 0)
    size = 0;

  // find w in our fixed size list
  int idx = -1;
  for (int i = 0; i < fixed_size_size_; i++) {
    if (fixed_size_[i] == child) {
      idx = i;
      break;
    }
  }

  // remove from array, if we want the widget to be flexible, but an entry was found
  if (size == 0 && idx >= 0) {
    for (int i = idx; i < fixed_size_size_ - 1; i++) {
      fixed_size_[i] = fixed_size_[i+1];
    }
    fixed_size_size_--;
    need_layout(1);
    return;
  }

  // if w is meant to be flexible and we didn't find it, we are done now
  if (size == 0)
    return;

  // if we have no entry yet, add to array of fixed size widgets
  if (idx == -1) {
    if (fixed_size_size_ == fixed_size_alloc_) {
      fixed_size_alloc_ = alloc_size(fixed_size_alloc_);
      fixed_size_ = (Fl_Widget **)realloc(fixed_size_, fixed_size_alloc_ * sizeof(Fl_Widget *));
    }
    fixed_size_[fixed_size_size_] = child;
    fixed_size_size_++;
  }

  // if the child size is meant to be fixed, set its new size
  if (horizontal())
    child->size(size, h()-margin_top_-margin_bottom_-Fl::box_dh(box()));
  else
    child->size(w()-margin_left_-margin_right_-Fl::box_dw(box()), size);
  need_layout(1);
}

/**
  Return whether the given widget has a fixed size or resizes dynamically.

  \param[in]  w   widget
  \return         whether the widget has a fixed size
  \retval     1   the widget has a fixed size
  \retval     0  the widget resizes dynamically
*/
int Fl_Flex::fixed(Fl_Widget *w) const {
  for (int i = 0; i < fixed_size_size_; i++) {
    if (w == fixed_size_[i]) {
      return 1;
    }
  }
  return 0;
}

/**
  Return new size to be allocated for array of fixed size widgets.

  This method is called when the array of fixed size widgets needs to be
  expanded. The current \p size is provided (size can be 0). The default
  method adds 8 to the current size.

  This can be used in derived classes to change the allocation strategy.
  Note that this method only \p queries the new size which shall be allocated
  but does not allocate the memory.

  \param[in]  size  current size
  \return     int   new size (to be allocated)
*/
int Fl_Flex::alloc_size(int size) const {
  return size + 8;
}
