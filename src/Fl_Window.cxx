//
// "$Id$"
//
// Window widget class for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

// The Fl_Window is a window in the fltk library.
// This is the system-independent portions.  The huge amount of 
// crap you need to do to communicate with X is in Fl_x.cxx, the
// equivalent (but totally different) crap for MSWindows is in Fl_win32.cxx
#include <config.h>
#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Window.H>
#include <stdlib.h>
#include "flstring.h"

#ifdef __APPLE_QUARTZ__
#include <FL/fl_draw.H>
#endif

char *Fl_Window::default_xclass_ = 0L;

void Fl_Window::_Fl_Window() {
  type(FL_WINDOW);
  box(FL_FLAT_BOX);
  if (Fl::scheme_bg_) {
    labeltype(FL_NORMAL_LABEL);
    align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    image(Fl::scheme_bg_);
  } else {
    labeltype(FL_NO_LABEL);
  }
  i = 0;
  xclass_ = 0;
  icon_ = new icon_data;
  memset(icon_, 0, sizeof(*icon_));
  iconlabel_ = 0;
  resizable(0);
  size_range_set = 0;
  minw = maxw = minh = maxh = 0;
  shape_data_ = NULL;

#if FLTK_ABI_VERSION >= 10301
  no_fullscreen_x = 0;
  no_fullscreen_y = 0;
  no_fullscreen_w = w();
  no_fullscreen_h = h();
#endif

#if FLTK_ABI_VERSION >= 10303
  fullscreen_screen_top = -1;
  fullscreen_screen_bottom = -1;
  fullscreen_screen_left = -1;
  fullscreen_screen_right = -1;
#endif

  callback((Fl_Callback*)default_callback);
}

Fl_Window::Fl_Window(int X,int Y,int W, int H, const char *l)
: Fl_Group(X, Y, W, H, l) {
  cursor_default = FL_CURSOR_DEFAULT;

  _Fl_Window();
  set_flag(FORCE_POSITION);
}

Fl_Window::Fl_Window(int W, int H, const char *l)
// fix common user error of a missing end() with current(0):
  : Fl_Group((Fl_Group::current(0),0), 0, W, H, l) {
  cursor_default = FL_CURSOR_DEFAULT;

  _Fl_Window();
  clear_visible();
}

Fl_Window::~Fl_Window() {
  hide();
  if (xclass_) {
    free(xclass_);
  }
  free_icons();
  delete icon_;
  if (shape_data_) {
    if (shape_data_->todelete_) delete shape_data_->todelete_;
#if defined(__APPLE__)
    if (shape_data_->mask) {
      CGImageRelease(shape_data_->mask);
    }
#endif
    delete shape_data_;
  }
}


/** Returns a pointer to the nearest parent window up the widget hierarchy.
    This will return sub-windows if there are any, or the parent window if there's no sub-windows.
    If this widget IS the top-level window, NULL is returned.
    \retval  NULL if no window is associated with this widget.
    \note for an Fl_Window widget, this returns its <I>parent</I> window 
          (if any), not <I>this</I> window.
    \see top_window()
*/
Fl_Window *Fl_Widget::window() const {
  for (Fl_Widget *o = parent(); o; o = o->parent())
    if (o->type() >= FL_WINDOW) return (Fl_Window*)o;
  return 0;
}

/** Returns a pointer to the top-level window for the widget.
    In other words, the 'window manager window' that contains this widget.
    This method differs from window() in that it won't return sub-windows (if there are any).
    \returns the top-level window, or NULL if no top-level window is associated with this widget.
    \see window()
*/
Fl_Window *Fl_Widget::top_window() const {
  const Fl_Widget *w = this;
  while (w->parent()) { w = w->parent(); }		// walk up the widget hierarchy to top-level item
  return const_cast<Fl_Widget*>(w)->as_window();	// return if window, or NULL if not
}

/**
  Finds the x/y offset of the current widget relative to the top-level window.
  \param[out] xoff,yoff Returns the x/y offset
  \returns the top-level window (or NULL for a widget that's not in any window)
*/
Fl_Window* Fl_Widget::top_window_offset(int& xoff, int& yoff) const {
  xoff = yoff = 0;
  const Fl_Widget *w = this;
  while (w && w->window()) {
    xoff += w->x();			// accumulate offsets
    yoff += w->y();
    w = w->window();			// walk up window hierarchy
  }
  return const_cast<Fl_Widget*>(w)->as_window();
}

/** Gets the x position of the window on the screen */
int Fl_Window::x_root() const {
  Fl_Window *p = window();
  if (p) return p->x_root() + x();
  return x();
}
/** Gets the y position of the window on the screen */
int Fl_Window::y_root() const {
  Fl_Window *p = window();
  if (p) return p->y_root() + y();
  return y();
}

void Fl_Window::label(const char *name) {
  label(name, iconlabel());	// platform dependent
}

/** Sets the window titlebar label to a copy of a character string */
void Fl_Window::copy_label(const char *a) {
  Fl_Widget::copy_label(a);
  label(label(), iconlabel());	// platform dependent
}

void Fl_Window::iconlabel(const char *iname) {
  label(label(), iname);	// platform dependent
}

// the Fl::atclose pointer is provided for back compatibility.  You
// can now just change the callback for the window instead.

/** Default callback for window widgets. It hides the window and then calls the default widget callback. */
void Fl::default_atclose(Fl_Window* window, void* v) {
  window->hide();
  Fl_Widget::default_callback(window, v); // put on Fl::read_queue()
}
/** Back compatibility: default window callback handler \see Fl::set_atclose() */
void (*Fl::atclose)(Fl_Window*, void*) = default_atclose;
/** Back compatibility: Sets the default callback v for win to call on close event */
void Fl_Window::default_callback(Fl_Window* win, void* v) {
  Fl::atclose(win, v);
}

/**  Returns the last window that was made current. \see Fl_Window::make_current() */
Fl_Window *Fl_Window::current() {
  return current_;
}

/** Returns the default xclass.

  \see Fl_Window::default_xclass(const char *)

 */
const char *Fl_Window::default_xclass()
{
  if (default_xclass_) {
    return default_xclass_;
  } else {
    return "FLTK";
  }
}

/** Sets the default window xclass.

  The default xclass is used for all windows that don't have their
  own xclass set before show() is called. You can change the default
  xclass whenever you want, but this only affects windows that are
  created (and shown) after this call.

  The given string \p xc is copied. You can use a local variable or
  free the string immediately after this call.

  If you don't call this, the default xclass for all windows will be "FLTK".
  You can reset the default xclass by specifying NULL for \p xc.

  If you call Fl_Window::xclass(const char *) for any window, then
  this also sets the default xclass, unless it has been set before.

  \param[in] xc default xclass for all windows subsequently created

  \see Fl_Window::xclass(const char *)
*/
void Fl_Window::default_xclass(const char *xc)
{
  if (default_xclass_) {
    free(default_xclass_);
    default_xclass_ = 0L;
  }
  if (xc) {
    default_xclass_ = strdup(xc);
  }
}

/** Sets the xclass for this window.

  A string used to tell the system what type of window this is. Mostly
  this identifies the picture to draw in the icon. This only works if
  called \e before calling show().

  <I>Under X</I>, this is turned into a XA_WM_CLASS pair by truncating at
  the first non-alphanumeric character and capitalizing the first character,
  and the second one if the first is 'x'.  Thus "foo" turns into "foo, Foo",
  and "xprog.1" turns into "xprog, XProg".

  <I>Under Microsoft Windows</I>, this string is used as the name of the
  WNDCLASS structure, though it is not clear if this can have any
  visible effect.

  \since FLTK 1.3 the passed string is copied. You can use a local
  variable or free the string immediately after this call. Note that
  FLTK 1.1 stores the \e pointer without copying the string.

  If the default xclass has not yet been set, this also sets the
  default xclass for all windows created subsequently.

  \see Fl_Window::default_xclass(const char *)
*/
void Fl_Window::xclass(const char *xc) 
{
  if (xclass_) {
    free(xclass_);
    xclass_ = 0L;
  }
  if (xc) {
    xclass_ = strdup(xc);
    if (!default_xclass_) {
      default_xclass(xc);
    }
  }
}

/** Returns the xclass for this window, or a default.

  \see Fl_Window::default_xclass(const char *)
  \see Fl_Window::xclass(const char *)
*/
const char *Fl_Window::xclass() const
{
  if (xclass_) {
    return xclass_;
  } else {
    return default_xclass();
  }
}

/** Sets a single default window icon.

  If \p icon is NULL the current default icons are removed.

  \param[in] icon default icon for all windows subsequently created or NULL

  \see Fl_Window::default_icons(const Fl_RGB_Image *[], int)
  \see Fl_Window::icon(const Fl_RGB_Image *)
  \see Fl_Window::icons(const Fl_RGB_Image *[], int)
 */
void Fl_Window::default_icon(const Fl_RGB_Image *icon) {
  if (icon)
    default_icons(&icon, 1);
  else
    default_icons(&icon, 0);
}

/** Sets the default window icons.

  The default icons are used for all windows that don't have their
  own icons set before show() is called. You can change the default
  icons whenever you want, but this only affects windows that are
  created (and shown) after this call.

  The given images in \p icons are copied. You can use a local
  variable or free the images immediately after this call.

  \param[in] icons default icons for all windows subsequently created
  \param[in] count number of images in \p icons. Set to 0 to remove
                   the current default icons

  \see Fl_Window::default_icon(const Fl_RGB_Image *)
  \see Fl_Window::icon(const Fl_RGB_Image *)
  \see Fl_Window::icons(const Fl_RGB_Image *[], int)
 */
void Fl_Window::default_icons(const Fl_RGB_Image *icons[], int count) {
  Fl_X::set_default_icons(icons, count);
}

/** Sets or resets a single window icon.

  A window icon \e can be changed while the window is shown, but this
  \e may be platform and/or window manager dependent. To be sure that
  the window displays the correct window icon you should always set the
  icon before the window is shown.

  If a window icon has not been set for a particular window, then the
  default window icon (see links below) or the system default icon will
  be used.

  \param[in] icon icon for this window, NULL to reset window icon.

  \see Fl_Window::default_icon(const Fl_RGB_Image *)
  \see Fl_Window::default_icons(const Fl_RGB_Image *[], int)
  \see Fl_Window::icons(const Fl_RGB_Image *[], int)
 */
void Fl_Window::icon(const Fl_RGB_Image *icon) {
  if (icon)
    icons(&icon, 1);
  else
    icons(&icon, 0);
}

/** Sets the window icons.

  You may set multiple window icons with different sizes. Dependent on
  the platform and system settings the best (or the first) icon will be
  chosen.

  The given images in \p icons are copied. You can use a local
  variable or free the images immediately after this call.

  If \p count is zero, current icons are removed. If \p count is greater than
  zero (must not be negative), then \p icons[] must contain at least \p count
  valid image pointers (not NULL). Otherwise the behavior is undefined.

  \param[in] icons icons for this window
  \param[in] count number of images in \p icons. Set to 0 to remove
                   the current icons

  \see Fl_Window::default_icon(const Fl_RGB_Image *)
  \see Fl_Window::default_icons(const Fl_RGB_Image *[], int)
  \see Fl_Window::icon(const Fl_RGB_Image *)
 */
void Fl_Window::icons(const Fl_RGB_Image *icons[], int count) {
  free_icons();

  if (count > 0) {
    icon_->icons = new Fl_RGB_Image*[count];
    icon_->count = count;
    // FIXME: Fl_RGB_Image lacks const modifiers on methods
    for (int i = 0;i < count;i++)
      icon_->icons[i] = (Fl_RGB_Image*)((Fl_RGB_Image*)icons[i])->copy();
  }

  if (i)
    i->set_icons();
}

/** Gets the current icon window target dependent data.
  \deprecated in 1.3.3
 */
const void *Fl_Window::icon() const {
  return icon_->legacy_icon;
}

/** Sets the current icon window target dependent data.
  \deprecated in 1.3.3
 */
void Fl_Window::icon(const void * ic) {
  free_icons();
  icon_->legacy_icon = ic;
}

/** Deletes all icons previously attached to the window.
 \see Fl_Window::icons(const Fl_RGB_Image *icons[], int count)
 */
void Fl_Window::free_icons() {
  int i;

  icon_->legacy_icon = 0L;

  if (icon_->icons) {
    for (i = 0;i < icon_->count;i++)
      delete icon_->icons[i];
    delete [] icon_->icons;
    icon_->icons = 0L;
  }

  icon_->count = 0;

#ifdef WIN32
  if (icon_->big_icon)
    DestroyIcon(icon_->big_icon);
  if (icon_->small_icon)
    DestroyIcon(icon_->small_icon);

  icon_->big_icon = NULL;
  icon_->small_icon = NULL;
#endif
}


#ifndef __APPLE__
/**
  Waits for the window to be displayed after calling show().

  Fl_Window::show() is not guaranteed to show and draw the window on
  all platforms immediately. Instead this is done in the background;
  particularly on X11 it will take a few messages (client server
  roundtrips) to display the window. Usually this small delay doesn't
  matter, but in some cases you may want to have the window instantiated
  and displayed synchronously.

  Currently (as of FLTK 1.3.4) this method has an effect on X11 and Mac OS.
  On Windows, show() is always synchronous. The effect of show() varies with
  versions of Mac OS X: early versions have the window appear on the screen
  when show() returns, later versions don't.
  If you want to write portable code and need this synchronous show() feature,
  add win->wait_for_expose() on all platforms, and FLTK will just do the
  right thing.

  This method can be used for displaying splash screens before
  calling Fl::run() or for having exact control over which window
  has the focus after calling show().

  If the window is not shown(), this method does nothing.

  \note Depending on the platform and window manager wait_for_expose()
    may not guarantee that the window is fully drawn when it is called.
    Under X11 it may only make sure that the window is \b mapped, i.e.
    the internal (OS dependent) window object was created (and maybe
    shown on the desktop as an empty frame or something like that).
    You may need to call Fl::flush() after wait_for_expose() to make
    sure the window and all its widgets are drawn and thus visible.

  \note FLTK does the best it can do to make sure that all widgets
    get drawn if you call wait_for_expose() and Fl::flush(). However,
    dependent on the window manager it can not be guaranteed that this
    does always happen synchronously. The only guaranteed behavior that
    all widgets are eventually drawn is if the FLTK event loop is run
    continuously, for instance with Fl::run().

  \see virtual void Fl_Window::show()

  Example code for displaying a window before calling Fl::run()

  \code
    Fl_Double_Window win = new Fl_Double_Window(...);

    // do more window initialization here ...

    win->show();                // show window
    win->wait_for_expose();     // wait, until displayed
    Fl::flush();                // make sure everything gets drawn

    // do more initialization work that needs some time here ...

    Fl::run();                  // start FLTK event loop
  \endcode

  Note that the window will not be responsive until the event loop
  is started with Fl::run().
*/

void Fl_Window::wait_for_expose() {
  if (!shown()) return;
  while (!i || i->wait_for_expose) {
    Fl::wait();
  }
}
#endif  // ! __APPLE__

//
// End of "$Id$".
//
