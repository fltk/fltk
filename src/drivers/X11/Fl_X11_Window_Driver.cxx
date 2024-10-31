//
// Definition of X11 window driver.
//
// Copyright 1998-2024 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//


#include <config.h>
#include "Fl_X11_Window_Driver.H"
#include "Fl_X11_Screen_Driver.H"
#if FLTK_USE_CAIRO
#  include <cairo-xlib.h>
#  include "../Cairo/Fl_X11_Cairo_Graphics_Driver.H"
#else
#  include "../Xlib/Fl_Xlib_Graphics_Driver.H"
#endif // FLTK_USE_CAIRO

#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl.H>
#include <FL/platform.H>
#include <string.h>
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#define ShapeBounding                   0
#define ShapeSet                        0

Window fl_window;


Fl_X11_Window_Driver::Fl_X11_Window_Driver(Fl_Window *win)
: Fl_Window_Driver(win)
{
  icon_ = new icon_data;
  shape_data_ = NULL;
  memset(icon_, 0, sizeof(icon_data));
#if USE_XFT
  screen_num_ = -1;
#endif
#if FLTK_USE_CAIRO
  cairo_ = NULL;
#endif
}


Fl_X11_Window_Driver::~Fl_X11_Window_Driver()
{
  if (shape_data_) {
    delete shape_data_->effective_bitmap_;
    delete shape_data_;
  }
  delete icon_;
}


// --- private

bool Fl_X11_Window_Driver::decorated_win_size(int &w, int &h)
{
  Fl_Window *win = pWindow;
  w = win->w();
  h = win->h();
  if (!win->shown() || win->parent() || !win->border() || !win->visible()) return false;
  Window root, parent, *children;
  unsigned n = 0;
  Status status = XQueryTree(fl_display, Fl_X::flx(win)->xid, &root, &parent, &children, &n);
  if (status != 0 && n) XFree(children);
  // when compiz is used, root and parent are the same window
  // and I don't know where to find the window decoration
  if (status == 0 || root == parent) return false;
  XWindowAttributes attributes;
  XGetWindowAttributes(fl_display, parent, &attributes);
  // sometimes, very wide window borders are reported
  // ignore them all:
  XWindowAttributes w_attributes;
  XGetWindowAttributes(fl_display, Fl_X::flx(win)->xid, &w_attributes);
  bool true_sides = false;
  if (attributes.width - w_attributes.width >= 20) {
    attributes.height -= (attributes.width - w_attributes.width);
    attributes.width = w_attributes.width;
  } else if (attributes.width > w_attributes.width) {
    true_sides = true;
  }

  int nscreen = screen_num();
  float s = Fl::screen_driver()->scale(nscreen);
  w = attributes.width / s;
  h = attributes.height / s;
  return true_sides;
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
  bool true_sides = decorated_win_size(w, h);
  return true_sides ? w : this->w();
}


void Fl_X11_Window_Driver::take_focus()
{
  Fl_X *i = Fl_X::flx(pWindow);
  if (!Fl_X11_Screen_Driver::ewmh_supported()) {

    // Save and restore the current group because 'show()' sets it to NULL.
    // See issue #515: Fl::focus() changes Fl_Group::current() to null.

    Fl_Group *cg = Fl_Group::current(); // save current group
    pWindow->show();                    // old WMs, XMapRaised
    Fl_Group::current(cg);              // restore current group
  }
  else if (i) {                         // New WMs use the NETWM attribute:
    activate_window();
  }
}


void Fl_X11_Window_Driver::draw_begin()
{
  if (shape_data_) {
    int nscreen = screen_num();
    float s = Fl::screen_driver()->scale(nscreen);
    if (( shape_data_->lw_ != int(s*w()) || shape_data_->lh_ != int(s*h()) ) && shape_data_->shape_) {
      // size of window has changed since last time
      combine_mask();
    }
  }
}


void Fl_X11_Window_Driver::flush_double()
{
  if (!shown()) return;
  flush_double(0);
}

void Fl_X11_Window_Driver::flush_double(int erase_overlay)
{
  pWindow->make_current(); // make sure fl_gc is non-zero
  Fl_X *i = Fl_X::flx(pWindow);
  if (!other_xid) {
    other_xid = new Fl_Image_Surface(w(), h(), 1);
#if FLTK_USE_CAIRO
    cairo_ = ((Fl_Cairo_Graphics_Driver*)other_xid->driver())->cr();
#endif
    pWindow->clear_damage(FL_DAMAGE_ALL);
  }
#if FLTK_USE_CAIRO
  ((Fl_X11_Cairo_Graphics_Driver*)fl_graphics_driver)->set_cairo(cairo_);
#endif
    if (pWindow->damage() & ~FL_DAMAGE_EXPOSE) {
      fl_clip_region(i->region); i->region = 0;
      fl_window = other_xid->offscreen();
# if defined(FLTK_HAVE_CAIROEXT)
      if (Fl::cairo_autolink_context()) Fl::cairo_make_current(pWindow);
# endif
      draw();
      fl_window = i->xid;
    }
  if (erase_overlay) fl_clip_region(0);
  int X = 0, Y = 0, W = 0, H = 0;
  fl_clip_box(0, 0, w(), h(), X, Y, W, H);
  if (other_xid) fl_copy_offscreen(X, Y, W, H, other_xid->offscreen(), X, Y);
}


void Fl_X11_Window_Driver::flush_overlay()
{
  if (!shown()) return;
  int erase_overlay = (pWindow->damage()&FL_DAMAGE_OVERLAY) | (overlay() == pWindow);
  pWindow->clear_damage((uchar)(pWindow->damage()&~FL_DAMAGE_OVERLAY));
  flush_double(erase_overlay);
  if (overlay() == pWindow) {
#if FLTK_USE_CAIRO
    float scale = fl_graphics_driver->scale();
    int W = pWindow->w() * scale, H = pWindow->h() * scale;
    cairo_surface_t *s = cairo_xlib_surface_create(fl_display, Fl_X::flx(pWindow)->xid, fl_visual->visual, W, H);
    cairo_t *overlay_cairo = cairo_create(s);
    cairo_surface_destroy(s);
    cairo_save(overlay_cairo);
    ((Fl_X11_Cairo_Graphics_Driver*)fl_graphics_driver)->set_cairo(overlay_cairo);
#endif
    pWindow->as_overlay_window()->draw_overlay();
#if FLTK_USE_CAIRO
    cairo_destroy(overlay_cairo);
#endif
  }
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
  shape_data_->effective_bitmap_ = bitmap;
  shape_data_->shape_ = img;
}

void Fl_X11_Window_Driver::shape(const Fl_Image* img) {
  if (shape_data_) {
    if (shape_data_->effective_bitmap_) { delete shape_data_->effective_bitmap_; }
  }
  else {
    shape_data_ = new shape_data_type;
  }
  memset(shape_data_, 0, sizeof(shape_data_type));
  pWindow->border(false);
  int d = img->d();
  if (d && img->count() >= 2) {
    shape_pixmap_((Fl_Image*)img);
    shape_data_->shape_ = (Fl_Image*)img;
  }
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
  float s = Fl::screen_driver()->scale(screen_num());
  shape_data_->lw_ = w()*s;
  shape_data_->lh_ = h()*s;
  Fl_Image* temp = shape_data_->effective_bitmap_ ? shape_data_->effective_bitmap_ : shape_data_->shape_;
  temp = temp->copy(shape_data_->lw_, shape_data_->lh_);
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
    for (int i = 0;i < count;i++) {
      icon_->icons[i] = (Fl_RGB_Image*)((Fl_RGB_Image*)icons[i])->copy();
      icon_->icons[i]->normalize();
    }
  }

  if (Fl_X::flx(pWindow))
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
 when called with negative 3rd argument, captures the window decoration.
 Other requirement to capture the window decoration:
   fl_window is the parent window of the top window
 */
void Fl_X11_Window_Driver::capture_titlebar_and_borders(Fl_RGB_Image*& top, Fl_RGB_Image*& left, Fl_RGB_Image*& bottom, Fl_RGB_Image*& right)
{
  top = left = bottom = right = NULL;
  if (pWindow->decorated_h() == h()) return;
  Window from = fl_window;
  Window root, parent, *children, child_win, xid = fl_xid(pWindow);
  unsigned n = 0;
  int do_it;
  int wsides, htop, ww, hh;
  do_it = (XQueryTree(fl_display, xid, &root, &parent, &children, &n) != 0 &&
           XTranslateCoordinates(fl_display, xid, parent, 0, 0, &wsides, &htop, &child_win) == True);
  if (n) XFree(children);
  if (!do_it) return;
  bool true_sides = Fl_X11_Window_Driver::decorated_win_size(ww, hh);
  float s = Fl::screen_driver()->scale(screen_num());
  if (true_sides) {
    XWindowAttributes attributes;
    XGetWindowAttributes(fl_display, parent, &attributes);
    ww = attributes.width;
    hh = attributes.height;
  } else {
    ww *= s;
    hh *= s;
  }
  if (!true_sides) htop -= wsides;
  fl_window = parent;
  if (htop) {
    if (true_sides) {
      top = Fl::screen_driver()->read_win_rectangle(1, 1, -(ww-2), hh-2, pWindow);
      if (top) top->scale(decorated_w(), decorated_h(), 0, 1);
    } else {
      top = Fl::screen_driver()->read_win_rectangle(wsides, wsides, -(ww-1), htop, pWindow);
      if (top) top->scale(w(), htop / s, 0, 1);
    }
  }
  fl_window = from;
}


// make X drawing go into this window (called by subclass flush() impl.)
void Fl_X11_Window_Driver::make_current() {
  if (!shown()) {
    fl_alert("Fl_Window::make_current(), but window is not shown().");
    Fl::fatal("Fl_Window::make_current(), but window is not shown().");
  }
  fl_window = fl_xid(pWindow);
  fl_graphics_driver->clip_region(0);

#if FLTK_USE_CAIRO
  float scale = Fl::screen_scale(screen_num()); // get the screen scaling factor
  if (!pWindow->as_double_window()) {
    if (!cairo_) {
      int W = pWindow->w() * scale, H = pWindow->h() * scale;
      cairo_surface_t *s = cairo_xlib_surface_create(fl_display, fl_window, fl_visual->visual, W, H);
      cairo_ = cairo_create(s);
      cairo_surface_destroy(s);
      cairo_save(cairo_);
    }
    ((Fl_X11_Cairo_Graphics_Driver*)fl_graphics_driver)->set_cairo(cairo_);
  }
  fl_graphics_driver->scale(scale);
#elif USE_XFT
  ((Fl_Xlib_Graphics_Driver*)fl_graphics_driver)->scale(Fl::screen_driver()->scale(screen_num()));
#endif

#ifdef FLTK_HAVE_CAIROEXT
  // update the cairo_t context
  if (Fl::cairo_autolink_context()) Fl::cairo_make_current(pWindow);
#endif
}


void Fl_X11_Window_Driver::hide() {
  Fl_X* ip = Fl_X::flx(pWindow);
  if (hide_common()) return;
  if (ip->region) Fl_Graphics_Driver::default_driver().XDestroyRegion(ip->region);
# if USE_XFT && ! FLTK_USE_CAIRO
  Fl_Xlib_Graphics_Driver::destroy_xft_draw(ip->xid);
  screen_num_ = -1;
# endif
# if FLTK_USE_CAIRO
  if (cairo_ && !pWindow->as_double_window()) {
    cairo_destroy(cairo_);
    cairo_ = NULL;
  }
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
  if (argc) {
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
}

int Fl_X11_Window_Driver::scroll(int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y,
                                 void (*draw_area)(void*, int,int,int,int), void* data)
{
  float s = Fl::screen_driver()->scale(screen_num());
  XCopyArea(fl_display, fl_window, fl_window, (GC)fl_graphics_driver->gc(),
            int(src_x*s), int(src_y*s), int(src_w*s), int(src_h*s), int(dest_x*s), int(dest_y*s));
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

void Fl_X11_Window_Driver::makeWindow()
{
  Fl_X::make_xid(pWindow, fl_visual, fl_colormap);
}

const Fl_Image* Fl_X11_Window_Driver::shape() {
  return shape_data_ ? shape_data_->shape_ : NULL;
}

Fl_Window *fl_x11_find(Window xid) {
  return Fl_Window_Driver::find((fl_uintptr_t)xid);
}

Window fl_x11_xid(const Fl_Window *win) {
  return (Window)Fl_Window_Driver::xid(win);
}


#if USE_XFT

Fl_X11_Window_Driver::type_for_resize_window_between_screens Fl_X11_Window_Driver::data_for_resize_window_between_screens_ = {0, false};

void Fl_X11_Window_Driver::resize_after_screen_change(void *data) {
  Fl_Window *win = (Fl_Window*)data;
  float f = Fl::screen_driver()->scale(data_for_resize_window_between_screens_.screen);
  Fl_Window_Driver::driver(win)->resize_after_scale_change(data_for_resize_window_between_screens_.screen, f, f);
  data_for_resize_window_between_screens_.busy = false;
}

#endif // USE_XFT

fl_uintptr_t Fl_X11_Window_Driver::os_id() {
  return fl_xid(pWindow);
}
