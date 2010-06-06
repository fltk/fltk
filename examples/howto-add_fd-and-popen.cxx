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
#include <stdio.h>
#include <unistd.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Multi_Browser.H>

#define PING_CMD "ping -i 2 -c 10 localhost"	// 'slow command' under unix

// GLOBALS
FILE *G_fp = NULL;

// Handler for add_fd() -- called whenever the ping command outputs a new line of data
void HandleFD(int fd, void *data) {
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
