// Fl_Overlay_Window.C

// A window using double-buffering and able to draw an overlay
// on top of that.  Uses the hardware to draw the overlay if
// possible, otherwise it just draws in the front buffer.

#include <config.h>
#include <FL/Fl.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/fl_draw.H>
#include <FL/x.H>

void Fl_Overlay_Window::show() {
  Fl_Double_Window::show();
  if (overlay_ && overlay_ != this) overlay_->show();
}

void Fl_Overlay_Window::hide() {
  Fl_Double_Window::hide();
}

void Fl_Overlay_Window::flush() {
  // turn off the bit set by redraw_overlay:
  clear_damage(damage()&~8);
  // even if damage() == 0, flush() will erase the fake overlay by
  // copying back buffer over it.  It will also set the clip to the
  // region made by all the expose events:
  Fl_Double_Window::flush();
  // Now draw the fake overlay, if any, using the current clip:
  if (overlay_ == this) draw_overlay();
}

void Fl_Overlay_Window::resize(int X, int Y, int W, int H) {
  Fl_Double_Window::resize(X,Y,W,H);
  if (overlay_ && overlay_!=this) overlay_->resize(0,0,w(),h());
}

Fl_Overlay_Window::~Fl_Overlay_Window() {
  hide();
//  delete overlay; this is done by ~Fl_Group
}

#if !HAVE_OVERLAY

int Fl_Overlay_Window::can_do_overlay() {return 0;}

void Fl_Overlay_Window::redraw_overlay() {overlay_ = this; damage(8);}

#else

extern XVisualInfo *fl_find_overlay_visual();
extern XVisualInfo *fl_overlay_visual;
extern Colormap fl_overlay_colormap;
extern unsigned long fl_transparent_pixel;
static GC gc;	// the GC used by all X windows
extern uchar fl_overlay; // changes how fl_color(x) works

class _Fl_Overlay : public Fl_Window {
  friend class Fl_Overlay_Window;
  void flush();
  void show();
public:
  _Fl_Overlay(int x, int y, int w, int h) :
    Fl_Window(x,y,w,h) {deactivate();}
};

int Fl_Overlay_Window::can_do_overlay() {
  return fl_find_overlay_visual() != 0;
}

void _Fl_Overlay::show() {
  if (shown()) {Fl_Window::show(); return;}
  fl_background_pixel = int(fl_transparent_pixel);
  Fl_X::make_xid(this, fl_overlay_visual, fl_overlay_colormap);
  fl_background_pixel = -1;
  // find the outermost window to tell wm about the colormap:
  Fl_Window *w = window();
  for (;;) {Fl_Window *w1 = w->window(); if (!w1) break; w = w1;}
  XSetWMColormapWindows(fl_display, fl_xid(w), &(Fl_X::i(this)->xid), 1);
}

void _Fl_Overlay::flush() {
  fl_window = fl_xid(this);
  if (!gc) gc = XCreateGC(fl_display, fl_xid(this), 0, 0);
  fl_gc = gc;
  fl_overlay = 1;
  Fl_Overlay_Window *w = (Fl_Overlay_Window *)parent();
  Fl_X *i = Fl_X::i(this);
  if (damage() != 2) XClearWindow(fl_display, fl_xid(this));
  fl_clip_region(i->region); i->region = 0;
  w->draw_overlay();
  fl_overlay = 0;
}

void Fl_Overlay_Window::redraw_overlay() {
  if (!fl_display) return; // this prevents fluid -c from opening display
  if (!overlay_) {
    if (can_do_overlay()) {
      Fl_Group::current(this);
      overlay_ = new _Fl_Overlay(0,0,w(),h());
      Fl_Group::current(0);
    } else {
      overlay_ = this;	// fake the overlay
    }
  }
  if (shown()) {
    if (overlay_ == this)
      damage(8);
    else if (!overlay_->shown())
      overlay_->show();
    else
      overlay_->redraw();
  }
}

#endif

// End of Fl_Overlay_Window.C
