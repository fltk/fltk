// Fl_Scroll.C

#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include <FL/fl_draw.H>

// Insure the scrollbars are the last children:
void Fl_Scroll::fix_scrollbar_order() {
  Fl_Widget*const* a = array();
  if (a[children()-1] != &scrollbar) {
    Fl_Widget** a = (Fl_Widget**)array();
    int i,j; for (i = j = 0; j < children(); j++)
      if (a[j] != &hscrollbar && a[j] != &scrollbar) a[i++] = a[j];
    a[i++] = &hscrollbar;
    a[i++] = &scrollbar;
  }
}

void Fl_Scroll::draw_clip(void* v,int X, int Y, int W, int H) {
  fl_clip(X,Y,W,H);
  Fl_Scroll* s = (Fl_Scroll*)v;
  // erase background if there is a boxtype:
  if (s->box() && !(s->damage()&128)) {
    fl_color(s->color());
    fl_rectf(X,Y,W,H);
  }
  Fl_Widget*const* a = s->array();
  int R = X; int B = Y; // track bottom & right edge of all children
  for (int i=s->children()-2; i--;) {
    Fl_Widget& o = **a++;
    s->draw_child(o);
    s->draw_outside_label(o);
    if (o.x()+o.w() > R) R = o.x()+o.w();
    if (o.y()+o.h() > B) B = o.y()+o.h();
  }
  // fill any area to right & bottom of widgets:
  if (R < X+W && B > Y) {
    fl_color(s->color());
    fl_rectf(R,Y,X+W-R,B-Y);
  }
  if (B < Y+H) {
    fl_color(s->color());
    fl_rectf(X,B,W,Y+H-B);
  }
  fl_pop_clip();
}

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

  if (d & 128) { // full redraw
    draw_box(box(),x(),y(),w(),h(),color());
    draw_clip(this, X, Y, W, H);
  } else {
    if (d & 2) { // scroll the contents:
      fl_scroll(X, Y, W, H, oldx-xposition_, oldy-yposition_, draw_clip, this);
    }
    if (d & 1) { // draw damaged children
      fl_clip(X, Y, W, H);
      Fl_Widget*const* a = array();
      for (int i=children()-2; i--;) update_child(**a++);
      fl_pop_clip();
    }
  }

  // accumulate bounding box of children:
  int l = X; int r = X; int t = Y; int b = Y;
  Fl_Widget*const* a = array();
  for (int i=children()-2; i--;) {
    Fl_Object* o = *a++;
    if (o->x() < l) l = o->x();
    if (o->y() < t) t = o->y();
    if (o->x()+o->w() > r) r = o->x()+o->w();
    if (o->y()+o->h() > b) b = o->y()+o->h();
  }

  // turn the scrollbars on and off as necessary:
  for (int z = 0; z<2; z++) {
    if ((type()&VERTICAL) && (type()&ALWAYS_ON || t < Y || b > Y+H)) {
      if (!scrollbar.visible()) {
	scrollbar.set_visible();
	W -= scrollbar.w();
	d = 128;
      }
    } else {
      if (scrollbar.visible()) {
	scrollbar.clear_visible();
	draw_clip(this,
		  scrollbar.align()&FL_ALIGN_LEFT ? X-scrollbar.w() : X+W,
		  Y, scrollbar.w(), H);
	W += scrollbar.w();
	d = 128;
      }
    }
    if ((type()&HORIZONTAL) && (type()&ALWAYS_ON || l < X || r > X+W)) {
      if (!hscrollbar.visible()) {
	hscrollbar.set_visible();
	H -= hscrollbar.h();
	d = 128;
      }
    } else {
      if (hscrollbar.visible()) {
	hscrollbar.clear_visible();
	draw_clip(this, X,
		  scrollbar.align()&FL_ALIGN_TOP ? Y-hscrollbar.h() : Y+H,
		  W, hscrollbar.h());
	H += hscrollbar.h();
	d = 128;
      }
    }
  }

  scrollbar.resize(scrollbar.align()&FL_ALIGN_LEFT ? X-scrollbar.w() : X+W,
		   Y, scrollbar.w(), H);
  scrollbar.value(oldy = yposition_ = Y, H, t, b-t);

  hscrollbar.resize(X,
		    scrollbar.align()&FL_ALIGN_TOP ? Y-hscrollbar.h() : Y+H,
		    W, hscrollbar.h());
  hscrollbar.value(oldx = xposition_ = X, W, l, r-l);

  // draw the scrollbars:
  if (d & 128) {
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

void Fl_Scroll::resize(int X, int Y, int W, int H) {
  fix_scrollbar_order();
  // move all the children:
  Fl_Widget*const* a = array();
  for (int i=children()-2; i--;) {
    Fl_Object* o = *a++;
    o->position(o->x()+X-x(), o->y()+Y-y());
  }
  Fl_Widget::resize(X,Y,W,H);
}

void Fl_Scroll::position(int X, int Y) {
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
  damage(2);
}

void Fl_Scroll::hscrollbar_cb(Fl_Widget* o, void*) {
  Fl_Scroll* s = (Fl_Scroll*)(o->parent());
  s->position(int(((Fl_Scrollbar*)o)->value()), s->yposition());
}

void Fl_Scroll::scrollbar_cb(Fl_Widget* o, void*) {
  Fl_Scroll* s = (Fl_Scroll*)(o->parent());
  s->position(s->xposition(), int(((Fl_Scrollbar*)o)->value()));
}

#define SLIDER_WIDTH 17

Fl_Scroll::Fl_Scroll(int X,int Y,int W,int H,const char* L)
  : Fl_Group(X,Y,W,H,L), 
    scrollbar(X+W-SLIDER_WIDTH,Y,SLIDER_WIDTH,H-SLIDER_WIDTH),
    hscrollbar(X,Y+H-SLIDER_WIDTH,W-SLIDER_WIDTH,SLIDER_WIDTH) {
  type(BOTH);
  xposition_ = 0;
  yposition_ = 0;
  hscrollbar.type(FL_HORIZONTAL);
  hscrollbar.callback(hscrollbar_cb);
  scrollbar.callback(scrollbar_cb);
}

int Fl_Scroll::handle(int event) {
  fix_scrollbar_order();
  return Fl_Group::handle(event);
}

// end of Fl_Scroll.C
