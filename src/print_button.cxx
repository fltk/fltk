//
// "Print Window" functions for the Fast Light Tool Kit (FLTK).
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

// Notes: This function must be activated by defining the preprocessor macro
// USE_PRINT_BUTTON on the commandline.
//
// Although this function is compiled on all platforms it is only used by
// platform specific code of fl_open_display() on Linux (X11) and Windows.
// macOS uses the "Print front window" menu in the application menu.
//
// The environment variable FLTK_PRINT_BUTTON can be defined to control the
// creation of the sometimes annoying print button feature at runtime if it
// has been compiled in (see above):
//
//  - FLTK_PRINT_BUTTON undefined: like 1 below (default)
//
//  - FLTK_PRINT_BUTTON = 0 : print front window disabled for this program
//  - FLTK_PRINT_BUTTON = 1 : window shown at startup
//  - FLTK_PRINT_BUTTON = 2 : window not shown, shortcut available to show
//  - FLTK_PRINT_BUTTON = 3 : window shown at startup, shortcut available
//
// If options 2 or 3 are used the shortcut can be used to show the window
// if it was not shown (2) or accidentally closed (3).
//
// Currently the shortcut can't be configured and is always ALT+SHIFT+'s'.
// Todo: make the shortcut configurable.

#include "print_button.h"

#include <FL/Fl_Printer.H>
#include <FL/Fl_PostScript.H>
#include <FL/Fl_Copy_Surface.H>

#ifdef USE_PRINT_BUTTON

#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>

#include <stdlib.h>
#include <stdio.h>

// define the optional rotation of print output in degrees to test it
// #define ROTATE 20.0

// Global variables to simplify and clarify the code:

static Fl_Window *print_window = 0;       // "print front window" dialog window
static Fl_Check_Button *deco_button = 0;  // window decoration button

// The button callback does the job for both printing and copying to the
// clipboard. The callback is called with 'mode' == (int)(data).
//  1: print window
//  2: copy window to clipboard
// else: see 2.

static void output_cb(Fl_Widget * /*unused*/, void *data) {

  print_window->hide();
  Fl_Window *win = Fl::first_window();

  // if no (other) window exists we return silently w/o showing the
  // print window again (which ends the program)

  if (!win) return;
  fl_print_or_copy_window(win, deco_button->value(), fl_int(data));
  print_window->show();
}

// Global event handler for screenshot (ctrl/alt/command + s)
// This pops up the "Print front window" dialog if it had been closed

static int shortcut_handler(int event) { // global shortcut handler

  // required key and keyboard states for shortcut
  // (should be configurable)

  const int key = 's';                 // global shortcut key
  const int state = FL_ALT | FL_SHIFT; //  | FL_CTRL | FL_COMMAND;

  if (print_window &&
      (event == FL_SHORTCUT || event == FL_KEYBOARD) &&
      ((Fl::event_state() & state) == state) &&
      (Fl::event_key() == key)) {
    print_window->show();
    return 1;
  }
  return 0;
}

// create and initialize the "Print/copy front window" dialog window

int fl_create_print_window() {
  static int first = 1;
  if (!first)
    return 0;
  first = 0;
  int val = 1;
  const char *print_button = fl_getenv("FLTK_PRINT_BUTTON");
  if (print_button)
    val = atoi(print_button);
  if (val) {
    // prevent becoming a subwindow
    Fl_Group *cg = Fl_Group::current();
    Fl_Group::current(0);
    print_window  =     new Fl_Window( 0,  0, 200, 110, "FLTK screenshot");
    Fl_Button *bp =     new Fl_Button(10, 10, 180,  30, "Print front window");
    Fl_Button *bc =     new Fl_Button(10, 40, 180,  30, "Copy front window");
    deco_button = new Fl_Check_Button(10, 70, 180,  30, "Window decoration");
    bp->callback(output_cb, (void *)1);
    bc->callback(output_cb, (void *)2);
    print_window->end();
    if (val & 1)
      print_window->show();
    // reset saved current group
    Fl_Group::current(cg);
    if (val & 2)
      Fl::add_handler(shortcut_handler);
  }
  return 1;
}

#else // USE_PRINT_BUTTON not defined

int fl_create_print_window() {
  return 0;
}

#endif // USE_PRINT_BUTTON

/* undocumented function:

  Print a window or copy its contents to the clipboard.

    win              The window to process
    grab_decoration  true means the window titlebar is processed too
    mode             1 means print, other means copy
*/
int fl_print_or_copy_window(Fl_Window *win, bool grab_decoration, int mode) {

  if (!win) return 0;

  int ww = grab_decoration ? win->decorated_w() : win->w();
  int wh = grab_decoration ? win->decorated_h() : win->h();

  if (mode == 1) { // print window

    // exchange the 2 constructors below to test class Fl_PostScript_File_Device
    Fl_Printer printer;
    //Fl_PostScript_File_Device printer;
    int w, h;
    if (printer.begin_job(1)) { // fail or cancel
      return 1;
    }
    if (printer.begin_page()) { // fail or cancel
      return 1;
    }
    printer.printable_rect(&w, &h);
    // scale the printer device so that the window fits on the page
    float scale = 1;
    if (ww > w || wh > h) {
      scale = (float)w / ww;
      if ((float)h / wh < scale)
        scale = (float)h / wh;
      printer.scale(scale, scale);
      printer.printable_rect(&w, &h);
    }
#ifdef ROTATE
    printer.scale(scale * 0.8, scale * 0.8);
    printer.printable_rect(&w, &h);
    printer.origin(w / 2, h / 2);
    printer.rotate(ROTATE);
    printer.print_widget(win, -win->w() / 2, -win->h() / 2);
#else
    printer.origin(w / 2, h / 2);
    if (grab_decoration) printer.draw_decorated_window(win, -ww / 2, -wh / 2);
    else printer.draw(win, -ww / 2, -wh / 2);
#endif
    printer.end_page();
    printer.end_job();

  } else { // copy window to clipboard

    Fl_Copy_Surface *surf = new Fl_Copy_Surface(ww, wh);
    if (grab_decoration)
      surf->draw_decorated_window(win); // draw the window content
    else
      surf->draw(win); // draw the window content
    delete surf;       // put the window on the clipboard

  } // print | copy
  return 0;
}
