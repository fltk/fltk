/*
 * "$Id: numericsort.c,v 1.7 1998/11/09 16:25:59 mike Exp $"
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
int numericsort(struct dirent **A, struct dirent **B) {
  const char* a = (*A)->d_name;
  const char* b = (*B)->d_name;

  for (;;) {
    if (isdigit(*a) && isdigit(*b)) {
      int anum = 0, bnum = 0;

      while (isdigit(*a)) anum = anum * 10 + *a++ - '0';
      while (isdigit(*b)) bnum = bnum * 10 + *b++ - '0';

      if (anum < bnum) return (-1);
      else if (anum > bnum) return (1);
    } else if (tolower(*a) < tolower(*b)) return (-1);
    else if (tolower(*a) > tolower(*b)) return (1);
    else {
      if (*a == '\0') return (0);
      a++;
      b++;
    }
  }
}

/*
 * End of "$Id: numericsort.c,v 1.7 1998/11/09 16:25:59 mike Exp $".
 */
