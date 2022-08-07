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

/**
  Fl_Flex is a container (layout) widget for one row or one column of widgets.

  It provides flexible positioning of its children either in one row or in one column.

  Fl_Flex is designed to be as simple as possible. You can set individual widget
  sizes or let Fl_Flex position and size the widgets to fit in the container.
  All "flexible" (i.e. non-fixed size) widgets are assigned the same width or
  height, respectively. For details see below.

  You can set the margins \b around all children at the inner side the box frame
  (if any). Fl_Flex supports setting different margin sizes on top, bottom, left
  and right sides.
  The default margin size is 0 on all edges of the container.

  You can set the gap size \b between all children. The gap size is always the
  same between all of its children. This is similar to the 'spacing' of Fl_Pack.
  The default gap size is 0.

  Fl_Flex can either consist of a single row, i.e. \p type(Fl_Flex::HORIZONTAL)
  or a single column, i.e. \p type(Fl_Flex::VERTICAL). The default value is
  Fl_Flex::VERTICAL for consistency with Fl_Pack but you can use \p type()
  to assign a row (Fl_Flex::HORIZONTAL) layout.

  If type() == Fl_Flex::HORIZONTAL widgets are resized horizontally to fit in
  the container and their height is the full Fl_Flex height minus border size
  and margins. You can set a fixed widget width by using set_size().

  If type() == Fl_Flex::VERTICAL widgets are resized vertically to fit in
  the container and their width is the full Fl_Flex width minus border size
  and margins. You can set a fixed widget height by using set_size().

  To create arbitrary spacing you can use invisible boxes of flexible or
  fixed sizes (see example below).

  Alternate constructors let you specify the layout as Fl_Flex::HORIZONTAL or
  Fl_Flex::VERTICAL directly. Fl_Flex::ROW is an alias of Fl_Flex::HORIZONTAL
  and Fl_Flex::COLUMN is an alias of Fl_Flex::VERTICAL.

  The default box type is FL_NO_BOX as inherited from Fl_Group. You \b may
  need to set a box type with a solid background depending on your layout.

  \b Important: You should always make sure that the Fl_Flex container cannot
  be resized smaller than its designed minimal size. This can usually be done by
  setting a size_range() on the window as shown in the example below. Fl_Flex
  does not take care of sensible sizes. If it is resized too small the behavior
  is undefined, i.e. widgets may overlap and/or shrink to zero size.

  \b Hint: In many cases Fl_Flex can be used as a drop-in replacement
  for Fl_Pack. This is the recommended single row/column container since
  FLTK 1.4.0. Its resizing behavior is much more predictable (as expected)
  than that of Fl_Pack which "resizes itself to shrink-wrap itself around
  all of the children".

  Fl_Flex containers can be nested so you can create flexible layouts with
  multiple columns and rows. However, if your UI design is more complex you
  may want to use Fl_Grid instead.
  At the time of this writing (Aug 7, 2022) Fl_Grid is not yet available
  but will be added before FLTK 1.4.0 gets released.

  Example:
  \image html Fl_Flex_simple.png
  \image latex  Fl_Flex_simple.png "Fl_Flex" width=6cm

  Example code:
  \code
  #include <FL/Fl.H>
  #include <FL/Fl_Double_Window.H>
  #include <FL/Fl_Flex.H>
  #include <FL/Fl_Box.H>
  #include <FL/Fl_Button.H>

  int main(int argc, char **argv) {
    Fl_Double_Window window(410, 40, "Simple Fl_Flex Demo");
    Fl_Flex flex(5, 5, 400, 30, Fl_Flex::HORIZONTAL);
    Fl_Button b1(0, 0, 0, 0, "File");
    Fl_Button b2(0, 0, 0, 0, "Save");
    Fl_Box    bx(0, 0, 0, 0);
    Fl_Button b3(0, 0, 0, 0, "Exit");
    flex.set_size(bx, 60); // set fix width of invisible box
    flex.gap(10);
    flex.end();
    window.resizable(flex);
    window.end();
    window.size_range(300, 30);
    window.show(argc, argv);
    return Fl::run();
}
  \endcode

  \since 1.4.0
*/
class FL_EXPORT Fl_Flex : public Fl_Group {

  int margin_left_;
  int margin_top_;
  int margin_right_;
  int margin_bottom_;
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

  /**
    Set the horizontal or vertical size of a child widget.

    \param[in]  w   widget to be affected
    \param[in]  size  width (Fl_Flex::HORIZONTAL) or height (Fl_Flex::VERTICAL)

    \see set_size(Fl_Widget *w, int size)
  */
  void set_size(Fl_Widget &w, int size) {
    set_size(&w, size);
  }

  void set_size(Fl_Widget *w, int size);
  int set_size(Fl_Widget *w) const;

protected:

  void init(int t = VERTICAL);

  virtual int alloc_size(int size) const;

public:

  /** Returns the left margin size of the widget.

    This returns the \b left margin of the widget which is not necessarily
    the same as all other margins.

    \note This method is useful if you never set different margin sizes.

    \see int margins(int *left, int *top, int *right, int *bottom)
      to get all four margin values.
    \return  size of left margin.
  */
  int margin() const { return margin_left_; }

  /** Returns all (four) margin sizes of the widget.

    All margin sizes are returned in the given arguments. If any argument
    is \p NULL the respective value is not returned.

    \param[in]  left    returns left margin if not \p NULL
    \param[in]  top     returns top margin if not \p NULL
    \param[in]  right   returns right margin if not \p NULL
    \param[in]  bottom  returns bottom margin if not \p NULL

    \return     whether all margins are equal
    \retval  1  all margins have the same size
    \retval  0  at least one margin has a different size
  */
  int margins(int *left, int *top, int *right, int *bottom) const {
    if (left) *left = margin_left_;
    if (top) *top = margin_top_;
    if (right) *right = margin_right_;
    if (bottom) *bottom = margin_bottom_;
    if (margin_left_ == margin_top_ && margin_top_ == margin_right_ && margin_right_ == margin_bottom_)
      return 1;
    return 0;
  }

  /** Set the margin and optionally the gap size of the widget.
    This method can be used to set both the margin and the gap size.

    If you don't use the second parameter \p g or supply a negative value
    the gap size is not changed.

    The margin is some free space inside the widget border \b around all child widgets.

    This method sets the margin to the same size at all four edges of the Fl_Flex widget.

    The gap size \p g is some free space \b between child widgets. Negative values
    (the default if this argument is omitted) do not change the gap value.

    \param[in]    m   margin size, must be \>= 0
    \param[in]    g   gap size, ignored (if negative)

    \see gap(int)
  */

  void margin(int m, int g = -1) {
    if (m < 0)
      m = 0;
    margin_left_ = margin_top_ = margin_right_ = margin_bottom_ = m;
    if (g >= 0)
      gap_ = g;
  }

  /** Set the margin sizes at all four edges of the Fl_Flex widget.

    The margin is the free space inside the widget border \b around all child widgets.

    You must use all four parameters of this method to set the four margins in the
    order \p left, \p top, \p right, \p bottom. Negative values are set to 0 (zero).

    To set all margins to equal sizes, use margin(int m)

    This method sets the margin to the same size at all four edges of the widget.

    \param[in]    left,top,right,bottom   margin sizes, must be \>= 0

    \see margin(int, int)
  */

  void margin(int left, int top, int right, int bottom) {
    margin_left_   = left < 0 ? 0 : left;
    margin_top_    = top < 0 ? 0 : top;
    margin_right_  = right < 0 ? 0 : right;
    margin_bottom_ = bottom < 0 ? 0 : bottom;
  }

  /** Return the gap size of the widget.
    \return gap size between all child widgets.
  */
  int gap() const {
    return gap_;
  }

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

  /**
    Gets the number of extra pixels of blank space that are added
    between the children.

    This method is the same as 'int gap()' and is defined to enable
    using Fl_Flex as a drop-in replacement of Fl_Pack.

    \see int gap()
  */
  int spacing() const {
    return gap_;
  }

  /**
    Sets the number of extra pixels of blank space that are added
    between the children.

    This method is the same as 'gap(int)' and is defined to enable
    using Fl_Flex as a drop-in replacement of Fl_Pack.

    \see void gap(int)
  */
  void spacing(int i) {
    gap(i);
  }

#if (1)

  // Additional methods for backwards compatibility with "original" Fl_Flex widget

  /**
    Deprecated.
    \deprecated Please use set_size(Fl_Widget *) instead.

    \see int set_size(Fl_Widget *)
  */
  bool isSetSize(Fl_Widget *w) const {
    return (bool)set_size(w);
  }

  /**
    Set the horizontal or vertical size of a child widget.
    \deprecated Please use set_size(Fl_Widget *, int) instead.

    \see set_size(Fl_Widget *, int)
  */
  void setSize(Fl_Widget *w, int size) {
    set_size(w, size);
  }

#endif

};

#endif // Fl_Flex_H
