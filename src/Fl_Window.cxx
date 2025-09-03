//
// Window widget class for the Fast Light Tool Kit (FLTK).
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

// The Fl_Window is a window in the fltk library.
// This is the system-independent portions.  The huge amount of
// crap you need to do to communicate with X is in Fl_x.cxx, the
// equivalent (but totally different) crap for Windows is in Fl_win32.cxx

#include <config.h>
#include <FL/Fl.H>
#include <FL/platform.H>
#include "Fl_Window_Driver.H"
#include "Fl_Screen_Driver.H"
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tooltip.H>
#include <FL/fl_draw.H>
#include <FL/fl_string_functions.h>
#include <stdlib.h>
#include "flstring.h"


char *Fl_Window::default_xclass_ = 0L;

char Fl_Window::show_next_window_iconic_ = 0;

Fl_Window *Fl_Window::current_;

void Fl_Window::_Fl_Window() {
  cursor_default = FL_CURSOR_DEFAULT;
  type(FL_WINDOW);
  box(FL_FLAT_BOX);
  if (Fl::scheme_bg_) {
    labeltype(FL_NORMAL_LABEL);
    align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    image(Fl::scheme_bg_);
  } else {
    labeltype(FL_NO_LABEL);
  }
  flx_ = 0;
  xclass_ = 0;
  iconlabel_ = 0;
  resizable(0);
  size_range_set_ = 0;
  minw_ = maxw_ = minh_ = maxh_ = 0;
  no_fullscreen_x = 0;
  no_fullscreen_y = 0;
  no_fullscreen_w = w();
  no_fullscreen_h = h();
  fullscreen_screen_top = -1;
  fullscreen_screen_bottom = -1;
  fullscreen_screen_left = -1;
  fullscreen_screen_right = -1;
  callback((Fl_Callback*)default_callback);
}

Fl_Window::Fl_Window(int X,int Y,int W, int H, const char *l) :
  Fl_Group(X, Y, W, H, l)
{
  pWindowDriver = Fl_Window_Driver::newWindowDriver(this);
  _Fl_Window();
  set_flag(FORCE_POSITION);
  if (!parent()) clear_visible();
}


Fl_Window::Fl_Window(int W, int H, const char *l) :
// fix common user error of a missing end() with current(0):
Fl_Group((Fl_Group::current(0),0), 0, W, H, l)
{
  pWindowDriver = Fl_Window_Driver::newWindowDriver(this);
  _Fl_Window();
  clear_visible();
}

Fl_Window::~Fl_Window() {
  hide();
  if (xclass_) {
    free(xclass_);
  }
  free_icons();
  delete pWindowDriver;
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
  while (w->parent()) { w = w->parent(); }              // walk up the widget hierarchy to top-level item
  return const_cast<Fl_Widget*>(w)->as_window();        // return if window, or NULL if not
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
    xoff += w->x();                     // accumulate offsets
    yoff += w->y();
    w = w->window();                    // walk up window hierarchy
  }
  return w ? const_cast<Fl_Widget*>(w)->as_window() : NULL;
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
  label(name, iconlabel());     // platform dependent
}

/** Sets the window titlebar label to a copy of a character string */
void Fl_Window::copy_label(const char *a) {
  Fl_Widget::copy_label(a);
  label(label(), iconlabel());  // platform dependent
}

void Fl_Window::iconlabel(const char *iname) {
  label(label(), iname);        // platform dependent
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
    default_xclass_ = fl_strdup(xc);
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
    xclass_ = fl_strdup(xc);
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
 \note See \ref osissues_wayland_window_icon for the Wayland platform.
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
 \note See \ref osissues_wayland_window_icon for the Wayland platform.
 */
void Fl_Window::default_icons(const Fl_RGB_Image *icons[], int count) {
  Fl::screen_driver()->open_display();
  Fl::screen_driver()->default_icons(icons, count);
}

/** Sets or resets a single window icon.

  A window icon \e can be changed while the window is shown, but this
  \e may be platform and/or window manager dependent. To be sure that
  the window displays the correct window icon you should always set the
  icon before the window is shown.

  If a window icon has not been set for a particular window, then the
  default window icon (see links below) or the system default icon will
  be used.

  This method makes an internal copy of the \p icon pixel buffer,
  so once set, the Fl_RGB_Image instance can be freed by the caller.

  \param[in] icon icon for this window, NULL to reset window icon.

  \see Fl_Window::default_icon(const Fl_RGB_Image *)
  \see Fl_Window::default_icons(const Fl_RGB_Image *[], int)
  \see Fl_Window::icons(const Fl_RGB_Image *[], int)
  \note See \ref osissues_wayland_window_icon for the Wayland platform.
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
  \note See \ref osissues_wayland_window_icon for the Wayland platform.
 */
void Fl_Window::icons(const Fl_RGB_Image *icons[], int count) {
  pWindowDriver->icons(icons, count);
}

/** Gets the current icon window target dependent data.
  \deprecated in 1.3.3
 */
const void *Fl_Window::icon() const {
  return pWindowDriver->icon();
}

/** Platform-specific method to set the window icon usable on Windows and X11 only.
 See \ref osissues_x_icon for its use under X11, and \ref osissues_icon_windows under Windows.
  \deprecated in 1.3.3 in favor of platform-independent methods Fl_Window::icon(const Fl_RGB_Image *icon)
 and Fl_Window::icons(const Fl_RGB_Image *icons[], int count).
 */
void Fl_Window::icon(const void * ic) {
  pWindowDriver->icon(ic);
}

/** Deletes all icons previously attached to the window.
 \see Fl_Window::icons(const Fl_RGB_Image *icons[], int count)
 */
void Fl_Window::free_icons() {
  pWindowDriver->free_icons();
}

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
  pWindowDriver->wait_for_expose();
}


int Fl_Window::decorated_w() const
{
  return pWindowDriver->decorated_w();
}


int Fl_Window::decorated_h() const
{
  return pWindowDriver->decorated_h();
}


void Fl_Window::flush()
{
  if (!shown()) return;
  make_current();
  fl_clip_region(flx_->region);
  flx_->region = 0;
  draw();
}


void Fl_Window::draw()
{
  Fl_Window *save_current = current_;
  bool to_display = Fl_Display_Device::display_device()->is_current();
  if (!to_display) current_ = this; // so drawing of background Fl_Tiled_Image is correct
  pWindowDriver->draw_begin();

  // The following is similar to Fl_Group::draw(), but ...
  //
  //  - draws the box at (0,0), i.e. with x=0 and y=0 instead of x() and y()
  //  - does NOT draw the label (text)
  //  - draws the image only if FL_ALIGN_INSIDE is set
  //
  // Note: The label (text) of top level windows is drawn in the title bar.
  //   Other windows do not draw their labels at all, unless drawn by their
  //   parent widgets or by special draw() methods (derived classes).

  if (damage() & ~FL_DAMAGE_CHILD) {     // draw the entire thing
    draw_box(box(),0,0,w(),h(),color()); // draw box with x/y = 0
    draw_backdrop();
  }
  draw_children();

  pWindowDriver->draw_end();
  if (!to_display) current_ = save_current;
}

/**
 Draw the background image if one is set and is aligned inside.
 */
void Fl_Window::draw_backdrop() {
  if (image() && (align() & FL_ALIGN_INSIDE)) { // draw the image only
    Fl_Label l1;
    memset(&l1,0,sizeof(l1));
    l1.align_ = align();
    l1.image = image();
    if (!active_r() && l1.image && l1.deimage) l1.image = l1.deimage;
    l1.type = labeltype();
    l1.h_margin_ = l1.v_margin_ = l1.spacing = 0;
    l1.draw(0,0,w(),h(),align());
  }
}

void Fl_Window::make_current()
{
  pWindowDriver->make_current();
  current_ = this;
}

void Fl_Window::label(const char *name, const char *mininame) {
  Fl_Widget::label(name);
  iconlabel_ = mininame;
  pWindowDriver->label(name, mininame);
}

void Fl_Window::show() {
  image(Fl::scheme_bg_);
  if (Fl::scheme_bg_) {
    labeltype(FL_NORMAL_LABEL);
    align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
  } else {
    labeltype(FL_NO_LABEL);
  }
  Fl_Tooltip::exit(this);
  if (!shown())
    default_size_range();
  pWindowDriver->show();
}

void Fl_Window::resize(int X,int Y,int W,int H) {
  pWindowDriver->resize(X, Y, W, H);
}

void Fl_Window::hide() {
  pWindowDriver->hide();
}


// FL_SHOW and FL_HIDE are called whenever the visibility of this widget
// or any parent changes.  We must correctly map/unmap the system's window.

// For top-level windows it is assumed the window has already been
// mapped or unmapped!!!  This is because this should only happen when
// Fl_Window::show() or Fl_Window::hide() is called, or in response to
// iconize/deiconize events from the system.
int Fl_Window::handle(int ev)
{
  if (parent()) {
    switch (ev) {
      case FL_SHOW:
        if (!shown()) show();
        else {
          pWindowDriver->map();
        }
        break;
      case FL_HIDE:
        if (shown()) {
          // Find what really turned invisible, if it was a parent window
          // we do nothing.  We need to avoid unnecessary unmap calls
          // because they cause the display to blink when the parent is
          // remapped.  However if this or any intermediate non-window
          // widget has really had hide() called directly on it, we must
          // unmap because when the parent window is remapped we don't
          // want to reappear.
          if (visible()) {
            Fl_Widget* p = parent();
            for (; p && p->visible(); p = p->parent()) { /* empty*/ }
            if (p && p->as_window()) break; // don't do the unmap
          }
          pWindowDriver->unmap();
        }
        break;
    }
  }

  return Fl_Group::handle(ev);
}

/**
  Sets the allowable range to which the user can resize this window.

  We recommend to call size_range() if you have a resizable() widget
  in a main window, and to call it after setting the resizable() and
  before show()'ing the window for best cross platform compatibility.

  If this function is \b not called, FLTK tries to figure out the range
  when the window is shown. Please see the protected method
  default_size_range() for details.

  It is undefined what happens if the current window size does not fit
  in the constraints passed to size_range().

  \note
  This only works for top-level windows and the exact behavior can be
  platform specific. To work correctly across all platforms size_range()
  must be called after setting the resizable() widget of the window and
  before the window is show()'n.

  Calling size_range() after the window has been shown may work on some
  but not all platforms. If you need to change the size_range() after
  the window has been shown, then you should consider to hide() and
  show() the window again, i.e. call hide(), size_range(), and show()
  in this order.

  Typical usage: call
  \code
      size_range(minWidth, minHeight);
  \endcode
  after setting the resizable widget and before calling show().
  This ensures that the window cannot be resized smaller than the given
  values by user interaction.

  \c maxWidth and \c maxHeight might be useful in some special cases but
  less frequently used.

  The other optional parameters \c deltaX, \c deltaY, and \c aspectRatio
  are not recommended because they may not work on all platforms and may
  even under X11 not be supported by all Window Managers.

  \param[in] minWidth,minHeight The smallest the window can be.
    Either value must be greater than 0.

  \param[in] maxWidth,maxHeight The largest the window can be. If either
    is equal to the minimum then you cannot resize in that direction.
    If either is zero then FLTK picks a maximum size in that direction
    such that the window will fill the screen.

  \param[in] deltaX,deltaY These are size increments. The window will be
    constrained to widths of <tt>minWidth + N * deltaX</tt>, where N is any
    non-negative integer. If these are less or equal to 1 they are ignored
    (this is always ignored on Windows).

  \param[in] aspectRatio A flag that indicates that the window should preserve
    its aspect ratio. This only works if both the maximum and minimum have
    the same aspect ratio (ignored on Windows and by many X window managers).
*/
void Fl_Window::size_range(int minWidth, int minHeight,
                           int maxWidth, int maxHeight,
                           int deltaX, int deltaY, int aspectRatio) {
  minw_           = minWidth;
  minh_           = minHeight;
  maxw_           = maxWidth;
  maxh_           = maxHeight;
  dw_             = deltaX;
  dh_             = deltaY;
  aspect_         = aspectRatio;
  size_range_set_ = 1;
  pWindowDriver->size_range();  // platform specific stuff
}

/**
  Gets the allowable range to which the user can resize this window.

  \param[out] minWidth, minHeight, maxWidth, maxHeight, deltaX, deltaY, aspectRatio
    are all pointers to integers that will receive the current respective value
    during the call. Every pointer can be NULL if that value is not needed.

  \retval 0 if size range not set
  \retval 1 if the size range was explicitly set by a call to Fl_Window::size_range()
            or has been calculated
  \see Fl_Window::size_range(int minWidth, int minHeight, int maxWidth, int maxHeight, int deltaX, int deltaY, int aspectRatio)

  \since 1.4.0
*/
uchar Fl_Window::get_size_range(int *minWidth, int *minHeight,
                                int *maxWidth, int *maxHeight,
                                int *deltaX, int *deltaY, int *aspectRatio) {
  if (minWidth) *minWidth = minw_;
  if (minHeight) *minHeight = minh_;
  if (maxWidth) *maxWidth = maxw_;
  if (maxHeight) *maxHeight = maxh_;
  if (deltaX) *deltaX = dw_;
  if (deltaY) *deltaY = dh_;
  if (aspectRatio) *aspectRatio = aspect_;
  return size_range_set_;
}

/**
  Protected method to calculate the default size range of a window.

  This method is called internally prior to showing a window to ensure that
  the window's size range values are calculated if a resizable() widget has
  been set but size_range() has not been called explicitly.

  This method does nothing if size_range() has been called before.

  Otherwise FLTK tries to figure out the window's size range from the
  setting of the window's resizable() widget as follows and roughly in
  the given order.

  -# If resizable() is NULL (this is the default) then the window cannot
    be resized and the resize border and max-size control will not be
    displayed for the window.

  -# If either dimension of resizable() is zero, then the window cannot
    resize in that direction.

  -# The resizable() widget is clipped to the window area.

  -# The non-resizable portion of the window is calculated as the difference
    of the window's size and the clipped resizable() widget's size.

  -# If either dimension of the clipped resizable() widget is greater
    than 100, then 100 is considered its minimum width/height. This
    allows the resizable widget to shrink below its original size.

  -# Finally the minimum width/height of the window is set to the
    non-resizable portion plus the width/height of the resizable()
    widget as calculated above.

  In simple words:
    - It is assumed that the resizable() widget can be indefinitely
      enlarged and/or shrunk to a minimum width/height of 100 unless
      it is smaller than that, which is then considered the minimum.
    - The window's size_range() minimum values are set to the sum
      of the non-resizable portion of the window and the previously
      calculated minimum size of the resizable() widget.

  Examples:
  \code
    Fl_Window win(400, 400);
    win.resizable(win);
    // win.size_range(100, 100, 0, 0);
  \endcode

  The minimum size of the resizable is 100, hence the minimum size
  of the total window is also 100 in both directions.

  \code
    Fl_Window win(400, 400);
    Fl_Box box(20, 20, 360, 360);
    win.resizable(box);
    // win.size_range(140, 140, 0, 0);
  \endcode

  The calculated minimum width and height would be 20 + 100 + 20 in both
  dimensions.

  \code
    Fl_Window win(400, 400);
    Fl_Box box(200, 0, 500, 300); // note: width 500 too large: clipped
    win.resizable(box);
    // win.size_range(300, 200, 0, 0);
  \endcode

  The width of the resizable is clipped to 200, hence the minimum size of
  the total window is also 200 (fix) + 100 (min. resizable) in x direction.
  The minimum value in y direction is 100 (resizable) + 100 (fixed part).

  The calculation is based on clipping the resizable widget to the window
  area to prevent programming errors and the assumption that the resizable
  widget can be shrunk to 100x100 or its original size, whichever is smaller.

  If this is not what you want, please use Fl_Window::size_range()
  explicitly so you can set any appropriate range.

  \since 1.4.0
*/
void Fl_Window::default_size_range() {

  if (size_range_set_)
    return;
  if (!resizable()) {
    size_range(w(), h(), w(), h());
    return;
  }

  // Calculate default size range depending on the resizable() widget

  Fl_Widget *r = resizable();

  int maxw = 0;
  int maxh = 0;

  // Clip the resizable() widget to the window

  int L = (r == this ? 0 : r->x());
  int R = L + r->w();
  if (R < 0 || L > w()) R = L; // outside the window
  else {
    if (L < 0)   L = 0;
    if (R > w()) R = w();
  }
  int rw = R - L;

  int T = (r == this ? 0 : r->y());
  int B = T + r->h();
  if (B < 0 || T > h()) B = T; // outside the window
  else {
    if (T < 0) T = 0;
    if (B > h()) B = h();
  }
  int rh = B - T;

  // Calculate the non-resizable part of the window (STR 3352)
  // before reducing the size of the resizable widget !
  int minw = w() - rw;
  int minh = h() - rh;

  // Limit the resizable dimensions to 100x100 according to the docs.
  // This makes the resizable widget shrinkable, otherwise it would
  // only be able to grow (issue #392)
  if (rw > 100) rw = 100;
  if (rh > 100) rh = 100;

  // Add the clipped resizable() width/height so we have at least
  // the non-resizable part + the clipped resizable() size
  minw += rw;
  minh += rh;

  // Disable resizing in the respective directions if any dimension
  // of the resizable widget is zero (see docs)
  if (r->w() == 0) minw = maxw = w();
  if (r->h() == 0) minh = maxh = h();

  // Finally set the size range
  size_range(minw, minh, maxw, maxh);
}

/**
  Protected method to determine whether a window is resizable.

  If size_range() has not yet been called this method calculates the
  default size range values by calling default_size_range().

  This method is for internal use only. The returned value is a bit mask
  and non-zero if the window is resizable in at least one direction.

  \return   non-zero if the window is resizable

  \retval  0  the window is not resizable
  \retval  1  the window is resizable in horizontal direction (w)
  \retval  2  the window is resizable in vertical direction (h)
  \retval  3  the window is resizable in both directions (w and h)

  \see default_size_range()

  \since 1.4.0
*/
int Fl_Window::is_resizable() {
  default_size_range();
  int ret = 0;
  if (minw_ != maxw_) ret |= 1;
  if (minh_ != maxh_) ret |= 2;
  return ret;
}

/** The number of the screen containing the mapped window.

  This method returns the screen number (0 .. n) of the window
  if it is shown, otherwise the result value is undefined.

  \note The return value is undefined before the window is shown and
    if the window has been hidden after it was previously shown.

  To position a window on a particular screen, use
  Fl_Window::screen_num(int) \b before you show() it.

  \return screen number of the window if it is shown

  \see Fl_Window::screen_num(int)

  \since 1.4.0
 */
int Fl_Window::screen_num() {
  return pWindowDriver->screen_num();
}

/** Set the number of the screen where to map the window.

  Call this and set also the window's desired x/y position before show()'ing
  the window. This can be necessary if a system has several screens with
  distinct scaling factors because the window's x() and y() may not suffice
  to uniquely identify one screen.

  Consider a system with two screens where the left screen is A pixels wide and
  has a scaling factor of 1 whereas the right screen has a scaling factor of 2.
  For the sake of simplicity, consider only the X coordinate of the window.
  FLTK coordinates translate directly to pixel coordinates on the left screen,
  whereas FLTK coordinates multiplied by 2 correspond to pixel coordinates
  on the right screen. Consequently, FLTK coordinates between A/2 + 1 and A-1
  can map to both screens. Therefore both the window coordinates and the screen
  number are necessary to uniquely identify where a window is to be mapped.

  The valid range of \p screen_num is 0 .. Fl::screen_count() - 1.

  This method does nothing if
  - the window is already shown() or
  - the given screen number is out of range.

  \param[in] screen_num screen number where the window is to be mapped

  \see Fl_Window::screen_num()
  \see Fl::screen_count()

  \since 1.4.0
*/
void Fl_Window::screen_num(int screen_num) {
  if (!shown() && screen_num >= 0 && screen_num < Fl::screen_count())
    pWindowDriver->screen_num(screen_num);
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
 The shared image should not have been scaled through Fl_Image::scale().

 Platform details:
 \li On the unix/linux platform, the SHAPE extension of the X server is required.
 This function does control the shape of Fl_Gl_Window instances.
 \li On the Windows platform, this function does nothing with class Fl_Gl_Window.
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

/** Returns the image controlling the window shape or NULL.

  \since 1.4.0
*/
const Fl_Image* Fl_Window::shape() {return pWindowDriver->shape();}

/** Returns true when a window is being rescaled.

  \since 1.4.0
*/
bool Fl_Window::is_a_rescale() {return Fl_Window_Driver::is_a_rescale_;}

/** Returns a platform-specific identification of a shown window, or 0 if not shown.

  \note This identification may differ from the platform-specific reference
        of an Fl_Window object used by functions fl_x11_xid(), fl_mac_xid(),
        fl_x11_find(), and fl_mac_find().

  - X11 platform: the window's XID.
  - macOS platform: The window number of the window’s window device.
  - other platforms: 0.

  \since 1.4.0
*/
fl_uintptr_t Fl_Window::os_id() { return pWindowDriver->os_id();}

/**
  Maximizes a top-level window to its current screen.

  This function is effective only with a show()'n, resizable, top-level window.
  Bordered and borderless windows can be used.
  Fullscreen windows can't be used.

  \see Fl_Window::un_maximize(), Fl_Window::maximize_active()

  \since 1.4.0
*/
void Fl_Window::maximize() {
  if (!shown() || parent() || !is_resizable() || maximize_active() || fullscreen_active())
    return;
  set_flag(MAXIMIZED);
  pWindowDriver->maximize();
}

/**
  Returns a previously maximized top-level window to its previous size.
  \see Fl_Window::maximize()

  \since 1.4.0
*/
void Fl_Window::un_maximize() {
  if (!shown() || parent() || !is_resizable() || !maximize_active() || fullscreen_active()) return;
  clear_flag(MAXIMIZED);
  pWindowDriver->un_maximize();
}

void Fl_Window::is_maximized_(bool b) {
  if (b) set_flag(MAXIMIZED);
  else clear_flag(MAXIMIZED);
}

/** Allow this subwindow to expand outside the area of its parent window.

  This is presently implemented only for the Wayland platform to help
  support window docking.

  \since 1.4.0
*/
void Fl_Window::allow_expand_outside_parent() {
  if (parent()) pWindowDriver->allow_expand_outside_parent();
}
