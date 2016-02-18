//
// "$Id$"
//
// Double-buffered window code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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
/** \file
 Fl_Double_Window implementation.
 */

#include "config_lib.h"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Printer.H>
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
    for (int j = 0; j < a->count; j++) {
      if (a->visinfo[j].visual == fl_visual->visualid
	  /*&& a->visinfo[j].perflevel > 0*/) {
        use_xdbe = 1; break;
      }
    }
    XdbeFreeVisualInfo(a);
  }
  return use_xdbe;
}
#endif


Fl_Double_Window::Fl_Double_Window(int W, int H, const char *l)
: Fl_Window(W,H,l),
  force_doublebuffering_(0)
{
  type(FL_DOUBLE_WINDOW);
}


Fl_Double_Window::Fl_Double_Window(int X, int Y, int W, int H, const char *l)
: Fl_Window(X,Y,W,H,l),
  force_doublebuffering_(0)
{
  type(FL_DOUBLE_WINDOW);
}


void Fl_Double_Window::show() {
  Fl_Window::show();
}


/** \addtogroup fl_drawings
 @{
 */
/** Copy a rectangular area of the given offscreen buffer into the current drawing destination.
 \param x,y	position where to draw the copied rectangle
 \param w,h	size of the copied rectangle
 \param pixmap  offscreen buffer containing the rectangle to copy
 \param srcx,srcy origin in offscreen buffer of rectangle to copy
 */
void fl_copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy) {
  fl_graphics_driver->copy_offscreen(x, y, w, h, pixmap, srcx, srcy);
}
/** @} */

/** see fl_copy_offscreen() */
void Fl_Graphics_Driver::copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy)
{
  fl_begin_offscreen(pixmap);
  uchar *img = fl_read_image(NULL, srcx, srcy, w, h, 0);
  fl_end_offscreen();
  fl_draw_image(img, x, y, w, h, 3, 0);
  delete[] img;
}

char fl_can_do_alpha_blending() {
  return Fl_Display_Device::display_device()->driver()->can_do_alpha_blending();
}

/**
  Forces the window to be redrawn.
*/
void Fl_Double_Window::flush() {flush(0);}

/**
  Forces the window to be redrawn.
  \param[in] eraseoverlay non-zero to erase overlay, zero to ignore

  Fl_Overlay_Window relies on flush(1) copying the back buffer to the
  front everywhere, even if damage() == 0, thus erasing the overlay,
  and leaving the clip region set to the entire window.
*/
void Fl_Double_Window::flush(int eraseoverlay) {
  if (!shown()) return;
  make_current(); // make sure fl_gc is non-zero
  Fl_X *myi = Fl_X::i(this);
  if (!myi) return; // window not yet created
  if (!myi->other_xid) {
#if USE_XDBE
    if (can_xdbe()) {
      myi->other_xid = XdbeAllocateBackBufferName(fl_display, fl_xid(this), XdbeCopied);
      myi->backbuffer_bad = 1;
    } else
#endif
#if defined(USE_X11) || defined(WIN32)
    myi->other_xid = fl_create_offscreen(w(), h());
    clear_damage(FL_DAMAGE_ALL);
#elif defined(__APPLE_QUARTZ__) // PORTME: platform double buffering
    if (force_doublebuffering_) {
      myi->other_xid = fl_create_offscreen(w(), h());
      clear_damage(FL_DAMAGE_ALL);
    }
#elif defined(FL_PORTING)
# pragma message "FL_PORTING: Fl_Window_Driver - call a function to clear any graphics port damage flags"
#else
# error unsupported platform
#endif
  }
#if USE_XDBE
  if (use_xdbe) {
    if (myi->backbuffer_bad || eraseoverlay) {
      // Make sure we do a complete redraw...
      if (myi->region) {XDestroyRegion(myi->region); myi->region = 0;}
      clear_damage(FL_DAMAGE_ALL);
      myi->backbuffer_bad = 0;
    }

    // Redraw as needed...
    if (damage()) {
      fl_clip_region(myi->region); myi->region = 0;
      fl_window = myi->other_xid;
      draw();
      fl_window = myi->xid;
    }

    // Copy contents of back buffer to window...
    XdbeSwapInfo s;
    s.swap_window = fl_xid(this);
    s.swap_action = XdbeCopied;
    XdbeSwapBuffers(fl_display, &s, 1);
    return;
  } else
#endif
  if (damage() & ~FL_DAMAGE_EXPOSE) {
    fl_clip_region(myi->region); myi->region = 0;
#ifdef WIN32
    void* _sgc = fl_graphics_driver->get_gc();
    HDC gc = fl_makeDC(myi->other_xid);
    fl_graphics_driver->set_gc(gc);
    int save = SaveDC(gc);
    fl_restore_clip(); // duplicate region into new gc
    draw();
    RestoreDC(gc, save);
    DeleteDC(gc);
    fl_graphics_driver->set_gc(_sgc);
    //# if defined(FLTK_USE_CAIRO)
    //if Fl::cairo_autolink_context() Fl::cairo_make_current(this); // capture gc changes automatically to update the cairo context adequately
    //# endif
#elif defined(__APPLE__) // PORTME: Fl_Window_Driver - platform double buffering
    if ( myi->other_xid ) {
      fl_begin_offscreen( myi->other_xid );
      fl_clip_region( 0 );
      draw();
      fl_end_offscreen();
    } else {
      draw();
    }
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: manage double buffered drawing"
#else // X:
    fl_window = myi->other_xid;
    draw();
    fl_window = myi->xid;
#endif
  }
  if (eraseoverlay) fl_clip_region(0);
  // on Irix (at least) it is faster to reduce the area copied to
  // the current clip region:
  int X,Y,W,H; fl_clip_box(0,0,w(),h(),X,Y,W,H);
  if (myi->other_xid) fl_copy_offscreen(X, Y, W, H, myi->other_xid, X, Y);
}

void Fl_Double_Window::resize(int X,int Y,int W,int H) {
  int ow = w();
  int oh = h();
  Fl_Window::resize(X,Y,W,H);
#if USE_XDBE
  if (use_xdbe) {
    Fl_X* myi = Fl_X::i(this);
    if (myi && myi->other_xid && (ow < w() || oh < h())) {
      // STR #2152: Deallocate the back buffer to force creation of a new one.
      XdbeDeallocateBackBufferName(fl_display,myi->other_xid);
      myi->other_xid = 0;
    }
    return;
  }
#endif
  Fl_X* myi = Fl_X::i(this);
  if (myi && myi->other_xid && (ow != w() || oh != h())) {
    fl_delete_offscreen(myi->other_xid);
    myi->other_xid = 0;
  }
}

void Fl_Double_Window::hide() {
  Fl_X* myi = Fl_X::i(this);
  if (myi && myi->other_xid) {
#if USE_XDBE
    if (!use_xdbe)
#endif
      fl_delete_offscreen(myi->other_xid);
  }
  Fl_Window::hide();
}

/**
  The destructor <I>also deletes all the children</I>. This allows a
  whole tree to be deleted at once, without having to keep a pointer to
  all the children in the user code.
*/
Fl_Double_Window::~Fl_Double_Window() {
  hide();
}


Fl_Overlay_Window::Fl_Overlay_Window(int W, int H, const char *l)
: Fl_Double_Window(W,H,l)
{
  overlay_ = 0;
  force_doublebuffering_=1;
  image(0);
}


Fl_Overlay_Window::Fl_Overlay_Window(int X, int Y, int W, int H, const char *l)
: Fl_Double_Window(X,Y,W,H,l)
{
  overlay_ = 0;
  force_doublebuffering_=1;
  image(0);
}


//
// End of "$Id$".
//
