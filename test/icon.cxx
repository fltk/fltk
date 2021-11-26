//
// Window icon test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_RGB_Image.H>

static Fl_Double_Window *win;

void choice_cb(Fl_Widget *, void *v) {
  Fl_Color c = (Fl_Color)fl_uint(v);
  if ( c != 0 ) {
    // choice was "Red", "Green"..
    static uchar buffer[32*32*3];         // static: issue #296
    Fl_RGB_Image rgbicon(buffer, 32, 32, 3);
    rgbicon.color_average(c, 0.0);
    win->icon(&rgbicon);                  // once assigned, 'rgbicon' can go out of scope
  } else {
    // choice was "None"..
    win->icon((Fl_RGB_Image*)0);          // reset window icon
  }
}

Fl_Menu_Item choices[] = {
  {"None",  0, choice_cb, fl_voidptr(0)},
  {"Red",   0, choice_cb, fl_voidptr(FL_RED)},
  {"Green", 0, choice_cb, fl_voidptr(FL_GREEN)},
  {"Blue",  0, choice_cb, fl_voidptr(FL_BLUE)},
  {0}
};

int main(int argc, char **argv) {
  Fl_Double_Window window(400,300, "FLTK Window Icon Test");
  win = &window;

  Fl_Choice choice(120,100,200,25,"Window icon:");
  choice.menu(choices);
  choice.callback(choice_cb);
  choice.when(FL_WHEN_RELEASE|FL_WHEN_NOT_CHANGED);
  choice.tooltip("Sets the application icon for window manager. "
                 "Affects e.g. titlebar, toolbar, Dock, Alt-Tab..");

  window.end();
  window.show(argc,argv);
  choice.do_callback();         // make default take effect
  return Fl::run();
}
