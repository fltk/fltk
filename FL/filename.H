//
// "$Id: filename.H,v 1.11.2.4.2.6 2002/05/02 11:11:00 easysw Exp $"
//
// Filename header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2002 by Bill Spitzak and others.
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
#  define FL_FILENAME_H

#  include "Fl_Export.H"

#  define FL_PATH_MAX 256 // all buffers are this length

FL_EXPORT const char *fl_filename_name(const char *); // return pointer to name
FL_EXPORT const char *fl_filename_ext(const char *); // return pointer to .ext
FL_EXPORT char *fl_filename_setext(char *to, int tolen, const char *ext); // clobber .ext
FL_EXPORT int fl_filename_expand(char *to, int tolen, const char *from); // do $x and ~x
FL_EXPORT int fl_filename_absolute(char *to, int tolen, const char *from); // prepend getcwd()
FL_EXPORT int fl_filename_relative(char *to, int tolen, const char *from); // make local to getcwd()
FL_EXPORT int fl_filename_match(const char *name, const char *pattern); // glob match
FL_EXPORT int fl_filename_isdir(const char *name);

#  ifdef __cplusplus
// Under WIN32, we include filename.H from numericsort.c; this should probably change
inline char *fl_filename_setext(char *to, const char *ext) { return fl_filename_setext(to, FL_PATH_MAX, ext); }
inline int fl_filename_expand(char *to, const char *from) { return fl_filename_expand(to, FL_PATH_MAX, from); }
inline int fl_filename_absolute(char *to, const char *from) { return fl_filename_absolute(to, FL_PATH_MAX, from); }
inline int fl_filename_relative(char *to, const char *from) { return fl_filename_relative(to, FL_PATH_MAX, from); }
#  endif // __cplusplus


// Portable "scandir" function.  Ugly but apparently necessary...

#  if defined(WIN32) && !defined(__CYGWIN__)

struct dirent {char d_name[1];};

#  else

#    include <sys/types.h>
#    include <dirent.h>
// warning: on some systems (very few nowadays?) <dirent.h> may not exist.
// The correct information is in one of these files:
//#include <sys/ndir.h>
//#include <sys/dir.h>
//#include <ndir.h>
// plus you must do the following #define:
//#define dirent direct
// It would be best to create a <dirent.h> file that does this...

#  endif

extern "C" {
  FL_EXPORT int fl_alphasort(dirent **, dirent **);
  FL_EXPORT int fl_casealphasort(dirent **, dirent **);
  FL_EXPORT int fl_casenumericsort(dirent **, dirent **);
  FL_EXPORT int fl_numericsort(dirent **, dirent **);

  typedef int (Fl_File_Sort_F)(dirent **, dirent **);
}

FL_EXPORT int fl_filename_list(const char *d, struct dirent ***list,
                               Fl_File_Sort_F *sort = fl_numericsort);


#endif

//
// End of "$Id: filename.H,v 1.11.2.4.2.6 2002/05/02 11:11:00 easysw Exp $".
//
