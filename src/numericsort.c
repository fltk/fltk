/*
 * "$Id: numericsort.c,v 1.10.2.4 2001/01/22 15:13:41 easysw Exp $"
 *
 * Numeric sorting routine for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2001 by Bill Spitzak and others.
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
 * Please report all bugs and problems to "fltk-bugs@fltk.org".
 */

/* My own scandir sorting function, useful for the film industry where
   we have many files with numbers in their names: */

#include <config.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>

#if defined(WIN32) && !defined(__CYGWIN__)
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
int numericsort(struct dirent **A, struct dirent **B) {
  const char* a = (*A)->d_name;
  const char* b = (*B)->d_name;
  int ret = 0;
  for (;;) {
    if (isdigit((unsigned)*a) && isdigit((unsigned)*b)) {
      int diff,magdiff;
      while (*a == '0') a++;
      while (*b == '0') b++;
      while (isdigit((unsigned)*a) && *a == *b) {a++; b++;}
      diff = (isdigit((unsigned)*a) && isdigit((unsigned)*b)) ? *a - *b : 0;
      magdiff = 0;
      while (isdigit((unsigned)*a)) {magdiff++; a++;}
      while (isdigit((unsigned)*b)) {magdiff--; b++;}
      if (magdiff) {ret = magdiff; break;} /* compare # of significant digits*/
      if (diff) {ret = diff; break;}	/* compare first non-zero digit */
    } else {
#if 1
      if ((ret = tolower((unsigned)*a)-tolower((unsigned)*b))) break; /* compare case-insensitve */
#else
      if ((ret = *a-*b)) break;	/* compare case-sensitive */
#endif
      if (!*a) break;
      a++; b++;
    }
  }
  if (!ret) return 0;
  else return (ret < 0) ? -1 : 1;
}

/*
 * End of "$Id: numericsort.c,v 1.10.2.4 2001/01/22 15:13:41 easysw Exp $".
 */
