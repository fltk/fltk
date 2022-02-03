//
// Header for "Print Window" functions for the Fast Light Tool Kit (FLTK).
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

#ifndef _SRC_FL_PRINT_BUTTON_H_
#define _SRC_FL_PRINT_BUTTON_H_

#include <FL/Fl_Window.H>

// These are all internal functions, do not FL_EXPORT these functions!
// These functions are mplemented in src/print_button.cxx

// Create and initialize the "Print/copy front window" dialog window

int fl_create_print_window();

// Print a window or copy its contents to the clipboard.

int fl_print_or_copy_window(Fl_Window *win, bool grab_decoration, int mode);

#endif // _SRC_FL_PRINT_BUTTON_H_
