// Fl_Double_Window.C

// A double-buffered window.  This is achieved by using the Xdbe extension,
// or a pixmap if that is not available.

// On systems that support double buffering "naturally" the base
// Fl_Window class will probably do double-buffer and this subclass
// does nothing.

#include <config.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/x.H>
#include <FL/fl_draw.H>

#if USE_XDBE

#include <X11/extensions/Xdbe.h>

static int use_xdbe;

static int can_xdbe() {
  static int tried;
  if (!tried) {
    tried = 1;
    int event_base, error_base;
    if (!XdbeQueryExtension(fl_display, &event_base, &error_base)) return 0;
    Drawable root = RootWindow(fl_display,fl_screen);
    int numscreens = 1;
    XdbeScreenVisualInfo *a = XdbeGetVisualInfo(fl_display,&root,&numscreens);
    if (!a) return 0;
    for (int j = 0; j < a->count; j++)
      if (a->visinfo[j].visual == fl_visual->visualid
	  /*&& a->visinfo[j].perflevel > 0*/) {use_xdbe = 1; break;}
    XdbeFreeVisualInfo(a);
  }
  return use_xdbe;
}
#endif

void Fl_Double_Window::show() {
#ifndef WIN32
  if (!shown()) { // don't set the background pixel
    fl_open_display();
    Fl_X::make_xid(this);
    return;
  }
#endif
  Fl_Window::show();
}

#ifdef WIN32

// I've removed the second one (never understool why
// it was there to begin with).

static HDC blt_gc;

void fl_switch_offscreen(HBITMAP bitmap) {
  if (!blt_gc) {
    blt_gc = CreateCompatibleDC(fl_gc);
    SetTextAlign(blt_gc, TA_BASELINE|TA_LEFT);
    SetBkMode(blt_gc, TRANSPARENT);
#if USE_COLORMAP
    if (fl_palette) SelectPalette(blt_gc, fl_palette, FALSE);
#endif
  }
  SelectObject(blt_gc, bitmap);
  fl_gc = blt_gc;
}

void fl_copy_offscreen(int x,int y,int w,int h,HBITMAP bitmap,int srcx,int srcy) {
  SelectObject(blt_gc, bitmap);
  BitBlt(window_dc, x, y, w, h, blt_gc, srcx, srcy, SRCCOPY);
}

#endif

// protected method used by Fl_Overlay_Window to fake overlay:
void Fl_Double_Window::_flush(int eraseoverlay) {
  make_current(); // make sure fl_gc is non-zero
  Fl_X *i = Fl_X::i(this);
  if (!i->other_xid) {
#if USE_XDBE
    if (can_xdbe()) i->other_xid =
      XdbeAllocateBackBufferName(fl_display, fl_xid(this), XdbeCopied);
    else
#endif
      i->other_xid = fl_create_offscreen(w(), h());
    clear_damage(~0);
  }
  XRectangle rect = {0,0,w(),h()};
  if (damage()) {
    if (	// don't draw if back buffer is ok
#if USE_XDBE
	use_xdbe ||
#endif
	damage() != 2) {
/*
#ifdef WIN32
      fl_begin_offscreen(i->other_xid);
      fl_clip_region(i->region); i->region = 0;
      draw();
      fl_end_offscreen();
#else
*/
#ifdef WIN32
      fl_begin_offscreen(i->other_xid);
#endif
      fl_window = i->other_xid;
      fl_clip_region(i->region); i->region = 0;
      draw();
      fl_window = i->xid;
#ifdef WIN32
      fl_end_offscreen();
#endif
//#endif
    }
  }
  fl_clip_region(0);
#if USE_XDBE
  if (i->region && !eraseoverlay) XClipBox(i->region, &rect);
  if (use_xdbe) {
    XdbeSwapInfo s;
    s.swap_window = fl_xid(this);
    s.swap_action = XdbeCopied;
    XdbeSwapBuffers(fl_display,&s,1);
  } else
#endif
    fl_copy_offscreen(rect.x, rect.y, rect.width, rect.height,
		      i->other_xid, rect.x, rect.y);
}

void Fl_Double_Window::flush() {_flush(0);}

void Fl_Double_Window::resize(int X,int Y,int W,int H) {
  int ow = w();
  int oh = h();
  Fl_Window::resize(X,Y,W,H);
#if USE_XDBE
  if (use_xdbe) return;
#endif
  Fl_X* i = Fl_X::i(this);
  if (i && i->other_xid && (ow != w() || oh != h())) {
    fl_delete_offscreen(i->other_xid);
    i->other_xid = 0;
  }
}

void Fl_Double_Window::hide() {
  Fl_X* i = Fl_X::i(this);
  if (i && i->other_xid) {
#if USE_XDBE
    if (!use_xdbe)
#endif
      fl_delete_offscreen(i->other_xid);
  }
  Fl_Window::hide();
}

Fl_Double_Window::~Fl_Double_Window() {
  hide();
}
