//
// "$Id: Fl_Double_Window.cxx,v 1.8 1998/10/21 14:20:02 mike Exp $"
//
// Double-buffered window code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#include <config.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/x.H>
#include <FL/fl_draw.H>

// On systems that support double buffering "naturally" the base
// Fl_Window class will probably do double-buffer and this subclass
// does nothing.

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
#define DAMAGE_TEST() (damage() && (use_xdbe || damage() != FL_DAMAGE_EXPOSE))
#else
#define DAMAGE_TEST() (damage() & ~FL_DAMAGE_EXPOSE)
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

// Code used to switch output to an off-screen window.  See macros in
// win32.H which save the old state in local variables.

HDC fl_makeDC(HBITMAP bitmap) {
  HDC new_gc = CreateCompatibleDC(fl_gc);
  SetTextAlign(new_gc, TA_BASELINE|TA_LEFT);
  SetBkMode(new_gc, TRANSPARENT);
#if USE_COLORMAP
  if (fl_palette) SelectPalette(new_gc, fl_palette, FALSE);
#endif
  SelectObject(new_gc, bitmap);
  return new_gc;
}

void fl_copy_offscreen(int x,int y,int w,int h,HBITMAP bitmap,int srcx,int srcy) {
  HDC new_gc = CreateCompatibleDC(fl_gc);
  SelectObject(new_gc, bitmap);
  BitBlt(fl_gc, x, y, w, h, new_gc, srcx, srcy, SRCCOPY);
  DeleteDC(new_gc);
}

extern void fl_restore_clip();

#endif

// Fl_Overlay_Window relies on flush() copying the back buffer to the
// front even if damage() == 0, thus erasing the overlay inside the region:

void Fl_Double_Window::flush() {
  make_current(); // make sure fl_gc is non-zero
  Fl_X *i = Fl_X::i(this);
  if (!i->other_xid) {
#if USE_XDBE
    if (can_xdbe()) i->other_xid =
      XdbeAllocateBackBufferName(fl_display, fl_xid(this), XdbeCopied);
    else
#endif
      i->other_xid = fl_create_offscreen(w(), h());
    clear_damage(FL_DAMAGE_ALL);
  }
#ifdef WIN32
  fl_clip_region(i->region); i->region = 0;
  if (DAMAGE_TEST()) {
    HDC _sgc = fl_gc;
    fl_gc = fl_makeDC(i->other_xid);
    fl_restore_clip(); // duplicate region into new gc
    draw();
    DeleteDC(fl_gc);
    fl_gc = _sgc;
  }
#else // X:
#if USE_XDBE
  int clipped = i->region != 0;
#endif
  fl_clip_region(i->region); i->region = 0;
  if (DAMAGE_TEST()) {
    fl_window = i->other_xid;
    draw();
    fl_window = i->xid;
  }
#if USE_XDBE
  // It appears that swapbuffers ignores the clip region (it has to
  // as the gc is not passed as an argument to it).  This causes it
  // to erase parts of the overlay that won't be redrawn, and (at least
  // on XFree86) it is slower.  So I don't use it unless the entire
  // window is being redrawn.   Sigh.
  if (use_xdbe && !clipped) {
    XdbeSwapInfo s;
    s.swap_window = fl_xid(this);
    s.swap_action = XdbeCopied;
    XdbeSwapBuffers(fl_display, &s, 1);
    // fl_clip_region(0); older fix for clipping problem but overlay blinked
    return;
  }
#endif
#endif
  // on Irix (at least) it is faster to reduce the area copied to
  // the current clip region:
  int X,Y,W,H; fl_clip_box(0,0,w(),h(),X,Y,W,H);
  fl_copy_offscreen(X, Y, W, H, i->other_xid, X, Y);
}

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

//
// End of "$Id: Fl_Double_Window.cxx,v 1.8 1998/10/21 14:20:02 mike Exp $".
//
