//
// Fl_Flex widget implementation for the Fast Light Tool Kit (FLTK).
//
// Copyright 2020 by Karsten Pedersen
// Copyright 2022 by Bill Spitzak and others.
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
#include <stdlib.h>

/**
  Construct a new Fl_Flex widget with the given position, size, and label.

  Fl_Flex is a container (layout) widget that provides flexible positioning
  of its children either in one row or in one column. Fl_Flex containers can
  be nested so you can create flexible layouts with multiple columns and rows.

  Fl_Flex is designed to be as simple as possible. You can set particular
  widget sizes or let Fl_Flex position and size the widgets to fit the container.

  You can set the margin \b around all children at the inner side the box frame
  (if any). This margin has the same size on top, bottom, and both sides.
  The default margin size is 3.

  You can set the gap size \b between all children. The gap size is the same between
  all children. This is similar to the 'spacing' of Fl_Pack.
  The default gap size is 3.

  Fl_Flex can either consist of a single row, i.e. type() = Fl_Flex::HORIZONTAL,
  or a single column, i.e. type() = Fl_Flex::VERTICAL. The default value is
  \p VERTICAL for consistency with Fl_Pack but you can use \p type() to assign
  a row (\p HORIZONTAL) layout.

  If type() == Fl_Flex::HORIZONTAL widgets are resized horizontally to fit in
  the container and their height is the full Fl_Flex height minus border size
  and margin. You can set a fixed widget width by using set_size().

  If type() == Fl_Flex::VERTICAL the above description applies equivalently
  to the row layout, i.e. widget heights can be set etc.

  Alternate constructors let you specify the layout as Fl_Flex::HORIZONTAL or
  Fl_Flex::VERTICAL directly. Fl_Flex::ROW is an alias of Fl_Flex::HORIZONTAL
  and Fl_Flex::COLUMN is an alias of Fl_Flex::VERTICAL.

  \note Please use the names (enum) rather than numerical values for
    the type() argument and the \p direction parameter of the alternate
    constructors. These constructors are backwards compatible with older
    versions of Fl_Flex.

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
  Construct a new Fl_Flex object specifying its layout.

  Use Fl_Flex::HORIZONTAL (aka Fl_Flex::ROW) or Fl_Flex::VERTICAL
  (aka Fl_Flex::COLUMN) as the \p direction argument.

  This constructor sets the position and size to (0, 0, 0, 0) which is suitable
  for nested Fl_Flex widgets. Use one of the other constructors to set the
  desired position and size as well.

  \param[in]  direction   horizontal (row) or vertical (column) layout

  \see Fl_Flex::Fl_Flex(int w, int h, int direction)
  \see Fl_Flex::Fl_Flex(int x, int y, int w, int h, int direction)
  \see Fl_Flex::Fl_Flex(int x, int y, int w, int h, const char *L)
*/
Fl_Flex::Fl_Flex(int direction)
  : Fl_Group(0, 0, 0, 0, 0) {
  init(direction);
}

/**
  Construct a new Fl_Flex object specifying its layout and size.

  Use Fl_Flex::HORIZONTAL (aka Fl_Flex::ROW) or Fl_Flex::VERTICAL
  (aka Fl_Flex::COLUMN) as the \p direction argument.

  This constructor sets the position to (x = 0, y = 0) which is suitable
  for nested Fl_Flex widgets. Use one of the other constructors to set the
  desired position as well.


  \param[in]  w,h   widget size
  \param[in]  direction   horizontal (row) or vertical (column) layout

  \see Fl_Flex::Fl_Flex(int direction)
  \see Fl_Flex::Fl_Flex(int x, int y, int w, int h, int direction)
  \see Fl_Flex::Fl_Flex(int x, int y, int w, int h, const char *L)

*/
Fl_Flex::Fl_Flex(int w, int h, int direction)
  : Fl_Group(0, 0, w, h, 0) {
  init(direction);
}


/**
  Construct a new Fl_Flex object specifying its layout, position, and size.

  Use Fl_Flex::HORIZONTAL (aka Fl_Flex::ROW) or Fl_Flex::VERTICAL
  (aka Fl_Flex::COLUMN) as the \p direction argument.

  This constructor sets the position and size of the widget which is suitable
  for outer Fl_Flex widgets but does not set a widget label.
  Use Fl_Widget::label() to set one if required.

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
  gap_    = 3;                // default gap size
  margin_ = 3;                // default margin size
  set_size_ = 0;              // array of fixed size widgets
  set_size_size_ = 0;         // number of fixed size widgets
  set_size_alloc_ = 0;        // allocated size of array of fixed size widgets
  type(HORIZONTAL);
  if (t == VERTICAL)
    type(VERTICAL);
}

Fl_Flex::~Fl_Flex() {
  if (set_size_)
    free(set_size_);
}

void Fl_Flex::resize(int x, int y, int w, int h) {

  Fl_Widget::resize(x, y, w, h);

  int cc = children();

  int dx = Fl::box_dx(box());
  int dy = Fl::box_dy(box());
  int dw = Fl::box_dw(box());
  int dh = Fl::box_dh(box());

  // Calculate total space minus gaps
  int gaps = cc > 1 ? cc - 1 : 0;
  int hori = horizontal();
  int space = (hori ? w - dw : h - dh) - 2 * margin_;

  // set x and y (start) position, calculate widget sizes
  int xp = x + dx + margin_;
  int yp = y + dy + margin_;
  int hh = h - dh - margin_ * 2; // if horizontal: constant height of widgets
  int vw = w - dw - margin_ * 2; // if vertical:   constant width of widgets

  int fw = cc;    // number of flexible widgets

  // Precalculate remaining space that can be distributed

  for (int i = 0; i < cc; i++) {
    Fl_Widget *c = child(i);
    if (c->visible()) {
      if (set_size(c)) {
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

  for (int i = 0; i < cc; i++) {
    Fl_Widget *c = child(i);
    if (!c->visible())
      continue;

    if (hori) {
      if (set_size(c)) {
        c->resize(xp, yp, c->w(), hh);
      } else {
        c->resize(xp, yp, sp, hh);
        if (--rem == 0) sp--;
      }
      xp += c->w() + gap_;
    } else {
      if (set_size(c)) {
        c->resize(xp, yp, vw, c->h());
      } else {
        c->resize(xp, yp, vw, sp);
        if (--rem == 0) sp--;
      }
      yp += c->h() + gap_;
    }
  }

} // resize()

/**
  Ends automatic child addition and resizes all children.

  This calculates the layout depending on all children and whether
  they have been assigned fix sizes or not.
*/
void Fl_Flex::end() {
  Fl_Group::end();
  resize(x(), y(), w(), h());
}

/**
  Set the horizontal or vertical size of a child widget.

  This sets either the width or height of a child widget, depending on the
  type() of the Fl_Flex container (Fl_Flex::HORIZONTAL or Fl_Flex::VERTICAL).
  The other dimension is set to the full width or height of the Fl_Flex widget.

  This can be used to set a fixed widget width or height of children
  of Fl_Flex so they are not resized dynamically.

  If \p size is 0 (zero) or negative the widget size is reset to flexible size.

  \param[in]  w   widget to be affected
  \param[in]  size  width (Fl_Flex::HORIZONTAL) or height (Fl_Flex::VERTICAL)
*/
void Fl_Flex::set_size(Fl_Widget *w, int size) {
  if (size <= 0)
    size = 0;
  w->resize(0, 0, size, size);

  int idx = -1;
  for (int i = 0; i < set_size_size_; i++) {
    if (set_size_[i] == w) {
      idx = i;
      break;
    }
  }

  // remove from array ?
  if (size == 0 && idx >= 0) {
    for (int i = idx; i < set_size_size_ - 2; i++) {
      set_size_[i] = set_size_[i+1];
    }
    set_size_size_--;
    return;
  }

  // add to array of fixed size widgets
  if (set_size_size_ == set_size_alloc_) {
    set_size_alloc_ = alloc_size(set_size_alloc_);
    set_size_ = (Fl_Widget **)realloc(set_size_, set_size_alloc_ * sizeof(Fl_Widget *));
  }
  set_size_[set_size_size_] = w;
  set_size_size_++;
}

/**
  Return whether the given widget has a fixed size or resizes dynamically.

  \param[in]  w   widget
  \return         whether the widget has a fixed size
  \retval     1   the widget has a fixed size
  \retval     0  the widget resizes dynamically
*/
int Fl_Flex::set_size(Fl_Widget *w) {
  for (int i = 0; i < set_size_size_; i++) {
    if (w == set_size_[i]) {
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
int Fl_Flex::alloc_size(int size) {
  return size + 8;
}
