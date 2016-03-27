//
// "$Id$"
//
// Grab/release code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Screen_Driver.H>

////////////////////////////////////////////////////////////////
// "Grab" is done while menu systems are up.  This has several effects:
// Events are all sent to the "grab window", which does not even
// have to be displayed (and in the case of Fl_Menu.cxx it isn't).
// The system is also told to "grab" events and send them to this app.
// This also modifies how Fl_Window::show() works, on X it turns on
// override_redirect, it does similar things on WIN32.

void Fl::grab(Fl_Window *win)
{
  screen_driver()->grab(win);
}


//
// End of "$Id$".
//
