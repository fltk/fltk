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


// return 0 for empty, 1 for same, 2 if intersecting
int Fl_Rect_Region::intersect_with(Fl_Rect_Region *a)
{
  if (is_empty()) {
    return EMPTY;
  }
  if (a->is_empty()) {
    clear();
    return EMPTY;
  }
  int lx = max(x(), a->x());
  int ly = max(y(), a->y());
  int lr = min(r(), a->r());
  int lb = min(b(), a->b());
  int lw = lr-lx;
  int lh = lb-ly;
  if (equals(lx, ly, lw, lh)) {
    return SAME;
  }
  set(lx, ly, lw, lh);
  if ( (w()<=0) || (h()<=0) ) {
    clear();
    return EMPTY;
  }
  return LESS;
}


void Fl_Rect_Region::print()
{
  Fl_Android_Application::log_i("-------- begin rect");
  Fl_Android_Application::log_i("Rect %d %d %d %d", x(), y(), w(), h());
}



Fl_Complex_Region::~Fl_Complex_Region()
{
  delete pSubregion; // recursively delete all subregions
  delete pNext; // recursively delete all following regions
}


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


void Fl_Complex_Region::print_data(int indent)
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


void Fl_Complex_Region::print()
{
  Fl_Android_Application::log_i("-------- begin region");
  print_data(0);
}




void Fl_Android_Graphics_Driver::restore_clip()
{
  fl_clip_state_number++;

  // TODO: we can optimize this by using some "copy on write" system
  Fl_Android_Application::log_i("------------ restore_clip");
  pDesktopRegion->print();
  //pClippingRegion->set(pWindowRegion);
  pClippingRegion->clone(pDesktopRegion);

  Fl_Region b = rstack[rstackptr];
  if (b) {
    pClippingRegion->intersect_with(b);
    pClippingRegion->print();
  }
  Fl_Android_Application::log_i("------------ restore_clip done");
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
    r = new Fl_Rect_Region(x,y,w,h);
    Fl_Region current = rstack[rstackptr];
    if (current) {
      r->intersect_with(current);
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
  if (!r) {
    X = x; Y = y; W = w; H = h;
    return 0;
  }

  Fl_Rect_Region a(x, y, w, h);
  int ret = a.intersect_with(r); // return 0 for empty, 1 for same, 2 if intersecting
  X = a.x();
  Y = a.y();
  W = a.w();
  H = a.h();

  return (ret!=1);
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
int Fl_Android_Graphics_Driver::not_clipped(int x, int y, int w, int h) {
  if (x+w <= 0 || y+h <= 0) return 0;
  Fl_Region r = rstack[rstackptr];
  if (!r) return 1;

  Fl_Rect_Region a(x, y, w, h); // return 0 for empty, 1 for same, 2 if intersecting
  return a.intersect_with(r);
}

#if 0

// --- clipping

int Fl_GDI_Graphics_Driver::clip_box(int x, int y, int w, int h, int& X, int& Y, int& W, int& H){
  X = x; Y = y; W = w; H = h;
  Fl_Region r = rstack[rstackptr];
  if (!r) return 0;
  // The win32 API makes no distinction between partial and complete
  // intersection, so we have to check for partial intersection ourselves.
  // However, given that the regions may be composite, we have to do
  // some voodoo stuff...
  Fl_Region rr = XRectangleRegion(x,y,w,h);
  Fl_Region temp = CreateRectRgn(0,0,0,0);
  int ret;
  if (CombineRgn(temp, rr, r, RGN_AND) == NULLREGION) { // disjoint
    W = H = 0;
    ret = 2;
  } else if (EqualRgn(temp, rr)) { // complete
    ret = 0;
  } else {	// partial intersection
    RECT rect;
    GetRgnBox(temp, &rect);
    if (Fl_Surface_Device::surface() != Fl_Display_Device::display_device()) { // if print context, convert coords from device to logical
      POINT pt[2] = { {rect.left, rect.top}, {rect.right, rect.bottom} };
      DPtoLP(gc_, pt, 2);
      X = pt[0].x; Y = pt[0].y; W = pt[1].x - X; H = pt[1].y - Y;
    }
    else {
      X = rect.left; Y = rect.top; W = rect.right - X; H = rect.bottom - Y;
    }
    ret = 1;
  }
  DeleteObject(temp);
  DeleteObject(rr);
  return ret;
}

int Fl_GDI_Graphics_Driver::not_clipped(int x, int y, int w, int h) {
  if (x+w <= 0 || y+h <= 0) return 0;
  Fl_Region r = rstack[rstackptr];
  if (!r) return 1;
  RECT rect;
  if (Fl_Surface_Device::surface() != Fl_Display_Device::display_device()) { // in case of print context, convert coords from logical to device
    POINT pt[2] = { {x, y}, {x + w, y + h} };
    LPtoDP(gc_, pt, 2);
    rect.left = pt[0].x; rect.top = pt[0].y; rect.right = pt[1].x; rect.bottom = pt[1].y;
  } else {
    rect.left = x; rect.top = y; rect.right = x+w; rect.bottom = y+h;
  }
  return RectInRegion(r,&rect);
}


#endif


//
// End of "$Id$".
//
