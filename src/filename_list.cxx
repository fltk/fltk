//
// "$Id"
//
// Filename list routines for the Fast Light Tool Kit (FLTK).
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

// Wrapper for scandir with const-correct function prototypes.

#include <config.h>
#include <FL/filename.H>

extern "C" {
  int numericsort(dirent **, dirent **);
#if HAVE_SCANDIR
#else
  int alphasort(dirent **, dirent **);
  int scandir (const char *dir, dirent ***namelist,
	       int (*select)(dirent *),
	       int (*compar)(dirent **, dirent **));
#endif
}

int filename_list(const char *d, dirent ***list) {
#if defined(_AIX) || defined(CRAY)
  // on some systems you may need to do this, due to a rather common
  // error in the prototype for the sorting function, where a level
  // of pointer indirection is missing:
  return scandir(d, list, 0, (int(*)(const void*,const void*))numericsort);
#else
  return scandir(d, list, 0, numericsort);
#endif
}

//
// End of "$Id: filename_list.cxx,v 1.4 1998/10/20 23:58:30 mike Exp $".
//
