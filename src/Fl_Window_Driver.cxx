//
// A base class for platform specific window handling code
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
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

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

#include "Fl_Window_Driver.H"
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <FL/platform.H>
#include "Fl_Screen_Driver.H"

extern void fl_throw_focus(Fl_Widget *o);


/**
 Create a new Window Driver.

 This class must be derived into a new class that manages windows
 on the target platform.
 */
Fl_Window_Driver::Fl_Window_Driver(Fl_Window *win)
  : pWindow(win) {
  wait_for_expose_value = 0;
  other_xid = 0;
  screen_num_ = 0;
}


Fl_Window_Driver::~Fl_Window_Driver() {
  // empty
}

// accessors to Fl_Window private stuff
int Fl_Window_Driver::force_position() {return pWindow->force_position(); }
void Fl_Window_Driver::force_position(int c) { pWindow->force_position(c); }
void Fl_Window_Driver::x(int X) {pWindow->x(X); }
void Fl_Window_Driver::y(int Y) {pWindow->y(Y); }
int Fl_Window_Driver::fullscreen_screen_top() {return pWindow->fullscreen_screen_top;}
int Fl_Window_Driver::fullscreen_screen_bottom() {return pWindow->fullscreen_screen_bottom;}
int Fl_Window_Driver::fullscreen_screen_left() {return pWindow->fullscreen_screen_left;}
int Fl_Window_Driver::fullscreen_screen_right() {return pWindow->fullscreen_screen_right;}
void Fl_Window_Driver::current(Fl_Window *c) {pWindow->current_ = c;}


/**
 Draw the window content.
 A new driver can add code before or after drawing an individual window.
 */
void Fl_Window_Driver::draw() { pWindow->draw(); }

/**
 Prepare this window for rendering.
 A new driver may prepare bitmaps and clipping areas for calls to the
 graphics driver.
 */
void Fl_Window_Driver::make_current() { }

/**
 Make the window visible and raise it to the top.
 */
void Fl_Window_Driver::show() { }


/**
 Change the window title.
 A new driver should provide an interface to change the title of the window
 in the title bar.
 */
void Fl_Window_Driver::label(const char *name, const char *mininame) {}

void Fl_Window_Driver::take_focus() {
  // nothing to do
}

void Fl_Window_Driver::flush_double() {
  pWindow->Fl_Window::flush();
}

void Fl_Window_Driver::flush_overlay() {
  pWindow->Fl_Window::flush();
}

void Fl_Window_Driver::draw_begin() {
  // nothing to do
}

void Fl_Window_Driver::draw_end() {
  // nothing to do
}

void Fl_Window_Driver::destroy_double_buffer() {
  delete other_xid;
  other_xid = 0;
}

void Fl_Window_Driver::shape_pixmap_(Fl_Image* pixmap) {
  Fl_RGB_Image* rgba = new Fl_RGB_Image((Fl_Pixmap*)pixmap);
  shape_alpha_(rgba, 3);
  delete rgba;
}

void Fl_Window_Driver::capture_titlebar_and_borders(Fl_RGB_Image*& top, Fl_RGB_Image*& left, Fl_RGB_Image*& bottom, Fl_RGB_Image*& right) {
  top = left = bottom = right = NULL;
}


// This function is available for use by platform-specific, Fl_Window_Driver-derived classes
int Fl_Window_Driver::hide_common() {
  pWindow->clear_visible();

  if (!shown()) return 1;

  // remove from the list of windows:
  Fl_X* ip = Fl_X::flx(pWindow);
  Fl_X** pp = &Fl_X::first;
  for (; *pp != ip; pp = &(*pp)->next) if (!*pp) return 1;
  *pp = ip->next;

  pWindow->flx_ = 0;

  // recursively remove any subwindows:
  for (Fl_X *wi = Fl_X::first; wi;) {
    Fl_Window* W = wi->w;
    if (W->window() == pWindow) {
      W->hide();
      W->set_visible();
      wi = Fl_X::first;
    } else wi = wi->next;
  }

  if (pWindow == Fl::modal_) { // we are closing the modal window, find next one:
    Fl_Window* W;
    for (W = Fl::first_window(); W; W = Fl::next_window(W))
      if (W->modal()) break;
    Fl::modal_ = W;
  }

  // Make sure no events are sent to this window:
  fl_throw_focus(pWindow);
  pWindow->handle(FL_HIDE);
  return 0;
}


void Fl_Window_Driver::use_border() {
  if (shown()) {
    pWindow->hide(); // hide and then show to reflect the new state of the window border
    pWindow->force_position(1);
    pWindow->show();
  }
}

void Fl_Window_Driver::size_range() {
  // *FIXME* This should not be necessary!
  // pWindow->size_range_set = 1;
}

int Fl_Window_Driver::can_do_overlay() {
  return 0;
}

void Fl_Window_Driver::redraw_overlay() {
  ((Fl_Overlay_Window*)pWindow)->overlay_ = pWindow;
  pWindow->clear_damage((uchar)(pWindow->damage()|FL_DAMAGE_OVERLAY));
  Fl::damage(FL_DAMAGE_CHILD);
}

void Fl_Window_Driver::flush()
{
  pWindow->flush();
}

int Fl_Window_Driver::set_cursor(Fl_Cursor) {
  return 0;
}

int Fl_Window_Driver::set_cursor(const Fl_RGB_Image*, int, int) {
  return 0;
}

void Fl_Window_Driver::wait_for_expose() {
  if (!shown()) return;
  Fl_X *i = Fl_X::flx(pWindow);
  while (!i || wait_for_expose_value) {
    Fl::wait();
  }
}

int Fl_Window_Driver::screen_num() {
  if (pWindow->parent()) {
    screen_num_ = Fl_Window_Driver::driver(pWindow->top_window())->screen_num();
  }
  return screen_num_ >= 0 ? screen_num_ : 0;
}

bool Fl_Window_Driver::is_a_rescale_ = false;

void Fl_Window_Driver::resize_after_scale_change(int ns, float old_f, float new_f) {
  screen_num(ns);
  Fl_Graphics_Driver::default_driver().scale(new_f);
  int X = int(pWindow->x() * old_f / new_f), Y = int(pWindow->y() * old_f / new_f);
  int W, H;
  if (pWindow->fullscreen_active()) {
    W = int(pWindow->w() * old_f / new_f); H = int(pWindow->h() * old_f / new_f);
  } else {
    W = pWindow->w(); H = pWindow->h();
    int sX, sY, sW, sH;
    Fl::screen_xywh(sX, sY, sW, sH, ns); // bounding box of new screen
    const int d = 5; // make sure new window centre is located in new screen
    if (X+W/2 < sX) X = sX-W/2+d;
    else if (X+W/2 > sX+sW-1) X = sX+sW-1-W/2-d;
    if (Y+H/2 < sY) Y = sY-H/2+d;
    else if (Y+H/2 > sY+sH-1) Y = sY+sH-1-H/2-d;
  }
  size_range(); // adjust the OS-level boundary size values for the window (#880)
  is_a_rescale_ = true;
  pWindow->resize(X, Y, W, H);
  is_a_rescale_ = false;
}

void Fl_Window_Driver::reposition_menu_window(int x, int y) {
  if (y != pWindow->y() || x != pWindow->x()) pWindow->Fl_Widget::position(x, y);
}

void Fl_Window_Driver::menu_window_area(int &X, int &Y, int &W, int &H, int nscreen) {
  int mx, my;
  Fl_Screen_Driver *scr_driver = Fl::screen_driver();
  if (nscreen < 0) nscreen = scr_driver->get_mouse(mx, my);
  scr_driver->screen_work_area(X, Y, W, H, nscreen);
}

/** Returns  the platform-specific reference of the given window, or NULL if that window isn't shown.
 \version 1.4.0 */
fl_uintptr_t Fl_Window_Driver::xid(const Fl_Window *win) {
  Fl_X *flx = win->flx_;
  return flx ? flx->xid : 0;
}

/** Returns a pointer to the Fl_Window corresponding to the platform-specific reference \p xid of a shown window.
 \version 1.4.0 */
Fl_Window *Fl_Window_Driver::find(fl_uintptr_t xid) {
  Fl_X *window;
  for (Fl_X **pp = &Fl_X::first; (window = *pp); pp = &window->next) {
    if (window->xid == xid) {
      if (window != Fl_X::first && !Fl::modal()) {
        // make this window be first to speed up searches
        // this is not done if modal is true to avoid messing up modal stack
        *pp = window->next;
        window->next = Fl_X::first;
        Fl_X::first = window;
      }
      return window->w;
    }
  }
  return 0;
}


void Fl_Window_Driver::maximize() {
  *no_fullscreen_x() = x();
  *no_fullscreen_y() = y();
  *no_fullscreen_w() = w();
  *no_fullscreen_h() = h();
  int X,Y,W,H;
  Fl::screen_work_area(X, Y, W, H, screen_num());
  int width = decorated_w();
  int height = decorated_h();
  int dw = (width - w());
  int dh = (height - h() - dw);
  bool need_hide_show = maximize_needs_hide();
  if (need_hide_show) hide(); // pb may occur in subwindow without this
  resize(X + dw/2, Y + dh + dw/2, W - dw, H - dh - dw);
  if (need_hide_show) show();
}


void Fl_Window_Driver::un_maximize() {
  resize(*no_fullscreen_x(), *no_fullscreen_y(),
         *no_fullscreen_w(), *no_fullscreen_h());
  *no_fullscreen_x() = 0;
  *no_fullscreen_y() = 0;
  *no_fullscreen_w() = 0;
  *no_fullscreen_h() = 0;
}

/**
 \}
 \endcond
 */
