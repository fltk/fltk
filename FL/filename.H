//
// "$Id: filename.H,v 1.11.2.4.2.2 2001/11/26 00:15:06 easysw Exp $"
//
// Filename header file for the Fast Light Tool Kit (FLTK).
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

#ifndef FL_FILENAME_H
#define FL_FILENAME_H

#  include "Fl_Export.H"

#define FL_PATH_MAX 256 // all buffers are this length

FL_EXPORT const char *filename_name(const char *); // return pointer to name
FL_EXPORT const char *filename_ext(const char *); // return pointer to .ext
FL_EXPORT char *filename_setext(char *to, int tolen, const char *ext); // clobber .ext
inline char *filename_setext(char *to, const char *ext) { return filename_setext(to, FL_PATH_MAX, ext); }
FL_EXPORT int filename_expand(char *to, int tolen, const char *from); // do $x and ~x
inline int filename_expand(char *to, const char *from) { return filename_expand(to, FL_PATH_MAX, from); }
FL_EXPORT int filename_absolute(char *to, int tolen, const char *from); // prepend getcwd()
inline int filename_absolute(char *to, const char *from) { return filename_absolute(to, FL_PATH_MAX, from); }
FL_EXPORT int filename_relative(char *to, int tolen, const char *from); // make local to getcwd()
inline int filename_relative(char *to, const char *from) { return filename_relative(to, FL_PATH_MAX, from); }
FL_EXPORT int filename_match(const char *name, const char *pattern); // glob match
FL_EXPORT int filename_isdir(const char *name);

// Portable "scandir" function.  Ugly but apparently necessary...

#if defined(WIN32) && !defined(__CYGWIN__)

struct dirent {char d_name[1];};

#else

#include <sys/types.h>
#include <dirent.h>
// warning: on some systems (very few nowadays?) <dirent.h> may not exist.
// The correct information is in one of these files:
//#include <sys/ndir.h>
//#include <sys/dir.h>
//#include <ndir.h>
// plus you must do the following #define:
//#define dirent direct
// It would be best to create a <dirent.h> file that does this...

#endif

FL_EXPORT int filename_list(const char *d, struct dirent ***list);

#endif

//
// End of "$Id: filename.H,v 1.11.2.4.2.2 2001/11/26 00:15:06 easysw Exp $".
//
