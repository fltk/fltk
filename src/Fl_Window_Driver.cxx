//
// "$Id$"
//
// A base class for platform specific window handling code
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2017 by Bill Spitzak and others.
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


#include <FL/Fl_Window_Driver.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <FL/x.H>

extern void fl_throw_focus(Fl_Widget *o);

Fl_Window_Driver::Fl_Window_Driver(Fl_Window *win) :
pWindow(win)
{
  shape_data_ = NULL;
  wait_for_expose_value = 0;
  other_xid = 0;
}


Fl_Window_Driver::~Fl_Window_Driver()
{
}

int Fl_Window_Driver::minw() {return pWindow->minw;}
int Fl_Window_Driver::minh() {return pWindow->minh;}
int Fl_Window_Driver::maxw() {return pWindow->maxw;}
int Fl_Window_Driver::maxh() {return pWindow->maxh;}
int Fl_Window_Driver::dw() {return pWindow->dw;}
int Fl_Window_Driver::dh() {return pWindow->dh;}
int Fl_Window_Driver::aspect() {return pWindow->aspect;}
int Fl_Window_Driver::force_position() {return pWindow->force_position(); }
void Fl_Window_Driver::force_position(int c) { pWindow->force_position(c); }
void Fl_Window_Driver::x(int X) {pWindow->x(X); }
void Fl_Window_Driver::y(int Y) {pWindow->y(Y); }
int Fl_Window_Driver::fullscreen_screen_top() {return pWindow->fullscreen_screen_top;}
int Fl_Window_Driver::fullscreen_screen_bottom() {return pWindow->fullscreen_screen_bottom;}
int Fl_Window_Driver::fullscreen_screen_left() {return pWindow->fullscreen_screen_left;}
int Fl_Window_Driver::fullscreen_screen_right() {return pWindow->fullscreen_screen_right;}
void Fl_Window_Driver::current(Fl_Window *c) {pWindow->current_ = c;}



unsigned char Fl_Window_Driver::size_range_set() {return pWindow->size_range_set;}

void Fl_Window_Driver::flush_Fl_Window() { pWindow->Fl_Window::flush(); }

void Fl_Window_Driver::flush_menu() { pWindow->Fl_Window::flush(); }

void Fl_Window_Driver::draw() { pWindow->draw(); }

void Fl_Window_Driver::make_current() { }

void Fl_Window_Driver::show() { }

void Fl_Window_Driver::show_menu() { pWindow->Fl_Window::show(); }

void Fl_Window_Driver::label(const char *name, const char *mininame) {}

void Fl_Window_Driver::take_focus()
{
  // nothing to do
}


void Fl_Window_Driver::flush_double()
{
  flush_Fl_Window();
}


void Fl_Window_Driver::flush_overlay()
{
  flush_Fl_Window();
}


void Fl_Window_Driver::draw_begin()
{
  // nothing to do
}


void Fl_Window_Driver::draw_end()
{
  // nothing to do
}



void Fl_Window_Driver::destroy_double_buffer() {
  fl_delete_offscreen(other_xid);
  other_xid = 0;
}


/** Assigns a non-rectangular shape to the window.
 This function gives an arbitrary shape (not just a rectangular region) to an Fl_Window.
 An Fl_Image of any dimension can be used as mask; it is rescaled to the window's dimension as needed.
 
 The layout and widgets inside are unaware of the mask shape, and most will act as though the window's
 rectangular bounding box is available
 to them. It is up to you to make sure they adhere to the bounds of their masking shape.
 
 The \p img argument can be an Fl_Bitmap, Fl_Pixmap, Fl_RGB_Image or Fl_Shared_Image:
 \li With Fl_Bitmap or Fl_Pixmap, the shaped window covers the image part where bitmap bits equal one,
 or where the pixmap is not fully transparent.
 \li With an Fl_RGB_Image with an alpha channel (depths 2 or 4), the shaped window covers the image part
 that is not fully transparent.
 \li With an Fl_RGB_Image of depth 1 (gray-scale) or 3 (RGB), the shaped window covers the non-black image part.
 \li With an Fl_Shared_Image, the shape is determined by rules above applied to the underlying image.
 The shared image should not have been scaled through Fl_Shared_Image::scale().
 
 Platform details:
 \li On the unix/linux platform, the SHAPE extension of the X server is required.
 This function does control the shape of Fl_Gl_Window instances.
 \li On the MSWindows platform, this function does nothing with class Fl_Gl_Window.
 \li On the Mac platform, OS version 10.4 or above is required.
 An 8-bit shape-mask is used when \p img is an Fl_RGB_Image:
 with depths 2 or 4, the image alpha channel becomes the shape mask such that areas with alpha = 0
 are out of the shaped window;
 with depths 1 or 3, white and black are in and out of the
 shaped window, respectively, and other colors give intermediate masking scores.
 This function does nothing with class Fl_Gl_Window.
 
 The window borders and caption created by the window system are turned off by default. They
 can be re-enabled by calling Fl_Window::border(1).
 
 A usage example is found at example/shapedwindow.cxx.
 
 \version 1.3.3
 */
void Fl_Window::shape(const Fl_Image* img) {pWindowDriver->shape(img);}


/** Set the window's shape with an Fl_Image.
 \see void shape(const Fl_Image* img)
 */
void Fl_Window::shape(const Fl_Image& img) {pWindowDriver->shape(&img);}

/** Returns non NULL when the window has been assigned a non-rectangular shape */
int Fl_Window::is_shaped() {return pWindowDriver->shape_data_ != NULL;}

void Fl_Window_Driver::shape_pixmap_(Fl_Image* pixmap) {
  Fl_RGB_Image* rgba = new Fl_RGB_Image((Fl_Pixmap*)pixmap);
  shape_alpha_(rgba, 3);
  delete rgba;
}

void Fl_Window_Driver::capture_titlebar_and_borders(Fl_Shared_Image*& top, Fl_Shared_Image*& left, Fl_Shared_Image*& bottom, Fl_Shared_Image*& right) {
  top = left = bottom = right = NULL;
}


// This function is available for use by platform-specific, Fl_Window_Driver-derived classes
int Fl_Window_Driver::hide_common() {
  pWindow->clear_visible();
  
  if (!shown()) return 1;
  
  // remove from the list of windows:
  Fl_X* ip = Fl_X::i(pWindow);
  Fl_X** pp = &Fl_X::first;
  for (; *pp != ip; pp = &(*pp)->next) if (!*pp) return 1;
  *pp = ip->next;
  
  pWindow->i = 0;
  
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
    pWindow->show();
  }
}

void Fl_Window_Driver::size_range() {
  pWindow->size_range_set = 1;
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

fl_uintptr_t Fl_Window_Driver::current_cursor() {
  return 0;
}


void Fl_Window_Driver::wait_for_expose() {
  if (!shown()) return;
  Fl_X *i = Fl_X::i(pWindow);
  while (!i || wait_for_expose_value) {
    Fl::wait();
  }
}

int Fl_Window_Driver::screen_num() {
  if (pWindow->parent()) return pWindow->top_window()->driver()->screen_num();
  return Fl::screen_num(x(), y(), w(), h());
}

bool Fl_Window_Driver::is_a_rescale_ = false;

void Fl_Window_Driver::resize_after_scale_change(int ns, float old_f, float new_f) {
  screen_num(ns);
  Fl_Graphics_Driver::default_driver().scale(new_f);
  int X = pWindow->x()*old_f/new_f, Y = pWindow->y()*old_f/new_f;
  int W, H;
  if (pWindow->fullscreen_active()) {
    W = pWindow->w() * old_f/new_f; H = pWindow->h() * old_f/new_f;
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
  is_a_rescale_ = true;
  size_range();
  pWindow->resize(X, Y, W, H);
  is_a_rescale_ = false;
}

//
// End of "$Id$".
//
