//
// "$Id$"
//
//     How to use popen() and Fl::add_fd() - erco 10/04/04
//     Originally from erco's cheat sheet, permission by author.
//
//     Shows how the interface can remain "alive" while external
//     command is running and outputing occassional data. For instance,
//     while the command is running, keyboard navigation works,
//     text can be highlighted, and the interface can be resized.
//
// Copyright 2010 Greg Ercolano.
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
#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Multi_Browser.H>

#ifdef WIN32
#  define PING_CMD "ping -n 10 localhost"	// 'slow command' under windows
#  ifdef _MSC_VER
#    define popen _popen
#    define pclose _pclose
#  else /*_MSC_VER*/
#    include <unistd.h>				// non-MS win32 compilers (untested)
#  endif /*_MSC_VER*/
#else
#  include <unistd.h>
#  define PING_CMD "ping -i 2 -c 10 localhost"	// 'slow command' under unix
#endif

// GLOBALS
FILE *G_fp = NULL;

// Handler for add_fd() -- called whenever the ping command outputs a new line of data
// Note: FL_SOCKET as 1st argument is used to fix a compiler error(!) on Windows 64-bit.
// Unfortunately we need this in FLTK 1.3 - should hopefully be fixed in 1.4 with a better solution.
void HandleFD(FL_SOCKET fd, void *data) {
  Fl_Multi_Browser *brow = (Fl_Multi_Browser*)data;
  char s[1024];
  if ( fgets(s, 1023, G_fp) == NULL ) {		// read the line of data
    Fl::remove_fd(fileno(G_fp));		// command ended? disconnect callback
    pclose(G_fp);				// close the descriptor
    brow->add(""); brow->add("<<DONE>>");	// append msg indicating command finished
    return;
  }
  brow->add(s);					// line of data read? append to widget
}

int main(int argc, char *argv[]) {
  Fl_Window win(600,600);
  Fl_Multi_Browser brow(10,10,580,580);
  if ( ( G_fp = popen(PING_CMD, "r") ) == NULL ) {	// start the external unix command
    perror("popen failed");
    return(1);
  }
  Fl::add_fd(fileno(G_fp), HandleFD, (void*)&brow);	// setup a callback for the popen()ed descriptor
  win.resizable(brow);
  win.show(argc, argv);
  return(Fl::run());
}

//
// End of "$Id$".
//
