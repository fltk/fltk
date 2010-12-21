//
// "$Id$"
//
//	An example of how to use Fl_Native_File_Chooser to open & save files.
//
// Copyright 2010 Greg Ercolano.
// Copyright 1998-2010 by Bill Spitzak and others.
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
#include <stdio.h>	// printf
#include <stdlib.h>	// exit,malloc
#include <string.h>	// strerror
#include <errno.h>	// errno
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_ask.H>

class Application : public Fl_Window {
  Fl_Native_File_Chooser *fc;
  // Does file exist?
  int exist(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp) { fclose(fp); return(1); }
    else    { return(0); }
  }
  // 'Open' the file
  void open(const char *filename) {
    printf("Open '%s'\n", filename);
  }
  // 'Save' the file
  //    Create the file if it doesn't exist
  //    and save something in it.
  //
  void save(const char *filename) {
    printf("Saving '%s'\n", filename);
    if ( !exist(filename) ) {
      FILE *fp = fopen(filename, "w");				// create file if it doesn't exist
      if ( fp ) {
        fprintf(fp, "Hello world.\n");
        fclose(fp);
      } else {
        fl_message("Error: %s: %s", filename, strerror(errno));
      }
    }
  }
  // Handle an 'Open' request from the menu
  static void open_cb(Fl_Widget *w, void *v) {
    Application *app = (Application*)v;
    app->fc->title("Open");
    app->fc->type(Fl_Native_File_Chooser::BROWSE_FILE);		// only picks files that exist
    switch ( app->fc->show() ) {
      case -1: break;	// Error
      case  1: break; 	// Cancel
      default:		// Choice
        app->fc->preset_file(app->fc->filename());
        app->open(app->fc->filename());
	break;
    }
  }
  // Handle a 'Save as' request from the menu
  static void saveas_cb(Fl_Widget *w, void *v) {
    Application *app = (Application*)v;
    app->fc->title("Save As");
    app->fc->type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);	// need this if file doesn't exist yet
    switch ( app->fc->show() ) {
      case -1: break;	// Error
      case  1: break; 	// Cancel
      default:		// Choice
        app->fc->preset_file(app->fc->filename());
        app->save(app->fc->filename());
	break;
    }
  }
  // Handle a 'Save' request from the menu
  static void save_cb(Fl_Widget *w, void *v) {
    Application *app = (Application*)v;
    if ( strlen(app->fc->filename()) == 0 ) {
      saveas_cb(w,v);
    } else {
      app->save(app->fc->filename());
    }
  }
  static void quit_cb(Fl_Widget *w, void *v) {
    exit(0);
  }
  // Return an 'untitled' default pathname
  const char* untitled_default() {
    static char *filename = 0;
    if ( !filename ) {
      const char *home =
        getenv("HOME") ? getenv("HOME") :		 // unix
	getenv("HOME_PATH") ? getenv("HOME_PATH") :	 // windows
	".";						 // other
      filename = (char*)malloc(strlen(home)+20);
      sprintf(filename, "%s/untitled.txt", home);
    }
    return(filename);
  }
public:
  // CTOR
  Application() : Fl_Window(400,200,"Native File Chooser Example") {
    Fl_Menu_Bar *menu = new Fl_Menu_Bar(0,0,400,25);
    menu->add("&File/&Open",  "^o", open_cb, (void*)this);
    menu->add("&File/&Save",  "^s", save_cb, (void*)this);
    menu->add("&File/&Save As", 0,  saveas_cb, (void*)this);
    menu->add("&File/&Quit",  "^q", quit_cb);
    // Initialize the file chooser
    fc = new Fl_Native_File_Chooser();
    fc->filter("Text\t*.txt\n");
    fc->preset_file(untitled_default());
    end();
  }
};

int main(int argc, char *argv[]) {
  Fl::scheme("gtk+");
  Application *app = new Application();
  app->show(argc,argv);
  return(Fl::run());
}

//
// End of "$Id$".
//
