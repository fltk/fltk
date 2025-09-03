//
// Cairo drawing test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

#include <FL/Fl.H>      // includes <FL/fl_config.h>

#ifdef FLTK_HAVE_CAIRO  // defined in <FL/fl_config.h> since FLTK 1.4.0

#include <FL/Fl_Cairo_Window.H>
#include <FL/Fl_Box.H>
#include <FL/platform.H>
#include <FL/fl_draw.H>
#include <FL/math.h>

#define DEF_WIDTH 0.03

// This demo program can be used in 3 modes. All 3 modes require configure
// option --enable-cairo or CMake FLTK_OPTION_CAIRO_WINDOW.
//
// 1) using class Fl_Cairo_Window useful when all the content of a window
//    is drawn with Cairo.
//    This is achieved setting #define USE_FL_CAIRO_WINDOW 1 below
// or
// 2) showing how to draw in an Fl_Double_Window using both Cairo and
//    the FLTK drawing API.
//    This is achieved setting #define USE_FL_CAIRO_WINDOW 0 below
// or
// 3) showing how to use "cairo extended use".
//    This is achieved when FLTK was built with one more option
//    (configure --enable-cairoext or CMake FLTK_OPTION_CAIRO_EXT)
//    which defines the preprocessor variable FLTK_HAVE_CAIROEXT.
//    If Fl::cairo_autolink_context(true); is called at the beginning
//    of main(), any overridden draw() function gets access to an adequate
//    Cairo context with Fl::cairo_cc() without having to call
//    Fl::cairo_make_current(Fl_Window*).


#define USE_FL_CAIRO_WINDOW 1

// draw centered text

static void centered_text(cairo_t *cr, double x0, double y0, double w0, double h0, const char *my_text) {
  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_OBLIQUE, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_source_rgba(cr, 0.9, 0.9, 0.4, 0.6);
  cairo_text_extents_t extents;
  cairo_text_extents(cr, my_text, &extents);
  double x = (extents.width / 2 + extents.x_bearing);
  double y = (extents.height / 2 + extents.y_bearing);
  cairo_move_to(cr, x0 + w0 / 2 - x, y0 + h0 / 2 - y);
  cairo_text_path(cr, my_text);
  cairo_fill_preserve(cr);
  cairo_set_source_rgba(cr, 0, 0, 0, 1);
  cairo_set_line_width(cr, 0.004);
  cairo_stroke(cr);
  cairo_set_line_width(cr, DEF_WIDTH);
}

// draw a button object with rounded corners and a label

static void round_button(cairo_t *cr, double x0, double y0,
                         double rect_width, double rect_height, double radius,
                         double r, double g, double b) {
  double x1, y1;
  x1 = x0 + rect_width;
  y1 = y0 + rect_height;
  if (!rect_width || !rect_height)
    return;
  if (rect_width / 2 < radius) {
    if (rect_height / 2 < radius) {
      cairo_move_to(cr, x0, (y0 + y1) / 2);
      cairo_curve_to(cr, x0, y0, x0, y0, (x0 + x1) / 2, y0);
      cairo_curve_to(cr, x1, y0, x1, y0, x1, (y0 + y1) / 2);
      cairo_curve_to(cr, x1, y1, x1, y1, (x1 + x0) / 2, y1);
      cairo_curve_to(cr, x0, y1, x0, y1, x0, (y0 + y1) / 2);
    } else {
      cairo_move_to(cr, x0, y0 + radius);
      cairo_curve_to(cr, x0, y0, x0, y0, (x0 + x1) / 2, y0);
      cairo_curve_to(cr, x1, y0, x1, y0, x1, y0 + radius);
      cairo_line_to(cr, x1, y1 - radius);
      cairo_curve_to(cr, x1, y1, x1, y1, (x1 + x0) / 2, y1);
      cairo_curve_to(cr, x0, y1, x0, y1, x0, y1 - radius);
    }
  } else {
    if (rect_height / 2 < radius) {
      cairo_move_to(cr, x0, (y0 + y1) / 2);
      cairo_curve_to(cr, x0, y0, x0, y0, x0 + radius, y0);
      cairo_line_to(cr, x1 - radius, y0);
      cairo_curve_to(cr, x1, y0, x1, y0, x1, (y0 + y1) / 2);
      cairo_curve_to(cr, x1, y1, x1, y1, x1 - radius, y1);
      cairo_line_to(cr, x0 + radius, y1);
      cairo_curve_to(cr, x0, y1, x0, y1, x0, (y0 + y1) / 2);
    } else {
      cairo_move_to(cr, x0, y0 + radius);
      cairo_curve_to(cr, x0, y0, x0, y0, x0 + radius, y0);
      cairo_line_to(cr, x1 - radius, y0);
      cairo_curve_to(cr, x1, y0, x1, y0, x1, y0 + radius);
      cairo_line_to(cr, x1, y1 - radius);
      cairo_curve_to(cr, x1, y1, x1, y1, x1 - radius, y1);
      cairo_line_to(cr, x0 + radius, y1);
      cairo_curve_to(cr, x0, y1, x0, y1, x0, y1 - radius);
    }
  }
  cairo_close_path(cr);

  cairo_pattern_t *pat =
    // cairo_pattern_create_linear (0.0, 0.0,  0.0, 1.0);
    cairo_pattern_create_radial(0.25, 0.24, 0.11, 0.24, 0.14, 0.35);
  cairo_pattern_set_extend(pat, CAIRO_EXTEND_REFLECT);

  cairo_pattern_add_color_stop_rgba(pat, 1.0, r, g, b, 1);
  cairo_pattern_add_color_stop_rgba(pat, 0.0, 1, 1, 1, 1);
  cairo_set_source(cr, pat);
  cairo_fill_preserve(cr);
  cairo_pattern_destroy(pat);

  // cairo_set_source_rgb (cr, 0.5, 0.5, 1); cairo_fill_preserve (cr);
  cairo_set_source_rgba(cr, 0, 0, 0.5, 0.3);
  cairo_stroke(cr);

  cairo_set_font_size(cr, 0.075);
  centered_text(cr, x0, y0, rect_width, rect_height, "FLTK loves Cairo!");
}

// draw the entire image (3 buttons), scaled to the given width and height

void draw_image(cairo_t *cr, int w, int h) {

  cairo_set_line_width(cr, DEF_WIDTH);
  cairo_scale(cr, w, h);

  round_button(cr, 0.1, 0.1, 0.8, 0.2, 0.4, 1, 0, 0);
  round_button(cr, 0.1, 0.4, 0.8, 0.2, 0.4, 0, 1, 0);
  round_button(cr, 0.1, 0.7, 0.8, 0.2, 0.4, 0, 0, 1);

} // draw_image()


#if USE_FL_CAIRO_WINDOW && !defined(FLTK_HAVE_CAIROEXT)

typedef Fl_Cairo_Window cairo_using_window;

#else // !USE_FL_CAIRO_WINDOW || defined(FLTK_HAVE_CAIROEXT)

class cairo_using_window : public Fl_Double_Window {
  void (*draw_with_cairo_)(cairo_using_window*, cairo_t*);

public:

  cairo_using_window(int w, int h, const char *title) : Fl_Double_Window(w, h, title) {
    Fl_Box *box = new Fl_Box(FL_NO_BOX, 0, 0, w, 25,
                             "Cairo and FLTK API in Fl_Double_Window");
    box->labelfont(FL_TIMES_BOLD);
    box->labelsize(12);
    box->labelcolor(FL_BLUE);
  }
  void draw() FL_OVERRIDE {
    Fl_Window::draw(); // perform drawings with the FLTK API
#ifndef FLTK_HAVE_CAIROEXT
    Fl::cairo_make_current(this); // announce Cairo will be used in this window
#endif
    cairo_t *cc = Fl::cairo_cc(); // get the adequate Cairo context
    draw_with_cairo_(this, cc);   // draw in this window using Cairo

    // flush Cairo drawings: necessary at least for Windows
    Fl::cairo_flush(cc);
  }

  void set_draw_cb( void (*cb)(cairo_using_window*, cairo_t*)) {
    draw_with_cairo_ = cb;
  }
};

#endif // USE_FL_CAIRO_WINDOW

// Cairo rendering cb called during Fl_Cairo_Window::draw()
// or cairo_using_window::draw().
static void my_cairo_draw_cb(cairo_using_window *window, cairo_t *cr) {
  draw_image(cr, window->w(), window->h());
}

int main(int argc, char **argv) {
#ifdef FLTK_HAVE_CAIROEXT
  Fl::cairo_autolink_context(true);
#endif
  cairo_using_window window(350, 350, "FLTK loves Cairo");

  window.resizable(&window);
  window.color(FL_WHITE);
  window.set_draw_cb(my_cairo_draw_cb);
  window.show(argc, argv);

  return Fl::run();
}

#else // (!FLTK_HAVE_CAIRO)

#include <FL/fl_ask.H>

int main(int argc, char **argv) {
  fl_message_title("This program needs a Cairo enabled FLTK library");
  fl_message(
    "Please configure FLTK with Cairo enabled (--enable-cairo or --enable-cairoext)\n"
    "or one of the CMake options FLTK_OPTION_CAIRO_WINDOW or FLTK_OPTION_CAIRO_EXT, respectively.");
  return 0;
}
#endif // (FLTK_HAVE_CAIRO)

