//
// "$Id: message.cxx,v 1.3 1998/10/21 14:21:35 mike Exp $"
//
// Message test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#include <stdio.h>

int main(int, char **) {
  fl_message("Spelling check sucessfull.");
  fl_alert("Quantum fluctuations in the space-time continuim detected.");
  printf("fl_ask returned %d\n",
    fl_ask("Do you really want to?"));
  printf("fl_choice returned %d\n",
    fl_choice("Choose one of the following:","choice0","choice1","choice2"));
  printf("fl_show_input returned \"%s\"\n",
    fl_show_input("Please enter a string:", "this is the default value"));
  printf("fl_password returned \"%s\"\n",
    fl_password("Enter your password:"));
  return 0;
}

//
// End of "$Id: message.cxx,v 1.3 1998/10/21 14:21:35 mike Exp $".
//
