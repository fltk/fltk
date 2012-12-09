//
// "$Id$"
//
// More font utilities for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl.H>
#include <FL/x.H>
#include "Fl_Font.H"
#include "flstring.h"
#include <stdlib.h>

#ifdef WIN32
#  include "fl_set_fonts_win32.cxx"
#elif defined(__APPLE__)
#  include "fl_set_fonts_mac.cxx"
#elif USE_XFT
#  include "fl_set_fonts_xft.cxx"
#else
#  include "fl_set_fonts_x.cxx"
#endif // WIN32

//
// End of "$Id$".
//
