// fl_overlay.C

// Extremely limited "overlay" support.  You can use this to drag out
// a rectangle in response to mouse events.  It is your responsibility
// to erase the overlay before drawing anything that might intersect
// it.

#include <FL/x.H>
#include <FL/fl_draw.H>

static int px,py,pw,ph;

static void draw_current_rect() {
#ifdef WIN32
  int old = SetROP2(fl_gc, R2_NOT);
  fl_rect(px, py, pw, ph);
  SetROP2(fl_gc, old);
#else
  XSetFunction(fl_display, fl_gc, GXxor);
  XSetForeground(fl_display, fl_gc, 0xffffffff);
  XDrawRectangle(fl_display, fl_window, fl_gc, px, py, pw, ph);
  XSetFunction(fl_display, fl_gc, GXcopy);
#endif
}

void fl_overlay_clear() {
  if (pw > 0) {draw_current_rect(); pw = 0;}
}

void fl_overlay_rect(int x, int y, int w, int h) {
  if (w < 0) {x += w; w = -w;} else if (!w) w = 1;
  if (h < 0) {y += h; h = -h;} else if (!h) h = 1;
  if (pw > 0) {
    if (x==px && y==py && w==pw && h==ph) return;
    draw_current_rect();
  }
  px = x; py = y; pw = w; ph = h;
  draw_current_rect();
}
