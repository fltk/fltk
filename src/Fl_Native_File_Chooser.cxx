// "$Id$"
//
// FLTK native OS file chooser widget
//
// Copyright 1998-2010 by Bill Spitzak and others.
// Copyright 2004 Greg Ercolano.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

// Use Windows' chooser
#ifdef WIN32
#include "Fl_Native_File_Chooser_WIN32.cxx"

// Use Apple's chooser
#elif defined(__APPLE__) // PORTME: Fl_Screen_Driver - native file chooser
#include <FL/Fl_Native_File_Chooser.H>

#elif defined(FL_PORTING)
# pragma message "FL_PORTING: implement the native file chooser interface"
# include "Fl_Native_File_Chooser_FLTK.cxx"

// All else falls back to FLTK's own chooser
#else
#include "Fl_Native_File_Chooser_FLTK.cxx"
#endif

const char *Fl_Native_File_Chooser::file_exists_message = "File exists. Are you sure you want to overwrite?";

//
// End of "$Id$".
//
