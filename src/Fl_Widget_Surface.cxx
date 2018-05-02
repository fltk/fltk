//
// "$Id$"
//
// Drivers code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

#include <FL/Fl_Widget_Surface.H>
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Window_Driver.H>
#include <FL/Fl_Screen_Driver.H>


/** The constructor.
 \param d can be nul.
 */
Fl_Widget_Surface::Fl_Widget_Surface(Fl_Graphics_Driver *d) : Fl_Surface_Device(d) {
  x_offset = 0;
  y_offset = 0;
}

/**
 @brief Draws the widget on the drawing surface.
 *
 The widget's position on the surface is determined by the last call to origin()
 and by the optional delta_x and delta_y arguments.
 Its dimensions are in points unless there was a previous call to scale().
 @param[in] widget Any FLTK widget (e.g., standard, custom, window).
 @param[in] delta_x Optional horizontal offset for positioning the widget relatively
 to the current origin of graphics functions.
 @param[in] delta_y Same as above, vertically.
 */
void Fl_Widget_Surface::draw(Fl_Widget* widget, int delta_x, int delta_y)
{
  int old_x, old_y, new_x, new_y, is_window;
  if ( ! widget->visible() ) return;
  is_window = (widget->as_window() != NULL);
  uchar old_damage = widget->damage();
  widget->damage(FL_DAMAGE_ALL);
  // set origin to the desired top-left position of the widget
  origin(&old_x, &old_y);
  new_x = old_x + delta_x;
  new_y = old_y + delta_y;
  if (!is_window) {
    new_x -= widget->x();
    new_y -= widget->y();
  }
  if (new_x != old_x || new_y != old_y) {
    translate(new_x - old_x, new_y - old_y );
  }
  // if widget is a window, clip all drawings to the window area
  if (is_window) {
    fl_push_clip(0, 0, widget->w(), widget->h());
  }
  // we do some trickery to recognize OpenGL windows and draw them via a plugin
  int drawn_by_plugin = 0;
  if (widget->as_gl_window()) {
    Fl_Plugin_Manager pm("fltk:device");
    Fl_Device_Plugin *pi = (Fl_Device_Plugin*)pm.plugin("opengl.device.fltk.org");
    if (pi) {
      drawn_by_plugin = pi->print(widget, 0, 0, 0);
    }
  }
  if (!drawn_by_plugin) {
    widget->draw();
    Fl_Overlay_Window *over = (is_window ? widget->as_window()->as_overlay_window() : NULL);
    if (over) over->draw_overlay();
  }
  if (is_window) fl_pop_clip();
  // find subwindows of widget and print them
  traverse(widget);
  // reset origin to where it was
  if (new_x != old_x || new_y != old_y) {
    untranslate();
  }
  if ((old_damage & FL_DAMAGE_CHILD) == 0) widget->clear_damage(old_damage);
  else widget->damage(FL_DAMAGE_ALL);
}


void Fl_Widget_Surface::traverse(Fl_Widget *widget)
{
  Fl_Group *g = widget->as_group();
  if (!g) return;
  int n = g->children();
  for (int i = 0; i < n; i++) {
    Fl_Widget *c = g->child(i);
    if ( !c->visible() ) continue;
    if ( c->as_window() ) {
      draw(c, c->x(), c->y());
    }
    else traverse(c);
  }
}

void Fl_Widget_Surface::translate(int x, int y)
{
}

/**
 @brief Computes the page coordinates of the current origin of graphics functions.
 *
 @param[out] x If non-null, *x is set to the horizontal page offset of graphics origin.
 @param[out] y Same as above, vertically.
 */
void Fl_Widget_Surface::origin(int *x, int *y)
{
  if (x) *x = x_offset;
  if (y) *y = y_offset;
}

/**
 @brief Sets the position in page coordinates of the origin of graphics functions.
 
 Arguments should be expressed relatively to the result of a previous printable_rect() call.
 That is, <tt>printable_rect(&w, &h); origin(w/2, 0);</tt> sets the graphics origin at the
 top center of the page printable area.
 Origin() calls are not affected by rotate() calls.
 Successive origin() calls don't combine their effects.
 @param[in] x Horizontal position in page coordinates of the desired origin of graphics functions.
 @param[in] y Same as above, vertically.
 */
void Fl_Widget_Surface::origin(int x, int y) {
  x_offset = x;
  y_offset = y;
}

/**
 @brief Prints a rectangular part of an on-screen window.
 
 @param win The window from where to capture.
 @param x The rectangle left
 @param y The rectangle top
 @param w The rectangle width
 @param h The rectangle height
 @param delta_x Optional horizontal offset from current graphics origin where to print the captured rectangle.
 @param delta_y As above, vertically.
 */
void Fl_Widget_Surface::print_window_part(Fl_Window *win, int x, int y, int w, int h, int delta_x, int delta_y)
{
  Fl_Surface_Device::push_current(Fl_Display_Device::display_device());
  Fl_Window *save_front = Fl::first_window();
  win->show();
  Fl::check();
  win->driver()->flush(); // makes the window current
  Fl_RGB_Image *img = Fl_Screen_Driver::traverse_to_gl_subwindows(win, x, y, w, h, NULL);
  img->scale(w, h, 1, 1);
  if (save_front != win) save_front->show();
  Fl_Surface_Device::pop_current();
  img->draw(delta_x, delta_y);
  delete img;
}

/**
 @brief Computes the width and height of the printable area of the page.
 
 Values are in the same unit as that used by FLTK drawing functions,
 are unchanged by calls to origin(), but are changed by scale() calls.
 Values account for the user-selected paper type and print orientation.
 @return 0 if OK, non-zero if any error
 */
int Fl_Widget_Surface::printable_rect(int *w, int *h) {return 1;}

/** Draws a window with its title bar and frame if any.
 
 \p x_offset and \p y_offset are optional coordinates of where to position the window top left.
 Equivalent to draw() if \p win is a subwindow or has no border.
 Use Fl_Window::decorated_w() and Fl_Window::decorated_h() to get the size of the window.
 */
void Fl_Widget_Surface::draw_decorated_window(Fl_Window *win, int x_offset, int y_offset)
{
  Fl_Shared_Image *top=0, *left=0, *bottom=0, *right=0;
  if (win->border() && !win->parent()) {
    win->driver()->capture_titlebar_and_borders(top, left, bottom, right);
  }
  int wsides = left ? left->w() : 0;
  int toph = top ? top->h() : 0;
  if (top) {
    top->draw(x_offset, y_offset);
    top->release();
  }
  if (left) {
    left->draw(x_offset, y_offset + toph);
    left->release();
  }
  if (right) {
    right->draw(x_offset + wsides + win->w(), y_offset + toph);
    right->release();
  }
  if (bottom) {
    bottom->draw(x_offset, y_offset + toph + win->h());
    bottom->release();
  }
  this->draw(win, x_offset + wsides, y_offset + toph);
}

//
// End of "$Id$".
//
