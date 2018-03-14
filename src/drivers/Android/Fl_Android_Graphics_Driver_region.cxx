//
// "$Id$"
//
// Clipping region routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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


void Fl_Rect_Region::set_empty()
{
  pLeft = pTop = pRight = pBottom = 0;
}


void Fl_Rect_Region::set(int x, int y, int w, int h)
{
  pLeft = x;
  pTop = y;
  pRight = x+w;
  pBottom = y+h;
}


void Fl_Rect_Region::set(const Fl_Rect_Region &r)
{
  pLeft = r.pLeft;
  pTop = r.pTop;
  pRight = r.pRight;
  pBottom = r.pBottom;
}


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


void Fl_Rect_Region::print(const char *label) const
{
  Fl_Android_Application::log_i("---> Fl_Rect_Region: %s", label);
  Fl_Android_Application::log_i("Rect %d %d %d %d", x(), y(), w(), h());
}

// -----------------------------------------------------------------------------

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
    Fl_Android_Application::log_i("%sBBox %d %d %d %d", space+16-indent, x(), y(), w(), h());
    pSubregion->print_data(indent+1);
  } else {
    Fl_Android_Application::log_i("%sRect %d %d %d %d", space+16-indent, x(), y(), w(), h());
  }
  if (pNext) {
    pNext->print_data(indent+1);
  }
}


void Fl_Complex_Region::set(const Fl_Rect_Region &r)
{
  delete pSubregion; pSubregion = 0;
  Fl_Rect_Region::set(r);
}


int Fl_Complex_Region::intersect_with(const Fl_Rect_Region &r)
{
  delete pSubregion; pSubregion = 0;
  // FIXME: handle complex regions!
  int ret = Fl_Rect_Region::intersect_with(r);
  return ret;
}


int Fl_Complex_Region::subtract(const Fl_Rect_Region &r)
{
  // FIXME: write me.
  if (pSubregion) {
    pSubregion->subtract(r);
  } else {
    // Check if we overlap at all
    Fl_Rect_Region s(r);
    this->print("---- subtract");
    s.print("");
    int intersects = s.intersect_with(*this);
    switch (intersects) {
      case EMPTY:
        // nothing to do
        break;
      case SAME:
        set_empty(); // FIXME: delete this Rect!
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
  return 0;
}


int Fl_Complex_Region::subtract_smaller_region(const Fl_Rect_Region &r)
{
  // subtract a smaller rect from a larger rect and create subrects as needed

  print("subtract_smaller_region");
  r.print("");
  // if there is only one single coordinte different, we can reuse this container
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
    Fl_Android_Application::log_w("Regions too complex. Not yet implemented");
  }
  print("subtract_smaller_region");
  r.print("");
  return 0;
}


#if 0

void Fl_Complex_Region::set(int x, int y, int w, int h)
{
  // TODO: refactor the next four lines out, repeating
  delete pSubregion;
  pSubregion = 0L;
  delete pNext;
  pNext = 0L;
  Fl_Rect_Region::set(x, y, w, h);
}

void Fl_Complex_Region::set(Fl_Rect *rect)
{
  delete pSubregion;
  pSubregion = 0L;
  delete pNext;
  pNext = 0L;
  Fl_Rect_Region::set(rect);
}

/**
 * Subtract a rectangle from this region.
 *
 * This operation may create multiple new subregions, but possibly also remove
 * subregions.
 *
 * @param x, y, w, h rectangular region that will be subtracted
 */
void Fl_Complex_Region::subtract(Fl_Rect *r)
{
  Fl_Android_Application::log_i("------------ subtract");
  this->print();
  Fl_Android_Application::log_i("--------");
  Fl_Android_Application::log_i("Rect %d %d %d %d", r->x(), r->y(), r->w(), r->h());
  // ----
  this->print();
  Fl_Android_Application::log_i("------------ subtract done");
  // FIXME: implement
  int x = 3;
}

void Fl_Complex_Region::intersect(Fl_Rect*)
{
  // FIXME: implement
}

void Fl_Complex_Region::clone(Fl_Complex_Region *r)
{
  // FIXME: implement
  // make this region simple and copy the bounding box
  set(r);
  if (r->pSubregion) {
    pSubregion = new Fl_Complex_Region();
    pSubregion->clone(r->pSubregion);
  }
  if (r->pNext) {
    pNext = new Fl_Complex_Region();
    pNext->clone(r->pNext);
  }
}

void Fl_Complex_Region::print()
{
  Fl_Android_Application::log_i("-------- begin region");
  print_data(0);
}

#endif



void Fl_Android_Graphics_Driver::restore_clip()
{
  fl_clip_state_number++;

  pClippingRegion.set(pDesktopWindowRegion);


  Fl_Region b = rstack[rstackptr];
  if (b) {
    pClippingRegion.intersect_with(*b);
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
