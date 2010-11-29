//
// "$Id$"
//
// Drag & Drop code for the Fast Light Tool Kit (FLTK).
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

// This file contains MacOS-specific code for fltk which is always linked
// in.  Search other files for "__APPLE__" or filenames ending in _mac.cxx
// for other system-specific code.

#include <FL/Fl.H>

extern int MACpreparedrag(void);

/**
 * drag and drop whatever is in the cut-copy-paste buffer
 * - create a selection first using: 
 *     Fl::copy(const char *stuff, int len, 0)
 */
int Fl::dnd()
{
  return MACpreparedrag();
}
  
//
// End of "$Id$".
//
