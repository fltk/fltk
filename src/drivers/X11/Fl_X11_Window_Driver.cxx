//
// "$Id$"
//
// Definition of X11 window driver.
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

#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl.H>
#include <string.h>
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#define ShapeBounding			0
#define ShapeSet			0

Window fl_window;


#if USE_XDBE
#include <X11/extensions/Xdbe.h>

// whether the Xdbe extension is usable.
// DO NOT call this if the window is not mapped, because we do not want fluid to open the display.
static int can_xdbe()
{
  static int tried = 0;
  static int use_xdbe = 0;
  if (!tried) {
    tried = 1;
    int event_base, error_base;
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


void Fl_X11_Window_Driver::flush_double_dbe(int erase_overlay)
{
  pWindow->make_current(); // make sure fl_gc is non-zero
  Fl_X *i = Fl_X::i(pWindow);
  if (!i->other_xid) {
    i->other_xid = XdbeAllocateBackBufferName(fl_display, fl_xid(pWindow), XdbeCopied);
    i->backbuffer_bad = 1;
    pWindow->clear_damage(FL_DAMAGE_ALL);
  }
  if (i->backbuffer_bad || erase_overlay) {
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
  s.swap_window = fl_xid(pWindow);
  s.swap_action = XdbeCopied;
  XdbeSwapBuffers(fl_display, &s, 1);
}

#endif // USE_XDBE


void Fl_X11_Window_Driver::destroy_double_buffer() {
#if USE_XDBE
  if (can_xdbe()) {
    Fl_X *i = Fl_X::i(pWindow);
    XdbeDeallocateBackBufferName(fl_display, i->other_xid);
    i->other_xid = 0;
  }
  else
#endif // USE_XDBE
    Fl_Window_Driver::destroy_double_buffer();
}

Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *w)
{
  return new Fl_X11_Window_Driver(w);
}


Fl_X11_Window_Driver::Fl_X11_Window_Driver(Fl_Window *win)
: Fl_Window_Driver(win)
{
  icon_ = new icon_data;
  memset(icon_, 0, sizeof(icon_data));
}


Fl_X11_Window_Driver::~Fl_X11_Window_Driver()
{
  if (shape_data_) {
    delete shape_data_->todelete_;
    delete shape_data_;
  }
  delete icon_;
}


// --- private

void Fl_X11_Window_Driver::decorated_win_size(int &w, int &h)
{
  Fl_Window *win = pWindow;
  w = win->w();
  h = win->h();
  if (!win->shown() || win->parent() || !win->border() || !win->visible()) return;
  Window root, parent, *children;
  unsigned n = 0;
  Status status = XQueryTree(fl_display, Fl_X::i(win)->xid, &root, &parent, &children, &n);
  if (status != 0 && n) XFree(children);
  // when compiz is used, root and parent are the same window
  // and I don't know where to find the window decoration
  if (status == 0 || root == parent) return;
  XWindowAttributes attributes;
  XGetWindowAttributes(fl_display, parent, &attributes);
  w = attributes.width;
  h = attributes.height;
}


// --- window data

int Fl_X11_Window_Driver::decorated_h()
{
  int w, h;
  decorated_win_size(w, h);
  return h;
}

int Fl_X11_Window_Driver::decorated_w()
{
  int w, h;
  
  decorated_win_size(w, h);
  return w;
}


void Fl_X11_Window_Driver::take_focus()
{
  Fl_X *i = Fl_X::i(pWindow);
  if (!Fl_X::ewmh_supported())
      pWindow->show(); // Old WMs, XMapRaised
    else if (i && i->x) // New WMs use the NETWM attribute:
      Fl_X::activate_window(i->xid);
}


void Fl_X11_Window_Driver::draw_begin()
{
  if (shape_data_) {
    if (( shape_data_->lw_ != pWindow->w() || shape_data_->lh_ != pWindow->h() ) && shape_data_->shape_) {
      // size of window has changed since last time
      combine_mask();
    }
  }
}


void Fl_X11_Window_Driver::flush_double()
{
  if (!pWindow->shown()) return;
#if USE_XDBE
  if (can_xdbe()) flush_double_dbe(0); else
#endif
    flush_double(0);
}

void Fl_X11_Window_Driver::flush_double(int erase_overlay)
{
  pWindow->make_current(); // make sure fl_gc is non-zero
  Fl_X *i = Fl_X::i(pWindow);
  if (!i->other_xid) {
      i->other_xid = fl_create_offscreen(pWindow->w(), pWindow->h());
    pWindow->clear_damage(FL_DAMAGE_ALL);
  }
    if (pWindow->damage() & ~FL_DAMAGE_EXPOSE) {
      fl_clip_region(i->region); i->region = 0;
      fl_window = i->other_xid;
      draw();
      fl_window = i->xid;
    }
  if (erase_overlay) fl_clip_region(0);
  int X,Y,W,H; fl_clip_box(0,0,pWindow->w(),pWindow->h(),X,Y,W,H);
  if (i->other_xid) fl_copy_offscreen(X, Y, W, H, i->other_xid, X, Y);
}


void Fl_X11_Window_Driver::flush_overlay()
{
  if (!pWindow->shown()) return;
  int erase_overlay = (pWindow->damage()&FL_DAMAGE_OVERLAY);
  pWindow->clear_damage((uchar)(pWindow->damage()&~FL_DAMAGE_OVERLAY));
#if USE_XDBE
  if (can_xdbe()) flush_double_dbe(erase_overlay); else
#endif
  flush_double(erase_overlay);
  Fl_Overlay_Window *oWindow = pWindow->as_overlay_window();
  if (oWindow->overlay_ == oWindow) oWindow->draw_overlay();
}


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


void Fl_X11_Window_Driver::icons(const Fl_RGB_Image *icons[], int count) {
  free_icons();
  
  if (count > 0) {
    icon_->icons = new Fl_RGB_Image*[count];
    icon_->count = count;
    // FIXME: Fl_RGB_Image lacks const modifiers on methods
    for (int i = 0;i < count;i++)
      icon_->icons[i] = (Fl_RGB_Image*)((Fl_RGB_Image*)icons[i])->copy();
  }
  
  if (Fl_X::i(pWindow))
    Fl_X::i(pWindow)->set_icons();
}

const void *Fl_X11_Window_Driver::icon() const {
  return icon_->legacy_icon;
}

void Fl_X11_Window_Driver::icon(const void * ic) {
  free_icons();
  icon_->legacy_icon = ic;
}

void Fl_X11_Window_Driver::free_icons() {
  int i;
  icon_->legacy_icon = 0L;
  if (icon_->icons) {
    for (i = 0;i < icon_->count;i++)
      delete icon_->icons[i];
    delete [] icon_->icons;
    icon_->icons = 0L;
  }
  icon_->count = 0;
}


/* Returns images of the captures of the window title-bar, and the left, bottom and right window borders
 (or NULL if a particular border is absent).
 Returned images can be deleted after use. Their depth and size may be platform-dependent.
 The top and bottom images extend from left of the left border to right of the right border.
 
 On the X11 platform, this function exploits a feature of fl_read_image() which, when called
 with negative 4th argument, captures the window decoration.
 */
void Fl_X11_Window_Driver::capture_titlebar_and_borders(Fl_Shared_Image*& top, Fl_Shared_Image*& left, Fl_Shared_Image*& bottom, Fl_Shared_Image*& right)
{
  Fl_RGB_Image *r_top, *r_left, *r_bottom, *r_right;
  top = left = bottom = right = NULL;
  if (pWindow->decorated_h() == pWindow->h()) return;
  Window from = fl_window;
  Fl_Surface_Device *previous = Fl_Surface_Device::surface();
  Fl_Display_Device::display_device()->set_current();
  pWindow->show();
  Fl::check();
  pWindow->make_current();
  Window root, parent, *children, child_win;
  unsigned n = 0;
  int do_it;
  int wsides, htop;
  do_it = (XQueryTree(fl_display, fl_window, &root, &parent, &children, &n) != 0 &&
           XTranslateCoordinates(fl_display, fl_window, parent, 0, 0, &wsides, &htop, &child_win) == True);
  if (n) XFree(children);
  if (!do_it) wsides = htop = 0;
  int hbottom = wsides;
  fl_window = parent;
  uchar *rgb;
  if (htop) {
    rgb = fl_read_image(NULL, 0, 0, - (pWindow->w() + 2 * wsides), htop);
    r_top = new Fl_RGB_Image(rgb, pWindow->w() + 2 * wsides, htop, 3);
    r_top->alloc_array = 1;
    top = Fl_Shared_Image::get(r_top);
  }
  if (wsides) {
    rgb = fl_read_image(NULL, 0, htop, -wsides, pWindow->h());
    r_left = new Fl_RGB_Image(rgb, wsides, pWindow->h(), 3);
    r_left->alloc_array = 1;
    left = Fl_Shared_Image::get(r_left);
    rgb = fl_read_image(NULL, pWindow->w() + wsides, htop, -wsides, pWindow->h());
    r_right = new Fl_RGB_Image(rgb, wsides, pWindow->h(), 3);
    r_right->alloc_array = 1;
    right = Fl_Shared_Image::get(r_right);
    rgb = fl_read_image(NULL, 0, htop + pWindow->h(), -(pWindow->w() + 2*wsides), hbottom);
    r_bottom = new Fl_RGB_Image(rgb, pWindow->w() + 2*wsides, hbottom, 3);
    r_bottom->alloc_array = 1;
    bottom = Fl_Shared_Image::get(r_bottom);
  }
  fl_window = from;
  previous->Fl_Surface_Device::set_current();
}

void Fl_X11_Window_Driver::wait_for_expose() {
  if (!pWindow->shown()) return;
  Fl_X *i = Fl_X::i(pWindow);
  while (!i || i->wait_for_expose) {
    Fl::wait();
  }
}


// make X drawing go into this window (called by subclass flush() impl.)
void Fl_X11_Window_Driver::make_current() {
  if (!pWindow->shown()) {
    fl_alert("Fl_Window::make_current(), but window is not shown().");
    Fl::fatal("Fl_Window::make_current(), but window is not shown().");
  }
  fl_window = fl_xid(pWindow);
  fl_graphics_driver->clip_region(0);
  
#ifdef FLTK_USE_CAIRO
  // update the cairo_t context
  if (Fl::cairo_autolink_context()) Fl::cairo_make_current(pWindow);
#endif
}


//
// End of "$Id$".
//
