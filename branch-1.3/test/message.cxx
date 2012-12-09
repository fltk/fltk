//
// "$Id$"
//
// Message test program for the Fast Light Tool Kit (FLTK).
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
#include <FL/fl_ask.H>
#include <stdio.h>

int main(int argc, char **argv) {
  Fl::scheme(NULL);
  Fl::args(argc, argv);
  Fl::get_system_colors();

  fl_message("Spelling check sucessful, %d errors found with %g%% confidence",
	     1002, 100*(15/77.0));

  fl_alert(
		"Quantum fluctuations in the space-time continuum detected,\n"
	  "you have %g seconds to comply.\n\n"
		"\"In physics, spacetime is any mathematical model that combines\n"
		"space and time into a single construct called the space-time\n"
		"continuum. Spacetime is usually interpreted with space being\n"
		"three-dimensional and time playing the role of the\n"
		"fourth dimension.\" - Wikipedia",
		10.0);

  printf("fl_choice returned %d\n",
    fl_choice("Do you really want to %s?", "No", "Yes", 0L, "continue"));

  printf("fl_choice returned %d\n",
    fl_choice("Choose one of the following:","choice0","choice1","choice2"));
  const char *r;

  r = fl_input("Please enter a string for '%s':", "this is the default value",
	       "testing");
  printf("fl_input returned \"%s\"\n", r ? r : "NULL");

  r = fl_password("Enter %s's password:", 0, "somebody");
  printf("fl_password returned \"%s\"\n", r ? r : "NULL");

  return 0;
}

//
// End of "$Id$".
//
