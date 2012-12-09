//
// "$Id$"
//
// Simple test of the Fl_Native_File_Chooser.
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
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//
#include <stdio.h>
#include <FL/Fl.H>
#include <FL/fl_ask.H>		// fl_beep()
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Native_File_Chooser.H>

// GLOBALS
Fl_Input *G_filename = NULL;

void Butt_CB(Fl_Widget*, void*) {
  // Create native chooser
  Fl_Native_File_Chooser native;
  native.title("Pick a file");
  native.type(Fl_Native_File_Chooser::BROWSE_FILE);
  native.filter("Text\t*.txt\n"
                "C Files\t*.{cxx,h,c}\n"
                "Apps\t*.{app}\n");		// TODO: need to add kNavSupportPackages to non-cocoa <FNFC>_MAC.cxx
  native.preset_file(G_filename->value());
  // Show native chooser
  switch ( native.show() ) {
    case -1: fprintf(stderr, "ERROR: %s\n", native.errmsg()); break;	// ERROR
    case  1: fprintf(stderr, "*** CANCEL\n"); fl_beep(); break;		// CANCEL
    default: 								// PICKED FILE
      if ( native.filename() ) {
        G_filename->value(native.filename());
      } else {
	G_filename->value("NULL");
      }
      break;
  }
}

int main(int argc, char **argv) {
  //// For a nicer looking browser under linux, call Fl_File_Icon::load_system_icons();
  //// (If you do this, you'll need to link with fltk_images)
  //// NOTE: If you do not load the system icons, the file chooser will still work, but
  ////       no icons will be shown. However, this means you do not need to link in the
  ////       fltk_images library, potentially reducing the size of your executable.
  //// Loading the system icons is not required by the OSX or Windows native file choosers.
#if !defined(WIN32) && !defined(__APPLE__)
  Fl_File_Icon::load_system_icons();
#endif

  int argn = 1;
#ifdef __APPLE__
  // OS X may add the process number as the first argument - ignore
  if (argc>argn && strncmp(argv[1], "-psn_", 5)==0)
    argn++;
#endif
  
  Fl_Window *win = new Fl_Window(600, 100, "Native File Chooser Test");
  win->size_range(300, 100, 0, 100);
  win->begin();
  {
    int y = 10;
    G_filename = new Fl_Input(80, y, win->w()-80-10, 25, "Filename");
    G_filename->value(argc <= argn ? "." : argv[argn]);
    G_filename->tooltip("Default filename");
    y += G_filename->h() + 5;
    Fl_Button *but = new Fl_Button(win->w()-80-10, win->h()-25-10, 80, 25, "Pick File");
    but->callback(Butt_CB);
    Fl_Box *dummy = new Fl_Box(80, 0, 430, 100);
    dummy->hide();
    win->resizable(dummy);
  }
  win->end();
  win->show(argc, argv);
  return(Fl::run());
}

//
// End of "$Id$".
//

