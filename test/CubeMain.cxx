//
// CubeView class .
//
// Copyright 1998-2024 by Bill Spitzak and others.
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
#include "CubeViewUI.h"

int
main(int argc, char **argv) {

    CubeViewUI *cvui=new CubeViewUI;

//Initial global objects.

    Fl::visual(FL_DOUBLE|FL_INDEX);

    cvui->show(argc, argv);

    return Fl::run();
}
