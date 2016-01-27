//
// "$Id$"
//
// Image drawing routines for the Fast Light Tool Kit (FLTK).
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

// I hope a simple and portable method of drawing color and monochrome
// images.  To keep this simple, only a single storage type is
// supported: 8 bit unsigned data, byte order RGB, and pixels are
// stored packed into rows with the origin at the top-left.  It is
// possible to alter the size of pixels with the "delta" argument, to
// add alpha or other information per pixel.  It is also possible to
// change the origin and direction of the image data by messing with
// the "delta" and "linedelta", making them negative, though this may
// defeat some of the shortcuts in translating the image for X.

// FIXME: use the correct macros for these conditions
// FIXME: eventuay get rid of this file and use the source files as modules.
#ifdef WIN32
#  include "drivers/GDI/Fl_GDI_Graphics_Driver_image.cxx"
#elif defined(__APPLE__)
// Apple Quartz driver in "drivers/Quartz/Fl_Quartz_Graphics_Driver_image.cxx"
#else
#  include "drivers/Xlib/Fl_Xlib_Graphics_Driver_image.cxx"
#endif

//
// End of "$Id$".
//
