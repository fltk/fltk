//
// "$Id$"
//
// FLTK native OS file chooser widget
//
// Copyright 1998-2010 by Bill Spitzak and others.
// Copyright 2004 Greg Ercolano.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

/* \file
   Fl_Native_File_Chooser widget. */

/**
  \class Fl_Native_File_Chooser

  This class lets an FLTK application easily and consistently access 
  the operating system's native file chooser. Some operating systems 
  have very complex and specific file choosers that many users want 
  access to specifically, instead of FLTK's default file chooser(s). 

  In cases where there is no native file browser, FLTK's own file browser
  is used instead.

  To use this widget correctly, use the following include in your code:
  \code
  #include <FL/Fl_Native_File_Chooser.H>
  \endcode
  Do not include the other Fl_Native_File_Choser_XXX.H files in your code;
  those are platform specific files that will be included automatically
  depending on your build platform.

  The following example shows how to pick a single file:
  \code
  // Create and post the local native file chooser
  #include <FL/Fl_Native_File_Chooser.H>
  [..]
  Fl_Native_File_Chooser fnfc;
  fnfc.title("Pick a file");
  fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
  fnfc.filter("Text\t*.txt\n"
              "C Files\t*.{cxx,h,c}");
  fnfc.directory("/var/tmp");           // default directory to use
  // Show native chooser
  switch ( fnfc.show() ) {
    case -1: printf("ERROR: %s\n", fnfc.errmsg());    break;  // ERROR
    case  1: printf("CANCEL\n");                      break;  // CANCEL
    default: printf("PICKED: %s\n", fnfc.filename()); break;  // FILE CHOSEN
  }
  \endcode

  <B>Platform Specific Caveats</B>

  - Under X windows, it's best if you call Fl_File_Icon::load_system_icons()
    at the start of main(), to enable the nicer looking file browser widgets.
    Use the static public attributes of class Fl_File_Chooser to localize
    the browser.
  - Some operating systems support certain OS specific options; see 
    Fl_Native_File_Chooser::options() for a list.

  \image html Fl_Native_File_Chooser.png "The Fl_Native_File_Chooser on different platforms."
  \image latex Fl_Native_File_Chooser.png "The Fl_Native_File_Chooser on different platforms" width=14cm

*/
#ifndef FL_NATIVE_FILE_CHOOSER_H
#define FL_NATIVE_FILE_CHOOSER_H

// Use Windows' chooser
#ifdef WIN32
#include <FL/Fl_Native_File_Chooser_WIN32.H>
#endif

// Use Apple's chooser
#ifdef __APPLE__
#include <FL/Fl_Native_File_Chooser_MAC.H>
#endif

// All else falls back to FLTK's own chooser
#if ! defined(__APPLE__) && !defined(WIN32)
#include <FL/Fl_Native_File_Chooser_FLTK.H>
#endif

#endif /*FL_NATIVE_FILE_CHOOSER_H*/

//
// End of "$Id$".
//
