//
// "$Id: filename_isdir.cxx,v 1.4.2.5 2001/01/22 15:13:40 easysw Exp $"
//
// Directory detection routines for the Fast Light Tool Kit (FLTK).
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

// Used by fl_file_chooser

#include <config.h>
#include <FL/filename.H>
#include <sys/stat.h>
#include <string.h>

int filename_isdir(const char* n) {
  struct stat	s;
#ifdef WIN32
  char		fn[1024];
  int		length;
  // This workaround brought to you by the fine folks at Microsoft!
  // (read lots of sarcasm in that...)
  strncpy(fn, n, sizeof(fn) - 1);
  fn[sizeof(fn) - 1] = '\0';
  length = strlen(fn);
  if (length > 0 && (fn[length - 1] == '/' || fn[length - 1] == '\\'))
    fn[length - 1] = '\0'; // Strip trailing slash...
  n = fn;
#endif
  return !stat(n, &s) && (s.st_mode&0170000)==0040000;
}

//
// End of "$Id: filename_isdir.cxx,v 1.4.2.5 2001/01/22 15:13:40 easysw Exp $".
//
