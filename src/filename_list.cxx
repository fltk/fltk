//
// "$Id: filename_list.cxx,v 1.10.2.11.2.8 2004/11/20 03:44:18 easysw Exp $"
//
// Filename list routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2004 by Bill Spitzak and others.
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

#include <FL/filename.H>
#include "flstring.h"


extern "C" {
#ifndef HAVE_SCANDIR
  int fl_scandir (const char *dir, dirent ***namelist,
	          int (*select)(dirent *),
	          int (*compar)(dirent **, dirent **));
#  define scandir	fl_scandir
#endif
}

int fl_alphasort(struct dirent **a, struct dirent **b) {
  return strcmp((*a)->d_name, (*b)->d_name);
}

int fl_casealphasort(struct dirent **a, struct dirent **b) {
  return strcasecmp((*a)->d_name, (*b)->d_name);
}


int fl_filename_list(const char *d, dirent ***list,
                     Fl_File_Sort_F *sort) {
#ifndef HAVE_SCANDIR
  return scandir(d, list, 0, sort);
#elif defined(__hpux) || defined(__CYGWIN__)
  // HP-UX, Cygwin define the comparison function like this:
  return scandir(d, list, 0, (int(*)(const dirent **, const dirent **))sort);
#elif defined(__osf__)
  // OSF, DU 4.0x
  return scandir(d, list, 0, (int(*)(dirent **, dirent **))sort);
#elif defined(_AIX)
  // AIX is almost standard...
  return scandir(d, list, 0, (int(*)(void*, void*))sort);
#elif !defined(__sgi)
  // The vast majority of UNIX systems want the sort function to have this
  // prototype, most likely so that it can be passed to qsort without any
  // changes:
  return scandir(d, list, 0, (int(*)(const void*,const void*))sort);
#else
  // This version is when we define our own scandir (WIN32 and perhaps
  // some Unix systems) and apparently on IRIX:
  return scandir(d, list, 0, sort);
#endif
}

//
// End of "$Id: filename_list.cxx,v 1.10.2.11.2.8 2004/11/20 03:44:18 easysw Exp $".
//
