/*
 * "$Id"
 *
 * Numeric sorting routine for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998 by Bill Spitzak and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "fltk-bugs@easysw.com".
 */

/* My own scandir sorting function, useful for the film industry where
   we have many files with numbers in their names: */

#include <config.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef WIN32
#include <FL/filename.H>
#else
#if HAVE_DIRENT_H
# include <dirent.h>
#else
# define dirent direct
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif
#endif

#ifdef __cplusplus
extern "C"
#endif
int numericsort(const struct dirent **A, const struct dirent **B) {
  const char* a = (*A)->d_name;
  const char* b = (*B)->d_name;
  int ret = 0;
  for (;;) {
    if (isdigit(*a) && isdigit(*b)) {
      int zdiff,diff,magdiff;
      zdiff = 0;
      while (*a == '0') {a++; zdiff++;}
      while (*b == '0') {b++; zdiff--;}
      while (isdigit(*a) && *a == *b) {a++; b++;}
      diff = (isdigit(*a) && isdigit(*b)) ? *a - *b : 0;
      magdiff = 0;
      while (isdigit(*a)) {magdiff++; a++;}
      while (isdigit(*b)) {magdiff--; b++;}
      if (ret);
      else if (magdiff) ret = magdiff;
      else if (diff) ret = diff;
      else if (zdiff) ret = zdiff;
    } else if (*a == *b) {
      if (!*a) return ret;
      a++; b++;
    } else
      return (*a-*b);
  }
}

//
// End of "$Id: numericsort.c,v 1.4 1998/10/19 20:46:57 mike Exp $".
//
