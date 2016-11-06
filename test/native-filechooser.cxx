//
// "$Id$"
//
// Simple test of the Fl_Native_File_Chooser.
//
// Copyright 1998-2016 by Bill Spitzak and others.
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
#include <string.h>		/* strstr() */
#include <FL/Fl.H>
#include <FL/fl_ask.H>		/* fl_beep() */
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Help_View.H>

// GLOBALS
Fl_Input *G_filename = NULL;
Fl_Multiline_Input *G_filter = NULL;

void PickFile_CB(Fl_Widget*, void*) {
  // Create native chooser
  Fl_Native_File_Chooser native;
  native.title("Pick a file");
  native.type(Fl_Native_File_Chooser::BROWSE_FILE);
  native.filter(G_filter->value());
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

void PickDir_CB(Fl_Widget*, void*) {
  // Create native chooser
  Fl_Native_File_Chooser native;
  native.title("Pick a Directory");
  native.directory(G_filename->value());
  native.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
  // Show native chooser
  switch ( native.show() ) {
    case -1: fprintf(stderr, "ERROR: %s\n", native.errmsg()); break;	// ERROR
    case  1: fprintf(stderr, "*** CANCEL\n"); fl_beep(); break;		// CANCEL
    default: 								// PICKED DIR
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
  
  Fl_Window *win = new Fl_Window(640, 400, "Native File Chooser Test");
  win->size_range(win->w(), win->h(), 0, 0);
  win->begin();
  {
    int x = 80, y = 10;
    G_filename = new Fl_Input(x, y, win->w()-80-10, 25, "Filename");
    G_filename->value(argc <= argn ? "." : argv[argn]);
    G_filename->tooltip("Default filename");

    y += G_filename->h() + 10;
    G_filter = new Fl_Multiline_Input(x, y, G_filename->w(), 100, "Filter");
    G_filter->value("Text\t*.txt\n"
                    "C Files\t*.{cxx,h,c,cpp}\n"
                    "Tars\t*.{tar,tar.gz}\n"
		    "Apps\t*.app");
    G_filter->tooltip("Filter to be used for browser.\n"
                      "An empty string may be used.\n");

    y += G_filter->h() + 10;
    Fl_Help_View *view = new Fl_Help_View(x, y, G_filename->w(), 200);
    view->box(FL_FLAT_BOX);
    view->color(win->color());
#define TAB "&lt;Tab&gt;"
    view->textfont(FL_HELVETICA);
    view->textsize(10);
    view->value("The Filter can be one or more filter patterns, one per line.\n"
		"Patterns can be:<ul>\n"
		"  <li>A single wildcard (e.g. <tt>\"*.txt\"</tt>)</li>\n"
		"  <li>Multiple wildcards (e.g. <tt>\"*.{cxx,h,H}\"</tt>)</li>\n"
		"  <li>A descriptive name followed by a " TAB " and a wildcard (e.g. <tt>\"Text Files" TAB "*.txt\"</tt>)</li>\n"
		"</ul>\n"
                "In the above \"Filter\" field, you can use <b><font color=#55f face=Courier>Ctrl-I</font></b> to enter " TAB " characters as needed.<br>\n"
		"Example:<pre>\n"
		"\n"
		"    Text<font color=#55f>&lt;Ctrl-I&gt;</font>*.txt\n"
		"    C Files<font color=#55f>&lt;Ctrl-I&gt;</font>*.{cxx,h,c,cpp}\n"
		"    Tars<font color=#55f>&lt;Ctrl-I&gt;</font>*.{tar,tar.gz}\n"
		"    Apps<font color=#55f>&lt;Ctrl-I&gt;</font>*.app\n"
		"</pre>\n");

    Fl_Button *but = new Fl_Button(win->w()-x-10, win->h()-25-10, 80, 25, "Pick File");
    but->callback(PickFile_CB);

    Fl_Button *butdir = new Fl_Button(but->x()-x-10, win->h()-25-10, 80, 25, "Pick Dir");
    butdir->callback(PickDir_CB);

    win->resizable(G_filter);
  }
  win->end();
  win->show(argc, argv);
  return(Fl::run());
}

//
// End of "$Id$".
//
