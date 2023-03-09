/*
  Dummy C file to build the dummy Cairo library for the FLTK project.

  Copyright 2023 by Bill Spitzak and others.

  This library is free software. Distribution and use rights are outlined in
  the file "COPYING" which should have been included with this file.  If this
  file is missing or damaged, see the license at:

      https://www.fltk.org/COPYING.php

  Please see the following page on how to report bugs and issues:

      https://www.fltk.org/bugs.php
*/

/*
  Note: since FLTK 1.4.0 Fl_Cairo_Window support is included in
  libfltk and libfltk_cairo is no longer necessary. This directory is
  used to build an "empty" dummy library for backwards compatibility,
  just in case users expect it to exist.

  The entire 'cairo' folder will be removed in a later FLTK release.
*/

int fl_cairo_dummy() {
  return 0;
}
