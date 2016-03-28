//
// "$Id$"
//
// Warning/error message code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2011 by Bill Spitzak and others.
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

// You can also override this by redefining all of these.

#include <FL/Fl.H>
#include <FL/Fl_System_Driver.H>
#include <stdarg.h>

void (*Fl::warning)(const char* format, ...) = Fl_System_Driver::warning;
void (*Fl::error)(const char* format, ...) = Fl_System_Driver::error;
void (*Fl::fatal)(const char* format, ...) = Fl_System_Driver::fatal;

//
// End of "$Id$".
//
