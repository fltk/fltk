//
// "$Id: filename_list.cxx,v 1.10.2.10 2001/04/13 19:13:14 easysw Exp $"
//
// Filename list routines for the Fast Light Tool Kit (FLTK).
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
#if defined(__hpux)
  // HP-UX defines the comparison function like this:
  return scandir(d, list, 0, (int(*)(const dirent **, const dirent **))numericsort);
#elif defined(__osf__)
  // OSF, DU 4.0x
  return scandir(d, list, 0, (int(*)(dirent **, dirent **))numericsort);
#elif defined(__aix)
  // AIX is almost standard...
  return scandir(d, list, 0, (int(*)(void*, void*))numericsort);
#elif HAVE_SCANDIR && !defined(__sgi)
  // The vast majority of Unix systems want the sort function to have this
  // prototype, most likely so that it can be passed to qsort without any
  // changes:
  return scandir(d, list, 0, (int(*)(const void*,const void*))numericsort);
#else
  // This version is when we define our own scandir (WIN32 and perhaps
  // some Unix systems) and apparently on Irix:
  return scandir(d, list, 0, numericsort);
#endif
}

//
// End of "$Id: filename_list.cxx,v 1.10.2.10 2001/04/13 19:13:14 easysw Exp $".
//
