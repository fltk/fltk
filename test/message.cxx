//
// "$Id: message.cxx,v 1.5.2.3 2001/01/22 15:13:41 easysw Exp $"
//
// Message test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#include <stdio.h>

int main(int, char **) {

  fl_message("Spelling check sucessfull, %d errors found with %g%% confidence",
	     1002, 100*(15/77.0));

  fl_alert("Quantum fluctuations in the space-time continuim detected, "
	   "you have %g seconds to comply.", 10.0);

  printf("fl_ask returned %d\n",
    fl_ask("Do you really want to %s?", "continue"));

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
// End of "$Id: message.cxx,v 1.5.2.3 2001/01/22 15:13:41 easysw Exp $".
//
