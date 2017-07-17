//
// "$Id$"
//
//	Simple example of using Fl_Native_File_Chooser.
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
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_message.H>
#include <stdlib.h>		/* exit() */
#include <string.h>		/* strcmp() */

Fl_Window *G_win = 0;
Fl_Menu_Button *G_menu = 0;
Fl_Native_File_Chooser *G_chooser = 0;

static void Menu_CB(Fl_Widget*,void*data) {
  // Handle the popup menu item the user picked..
  const char *choice = (const char*)data;
  if ( strcmp(choice, "quit") == 0 ) {		// Handle "quit"
    exit(0);
  } else if ( strcmp(choice, "open") == 0 ) {	// Handle "open"
    if ( G_chooser == 0 ) {
      // Create an instance of file chooser we can reuse..
      G_chooser = new Fl_Native_File_Chooser();
      G_chooser->directory(".");				// directory to start browsing with
      G_chooser->preset_file("nativefilechooser-simple.cxx");	// file to start with
      G_chooser->filter("C++\t*.{cxx,cpp,h,H}\n");
      G_chooser->type(Fl_Native_File_Chooser::BROWSE_FILE);	// only picks files that exist
      G_chooser->title("Pick a file please..");			// custom title for chooser window
    }
    // Show the chooser
    //    This blocks while chooser is open.
    //
    switch ( G_chooser->show() ) {
      case -1: break;	// Error
      case  1: break; 	// Cancel
      default:		// Choice
	G_chooser->preset_file(G_chooser->filename());
	fl_message("You chose: %s", G_chooser->filename());
	break;
    }
  }
}

int main(int argc, char *argv[]) {
  Fl::scheme("gtk+");
  G_win = new Fl_Window(640,480,"Test Native File Chooser");
  G_win->tooltip("Use right-click for popup menu..");
  {
    // Setup right-click menu for window..
    G_menu = new Fl_Menu_Button(0,20,640,480,"Popup Menu");
    G_menu->type(Fl_Menu_Button::POPUP3);
    G_menu->add("Open File Chooser..", 0, Menu_CB, (void*)"open");
    G_menu->add("Quit",                0, Menu_CB, (void*)"quit");
  }
  G_win->end();
  G_win->show(argc,argv);
  return(Fl::run());
}

//
// End of "$Id$".
//
