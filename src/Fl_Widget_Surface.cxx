//
// Drivers code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

#include <FL/Fl_Widget_Surface.H>
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <FL/platform.H>
#include "Fl_Window_Driver.H"
#include "Fl_Screen_Driver.H"


/** The constructor.
 \param d can be nul.
 */
Fl_Widget_Surface::Fl_Widget_Surface(Fl_Graphics_Driver *d) : Fl_Surface_Device(d) {
  x_offset = 0;
  y_offset = 0;
}

/**
 \brief Draws the widget on the drawing surface.

 The widget's position on the surface is determined by the last call to origin()
 and by the optional delta_x and delta_y arguments.
 Its dimensions are in points unless there was a previous call to scale().
 \param[in] widget Any FLTK widget (e.g., standard, custom, window).
 \param[in] delta_x,delta_y Optional horizontal and vertical offsets for positioning the widget top left relatively
 to the current origin of graphics.
 */
void Fl_Widget_Surface::draw(Fl_Widget* widget, int delta_x, int delta_y)
{
  int old_x, old_y, new_x, new_y, is_window;
  if ( ! widget->visible() ) return;
  bool need_push = !is_current();
  if (need_push) Fl_Surface_Device::push_current(this);
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
    Fl_Device_Plugin *plugin = Fl_Device_Plugin::opengl_plugin();
    if (plugin) {
      drawn_by_plugin = plugin->print(widget);
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
  if (need_push) Fl_Surface_Device::pop_current();
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

/**
 Translates the current graphics origin accounting for the current rotation.

 Each translate() call must be matched by an untranslate() call.
 Successive translate() calls add up their effects.
 */
void Fl_Widget_Surface::translate(int x, int y)
{
}

/**
 Undoes the effect of a previous translate() call.
 */
void Fl_Widget_Surface::untranslate()
{
}

/**
 \brief Computes the coordinates of the current origin of graphics functions.

 \param[out] x,y If non-null, *x and *y are set to the horizontal and vertical coordinates of the graphics origin.
 */
void Fl_Widget_Surface::origin(int *x, int *y)
{
  if (x) *x = x_offset;
  if (y) *y = y_offset;
}

/**
 \brief Sets the position of the origin of graphics in the drawable part of the drawing surface.

 Arguments should be expressed relatively to the result of a previous printable_rect() call.
 That is, <tt>printable_rect(&w, &h); origin(w/2, 0);</tt> sets the graphics origin at the
 top center of the drawable area. Successive origin() calls don't combine their effects.
 Origin() calls are not affected by rotate() calls (for classes derived from Fl_Paged_Device).
 \param[in] x,y Horizontal and vertical positions in the drawing surface of the desired origin of graphics.
 */
void Fl_Widget_Surface::origin(int x, int y) {
  x_offset = x;
  y_offset = y;
}

/**
 Draws a rectangular part of an on-screen window.

 \param win The window from where to capture. Can be an Fl_Gl_Window. Sub-windows that intersect the rectangle are also captured.
 \param x The rectangle left
 \param y The rectangle top
 \param w The rectangle width
 \param h The rectangle height
 \param delta_x,delta_y Optional horizontal and vertical offsets from current graphics origin where to draw the top left of the captured rectangle.
 */
void Fl_Widget_Surface::print_window_part(Fl_Window *win, int x, int y, int w, int h, int delta_x, int delta_y)
{
  if (!win->shown()) return;
  bool need_push = !Fl_Display_Device::display_device()->is_current();
  if (need_push) Fl_Surface_Device::push_current(Fl_Display_Device::display_device());
  Fl_Window *save_front = Fl::first_window();
  win->show();
  Fl::check();
  Fl_Window_Driver::driver(win)->flush(); // makes the window current
  Fl_RGB_Image *img = Fl_Screen_Driver::traverse_to_gl_subwindows(win, x, y, w, h, NULL);
  if (img) img->scale(w, h, 1, 1);
  if (save_front != win) save_front->show();
  if (need_push) Fl_Surface_Device::pop_current();
  if (img) {
    need_push = !is_current();
    if (need_push) Fl_Surface_Device::push_current(this);
    img->draw(delta_x, delta_y);
    if (need_push) Fl_Surface_Device::pop_current();
    delete img;
  }
}

/**
 Computes the width and height of the drawable area of the drawing surface.

 Values are in the same unit as that used by FLTK drawing functions and are unchanged by calls to origin().
 If the object is derived from class Fl_Paged_Device, values account for the user-selected paper type and print orientation
 and are changed by scale() calls.
 \return 0 if OK, non-zero if any error
 */
int Fl_Widget_Surface::printable_rect(int *w, int *h) {return 1;}

/** Draws a window with its title bar and frame if any.

 \p win_offset_x and \p win_offset_y are optional coordinates of where to position the window top left.
 Equivalent to draw() if \p win is a subwindow or has no border.
 Use Fl_Window::decorated_w() and Fl_Window::decorated_h() to get the size of the framed window.
 */
void Fl_Widget_Surface::draw_decorated_window(Fl_Window *win, int win_offset_x, int win_offset_y)
{
  Fl_RGB_Image *top=0, *left=0, *bottom=0, *right=0;
  if (win->shown() && win->border() && !win->parent()) {
    Fl_Window_Driver::driver(win)->capture_titlebar_and_borders(top, left, bottom, right);
  }
  bool need_push = !is_current();
  if (need_push) Fl_Surface_Device::push_current(this);
  int wsides = left ? left->w() : 0;
  int toph = top ? top->h() : 0;
  if (top) {
    top->draw(win_offset_x, win_offset_y);
    delete top;
  }
  if (left) {
    left->draw(win_offset_x, win_offset_y + toph);
    delete left;
  }
  if (right) {
    right->draw(win_offset_x + wsides + win->w(), win_offset_y + toph);
    delete right;
  }
  if (bottom) {
    bottom->draw(win_offset_x, win_offset_y + toph + win->h());
    delete bottom;
  }
  this->draw(win, win_offset_x + wsides, win_offset_y + toph);
  if (need_push) Fl_Surface_Device::pop_current();
}
