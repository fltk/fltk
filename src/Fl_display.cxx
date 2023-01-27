//
// Display function for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

// Startup method to set what display to use.
// Using setenv makes programs that are exec'd use the same display.

#include <FL/Fl.H>
#include "Fl_Screen_Driver.H"

/**
 Sets the X or Wayland display to use for all windows.

 This sets the environment variable $DISPLAY or $WAYLAND_DISPLAY to the passed string,
 so this only works before you show() the first window or otherwise open the display.

 This does nothing on other platforms.
*/
void Fl::display(const char *d)
{
  screen_driver()->display(d);
}
