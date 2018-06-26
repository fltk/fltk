//
// "$Id$"
//
// Window minification code for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl_Window.H>
#include "Fl_Window_Driver.H"

void Fl_Window::iconize() {
  if (!shown()) {
    show_iconic_ = 1;
    show();
  } else {
    pWindowDriver->iconize();
  }
}

//
// End of "$Id$".
//
