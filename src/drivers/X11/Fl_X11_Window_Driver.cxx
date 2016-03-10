//
// "$Id$"
//
// Definition of Apple Cocoa window driver.
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


#include "../../config_lib.h"
#include "Fl_X11_Window_Driver.H"
#include <FL/fl_draw.H>
#include <string.h>
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#define ShapeBounding			0
#define ShapeSet			0

#if USE_XDBE
#include <X11/extensions/Xdbe.h>

static int can_xdbe(); // forward

// class to be used only if Xdbe is used
class Fl_X11_Dbe_Window_Driver : public Fl_X11_Window_Driver {
public:
  Fl_X11_Dbe_Window_Driver(Fl_Window *w) : Fl_X11_Window_Driver(w) {}
  virtual int double_flush(int eraseoverlay);
  virtual void destroy_double_buffer();
};
#endif // USE_XDBE


Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *w)
{
#if USE_XDBE
  if (w->as_double_window() && can_xdbe())
    return new Fl_X11_Dbe_Window_Driver(w);
  else
#endif
    return new Fl_X11_Window_Driver(w);
}


Fl_X11_Window_Driver::Fl_X11_Window_Driver(Fl_Window *win)
: Fl_Window_Driver(win)
{
}


Fl_X11_Window_Driver::~Fl_X11_Window_Driver()
{
  if (shape_data_) {
    delete shape_data_->todelete_;
    delete shape_data_;
  }
}

void Fl_X11_Window_Driver::take_focus()
{
  Fl_X *i = Fl_X::i(pWindow);
  if (!Fl_X::ewmh_supported())
      pWindow->show(); // Old WMs, XMapRaised
    else if (i->x) // New WMs use the NETWM attribute:
      Fl_X::activate_window(i->xid);
}

#if USE_XDBE

static int can_xdbe() { // whether the Xdbe extension is usable
  static int tried;
  static int use_xdbe = 0;
  if (!tried) {
    tried = 1;
    int event_base, error_base;
    fl_open_display();
    if (!XdbeQueryExtension(fl_display, &event_base, &error_base)) return 0;
    Drawable root = RootWindow(fl_display,fl_screen);
    int numscreens = 1;
    XdbeScreenVisualInfo *a = XdbeGetVisualInfo(fl_display,&root,&numscreens);
    if (!a) return 0;
    for (int j = 0; j < a->count; j++) {
      if (a->visinfo[j].visual == fl_visual->visualid) {
        use_xdbe = 1; break;
      }
    }
    XdbeFreeVisualInfo(a);
  }
  return use_xdbe;
}

int Fl_X11_Dbe_Window_Driver::double_flush(int eraseoverlay) {
  Fl_X *i = Fl_X::i(pWindow);
    if (!i->other_xid) {
      i->other_xid = XdbeAllocateBackBufferName(fl_display, i->xid, XdbeCopied);
      i->backbuffer_bad = 1;
      pWindow->clear_damage(FL_DAMAGE_ALL);
    }
    if (i->backbuffer_bad || eraseoverlay) {
      // Make sure we do a complete redraw...
      if (i->region) {XDestroyRegion(i->region); i->region = 0;}
      pWindow->clear_damage(FL_DAMAGE_ALL);
      i->backbuffer_bad = 0;
    }
    // Redraw as needed...
    if (pWindow->damage()) {
      fl_clip_region(i->region); i->region = 0;
      fl_window = i->other_xid;
      draw();
      fl_window = i->xid;
    }
    // Copy contents of back buffer to window...
    XdbeSwapInfo s;
    s.swap_window = i->xid;
    s.swap_action = XdbeCopied;
    XdbeSwapBuffers(fl_display, &s, 1);
    return 1;
}

void Fl_X11_Dbe_Window_Driver::destroy_double_buffer() {
  Fl_X *i = Fl_X::i(pWindow);
  XdbeDeallocateBackBufferName(fl_display, i->other_xid);
  i->other_xid = 0;
}
#endif // USE_XDBE


void Fl_X11_Window_Driver::shape_bitmap_(Fl_Image* b) {
  shape_data_->shape_ = b;
}

void Fl_X11_Window_Driver::shape_alpha_(Fl_Image* img, int offset) {
  int i, j, d = img->d(), w = img->w(), h = img->h(), bytesperrow = (w+7)/8;
  unsigned u;
  uchar byte, onebit;
  // build an Fl_Bitmap covering the non-fully transparent/black part of the image
  const uchar* bits = new uchar[h*bytesperrow]; // to store the bitmap
  const uchar* alpha = (const uchar*)*img->data() + offset; // points to alpha value of rgba pixels
  for (i = 0; i < h; i++) {
    uchar *p = (uchar*)bits + i * bytesperrow;
    byte = 0;
    onebit = 1;
    for (j = 0; j < w; j++) {
      if (d == 3) {
        u = *alpha;
        u += *(alpha+1);
        u += *(alpha+2);
      }
      else u = *alpha;
      if (u > 0) { // if the pixel is not fully transparent/black
        byte |= onebit; // turn on the corresponding bit of the bitmap
      }
      onebit = onebit << 1; // move the single set bit one position to the left
      if (onebit == 0 || j == w-1) {
        onebit = 1;
        *p++ = byte; // store in bitmap one pack of bits
        byte = 0;
      }
      alpha += d; // point to alpha value of next pixel
    }
  }
  Fl_Bitmap* bitmap = new Fl_Bitmap(bits, w, h);
  bitmap->alloc_array = 1;
  shape_bitmap_(bitmap);
  shape_data_->todelete_ = bitmap;
}

void Fl_X11_Window_Driver::shape(const Fl_Image* img) {
  if (shape_data_) {
    if (shape_data_->todelete_) { delete shape_data_->todelete_; }
  }
  else {
    shape_data_ = new shape_data_type;
  }
  memset(shape_data_, 0, sizeof(shape_data_type));
  pWindow->border(false);
  int d = img->d();
  if (d && img->count() >= 2) shape_pixmap_((Fl_Image*)img);
  else if (d == 0) shape_bitmap_((Fl_Image*)img);
  else if (d == 2 || d == 4) shape_alpha_((Fl_Image*)img, d - 1);
  else if ((d == 1 || d == 3) && img->count() == 1) shape_alpha_((Fl_Image*)img, 0);
}


void Fl_X11_Window_Driver::combine_mask()
{
  typedef void (*XShapeCombineMask_type)(Display*, int, int, int, int, Pixmap, int);
  static XShapeCombineMask_type XShapeCombineMask_f = NULL;
  static int beenhere = 0;
  typedef Bool (*XShapeQueryExtension_type)(Display*, int*, int*);
  if (!beenhere) {
    beenhere = 1;
#if HAVE_DLSYM && HAVE_DLFCN_H
    fl_open_display();
    void *handle = dlopen(NULL, RTLD_LAZY); // search symbols in executable
    XShapeQueryExtension_type XShapeQueryExtension_f = (XShapeQueryExtension_type)dlsym(handle, "XShapeQueryExtension");
    XShapeCombineMask_f = (XShapeCombineMask_type)dlsym(handle, "XShapeCombineMask");
    // make sure that the X server has the SHAPE extension
    int error_base, shapeEventBase;
    if ( !( XShapeQueryExtension_f && XShapeCombineMask_f &&
           XShapeQueryExtension_f(fl_display, &shapeEventBase, &error_base) ) ) XShapeCombineMask_f = NULL;
#endif
  }
  if (!XShapeCombineMask_f) return;
  shape_data_->lw_ = pWindow->w();
  shape_data_->lh_ = pWindow->h();
  Fl_Image* temp = shape_data_->shape_->copy(shape_data_->lw_, shape_data_->lh_);
  Pixmap pbitmap = XCreateBitmapFromData(fl_display, fl_xid(pWindow),
                                         (const char*)*temp->data(),
                                         temp->w(), temp->h());
  XShapeCombineMask_f(fl_display, fl_xid(pWindow), ShapeBounding, 0, 0, pbitmap, ShapeSet);
  if (pbitmap != None) XFreePixmap(fl_display, pbitmap);
  delete temp;
}


void Fl_X11_Window_Driver::draw() {
  if (shape_data_) {
    if (( shape_data_->lw_ != pWindow->w() || shape_data_->lh_ != pWindow->h() ) && shape_data_->shape_) {
      // size of window has changed since last time
      combine_mask();
    }
  }
  Fl_Window_Driver::draw();
}

//
// End of "$Id$".
//
