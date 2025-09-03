//
// Simple demo of drawing an "X" in Cairo (antialiased lines)
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

#include <FL/Fl.H>               // includes <FL/fl_config.h>

#ifdef FLTK_HAVE_CAIRO           // Builds example code only if cairo enabled
#include <FL/Fl_Cairo_Window.H>

// Cairo rendering cb called during Fl_Cairo_Window::draw()
static void my_cairo_draw_cb(Fl_Cairo_Window *window, cairo_t *cr) {
    const double xmax = (window->w() - 1);
    const double ymax = (window->h() - 1);

    // Set antialiasing mode. We could check the Cairo version at compile time but we'd
    // also have to check the runtime version which would make this demo too complicated.

    // CAIRO_ANTIALIAS_BEST    is available since Cairo version 1.12,
    // CAIRO_ANTIALIAS_DEFAULT is available since Cairo version 1.0.

    cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);             // use default antialiasing

    // Draw orange "X"
    //     Draws an X to four corners of resizable window.
    //     See Fl_Cairo_Window docs for more info.
    //
    cairo_set_line_width(cr, 1.00);                               // line width for drawing
    cairo_set_source_rgb(cr, 1.0, 0.5, 0.0);                      // orange
    cairo_move_to(cr, 0.0, 0.0);  cairo_line_to(cr, xmax, ymax);  // draw diagonal "\"
    cairo_move_to(cr, 0.0, ymax); cairo_line_to(cr, xmax, 0.0);   // draw diagonal "/"
    cairo_stroke(cr);                                             // stroke the lines
}

int main(int argc, char **argv) {
    Fl_Cairo_Window window(300, 300, "Cairo Draw 'X'");
    window.color(FL_BLACK);                                       // cairo window's default bg color
    window.size_range(50,50,-1,-1);                               // allow resize 50,50 and up
    window.resizable(&window);                                    // allow window to be resized
    window.set_draw_cb(my_cairo_draw_cb);                         // draw callback for cairo drawing
    window.show(argc, argv);
    return Fl::run();
}

// The code that follows just allows the example to build even if cairo wasn't enabled.
#else // (!FLTK_HAVE_CAIRO)
#include <FL/fl_ask.H>
int main(int argc, char **argv) {
    fl_message_title("This program needs a Cairo enabled FLTK library");
    fl_message("Please configure FLTK with Cairo enabled (--enable-cairo or --enable-cairoext)\n"
               "or one of the CMake options FLTK_OPTION_CAIRO_WINDOW or FLTK_OPTION_CAIRO_EXT, respectively.");
    return 0;
}
#endif // (FLTK_HAVE_CAIRO)
