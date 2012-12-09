//
// "$Id$"
//
// CubeView class .
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

#include <config.h>
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


//
// End of "$Id$".
//
