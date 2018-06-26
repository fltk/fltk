//
// "$Id$"
//
// Display function for the Fast Light Tool Kit (FLTK).
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

// Startup method to set what display to use.
// Using setenv makes programs that are exec'd use the same display.

#include <FL/Fl.H>
#include "Fl_Screen_Driver.H"

/**
 \brief Sets the X display to use for all windows.  

 Actually this just sets the environment variable $DISPLAY to the passed string, 
 so this only works before you show() the first window or otherwise open the 
 display.
 
 This does nothing on other platforms.
*/
void Fl::display(const char *d)
{
  screen_driver()->display(d);
}

//
// End of "$Id$".
//
