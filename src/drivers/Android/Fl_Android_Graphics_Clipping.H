//
// "$Id$"
//
// Graphics regions and clipping for the Fast Light Tool Kit (FLTK).
//
// Copyright 2018 by Bill Spitzak and others.
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

/**
 \file Fl_Android_Graphics_Clipping.H
 \brief Graphics regions and clipping for the Fast Light Tool Kit (FLTK).
 */

#ifndef FL_ANDROID_GRAPHICS_CLIPPING_H
#define FL_ANDROID_GRAPHICS_CLIPPING_H

#include <FL/Fl_Graphics_Driver.H>
#include <limits.h>


class Fl_Android_Window_Driver;


/**
 * The Fl_Rect_Region describes a rectangular clipping region.
 *
 * Contrary to common FLTK convention, rectangles are stored with coordinates
 * instead of their width and height to accelerate calculations. The discreet
 * constructor however uses the old convention for convenience.
 */
class Fl_Rect_Region
{
public:
  enum Type {
    EMPTY = 0, SAME, LESS, MORE, INFINITE
  };

  Fl_Rect_Region();
  Fl_Rect_Region(int x, int y, int w, int h);
  Fl_Rect_Region(const Fl_Rect_Region&);
  Fl_Rect_Region(enum Type what);
  virtual ~Fl_Rect_Region() { }

  int x() const { return pLeft; }
  int y() const { return pTop; }
  int w() const { return pRight - pLeft; }
  int h() const { return pBottom - pTop; }

  int left() const { return pLeft; }
  int top() const { return pTop; }
  int right() const { return pRight; }
  int bottom() const { return pBottom; }

  bool is_empty() const;
  bool is_infinite() const;

  virtual void set_empty();
  void set(int x, int y, int w, int h);
  void set_ltrb(int l, int t, int r, int b);
  virtual void set(const Fl_Rect_Region &r);
  virtual int intersect_with(const Fl_Rect_Region &r);
  void add_to_bbox(const Fl_Rect_Region &r);

  virtual void print(const char*) const;

protected:
  int pLeft, pTop, pRight, pBottom;

private:
  Fl_Rect_Region&  operator = (const Fl_Rect_Region& other);
};


/**
 * The Fl_Complex_Region represents a clipping region of any shape.
 *
 * This class is organized in a tree-like structure. If the region is
 * rectangular, is_simple() returns 1 and the rectangle can be used just
 * as in Fl_Rect_Region.
 *
 * If a more complex representation is needed, subregions are created which are
 * guaranteed to lie within the bounding box of the current region. Subregions
 * themselves can again contain sub-subregions to describe the entire clipping
 * region, effectively creating a tree where the leafs contain the rectangles
 * that together describe the clipping area.
 *
 * To make life easier, Fl_Complex_Region provides two types of iterator to
 * travers the entire tree.
 *
 * 1. Fl_Complex_Region itself is compatible to C++11 range-based loops and
 *    can bewalked simply by writing ``for (auto &&it: rgn) { ... }``.
 *
 * 2. Fl_Complex_Region provides an alternative iterator that loop only through
 *    leafs that intersects with a given rectangle. The returned object
 *    provides access to the readily clipped rectangle.
 *
 * @code
 * Fl_Complex_Region rgn(something);
 * for (auto &&it: rgn.iterate(Fl_Rect_Region(0, 0, 100, 100)) {
 *    draw_something(it->rect());
 * }
 * @endcode
 *
 */
class Fl_Complex_Region : public Fl_Rect_Region
{
  class Iterator {
  public:
    Iterator(Fl_Complex_Region *r);
    bool operator!= (const Iterator& other) const;
    const Iterator& operator++ ();
    Fl_Complex_Region *operator* () const;
    Fl_Complex_Region *pRegion;
  };

  class Overlapping {
    class OverlappingIterator {
    public:
      OverlappingIterator(Overlapping *ov);
      bool operator!= (const OverlappingIterator& other) const;
      const OverlappingIterator& operator++ ();
      Overlapping *operator* () const;
      Overlapping *pOv;
    };
  public:
    Overlapping(Fl_Complex_Region *rgn, const Fl_Rect_Region &rect);
    OverlappingIterator begin();
    OverlappingIterator end();
    Fl_Rect_Region &clipped_rect();
    bool intersects();
    bool find_intersecting();
    bool find_next();
    Fl_Complex_Region *pRegion;
    Fl_Rect_Region pOriginalRect;
    Fl_Rect_Region pClippedRect;
  };

public:
  Fl_Complex_Region();
  Fl_Complex_Region(const Fl_Rect_Region&);
  virtual ~Fl_Complex_Region() override;
  void delete_all_subregions();

  virtual void set(const Fl_Rect_Region &r) override;
  void set(const Fl_Complex_Region &r);
  virtual void set_empty() override { delete pSubregion; pSubregion=0L; Fl_Rect_Region::set_empty(); }
  Fl_Complex_Region *subregion() const { return pSubregion; }
  Fl_Complex_Region *next() const { return pNext; }
  Fl_Complex_Region *parent() const { return pParent; }
  char is_simple() const { return pSubregion==0; }
  char is_complex() const { return pSubregion!=0; }

  virtual int intersect_with(const Fl_Rect_Region &r) override;
  int subtract(const Fl_Rect_Region &r);

  virtual void print(const char*) const override;

  Iterator begin();
  Iterator end();

  Overlapping overlapping(const Fl_Rect_Region &r);

protected:
  void print_data(int indent) const;
  int subtract_smaller_region(const Fl_Rect_Region &r);
  Fl_Complex_Region *add_subregion();
  void compress();

  Fl_Complex_Region *pSubregion = 0L;
  Fl_Complex_Region *pParent = 0L;
  Fl_Complex_Region *pNext = 0L;
};


#endif // FL_ANDROID_GRAPHICS_CLIPPING_H

//
// End of "$Id$".
//
