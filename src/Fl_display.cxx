//
// "$Id: Fl_display.cxx,v 1.4.2.3 2001/01/22 15:13:40 easysw Exp $"
//
// Display function for the Fast Light Tool Kit (FLTK).
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

// Startup method to set what display to use.
// Using setenv makes programs that are exec'd use the same display.

#include <FL/Fl.H>
#include <stdlib.h>
#include <string.h>

void Fl::display(const char *d) {
  char *e = new char[strlen(d)+13];
  strcpy(e,"DISPLAY=");
  strcpy(e+8,d);
  for (char *c = e+8; *c!=':'; c++) if (!*c) {strcpy(c,":0.0"); break;}
  putenv(e);
}

//
// End of "$Id: Fl_display.cxx,v 1.4.2.3 2001/01/22 15:13:40 easysw Exp $".
//
