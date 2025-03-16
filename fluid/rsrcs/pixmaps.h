//
// Fluid GUI Image header for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

#ifndef _FLUID_PIXMAPS_H
#define _FLUID_PIXMAPS_H

class Fl_Pixmap;

extern Fl_Pixmap *bind_pixmap;
extern Fl_Pixmap *lock_pixmap;
extern Fl_Pixmap *protected_pixmap;
extern Fl_Pixmap *invisible_pixmap;
extern Fl_Pixmap *compressed_pixmap;

extern Fl_Pixmap *pixmap[];

void loadPixmaps();

#endif // _FLUID_PIXMAPS_H
