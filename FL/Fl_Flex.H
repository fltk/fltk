//
// Fl_Flex widget header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Flex_H
#define Fl_Flex_H

#include <FL/Fl_Group.H>

class FL_EXPORT Fl_Flex : public Fl_Group {

  int margin_;
  int gap_;
  int set_size_size_;
  int set_size_alloc_;
  Fl_Widget **set_size_;

public:

  enum { // values for type(int)
    VERTICAL   = 0,     ///< vertical layout (one column)
    HORIZONTAL = 1,     ///< horizontal layout (one row)
    COLUMN     = 0,     ///< alias for VERTICAL
    ROW        = 1      ///< alias for HORIZONTAL
  };

  // FLTK standard constructor
  Fl_Flex(int X, int Y, int W, int H, const char *L = 0);

  // original Fl_Flex constructors:
  // backwards compatible if direction *names* { ROW | COLUMN } are used

  Fl_Flex(int direction);
  Fl_Flex(int w, int h, int direction);
  Fl_Flex(int x, int y, int w, int h, int direction);

  virtual ~Fl_Flex();

  virtual void end();
  virtual void resize(int x, int y, int w, int h);

  void set_size(Fl_Widget *w, int size);
  int set_size(Fl_Widget *w);

protected:

  void init(int t = VERTICAL);

  int alloc_size(int size);

public:

  /** Return the margin size of the widget.
    \return margin size.
  */
  int margin() { return margin_; }

  /** Set the margin and optionally the gap size of the widget.
    This method can be used to set both the margin and the gap size.

    If you don't use the second parameter \p g or supply a negative value
    the gap size is not changed.

    The margin is some free space inside the widget border \b around all child
    widgets. It has the same size at all four edges of the Fl_Flex widget.

    The gap size \p g is some free space \b between child widgets.

    \param[in]    m   margin size, must be \>= 0
    \param[in]    g   gap size, must be \>= 0, or will be ignored (if negative)

    \see gap(int)
  */

  void margin(int m, int g = -1) {
    margin_ = m < 0 ? 0 : m;
    if (g >= 0)
      gap_ = g;
  }

  /** Return the gap size of the widget.
    \return gap size between all child widgets.
  */
  int gap() { return gap_; }

  /**
    Set the gap size of the widget.

    The gap size is some free space \b between child widgets.
    The size must be \>= 0. Negative values are clamped to 0.

    \param[in]  g   gap size
  */
  void gap(int g) {
    gap_ = g < 0 ? 0 : g;
  }

  /** Returns non-zero (true) if Fl_Flex alignment is horizontal (row mode).

    \returns non-zero if Fl_Flex alignment is horizontal
    \retval 1   if type() == Fl_Flex::HORIZONTAL
    \retval 0   if type() == Fl_Flex::VERTICAL

    See class Fl_Flex documentation for details.
  */
  int horizontal() const {
    return type() == Fl_Flex::HORIZONTAL ? 1 : 0;
  }

  /**
    Calculates the layout of the widget and redraws it.

    If you change widgets in the Fl_Flex container you should call this method
    to force recalculation of child widget sizes and positions. This can be
    useful (necessary) if you hide(), show(), add() or remove() children.

    This method also calls redraw() on the Fl_Flex widget.
  */
  void layout() {
    resize(x(), y(), w(), h());
    redraw();
  }

#if (1)

  // Additional methods for backwards compatibility with "original" Fl_Flex widget

  /**
    Deprecated.
    \deprecated Please use set_size(Fl_Widget *) instead.
  */
  bool isSetSize(Fl_Widget *w) {
    return (bool)set_size(w);
  }

  /**
    Set the horizontal or vertical size of a child widget.
    \deprecated Please use set_size(Fl_Widget *, int) instead.
  */
  void setSize(Fl_Widget *w, int size) {
    set_size(w, size);
  }

#endif

};

#endif // Fl_Flex_H
