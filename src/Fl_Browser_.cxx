//
// "$Id: Fl_Browser_.cxx,v 1.10.2.15 2001/01/22 15:13:39 easysw Exp $"
//
// Base Browser widget class for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Browser_.H>
#include <FL/fl_draw.H>


// This is the base class for browsers.  To be useful it must be
// subclassed and several virtual functions defined.  The
// Forms-compatable browser and the file chooser's browser are
// subclassed off of this.

// Yes, I know this should be a template...

// This has been designed so that the subclass has complete control
// over the storage of the data, although because next() and prev()
// functions are used to index, it works best as a linked list or as a
// large block of characters in which the line breaks must be searched
// for.

// A great deal of work has been done so that the "height" of a data
// object does not need to be determined until it is drawn.  This was
// done for the file chooser, because the height requires doing stat()
// to see if the file is a directory, which can be annoyingly slow
// over the network.

/* redraw bits:
   1 = redraw children (the scrollbar)
   2 = redraw one or two items
   4 = redraw all items
*/

static void scrollbar_callback(Fl_Widget* s, void*) {
  ((Fl_Browser_*)(s->parent()))->position(int(((Fl_Scrollbar*)s)->value()));
}

static void hscrollbar_callback(Fl_Widget* s, void*) {
  ((Fl_Browser_*)(s->parent()))->hposition(int(((Fl_Scrollbar*)s)->value()));
}

int Fl_Browser_::scrollbar_width_ = 16;

// return where to draw the actual box:
void Fl_Browser_::bbox(int& X, int& Y, int& W, int& H) const {
  Fl_Boxtype b = box() ? box() : FL_DOWN_BOX;
  X = x()+Fl::box_dx(b);
  Y = y()+Fl::box_dy(b);
  W = w()-Fl::box_dw(b);
  H = h()-Fl::box_dh(b);
  if (scrollbar.visible()) {
    W -= scrollbar_width_;
    if (scrollbar.align() & FL_ALIGN_LEFT) X += scrollbar_width_;
  }
  if (W < 0) W = 0;
  if (hscrollbar.visible()) {
    H -= scrollbar_width_;
    if (scrollbar.align() & FL_ALIGN_TOP) Y += scrollbar_width_;
  }
  if (H < 0) H = 0;
}

int Fl_Browser_::leftedge() const {
  int X, Y, W, H; bbox(X, Y, W, H);
  return X;
}

// The scrollbars may be moved again by draw(), since each one's size
// depends on whether the other is visible or not.  This skips over
// Fl_Group::resize since it moves the scrollbars uselessly.
void Fl_Browser_::resize(int X, int Y, int W, int H) {
  Fl_Widget::resize(X, Y, W, H);
  // move the scrollbars so they can respond to events:
  bbox(X,Y,W,H);
  scrollbar.resize(
	scrollbar.align()&FL_ALIGN_LEFT ? X-scrollbar_width_ : X+W,
	Y, scrollbar_width_, H);
  hscrollbar.resize(
	X, scrollbar.align()&FL_ALIGN_TOP ? Y-scrollbar_width_ : Y+H,
	W, scrollbar_width_);
}

// Cause minimal update to redraw the given item:
void Fl_Browser_::redraw_line(void* l) {
  if (!redraw1 || redraw1 == l) {redraw1 = l; damage(FL_DAMAGE_EXPOSE);}
  else if (!redraw2 || redraw2 == l) {redraw2 = l; damage(FL_DAMAGE_EXPOSE);}
  else damage(FL_DAMAGE_SCROLL);
}

// Figure out top() based on position():
void Fl_Browser_::update_top() {
  if (!top_) top_ = item_first();
  if (position_ != real_position_) {
    void* l;
    int ly;
    int y = position_;
    // start from either head or current position, whichever is closer:
    if (!top_ || y <= (real_position_/2)) {
      l = item_first();
      ly = 0;
    } else {
      l = top_;
      ly = real_position_-offset_;
    }
    if (!l) {
      top_ = 0;
      offset_ = 0;
      real_position_ = 0;
    } else {
      int h = item_quick_height(l);
      // step through list until we find line containing this point:
      while (ly > y) {
	void* l1 = item_prev(l);
	if (!l1) {ly = 0; break;} // hit the top
	l = l1;
	h = item_quick_height(l);
	ly -= h;
      }
      while ((ly+h) <= y) {
	void* l1 = item_next(l);
	if (!l1) {y = ly+h-1; break;}
	l = l1;
	ly += h;
	h = item_quick_height(l);
      }
      // top item must *really* be visible, use slow height:
      for (;;) {
	h = item_height(l);
	if ((ly+h) > y) break; // it is big enough to see
	// go up to top of previous item:
	void* l1 = item_prev(l);
	if (!l1) {ly = y = 0; break;} // hit the top
	l = l1; y = position_ = ly = ly-item_quick_height(l);
      }
      // use it:
      top_ = l;
      offset_ = y-ly;
      real_position_ = y;
    }
    damage(FL_DAMAGE_SCROLL);
  }
}

// Change position(), top() will update when update_top() is called
// (probably by draw() or handle()):
void Fl_Browser_::position(int y) {
  if (y < 0) y = 0;
  if (y == position_) return;
  position_ = y;
  if (y != real_position_) redraw_lines();
}

void Fl_Browser_::hposition(int x) {
  if (x < 0) x = 0;
  if (x == hposition_) return;
  hposition_ = x;
  if (x != real_hposition_) redraw_lines();
}

// Tell whether item is currently displayed:
int Fl_Browser_::displayed(void* x) const {
  int X, Y, W, H; bbox(X, Y, W, H);
  int yy = H+offset_;
  for (void* l = top_; l && yy > 0; l = item_next(l)) {
    if (l == x) return 1;
    yy -= item_height(l);
  }
  return 0;
}

// Insure this item is displayed:
// Messy because we have no idea if it is before top or after bottom:
void Fl_Browser_::display(void* x) {
  update_top();
  if (x == item_first()) {position(0); return;}
  int X, Y, W, H; bbox(X, Y, W, H);
  void* l = top_;
  Y = -offset_;
  // see if it is at the top or just above it:
  if (l == x) {position(real_position_+Y); return;} // scroll up a bit
  void* lp = item_prev(l);
  if (lp == x) {position(real_position_+Y-item_quick_height(lp)); return;}
  // search forward for it:
  for (; l; l = item_next(l)) {
    int h1 = item_quick_height(l);
    if (l == x) {
      if (Y <= H) { // it is visible or right at bottom
	Y = Y+h1-H; // find where bottom edge is
	if (Y > 0) position(real_position_+Y); // scroll down a bit
      } else {
	position(real_position_+Y-(H-h1)/2); // center it
      }
      return;
    }
    Y += h1;
  }
  // search backward for it, if found center it:
  l = lp;
  Y = -offset_;
  for (; l; l = item_prev(l)) {
    int h1 = item_quick_height(l);
    Y -= h1;
    if (l == x) {
      if ((Y + h1) >= 0) position(real_position_+Y);
      else position(real_position_+Y-(H-h1)/2);
      return;
    }
  }
}

// redraw, has side effect of updating top and setting scrollbar:

void Fl_Browser_::draw() {
  int drawsquare = 0;
  if (damage() & FL_DAMAGE_ALL) { // redraw the box if full redraw
    Fl_Boxtype b = box() ? box() : FL_DOWN_BOX;
    draw_box(b, x(), y(), w(), h(), color());
    drawsquare = 1;
  }

  update_top();
  int full_width_ = full_width();
  int full_height_ = full_height();
  int X, Y, W, H; bbox(X, Y, W, H);
  int dont_repeat = 0;
J1:
  // see if scrollbar needs to be switched on/off:
  if ((has_scrollbar_ & VERTICAL) && (
	(has_scrollbar_ & ALWAYS_ON) || position_ || full_height_ > H)) {
    if (!scrollbar.visible()) {
      scrollbar.set_visible();
      drawsquare = 1;
      bbox(X, Y, W, H);
    }
  } else {
    top_ = item_first(); real_position_ = offset_ = 0;
    if (scrollbar.visible()) {
      scrollbar.clear_visible();
      clear_damage(damage()|FL_DAMAGE_SCROLL);
    }
  }

  if ((has_scrollbar_ & HORIZONTAL) && (
	(has_scrollbar_ & ALWAYS_ON) || hposition_ || full_width_ > W)) {
    if (!hscrollbar.visible()) {
      hscrollbar.set_visible();
      drawsquare = 1;
      bbox(X, Y, W, H);
    }
  } else {
    real_hposition_ = 0;
    if (hscrollbar.visible()) {
      hscrollbar.clear_visible();
      clear_damage(damage()|FL_DAMAGE_SCROLL);
    }
  }

  // Check the vertical scrollbar again, just in case it needs to be drawn
  // because the horizontal one is drawn.  There should be a cleaner way
  // to do this besides copying the same code...
  if ((has_scrollbar_ & VERTICAL) && (
	(has_scrollbar_ & ALWAYS_ON) || position_ || full_height_ > H)) {
    if (!scrollbar.visible()) {
      scrollbar.set_visible();
      drawsquare = 1;
      bbox(X, Y, W, H);
    }
  } else {
    top_ = item_first(); real_position_ = offset_ = 0;
    if (scrollbar.visible()) {
      scrollbar.clear_visible();
      clear_damage(damage()|FL_DAMAGE_SCROLL);
    }
  }

  bbox(X, Y, W, H);

  fl_clip(X, Y, W, H);
  // for each line, draw it if full redraw or scrolled.  Erase background
  // if not a full redraw or if it is selected:
  void* l = top();
  int yy = -offset_;
  for (; l && yy < H; l = item_next(l)) {
    int hh = item_height(l);
    if (hh <= 0) continue;
    if ((damage()&(FL_DAMAGE_SCROLL|FL_DAMAGE_ALL)) || l == redraw1 || l == redraw2) {
      if (item_selected(l)) {
	fl_color(active_r() ? selection_color() : inactive(selection_color()));
	fl_rectf(X, yy+Y, W, hh);
      } else if (!(damage()&FL_DAMAGE_ALL)) {
	fl_color(color());
	fl_rectf(X, yy+Y, W, hh);
      }
      item_draw(l, X-hposition_, yy+Y, W+hposition_, hh);
      if (l == selection_) {
	fl_color(active_r() ? textcolor() : inactive(textcolor()));
	fl_rect(X+1, yy+Y, W-2, hh);
      }
      int w = item_width(l);
      if (w > max_width) {max_width = w; max_width_item = l;}
    }
    yy += hh;
  }
  // erase the area below last line:
  if (!(damage()&FL_DAMAGE_ALL) && yy < H) {
    fl_color(color());
    fl_rectf(X, yy+Y, W, H-yy);
  }
  fl_pop_clip();
  redraw1 = redraw2 = 0;

  if (!dont_repeat) {
    dont_repeat = 1;
    // see if changes to full_height caused by calls to slow_height
    // caused scrollbar state to change, in which case we have to redraw:
    full_height_ = full_height();
    full_width_ = full_width();
    if ((has_scrollbar_ & VERTICAL) &&
	((has_scrollbar_ & ALWAYS_ON) || position_ || full_height_>H)) {
      if (!scrollbar.visible()) goto J1;
    } else {
      if (scrollbar.visible()) goto J1;
    }
    if ((has_scrollbar_ & HORIZONTAL) &&
	((has_scrollbar_ & ALWAYS_ON) || hposition_ || full_width_>W)) {
      if (!hscrollbar.visible()) goto J1;
    } else {
      if (hscrollbar.visible()) goto J1;
    }
  }

  // update the scrollbars and redraw them:
  int dy = top_ ? item_quick_height(top_) : 0; if (dy < 10) dy = 10;
  if (scrollbar.visible()) {
    scrollbar.damage_resize(
	scrollbar.align()&FL_ALIGN_LEFT ? X-scrollbar_width_ : X+W,
	Y, scrollbar_width_, H);
    scrollbar.value(position_, H, 0, full_height_);
    scrollbar.linesize(dy);
    if (drawsquare) draw_child(scrollbar);
    else update_child(scrollbar);
  }
  if (hscrollbar.visible()) {
    hscrollbar.damage_resize(
	X, scrollbar.align()&FL_ALIGN_TOP ? Y-scrollbar_width_ : Y+H,
	W, scrollbar_width_);
    hscrollbar.value(hposition_, W, 0, full_width_);
    hscrollbar.linesize(dy);
    if (drawsquare) draw_child(hscrollbar);
    else update_child(hscrollbar);
  }

  // draw that little square between the scrollbars:
  if (drawsquare && scrollbar.visible() && hscrollbar.visible()) {
    fl_color(parent()->color());
    fl_rectf(scrollbar.x(), hscrollbar.y(), scrollbar_width_,scrollbar_width_);
  }

  real_hposition_ = hposition_;
}

// Quick way to delete and reset everything:
void Fl_Browser_::new_list() {
  top_ = 0;
  position_ = real_position_ = 0;
  hposition_ = real_hposition_ = 0;
  selection_ = 0;
  offset_ = 0;
  max_width = 0;
  max_width_item = 0;
  redraw_lines();
}

// Tell it that this item is going away, and that this must remove
// all pointers to it:
void Fl_Browser_::deleting(void* l) {
  if (displayed(l)) {
    redraw_lines();
    if (l == top_) {
      real_position_ -= offset_;
      offset_ = 0;
      top_ = item_next(l);
      if (!top_) top_ = item_prev(l);
    }
  } else {
    // we don't know where this item is, recalculate top...
    real_position_ = 0;
    top_ = 0;
  }
  if (l == selection_) selection_ = 0;
  if (l == max_width_item) {max_width_item = 0; max_width = 0;}
}

void Fl_Browser_::replacing(void* a, void* b) {
  redraw_line(a);
  if (a == selection_) selection_ = b;
  if (a == top_) top_ = b;
  if (a == max_width_item) {max_width_item = 0; max_width = 0;}
}

void Fl_Browser_::inserting(void* a, void* b) {
  if (displayed(a)) redraw_lines();
  if (a == top_) top_ = b;
}

void* Fl_Browser_::find_item(int my) {
  update_top();
  int X, Y, W, H; bbox(X, Y, W, H);
  void* l;
  int yy = Y-offset_;
  for (l = top_; l; l = item_next(l)) {
    int hh = item_height(l); if (hh <= 0) continue;
    yy += hh;
    if (my <= yy || yy>=(Y+H)) return l;
  }
  return 0;
}

int Fl_Browser_::select(void* l, int i, int docallbacks) {
  if (type() == FL_MULTI_BROWSER) {
    if (selection_ != l) {
      if (selection_) redraw_line(selection_);
      selection_ = l;
      redraw_line(l);
    }
    if ((!i)==(!item_selected(l))) return 0;
    item_select(l, i);
    redraw_line(l);
  } else {
    if (i && selection_ == l) return 0;
    if (!i && selection_ != l) return 0;
    if (selection_) {
      item_select(selection_, 0);
      redraw_line(selection_);
      selection_ = 0;
    }
    if (i) {
      item_select(l, 1);
      selection_ = l;
      redraw_line(l);
      display(l);
    }
  }	    
  Fl::event_clicks(0);
  if (docallbacks) do_callback();
  return 1;
}

int Fl_Browser_::deselect(int docallbacks) {
  if (type() == FL_MULTI_BROWSER) {
    int change = 0;
    for (void* p = item_first(); p; p = item_next(p))
      change |= select(p, 0, docallbacks);
    return change;
  } else {
    if (!selection_) return 0;
    item_select(selection_, 0);
    redraw_line(selection_);
    selection_ = 0;
    return 1;
  }
}

int Fl_Browser_::select_only(void* l, int docallbacks) {
  if (!l) return deselect(docallbacks);
  int change = 0;
  if (type() == FL_MULTI_BROWSER) {
    for (void* p = item_first(); p; p = item_next(p))
      if (p != l) change |= select(p, 0, docallbacks);
  }
  change |= select(l, 1, docallbacks);
  display(l);
  return change;
}

int Fl_Browser_::handle(int event) {

  // must do shortcuts first or the scrollbar will get them...
  if ((event == FL_SHORTCUT || event == FL_KEYBOARD)
      && type() >= FL_HOLD_BROWSER) {
    void* l1 = selection_;
    void* l = l1; if (!l) l = top_; if (!l) l = item_first();
    if (l) {
      if (type()==FL_HOLD_BROWSER) switch (Fl::event_key()) {
      case FL_Down:
	while ((l = item_next(l)))
	  if (item_height(l)>0) {select_only(l, 1); break;}
	return 1;
      case FL_Up:
	while ((l = item_prev(l))) if (item_height(l)>0) {
	  select_only(l, 1); break;}
	return 1;
      } else switch (Fl::event_key()) {
      case FL_Enter:
	select_only(l, 1);
	return 1;
      case ' ':
	selection_ = l;
	select(l, !item_selected(l), 1);
	return 1;
      case FL_Down:
	while ((l = item_next(l))) {
	  if (Fl::event_state(FL_SHIFT|FL_CTRL))
	    select(l, l1 ? item_selected(l1) : 1, 1);
	  if (item_height(l)>0) goto J1;
	}
	return 1;
      case FL_Up:
	while ((l = item_prev(l))) {
	  if (Fl::event_state(FL_SHIFT|FL_CTRL))
	    select(l, l1 ? item_selected(l1) : 1, 1);
	  if (item_height(l)>0) goto J1;
	}
	return 1;
      J1:
	if (selection_) redraw_line(selection_);
	selection_ = l; redraw_line(l);
	display(l);
	return 1;
      }
    }
  }

  if (Fl_Group::handle(event)) return 1;
  int X, Y, W, H; bbox(X, Y, W, H);
  int my;
  static char change;
  static char whichway;
  static int py;
  switch (event) {
  case FL_PUSH:
    if (!Fl::event_inside(X, Y, W, H)) return 0;
    my = py = Fl::event_y();
    change = 0;
    if (type() == FL_NORMAL_BROWSER || !top_)
      ;
    else if (type() != FL_MULTI_BROWSER) {
      change = select_only(find_item(my), when() & FL_WHEN_CHANGED);
    } else {
      void* l = find_item(my);
      whichway = 1;
      if (Fl::event_state(FL_CTRL)) { // toggle selection:
	if (l) {
	  whichway = !item_selected(l);
	  change = select(l, whichway, when() & FL_WHEN_CHANGED);
	}
      } else if (Fl::event_state(FL_SHIFT)) { // extend selection:
	change = 0;
	if (selection_ && l != selection_) {
	  void *m = l;
	  int down = 0;
	  whichway = item_selected(selection_);
	  for (m = selection_; m; m = item_next(m)) {
	    if (m == l) down = 1;
	  }
	  void *n;
	  if (down) {
	    m = selection_;
	    n = l;
	  } else {
	    m = l;
	    n = selection_;
	  }
	  while (m != n) {
	    select(m, whichway, when() & FL_WHEN_CHANGED);
	    m = item_next(m);
	  }
	  select(n, whichway, when() & FL_WHEN_CHANGED);
	  select(l, whichway, when() & FL_WHEN_CHANGED);
	  change = 1;
	}
      } else { // select only this item
	change = select_only(l, when() & FL_WHEN_CHANGED);
      }
    }
    return 1;
  case FL_DRAG:
    // do the scrolling first:
    my = Fl::event_y();
    if (my < Y && my < py) {
      int p = real_position_+my-Y;
      if (p<0) p = 0;
      position(p);
    } else if (my > (Y+H) && my > py) {
      int p = real_position_+my-(Y+H);
      int h = full_height()-H; if (p > h) p = h;
      if (p<0) p = 0;
      position(p);
    }
    if (type() == FL_NORMAL_BROWSER || !top_)
      ;
    else if (type() == FL_MULTI_BROWSER) {
      void* l = find_item(my);
      void* t; void* b; // this will be the range to change
      if (my > py) { // go down
	t = selection_ ? item_next(selection_) : 0;
	b = l ? item_next(l) : 0;
      } else {	// go up
	t = l;
	b = selection_;
      }
      for (; t && t != b; t = item_next(t))
	change |= select(t, whichway, when() & FL_WHEN_CHANGED);
      if (l) selection_ = l;
    } else {
      void* l1 = selection_;
      void* l =
	(Fl::event_x()<x() || Fl::event_x()>x()+w()) ? selection_ :
	find_item(my);
      select_only(l, when() & FL_WHEN_CHANGED);
      change = (l != l1);
    }
    py = my;
    return 1;
  case FL_RELEASE:
    if (type() == FL_SELECT_BROWSER) {
      void* t = selection_; deselect(); selection_ = t;
    }
    if (change) {
      if (when() & FL_WHEN_RELEASE) do_callback();
      else if (!(when()&FL_WHEN_CHANGED)) set_changed();
    } else {
      if (when() & FL_WHEN_NOT_CHANGED) do_callback();
    }
    return 1;
  }

  return 0;
}

Fl_Browser_::Fl_Browser_(int x, int y, int w, int h, const char* l)
  : Fl_Group(x, y, w, h, l),
    scrollbar(0, 0, 0, 0, 0), // they will be resized by draw()
    hscrollbar(0, 0, 0, 0, 0)
{
  box(FL_NO_BOX);
  align(FL_ALIGN_BOTTOM);
  position_ = real_position_ = 0;
  hposition_ = real_hposition_ = 0;
  offset_ = 0;
  top_ = 0;
  when(FL_WHEN_RELEASE_ALWAYS);
  selection_ = 0;
  color(FL_WHITE);
  selection_color(FL_SELECTION_COLOR);
  scrollbar.callback(scrollbar_callback);
//scrollbar.align(FL_ALIGN_LEFT|FL_ALIGN_BOTTOM); // back compatability?
  hscrollbar.callback(hscrollbar_callback);
  hscrollbar.type(FL_HORIZONTAL);
  textfont_ = FL_HELVETICA;
  textsize_ = FL_NORMAL_SIZE;
  textcolor_ = FL_BLACK;
  has_scrollbar_ = BOTH;
  max_width = 0;
  max_width_item = 0;
  redraw1 = redraw2 = 0;
  end();
}

// Default versions of some of the virtual functions:

int Fl_Browser_::item_quick_height(void* l) const {
  return item_height(l);
}

int Fl_Browser_::incr_height() const {
  return item_quick_height(item_first());
}

int Fl_Browser_::full_height() const {
  int t = 0;
  for (void* p = item_first(); p; p = item_next(p))
    t += item_quick_height(p);
  return t;
}

int Fl_Browser_::full_width() const {
  return max_width;
}

void Fl_Browser_::item_select(void*, int) {}

int Fl_Browser_::item_selected(void* l) const {return l==selection_;}

//
// End of "$Id: Fl_Browser_.cxx,v 1.10.2.15 2001/01/22 15:13:39 easysw Exp $".
//
