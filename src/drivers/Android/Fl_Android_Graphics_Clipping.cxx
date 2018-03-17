//
// "$Id$"
//
// Clipping region routines for the Fast Light Tool Kit (FLTK).
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


#include "../../config_lib.h"
#include "Fl_Android_Graphics_Driver.H"
#include "Fl_Android_Application.H"
#include <FL/platform.H>


/**
 * Create an empty clipping region.
 */
Fl_Rect_Region::Fl_Rect_Region() :
        pLeft(0), pTop(0), pRight(0), pBottom(0)
{
}

/**
 * Create a clipping region based on position and size.
 * @param x, y position
 * @param w, h size
 */
Fl_Rect_Region::Fl_Rect_Region(int x, int y, int w, int h) :
        pLeft(x), pTop(y), pRight(x+w), pBottom(y+h)
{
}

/**
 * Clone a clipping rectangle.
 */
Fl_Rect_Region::Fl_Rect_Region(const Fl_Rect_Region &r) :
        pLeft(r.pLeft), pTop(r.pTop),
        pRight(r.pRight), pBottom(r.pBottom)
{
}

/**
 * Clone a clipping rectangle.
 * The pointer can be NULL if an empty rectangle is needed.
 */
Fl_Rect_Region::Fl_Rect_Region(enum Type what)
{
  if (what==INFINITE) {
    pLeft = pTop = INT_MIN;
    pRight = pBottom = INT_MAX;
  } else {
    pLeft = pTop = pRight = pBottom = 0;
  }
}

/**
 * If the rectangle has no width or height, it's considered empty.
 * @return true, if everything will be clipped and there is nothing to draw
 */
bool Fl_Rect_Region::is_empty() const
{
  return (pRight<=pLeft || pBottom<=pTop);
}

/**
 * Return true, if the rectangle is of unlimited size and nothing should be clipped.
 * @return treu, if there is no clipping
 */
bool Fl_Rect_Region::is_infinite() const
{
  return (pLeft==INT_MIN);
}

/**
 * Set an empty clipping rect.
 */
void Fl_Rect_Region::set_empty()
{
  pLeft = pTop = pRight = pBottom = 0;
}

/**
 * Set a clipping rect using position and size
 * @param x, y position
 * @param w, h size
 */
void Fl_Rect_Region::set(int x, int y, int w, int h)
{
  pLeft = x;
  pTop = y;
  pRight = x+w;
  pBottom = y+h;
}

/**
 * Set a rectangle using the coordinates of two points, top left and bottom right.
 * @param l, t left and top coordinate
 * @param r, b right and bottom coordinate
 */
void Fl_Rect_Region::set_ltrb(int l, int t, int r, int b)
{
  pLeft = l;
  pTop = t;
  pRight = r;
  pBottom = b;
}

/**
 * Copy the corrdinates from another rect.
 * @param r source rectangle
 */
void Fl_Rect_Region::set(const Fl_Rect_Region &r)
{
  pLeft = r.pLeft;
  pTop = r.pTop;
  pRight = r.pRight;
  pBottom = r.pBottom;
}

/**
 * Set this rect to be the intersecting area between the original rect and another rect.
 * @param r another rectangular region
 * @return EMPTY, if rectangles are not intersecting, SAME if this and rect are
 *      equal, LESS if the new rect is smaller than the original rect
 */
int Fl_Rect_Region::intersect_with(const Fl_Rect_Region &r)
{
  if (is_empty()) {
    return EMPTY;
  }
  if (r.is_empty()) {
    set_empty();
    return EMPTY;
  }
  bool same = true;
  if ( pLeft != r.pLeft ) {
    same = false;
    if ( r.pLeft > pLeft ) pLeft = r.pLeft;
  }
  if ( pTop != r.pTop ) {
    same = false;
    if ( r.pTop > pTop ) pTop = r.pTop;
  }
  if ( pRight != r.pRight ) {
    same = false;
    if ( r.pRight < pRight ) pRight = r.pRight;
  }
  if ( pBottom != r.pBottom ) {
    same = false;
    if ( r.pBottom < pBottom ) pBottom = r.pBottom;
  }
  if (same)
    return SAME;
  if (is_empty())
    return EMPTY;
  return LESS;
}

/**
 * Use rectangle as a bounding box and add the outline of another rect.
 */
void Fl_Rect_Region::add_to_bbox(const Fl_Rect_Region &r)
{
  if (r.pLeft<pLeft) pLeft = r.pLeft;
  if (r.pTop<pTop) pTop = r.pTop;
  if (r.pRight>pRight) pRight = r.pRight;
  if (r.pBottom>pBottom) pBottom = r.pBottom;
}

/**
 * Print the coordinates of the rect to the log.
 * @param label some text that is logged with this message.
 */
void Fl_Rect_Region::print(const char *label) const
{
  Fl_Android_Application::log_i("---> Fl_Rect_Region: %s", label);
  Fl_Android_Application::log_i("Rect l:%d t:%d r:%d b:%d", left(), top(), right(), bottom());
}

// =============================================================================

/**
 * Create an empty complex region.
 */
Fl_Complex_Region::Fl_Complex_Region() :
        Fl_Rect_Region()
{
}

/**
 * Create a complex region with the same bounds as the give rect.
 * @param r region size
 */
Fl_Complex_Region::Fl_Complex_Region(const Fl_Rect_Region &r) :
        Fl_Rect_Region(r)
{
}

/**
 * Delete this region, all subregions recursively, and all following regions.
 */
Fl_Complex_Region::~Fl_Complex_Region()
{
  delete pSubregion; // recursively delete all subregions
  delete pNext; // recursively delete all following regions
}

/**
 * Print the entire content of this region recursively.
 */
void Fl_Complex_Region::print(const char *label) const
{
  Fl_Android_Application::log_i("---> Fl_Complex_Region: %s", label);
  print_data(0);
}

/*
 * Print the rectangular data only.
 */
void Fl_Complex_Region::print_data(int indent) const
{
  static const char *space = "                ";
  if (pSubregion) {
    Fl_Android_Application::log_i("%sBBox l:%d t:%d r:%d b:%d", space+16-indent, left(), top(), right(), bottom());
    pSubregion->print_data(indent+1);
  } else {
    Fl_Android_Application::log_i("%sRect l:%d t:%d r:%d b:%d", space+16-indent, left(), top(), right(), bottom());
  }
  if (pNext) {
    pNext->print_data(indent);
  }
}

/**
 * Replace this region with a rectangle.
 * @param r the source rectangle
 */
void Fl_Complex_Region::set(const Fl_Rect_Region &r)
{
  Fl_Rect_Region::set(r);
  delete pSubregion; pSubregion = 0;
}

/**
 * Replace this region with a copy of another region.
 * This operation can be expensive for very complex regions.
 * @param r the source region
 */
void Fl_Complex_Region::set(const Fl_Complex_Region &r)
{
  // outline:
  // clear this region and copy the coordinates from r
  delete pSubregion; pSubregion = 0;
  Fl_Rect_Region::set((const Fl_Rect_Region&)r);
  if (r.pSubregion) {
    pSubregion = new Fl_Complex_Region();
    pSubregion->set(*r.subregion());
  }
  if (r.pNext) {
    pNext = new Fl_Complex_Region();
    pNext->set(*r.next());
  }
}

/**
 * Set this region to the intersection of the original region and some rect.
 * @param r intersect with this rectangle
 * @return EMPTY, SAME, LESS
 */
int Fl_Complex_Region::intersect_with(const Fl_Rect_Region &r)
{
  set(r);
  return LESS;
  delete pSubregion; pSubregion = 0;
  // FIXME: handle complex regions!
  int ret = Fl_Rect_Region::intersect_with(r);
  return ret;
}

/**
 * Subtract a rectangular region from this region.
 * @param r the rect that we want removed
 * @return currently 0, but could return something meaningful
 */
int Fl_Complex_Region::subtract(const Fl_Rect_Region &r)
{
  if (pSubregion) {
    pSubregion->subtract(r);
  } else {
    // Check if we overlap at all
    Fl_Rect_Region s(r);
    int intersects = s.intersect_with(*this);
    switch (intersects) {
      case EMPTY:
        // nothing to do
        break;
      case SAME:
        set_empty(); // Will be deleted by compress()
        break;
      case LESS:
        // split this rect into 1, 2, 3, or 4 new ones
        subtract_smaller_region(s);
        break;
      default:
        Fl_Android_Application::log_e("Invalid case in %s:%d", __FUNCTION__, __LINE__);
        break;
    }
    if (pNext) {
      pNext->subtract(r);
    }
  }
  compress();
  return 0;
}

/**
 * Compress the subregion of this region if possible and update the bounding
 * box of this region.
 *
 * Does not recurse down the tree!
 */
void Fl_Complex_Region::compress()
{
  // Can't compress anything that does not have a subregion
  if (!pSubregion) return;

  // remove all empty regions, because the really don't add anything (literally)
  Fl_Complex_Region *rgn = pSubregion, **rgnPtr = &pSubregion;
  while (rgn) {
    if (rgn->is_empty()) {
      *rgnPtr = rgn->pNext;
      delete rgn;
    }
    rgnPtr = &rgn->pNext;
    rgn = *rgnPtr;
  }
  if (!pSubregion) return;

  // find rectangles that can be merged into a single new rectangle

  // if there is only a single subregion left, merge it into this region
  if (pSubregion->pNext==nullptr) {
    set((Fl_Rect_Region&)*pSubregion); // deletes subregion for us
  }
  if (!pSubregion) return;

  // finally, update the boudning box
  Fl_Rect_Region::set((Fl_Rect_Region&)*pSubregion);
  for (rgn=pSubregion->pNext; rgn; rgn=rgn->pNext) {
    add_to_bbox(*rgn);
  }
}

/**
 * Subtract a smaller rect from a larger rect, potentially creating four new rectangles.
 * This assumes that the calling region is NOT complex.
 * @param r subtract the area of this rectangle; r must fit within ``this``.
 * @return currently 0, but this may change
 */
int Fl_Complex_Region::subtract_smaller_region(const Fl_Rect_Region &r)
{
  // subtract a smaller rect from a larger rect and create subrects as needed
  // if there is only one single coordinate different, we can reuse this container
  if (left()==r.left() && top()==r.top() && right()==r.right() && bottom()==r.bottom()) {
    // this should not happen
    set_empty();
  } else if (left()!=r.left() && top()==r.top() && right()==r.right() && bottom()==r.bottom()) {
    pRight = r.left();
  } else if (left()==r.left() && top()!=r.top() && right()==r.right() && bottom()==r.bottom()) {
    pBottom = r.top();
  } else if (left()==r.left() && top()==r.top() && right()!=r.right() && bottom()==r.bottom()) {
    pLeft = r.right();
  } else if (left()==r.left() && top()==r.top() && right()==r.right() && bottom()!=r.bottom()) {
    pTop = r.bottom();
  } else {
    // create multiple regions
    if (pTop!=r.top()) {
      Fl_Complex_Region *s = add_subregion();
      s->set_ltrb(pLeft, pTop, pRight, r.top());
    }
    if (pBottom!=r.bottom()) {
      Fl_Complex_Region *s = add_subregion();
      s->set_ltrb(pLeft, r.bottom(), pRight, pBottom);
    }
    if (pLeft!=r.left()) {
      Fl_Complex_Region *s = add_subregion();
      s->set_ltrb(pLeft, r.top(), r.left(), r.bottom());
    }
    if (pRight!=r.right()) {
      Fl_Complex_Region *s = add_subregion();
      s->set_ltrb(r.right(), r.top(), pRight, r.bottom());
    }
  }
  return 0;
}

/**
 * Add an empty subregion to the current region.
 * @return a pointer to the newly created region.
 */
Fl_Complex_Region *Fl_Complex_Region::add_subregion()
{
  Fl_Complex_Region *r = new Fl_Complex_Region();
  r->pParent = this;
  r->pNext = pSubregion;
  pSubregion = r;
  return r;
}


// -----------------------------------------------------------------------------

/**
 * Returns an interator object for loops that traverse the entire region tree.
 * C++11 interface to range-based loops.
 * @return Iterator pointing to the first element.
 */
Fl_Complex_Region::Iterator Fl_Complex_Region::begin()
{
  return Iterator(this);
}

/**
 * Returns an interator object to mark the end of travesing the tree.
 * C++11 interface to range-based loops.
 * @return
 */
Fl_Complex_Region::Iterator Fl_Complex_Region::end()
{
  return Iterator(0L);
}

/**
 * Create an iterator to walk the entire tree.
 * @param r Iterate through this region, r must not have a parent().
 */
Fl_Complex_Region::Iterator::Iterator(Fl_Complex_Region *r) :
        pRegion(r)
{
}

/**
 * Compare two iterators.
 * C++11 needs this to find the end of a for loop.
 * @param other
 * @return
 */
bool Fl_Complex_Region::Iterator::operator!=(const Iterator &other) const
{
  return pRegion != other.pRegion;
}

/**
 * Set the iterator to the next object in the tree, down first.
 * C++11 needs this to iterate in a for loop.
 * @return
 */
const Fl_Complex_Region::Iterator &Fl_Complex_Region::Iterator::operator++()
{
  if (pRegion->subregion()) {
    pRegion = pRegion->subregion();
  } else if (pRegion->next()) {
    pRegion = pRegion->next();
  } else {
    pRegion = pRegion->parent();
  }
  return *this;
}

/**
 * Return the current object while iterating through the tree.
 * @return
 */
Fl_Complex_Region *Fl_Complex_Region::Iterator::operator*() const
{
  return pRegion;
}

// -----------------------------------------------------------------------------

/**
 * Use this to iterate through a region, hitting only nodes that intersect with this rect.
 * @param r find all parts of the region that intersect with this rect.
 * @return an object that can be used in range-based for loops in C++11.
 */
Fl_Complex_Region::Overlapping Fl_Complex_Region::overlapping(const Fl_Rect_Region &r)
{
  return Overlapping(this, r);
}

/**
 * A helper object for iterating through a region, finding only overlapping rects.
 * @param rgn
 * @param rect
 */
Fl_Complex_Region::Overlapping::Overlapping(Fl_Complex_Region *rgn,
                                            const Fl_Rect_Region &rect) :
        pRegion(rgn),
        pOriginalRect(rect),
        pClippedRect(rect)
{
}

/**
 * Return an itertor for the first clipping rectangle inside the region.
 * @return
 */
Fl_Complex_Region::Overlapping::OverlappingIterator Fl_Complex_Region::Overlapping::begin()
{
  find_intersecting();
  return OverlappingIterator(this);
}

/**
 * Return an iterator for the end of forward iteration.
 * @return
 */
Fl_Complex_Region::Overlapping::OverlappingIterator Fl_Complex_Region::Overlapping::end()
{
  return OverlappingIterator(0L);
}

/**
 * Return the result of intersecting the original rect with this iterator.
 * @return
 */
Fl_Rect_Region &Fl_Complex_Region::Overlapping::clipped_rect()
{
  return pClippedRect;
}

/**
 * Store the intersection in pClippedRect and return true if there was an intersection.
 * @return
 */
bool Fl_Complex_Region::Overlapping::intersects()
{
  return (pClippedRect.intersect_with(*pRegion) != EMPTY);
}

/**
 * Find the next element in the tree that actually intersects with the initial rect.
 * Starting the search at the current object, NOT the next object.
 * @return
 */
bool Fl_Complex_Region::Overlapping::find_intersecting()
{
  for (;;) {
    if (!pRegion) return false;
    pClippedRect.set(pOriginalRect);
    if (intersects()) {
      if (!pRegion->subregion()) {
        return true;
      } else {
        pRegion = pRegion->subregion();
      }
    } else {
      find_next();
    }
  }
}

/**
 * Find the next object in the tree, complex, simple, intersecting or not.
 * @return
 */
bool Fl_Complex_Region::Overlapping::find_next()
{
  if (pRegion->subregion()) {
    pRegion = pRegion->subregion();
  } else if (pRegion->next()) {
    pRegion = pRegion->next();
  } else {
    pRegion = pRegion->parent(); // can be NULL
  }
  return (pRegion != 0L);
}

// -----------------------------------------------------------------------------

/**
 * Create the actual iterator for finding true clipping rects.
 * @see Fl_Complex_Region::Overlapping
 * @param ov
 */
Fl_Complex_Region::Overlapping::OverlappingIterator::OverlappingIterator(
        Overlapping *ov) :
        pOv(ov)
{
}

/**
 * Compare two iterator.
 * This is used by C++11 range0based for loops to find the end of the range.
 * @param other
 * @return
 */
bool Fl_Complex_Region::Overlapping::OverlappingIterator::operator!=(
        const OverlappingIterator &other) const
{
  auto thisRegion = pOv ? pOv->pRegion : nullptr;
  auto otherRegion = other.pOv ? other.pOv->pRegion : nullptr;
  return thisRegion != otherRegion;
}

/**
 * Wrapper to find and set the next intersecting rectangle.
 * @see Fl_Complex_Region::Overlapping::find_intersecting
 * @see Fl_Complex_Region::Overlapping::find_next
 * @return
 */
const Fl_Complex_Region::Overlapping::OverlappingIterator &
Fl_Complex_Region::Overlapping::OverlappingIterator::operator++()
{
  pOv->find_next();
  if (pOv->pRegion)
    pOv->find_intersecting();
  return *this;
}

/**
 * Return the Fl_Complex_Region::Overlapping state for this iterator.
 * This gives the user access to the current rectangular fragment of
 * the clipping region.
 * @return
 */
Fl_Complex_Region::Overlapping *
Fl_Complex_Region::Overlapping::OverlappingIterator::operator*() const
{
  return pOv;
}

// =============================================================================


void Fl_Android_Graphics_Driver::restore_clip()
{
  fl_clip_state_number++;

  // find the current user clipping rectangle
  Fl_Region b = rstack[rstackptr]; // Fl_Region is a pointer to Fl_Rect_Region
  if (b) {
    if (b->is_empty()) {
      // if this is an empty region, the intersection is always empty as well
      pClippingRegion.set_empty();
    } else {
      // if there is a region, copy the full window region
      pClippingRegion.set(pDesktopWindowRegion);
      if (!b->is_infinite()) {
        // if the rect has dimensions, calculate the intersection
        pClippingRegion.intersect_with(*b);
      }
    }
  } else {
    // no rect? Just copy the window region
    pClippingRegion.set(pDesktopWindowRegion);
  }
}


void Fl_Android_Graphics_Driver::clip_region(Fl_Region r)
{
  Fl_Region oldr = rstack[rstackptr];
  if (oldr)
    ::free(oldr);
  rstack[rstackptr] = r;
  restore_clip();
}


Fl_Region Fl_Android_Graphics_Driver::clip_region()
{
  return rstack[rstackptr];
}


void Fl_Android_Graphics_Driver::push_clip(int x, int y, int w, int h)
{
  Fl_Region r;
  if (w > 0 && h > 0) {
    r = new Fl_Rect_Region(x, y, w, h);
    Fl_Region current = rstack[rstackptr];
    if (current) {
      r->intersect_with(*current);
    }
  } else { // make empty clip region:
    r = new Fl_Rect_Region();
  }
  if (rstackptr < region_stack_max) rstack[++rstackptr] = r;
  else Fl::warning("Fl_Android_Graphics_Driver::push_clip: clip stack overflow!\n");
  restore_clip();
}


void Fl_Android_Graphics_Driver::push_no_clip()
{
  if (rstackptr < region_stack_max) rstack[++rstackptr] = 0;
  else Fl::warning("Fl_Android_Graphics_Driver::push_no_clip: clip stack overflow!\n");
  restore_clip();
}


void Fl_Android_Graphics_Driver::pop_clip()
{
  if (rstackptr > 0) {
    Fl_Region oldr = rstack[rstackptr--];
    if (oldr)
      ::free(oldr);
  } else Fl::warning("Fl_Android_Graphics_Driver::pop_clip: clip stack underflow!\n");
  restore_clip();
}

/*
 Intersects the rectangle with the current clip region and returns the
 bounding box of the result.

 Returns non-zero if the resulting rectangle is different to the original.
 This can be used to limit the necessary drawing to a rectangle.
 \p W and \p H are set to zero if the rectangle is completely outside the region.
 \param[in] x,y,w,h position and size of rectangle
 \param[out] X,Y,W,H position and size of resulting bounding box.
 \returns Non-zero if the resulting rectangle is different to the original.
 */
int Fl_Android_Graphics_Driver::clip_box(int x, int y, int w, int h, int& X, int& Y, int& W, int& H)
{
  Fl_Region r = rstack[rstackptr];
  if (r) {
    Fl_Rect_Region a(x, y, w, h);
    int ret = a.intersect_with(*r);
    X = a.x();
    Y = a.y();
    W = a.w();
    H = a.h();
    return (ret!=Fl_Rect_Region::SAME);
  } else {
    X = x; Y = y; W = w; H = h;
    return 0;
  }
}

/*
 Does the rectangle intersect the current clip region?
 \param[in] x,y,w,h position and size of rectangle
 \returns non-zero if any of the rectangle intersects the current clip
 region. If this returns 0 you don't have to draw the object.

 \note
 Under X this returns 2 if the rectangle is partially clipped,
 and 1 if it is entirely inside the clip region.
 */
int Fl_Android_Graphics_Driver::not_clipped(int x, int y, int w, int h)
{
  if (w <= 0 || h <= 0) return 0;
  Fl_Region r = rstack[rstackptr];
  if (r) {
    Fl_Rect_Region a(x, y, w, h); // return 0 for empty, 1 for same, 2 if intersecting
    return a.intersect_with(*r);
  } else {
    return 1;
  }
}


//
// End of "$Id$".
//
