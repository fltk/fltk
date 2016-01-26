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
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement changes in font in its own file"
#else
// now included for fl_font.cxx, but will be its own source code module in drivers/Xlib/Fl_Xlib_Graphics_Driver_font..."
#endif // WIN32

//
// End of "$Id$".
//
