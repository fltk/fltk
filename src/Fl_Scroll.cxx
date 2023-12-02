//
// Scroll widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Tiled_Image.H>
#include <FL/Fl_Scroll.H>
#include <FL/fl_draw.H>

/** Clear all but the scrollbars... */
void Fl_Scroll::clear() {
  // Note: the scrollbars are removed from the group before calling
  // Fl_Group::clear() to take advantage of the optimized widget removal
  // and deletion. Finally they are added to Fl_Scroll's group again. This
  // is MUCH faster than removing the widgets one by one (STR #2409).

  remove(scrollbar);
  remove(hscrollbar);
  Fl_Group::clear();
  add(hscrollbar);
  add(scrollbar);
}

/** Insure the scrollbars are the last children. */
void Fl_Scroll::fix_scrollbar_order() {
  Fl_Widget** a = (Fl_Widget**)array();
  if (a[children()-1] != &scrollbar) {
    int i,j; for (i = j = 0; j < children(); j++)
      if (a[j] != &hscrollbar && a[j] != &scrollbar) a[i++] = a[j];
    a[i++] = &hscrollbar;
    a[i++] = &scrollbar;
  }
}

// Draw widget's background and children within a specific clip region
//    So widget can just redraw damaged parts.
//
void Fl_Scroll::draw_clip(void* v,int X, int Y, int W, int H) {
  fl_push_clip(X,Y,W,H);
  Fl_Scroll* s = (Fl_Scroll*)v;
  // erase background as needed...
  switch (s->box()) {
    case FL_NO_BOX :
    case FL_UP_FRAME :
    case FL_DOWN_FRAME :
    case FL_THIN_UP_FRAME :
    case FL_THIN_DOWN_FRAME :
    case FL_ENGRAVED_FRAME :
    case FL_EMBOSSED_FRAME :
    case FL_BORDER_FRAME :
    case _FL_SHADOW_FRAME :
    case _FL_ROUNDED_FRAME :
    case _FL_OVAL_FRAME :
    case _FL_PLASTIC_UP_FRAME :
    case _FL_PLASTIC_DOWN_FRAME :
        if (s->parent() == (Fl_Group *)s->window() && Fl::scheme_bg_) {
	  Fl::scheme_bg_->draw(X-(X%((Fl_Tiled_Image *)Fl::scheme_bg_)->image()->w()),
	                       Y-(Y%((Fl_Tiled_Image *)Fl::scheme_bg_)->image()->h()),
	                       W+((Fl_Tiled_Image *)Fl::scheme_bg_)->image()->w(),
			       H+((Fl_Tiled_Image *)Fl::scheme_bg_)->image()->h());
	  break;
        }

    default :
	fl_color(s->color());
	fl_rectf(X,Y,W,H);
	break;
  }
  Fl_Widget*const* a = s->array();
  for (int i=s->children()-2; i--;) {
    Fl_Widget& o = **a++;
    s->draw_child(o);
    s->draw_outside_label(o);
  }
  fl_pop_clip();
}

/**
  Calculate visibility/size/position of scrollbars, find children's bounding box.

  The \p si parameter will be filled with data from the calculations.
  Derived classes can make use of this call to figure out the scrolling area
  eg. during resize() handling.

  \param[in,out] si -- ScrollInfo structure
*/
void Fl_Scroll::recalc_scrollbars(ScrollInfo &si) {

  // inner box of widget (excluding scrollbars)
  si.innerbox.x = x()+Fl::box_dx(box());
  si.innerbox.y = y()+Fl::box_dy(box());
  si.innerbox.w = w()-Fl::box_dw(box());
  si.innerbox.h = h()-Fl::box_dh(box());

  // accumulate a bounding box for all the children
  si.child.l = si.innerbox.x;
  si.child.r = si.innerbox.x;
  si.child.b = si.innerbox.y;
  si.child.t = si.innerbox.y;
  int first = 1;
  Fl_Widget*const* a = array();
  for (int i=children(); i--;) {
    Fl_Widget* o = *a++;
    if ( o==&scrollbar || o==&hscrollbar ) continue;
    if ( first ) {
        first = 0;
	si.child.l = o->x();
	si.child.r = o->x()+o->w();
	si.child.b = o->y()+o->h();
	si.child.t = o->y();
    } else {
	if (o->x() < si.child.l) si.child.l = o->x();
	if (o->y() < si.child.t) si.child.t = o->y();
	if (o->x()+o->w() > si.child.r) si.child.r = o->x()+o->w();
	if (o->y()+o->h() > si.child.b) si.child.b = o->y()+o->h();
    }
  }

  // Turn the scrollbars on and off as necessary.
  // See if children would fit if we had no scrollbars...
  {
    int X = si.innerbox.x;
    int Y = si.innerbox.y;
    int W = si.innerbox.w;
    int H = si.innerbox.h;

    si.scrollsize = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size();
    si.vneeded = 0;
    si.hneeded = 0;
    if (type() & VERTICAL) {
      if ((type() & ALWAYS_ON) || si.child.t < Y || si.child.b > Y+H) {
	si.vneeded = 1;
	W -= si.scrollsize;
	if (scrollbar.align() & FL_ALIGN_LEFT) X += si.scrollsize;
      }
    }
    if (type() & HORIZONTAL) {
      if ((type() & ALWAYS_ON) || si.child.l < X || si.child.r > X+W) {
	si.hneeded = 1;
	H -= si.scrollsize;
	if (scrollbar.align() & FL_ALIGN_TOP) Y += si.scrollsize;
	// recheck vertical since we added a horizontal scrollbar
	if (!si.vneeded && (type() & VERTICAL)) {
	  if ((type() & ALWAYS_ON) || si.child.t < Y || si.child.b > Y+H) {
	    si.vneeded = 1;
	    W -= si.scrollsize;
	    if (scrollbar.align() & FL_ALIGN_LEFT) X += si.scrollsize;
	  }
	}
      }
    }
    si.innerchild.x = X;
    si.innerchild.y = Y;
    si.innerchild.w = W;
    si.innerchild.h = H;
  }

  // calculate hor scrollbar position
  si.hscroll.x = si.innerchild.x;
  si.hscroll.y = (scrollbar.align() & FL_ALIGN_TOP)
		     ? si.innerbox.y
		     : si.innerbox.y + si.innerbox.h - si.scrollsize;
  si.hscroll.w = si.innerchild.w;
  si.hscroll.h = si.scrollsize;

  // calculate ver scrollbar position
  si.vscroll.x = (scrollbar.align() & FL_ALIGN_LEFT)
                     ? si.innerbox.x
		     : si.innerbox.x + si.innerbox.w - si.scrollsize;
  si.vscroll.y = si.innerchild.y;
  si.vscroll.w = si.scrollsize;
  si.vscroll.h = si.innerchild.h;

  // calculate h/v scrollbar values (pos/size/first/total)
  si.hscroll.pos = si.innerchild.x - si.child.l;
  si.hscroll.size = si.innerchild.w;
  si.hscroll.first = 0;
  si.hscroll.total = si.child.r - si.child.l;
  if ( si.hscroll.pos < 0 ) { si.hscroll.total += (-si.hscroll.pos); si.hscroll.first = si.hscroll.pos; }

  si.vscroll.pos = si.innerchild.y - si.child.t;
  si.vscroll.size = si.innerchild.h;
  si.vscroll.first = 0;
  si.vscroll.total = si.child.b - si.child.t;
  if ( si.vscroll.pos < 0 ) { si.vscroll.total += (-si.vscroll.pos); si.vscroll.first = si.vscroll.pos; }

//  printf("DEBUG --- ScrollInfo ---\n");
//  printf("DEBUG        scrollsize: %d\n", si.scrollsize);
//  printf("DEBUG  hneeded, vneeded: %d %d\n", si.hneeded, si.vneeded);
//  printf("DEBUG     innerbox.x, si.innerbox.y, si.innerbox.w,si.innerbox.h);
//  printf("DEBUG   innerchild.xywh: %d %d %d %d\n", si.innerchild.x, si.innerchild.y, si.innerchild.w, si.innerchild.h);
//  printf("DEBUG        child lrbt: %d %d %d %d\n", si.child.l, si.child.r, si.child.b, si.child.t);
//  printf("DEBUG      hscroll xywh: %d %d %d %d\n", si.hscroll.x, si.hscroll.y, si.hscroll.w, si.hscroll.h);
//  printf("DEBUG      vscroll xywh: %d %d %d %d\n", si.vscroll.x, si.vscroll.y, si.vscroll.w, si.vscroll.h);
//  printf("DEBUG  horz scroll vals: %d %d %d %d\n", si.hscroll.pos, si.hscroll.size, si.hscroll.first, si.hscroll.total);
//  printf("DEBUG  vert scroll vals: %d %d %d %d\n", si.vscroll.pos, si.vscroll.size, si.vscroll.first, si.vscroll.total);
//  printf("DEBUG \n");
}

/**
  Returns the bounding box for the interior of the scrolling area, inside
  the scrollbars.

  Currently this is only reliable after draw(), and before any resizing of
  the Fl_Scroll or any child widgets occur.

  \todo The visibility of the scrollbars ought to be checked/calculated
  outside of the draw() method (STR #1895).
*/
void Fl_Scroll::bbox(int& X, int& Y, int& W, int& H) {
  X = x()+Fl::box_dx(box());
  Y = y()+Fl::box_dy(box());
  W = w()-Fl::box_dw(box());
  H = h()-Fl::box_dh(box());
  if (scrollbar.visible()) {
    W -= scrollbar.w();
    if (scrollbar.align() & FL_ALIGN_LEFT) X += scrollbar.w();
  }
  if (hscrollbar.visible()) {
    H -= hscrollbar.h();
    if (scrollbar.align() & FL_ALIGN_TOP) Y += hscrollbar.h();
  }
}

void Fl_Scroll::draw() {
  fix_scrollbar_order();
  int X,Y,W,H; bbox(X,Y,W,H);

  uchar d = damage();

  if (d & FL_DAMAGE_ALL) { // full redraw
    draw_box(box(),x(),y(),w(),h(),color());
    draw_clip(this, X, Y, W, H);
  } else {
    if (d & FL_DAMAGE_SCROLL) {
      // scroll the contents:
      fl_scroll(X, Y, W, H, oldx-xposition_, oldy-yposition_, draw_clip, this);

      // Erase the background as needed...
      Fl_Widget*const* a = array();
      int L, R, T, B;
      L = 999999;
      R = 0;
      T = 999999;
      B = 0;
      for (int i=children()-2; i--; a++) {
        if ((*a)->x() < L) L = (*a)->x();
	if (((*a)->x() + (*a)->w()) > R) R = (*a)->x() + (*a)->w();
        if ((*a)->y() < T) T = (*a)->y();
	if (((*a)->y() + (*a)->h()) > B) B = (*a)->y() + (*a)->h();
      }
      if (L > X) draw_clip(this, X, Y, L - X, H);
      if (R < (X + W)) draw_clip(this, R, Y, X + W - R, H);
      if (T > Y) draw_clip(this, X, Y, W, T - Y);
      if (B < (Y + H)) draw_clip(this, X, B, W, Y + H - B);
    }
    if (d & FL_DAMAGE_CHILD) { // draw damaged children
      fl_push_clip(X, Y, W, H);
      Fl_Widget*const* a = array();
      for (int i=children()-2; i--;) update_child(**a++);
      fl_pop_clip();
    }
  }

  // Calculate where scrollbars should go, and draw them
  {
      ScrollInfo si;
      recalc_scrollbars(si);

      // Now that we know what's needed, make it so.
      if (si.vneeded && !scrollbar.visible()) {
	scrollbar.set_visible();
	d = FL_DAMAGE_ALL;
      }
      else if (!si.vneeded && scrollbar.visible()) {
	scrollbar.clear_visible();
	draw_clip(this, si.vscroll.x, si.vscroll.y, si.vscroll.w, si.vscroll.h);
	d = FL_DAMAGE_ALL;
      }
      if (si.hneeded && !hscrollbar.visible()) {
	hscrollbar.set_visible();
	d = FL_DAMAGE_ALL;
      }
      else if (!si.hneeded && hscrollbar.visible()) {
	hscrollbar.clear_visible();
	draw_clip(this, si.hscroll.x, si.hscroll.y, si.hscroll.w, si.hscroll.h);
	d = FL_DAMAGE_ALL;
      }
      else if ( hscrollbar.h() != si.scrollsize || scrollbar.w() != si.scrollsize ) {
         // scrollsize changed
         d = FL_DAMAGE_ALL;
      }

      scrollbar.resize(si.vscroll.x, si.vscroll.y, si.vscroll.w, si.vscroll.h);
      oldy = yposition_ = si.vscroll.pos;	// si.innerchild.y - si.child.t;
      scrollbar.value(si.vscroll.pos, si.vscroll.size, si.vscroll.first, si.vscroll.total);

      hscrollbar.resize(si.hscroll.x, si.hscroll.y, si.hscroll.w, si.hscroll.h);
      oldx = xposition_ = si.hscroll.pos;	// si.innerchild.x - si.child.l;
      hscrollbar.value(si.hscroll.pos, si.hscroll.size, si.hscroll.first, si.hscroll.total);
  }

  // draw the scrollbars:
  if (d & FL_DAMAGE_ALL) {
    draw_child(scrollbar);
    draw_child(hscrollbar);
    if (scrollbar.visible() && hscrollbar.visible()) {
      // fill in the little box in the corner
      fl_color(color());
      fl_rectf(scrollbar.x(), hscrollbar.y(), scrollbar.w(), hscrollbar.h());
    }
  } else {
    update_child(scrollbar);
    update_child(hscrollbar);
  }
}

/**
  Resizes the Fl_Scroll widget and moves its children if necessary.

  The Fl_Scroll widget first resizes itself, and then it moves all its
  children if (and only if) the Fl_Scroll widget has been moved. The
  children are moved by the same amount as the Fl_Scroll widget has been
  moved, hence all children keep their relative positions.

  \note Fl_Scroll::resize() does \b not call Fl_Group::resize(), and
  child widgets are \b not resized.

  Since children of an Fl_Scroll are not resized, the resizable() widget
  is ignored (if it is set).

  The scrollbars are moved to their proper positions, as given by
  Fl_Scroll::scrollbar.align(), and switched on or off as necessary.

  \note Due to current (FLTK 1.3.x) implementation constraints some of this
  may effectively be postponed until the Fl_Scroll is drawn the next time.
  This may change in a future release.

  \sa Fl_Group::resizable()
  \sa Fl_Widget::resize(int,int,int,int)
*/
void Fl_Scroll::resize(int X, int Y, int W, int H) {
  int dx = X-x(), dy = Y-y();
  int dw = W-w(), dh = H-h();
  Fl_Widget::resize(X,Y,W,H); // resize _before_ moving children around
  fix_scrollbar_order();
  // move all the children:
  Fl_Widget*const* a = array();
  for (int i=children()-2; i--;) {
    Fl_Widget* o = *a++;
    o->position(o->x()+dx, o->y()+dy);
  }
  if (dw==0 && dh==0) {
    char pad = ( scrollbar.visible() && hscrollbar.visible() );
    char al = ( (scrollbar.align() & FL_ALIGN_LEFT) != 0 );
    char at = ( (scrollbar.align() & FL_ALIGN_TOP)  !=0 );
    scrollbar.position(al?X:X+W-scrollbar.w(), (at&&pad)?Y+hscrollbar.h():Y);
    hscrollbar.position((al&&pad)?X+scrollbar.w():X, at?Y:Y+H-hscrollbar.h());
  } else {
    // FIXME recalculation of scrollbars needs to be moved out of "draw()" (STR #1895)
    redraw(); // need full recalculation of scrollbars
  }
}

/**  Moves the contents of the scroll group to a new position.

  This is like moving the scrollbars of the Fl_Scroll around. For instance:
  \code
    Fl_Scroll scroll (10,10,200,200);
    Fl_Box b1 ( 10, 10,50,50,"b1"); // relative (x,y) = (0,0)
    Fl_Box b2 ( 60, 60,50,50,"b2"); // relative (x,y) = (50,50)
    Fl_Box b3 ( 60,110,50,50,"b3"); // relative (x,y) = (50,100)
    // populate scroll with more children ...
    scroll.end();
    scroll.scroll_to(50,100);
  \endcode
  will move the logical origin of the internal scroll area to (-50,-100)
  relative to the origin of the Fl_Scroll (10,10), i.e. Fl_Box b3 will
  be visible in the top left corner of the scroll area.
*/
void Fl_Scroll::scroll_to(int X, int Y) {
  int dx = xposition_-X;
  int dy = yposition_-Y;
  if (!dx && !dy) return;
  xposition_ = X;
  yposition_ = Y;
  Fl_Widget*const* a = array();
  for (int i=children(); i--;) {
    Fl_Widget* o = *a++;
    if (o == &hscrollbar || o == &scrollbar) continue;
    o->position(o->x()+dx, o->y()+dy);
  }
  if (parent() == (Fl_Group *)window() && Fl::scheme_bg_) damage(FL_DAMAGE_ALL);
  else damage(FL_DAMAGE_SCROLL);
}

void Fl_Scroll::hscrollbar_cb(Fl_Widget* o, void*) {
  Fl_Scroll* s = (Fl_Scroll*)(o->parent());
  s->scroll_to(int(((Fl_Scrollbar*)o)->value()), s->yposition());
}

void Fl_Scroll::scrollbar_cb(Fl_Widget* o, void*) {
  Fl_Scroll* s = (Fl_Scroll*)(o->parent());
  s->scroll_to(s->xposition(), int(((Fl_Scrollbar*)o)->value()));
}
/**
  Creates a new Fl_Scroll widget using the given position,
  size, and label string. The default boxtype is FL_NO_BOX.

  The destructor <I>also deletes all the children</I>. This allows a
  whole tree to be deleted at once, without having to keep a pointer to
  all the children in the user code. A kludge has been done so the
  Fl_Scroll and all of its children can be automatic (local)
  variables, but you must declare the Fl_Scroll <I>first</I>, so
  that it is destroyed last.
*/
Fl_Scroll::Fl_Scroll(int X,int Y,int W,int H,const char* L)
  : Fl_Group(X,Y,W,H,L),
    scrollbar(X+W-Fl::scrollbar_size(),Y,
              Fl::scrollbar_size(),H-Fl::scrollbar_size()),
    hscrollbar(X,Y+H-Fl::scrollbar_size(),
               W-Fl::scrollbar_size(),Fl::scrollbar_size()) {
  type(BOTH);
  xposition_ = oldx = 0;
  yposition_ = oldy = 0;
  scrollbar_size_ = 0;
  hscrollbar.type(FL_HORIZONTAL);
  hscrollbar.callback(hscrollbar_cb);
  scrollbar.callback(scrollbar_cb);
}

int Fl_Scroll::handle(int event) {
  fix_scrollbar_order();
  return Fl_Group::handle(event);
}

//
// End of "$Id$".
//
