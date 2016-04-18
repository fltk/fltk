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
#include "Fl_X11_Screen_Driver.H"
#include "../Xlib/Fl_Xlib_Graphics_Driver.H"

#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Screen_Driver.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Menu_Window.H>
#include <FL/Fl_Tooltip.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl.H>
#include <FL/x.H>
#include <string.h>
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#define ShapeBounding			0
#define ShapeSet			0

#if HAVE_OVERLAY
extern XVisualInfo *fl_find_overlay_visual();
extern XVisualInfo *fl_overlay_visual;
extern Colormap fl_overlay_colormap;
extern unsigned long fl_transparent_pixel;
extern uchar fl_overlay; // changes how fl_color(x) works
#endif

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
    backbuffer_bad = 1;
    pWindow->clear_damage(FL_DAMAGE_ALL);
  }
  if (backbuffer_bad || erase_overlay) {
    // Make sure we do a complete redraw...
    if (i->region) {Fl_Graphics_Driver::default_driver().XDestroyRegion(i->region); i->region = 0;}
    pWindow->clear_damage(FL_DAMAGE_ALL);
    backbuffer_bad = 0;
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
  if (!Fl_X11_Screen_Driver::ewmh_supported())
    pWindow->show();		// Old WMs, XMapRaised
  else if (i)			// New WMs use the NETWM attribute:
    Fl_X::activate_window(i->xid);
}


void Fl_X11_Window_Driver::draw_begin()
{
  if (shape_data_) {
    if (( shape_data_->lw_ != w() || shape_data_->lh_ != h() ) && shape_data_->shape_) {
      // size of window has changed since last time
      combine_mask();
    }
  }
}


void Fl_X11_Window_Driver::flush_double()
{
  if (!shown()) return;
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
      i->other_xid = fl_create_offscreen(w(), h());
    pWindow->clear_damage(FL_DAMAGE_ALL);
  }
    if (pWindow->damage() & ~FL_DAMAGE_EXPOSE) {
      fl_clip_region(i->region); i->region = 0;
      fl_window = i->other_xid;
      draw();
      fl_window = i->xid;
    }
  if (erase_overlay) fl_clip_region(0);
  int X,Y,W,H; fl_clip_box(0,0,w(),h(),X,Y,W,H);
  if (i->other_xid) fl_copy_offscreen(X, Y, W, H, i->other_xid, X, Y);
}


void Fl_X11_Window_Driver::flush_overlay()
{
  if (!shown()) return;
  int erase_overlay = (pWindow->damage()&FL_DAMAGE_OVERLAY);
  pWindow->clear_damage((uchar)(pWindow->damage()&~FL_DAMAGE_OVERLAY));
#if USE_XDBE
  if (can_xdbe()) flush_double_dbe(erase_overlay); else
#endif
  flush_double(erase_overlay);
  Fl_Overlay_Window *oWindow = pWindow->as_overlay_window();
  if (overlay() == oWindow) oWindow->draw_overlay();
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
  shape_data_->lw_ = w();
  shape_data_->lh_ = h();
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
    set_icons();
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
 
 This function exploits a feature of Fl_X11_Screen_Driver::read_win_rectangle() which,
 when called with negative 4th argument, captures the window decoration.
 */
void Fl_X11_Window_Driver::capture_titlebar_and_borders(Fl_Shared_Image*& top, Fl_Shared_Image*& left, Fl_Shared_Image*& bottom, Fl_Shared_Image*& right)
{
  Fl_RGB_Image *r_top, *r_left, *r_bottom, *r_right;
  top = left = bottom = right = NULL;
  if (pWindow->decorated_h() == h()) return;
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
    rgb = Fl::screen_driver()->read_win_rectangle(NULL, 0, 0, - (w() + 2 * wsides), htop, 0);
    r_top = new Fl_RGB_Image(rgb, w() + 2 * wsides, htop, 3);
    r_top->alloc_array = 1;
    top = Fl_Shared_Image::get(r_top);
  }
  if (wsides) {
    rgb = Fl::screen_driver()->read_win_rectangle(NULL, 0, htop, -wsides, h(), 0);
    r_left = new Fl_RGB_Image(rgb, wsides, h(), 3);
    r_left->alloc_array = 1;
    left = Fl_Shared_Image::get(r_left);
    rgb = Fl::screen_driver()->read_win_rectangle(NULL, w() + wsides, htop, -wsides, h(), 0);
    r_right = new Fl_RGB_Image(rgb, wsides, h(), 3);
    r_right->alloc_array = 1;
    right = Fl_Shared_Image::get(r_right);
    rgb = Fl::screen_driver()->read_win_rectangle(NULL, 0, htop + h(), -(w() + 2*wsides), hbottom, 0);
    r_bottom = new Fl_RGB_Image(rgb, w() + 2*wsides, hbottom, 3);
    r_bottom->alloc_array = 1;
    bottom = Fl_Shared_Image::get(r_bottom);
  }
  fl_window = from;
  previous->Fl_Surface_Device::set_current();
}

void Fl_X11_Window_Driver::wait_for_expose() {
  if (!shown()) return;
  Fl_X *i = Fl_X::i(pWindow);
  while (!i || i->wait_for_expose) {
    Fl::wait();
  }
}


// make X drawing go into this window (called by subclass flush() impl.)
void Fl_X11_Window_Driver::make_current() {
  if (!shown()) {
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


void Fl_X11_Window_Driver::show_menu()
{
#if HAVE_OVERLAY
  if (!shown() && ((Fl_Menu_Window*)pWindow)->overlay() && fl_find_overlay_visual()) {
    XInstallColormap(fl_display, fl_overlay_colormap);
    fl_background_pixel = int(fl_transparent_pixel);
    Fl_X::make_xid(pWindow, fl_overlay_visual, fl_overlay_colormap);
    fl_background_pixel = -1;
  } else
#endif
    pWindow->Fl_Window::show();
}


void Fl_X11_Window_Driver::hide() {
  Fl_X* ip = Fl_X::i(pWindow);
  if (hide_common()) return;
  if (ip->region) Fl_Graphics_Driver::default_driver().XDestroyRegion(ip->region);
# if USE_XFT
  Fl_Xlib_Graphics_Driver::destroy_xft_draw(ip->xid);
# endif
  // this test makes sure ip->xid has not been destroyed already
  if (ip->xid) XDestroyWindow(fl_display, ip->xid);
  delete ip;
}


void Fl_X11_Window_Driver::map() {
  XMapWindow(fl_display, fl_xid(pWindow)); // extra map calls are harmless
}


void Fl_X11_Window_Driver::unmap() {
  XUnmapWindow(fl_display, fl_xid(pWindow));
}


// Turning the border on/off by changing the motif_wm_hints property
// works on Irix 4DWM.  Does not appear to work for any other window
// manager.  Fullscreen still works on some window managers (fvwm is one)
// because they allow the border to be placed off-screen.

// Unfortunately most X window managers ignore changes to the border
// and refuse to position the border off-screen, so attempting to make
// the window full screen will lose the size of the border off the
// bottom and right.
void Fl_X11_Window_Driver::use_border() {
  if (shown()) sendxjunk();
}

void Fl_X11_Window_Driver::size_range() {
  Fl_Window_Driver::size_range();
  if (shown()) sendxjunk();
}

void Fl_X11_Window_Driver::iconize() {
  XIconifyWindow(fl_display, fl_xid(pWindow), fl_screen);
}

void Fl_X11_Window_Driver::decoration_sizes(int *top, int *left,  int *right, int *bottom) {
  // Ensure border is on screen; these values are generic enough
  // to work with many window managers, and are based on KDE defaults.
  *top = 20;
  *left = 4;
  *right = 4;
  *bottom = 8;
}

void Fl_X11_Window_Driver::show_with_args_begin() {
  // Get defaults for drag-n-drop and focus...
  const char *key = 0, *val;
  
  if (Fl::first_window()) key = Fl::first_window()->xclass();
  if (!key) key = "fltk";
  
  val = XGetDefault(fl_display, key, "dndTextOps");
  if (val) Fl::dnd_text_ops(strcasecmp(val, "true") == 0 ||
                            strcasecmp(val, "on") == 0 ||
                            strcasecmp(val, "yes") == 0);
  
  val = XGetDefault(fl_display, key, "tooltips");
  if (val) Fl_Tooltip::enable(strcasecmp(val, "true") == 0 ||
                              strcasecmp(val, "on") == 0 ||
                              strcasecmp(val, "yes") == 0);
  
  val = XGetDefault(fl_display, key, "visibleFocus");
  if (val) Fl::visible_focus(strcasecmp(val, "true") == 0 ||
                             strcasecmp(val, "on") == 0 ||
                             strcasecmp(val, "yes") == 0);
}


void Fl_X11_Window_Driver::show_with_args_end(int argc, char **argv) {
  // set the command string, used by state-saving window managers:
  int j;
  int n=0; for (j=0; j<argc; j++) n += strlen(argv[j])+1;
  char *buffer = new char[n];
  char *p = buffer;
  for (j=0; j<argc; j++) for (const char *q = argv[j]; (*p++ = *q++););
  XChangeProperty(fl_display, fl_xid(pWindow), XA_WM_COMMAND, XA_STRING, 8, 0,
                  (unsigned char *)buffer, p-buffer-1);
  delete[] buffer;
}


#if HAVE_OVERLAY

class _Fl_Overlay : public Fl_Window {
  friend class Fl_Overlay_Window;
  void flush();
  void show();
public:
  _Fl_Overlay(int x, int y, int w, int h) : Fl_Window(x,y,w,h) {
    set_flag(INACTIVE);
  }
};

/*int Fl_Overlay_Window::can_do_overlay() {
 return fl_find_overlay_visual() != 0;
 }*/

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
#if defined(FLTK_USE_CAIRO)
  if (Fl::cairo_autolink_context()) Fl::cairo_make_current(this); // capture gc changes automatically to update the cairo context adequately
#endif
  fl_overlay = 1;
  Fl_Overlay_Window *w = (Fl_Overlay_Window *)parent();
  Fl_X *myi = Fl_X::i(this);
  if (damage() != FL_DAMAGE_EXPOSE) XClearWindow(fl_display, fl_xid(this));
  fl_clip_region(myi->region); myi->region = 0;
  w->draw_overlay();
  fl_overlay = 0;
}
#endif // HAVE_OVERLAY


int Fl_X11_Window_Driver::can_do_overlay() {
#if HAVE_OVERLAY
  return fl_find_overlay_visual() != 0;
#endif
  return Fl_Window_Driver::can_do_overlay();
}

void Fl_X11_Window_Driver::redraw_overlay() {
#if HAVE_OVERLAY
  if (!fl_display) return; // this prevents fluid -c from opening display
  if (!overlay()) {
    if (can_do_overlay()) {
      Fl_Group::current(pWindow);
      overlay(new _Fl_Overlay(0,0,w(),h()));
      Fl_Group::current(0);
    } else {
      overlay(pWindow);	// fake the overlay
    }
  }
  if (shown()) {
    if (overlay() == pWindow) {
      pWindow->clear_damage(pWindow->damage()|FL_DAMAGE_OVERLAY);
      Fl::damage(FL_DAMAGE_CHILD);
    } else if (!overlay()->shown())
      overlay()->show();
    else
      overlay()->redraw();
  }
  return;
#endif
  Fl_Window_Driver::redraw_overlay();
}

void Fl_X11_Window_Driver::flush_menu() {
#if HAVE_OVERLAY
   if (!fl_overlay_visual || !overlay()) {flush_single(); return;}
   Fl_X *myi = Fl_X::i(pWindow);
   fl_window = myi->xid;
# if defined(FLTK_USE_CAIRO)
   // capture gc changes automatically to update the cairo context adequately
   if(Fl::autolink_context()) Fl::cairo_make_current(fl_graphics_driver->gc());
# endif
   fl_overlay = 1;
   fl_clip_region(myi->region); myi->region = 0; current(pWindow);
   draw();
   fl_overlay = 0;
#else
   flush_single();
#endif
}

void Fl_X11_Window_Driver::erase_menu() {
#if HAVE_OVERLAY
  if (pWindow->shown())  XClearWindow(fl_display, fl_xid(pWindow));
#endif
}

int Fl_X11_Window_Driver::scroll(int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y,
                                 void (*draw_area)(void*, int,int,int,int), void* data)
{
  XCopyArea(fl_display, fl_window, fl_window, (GC)fl_graphics_driver->gc(),
            src_x, src_y, src_w, src_h, dest_x, dest_y);
  // we have to sync the display and get the GraphicsExpose events! (sigh)
  for (;;) {
    XEvent e; XWindowEvent(fl_display, fl_window, ExposureMask, &e);
    if (e.type == NoExpose) break;
    // otherwise assume it is a GraphicsExpose event:
    draw_area(data, e.xexpose.x, e.xexpose.y,
              e.xexpose.width, e.xexpose.height);
    if (!e.xgraphicsexpose.count) break;
  }
  return 0;
}

//
// End of "$Id$".
//
