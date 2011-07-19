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
#endif

// Use Apple's chooser
#ifdef __APPLE__
#include <FL/Fl_Native_File_Chooser.H>
#endif

// All else falls back to FLTK's own chooser
#if ! defined(__APPLE__) && !defined(WIN32)
#include "Fl_Native_File_Chooser_FLTK.cxx"
#endif

const char *Fl_Native_File_Chooser::file_exists_message = "File exists. Are you sure you want to overwrite?";

//
// End of "$Id$".
//
