//
// "$Id: filename_absolute.cxx,v 1.5.2.4.2.5 2002/01/01 15:11:31 easysw Exp $"
//
// Filename expansion routines for the Fast Light Tool Kit (FLTK).
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

/* expand a file name by prepending current directory, deleting . and
   .. (not really correct for symbolic links) between the prepended
   current directory.  Use $PWD if it exists.
   Returns true if any changes were made.
*/

#include <FL/filename.H>
#include <stdlib.h>
#include "flstring.h"
#include <ctype.h>
#if defined(WIN32) && !defined(__CYGWIN__)
# include <direct.h>
//# define getcwd(a,b) _getdcwd(0,a,b)
#else
# include <unistd.h>
# ifdef __EMX__
#  define getcwd _getcwd2
# endif
#endif

#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
inline int isdirsep(char c) {return c=='/' || c=='\\';}
#else
#define isdirsep(c) ((c)=='/')
#endif

int filename_absolute(char *to, int tolen, const char *from) {
  if (isdirsep(*from) || *from == '|'
#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
      || from[1]==':'
#endif
      ) {
    strncpy(to, from, tolen - 1);
    to[tolen - 1] = '\0';
    return 0;
  }

  char *a;
  char *temp = new char[tolen];
  const char *start = from;

  a = getcwd(temp, tolen);
  if (!a) {
    strncpy(to, from, tolen - 1);
    to[tolen - 1] = '\0';
    delete[] temp;
    return 0;
  }
#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
  for (a = temp; *a; a++) if (*a=='\\') *a = '/'; // ha ha
#else
  a = temp+strlen(temp);
#endif
  if (isdirsep(*(a-1))) a--;
  /* remove intermediate . and .. names: */
  while (*start == '.') {
    if (start[1]=='.' && isdirsep(start[2])) {
      char *b;
      for (b = a-1; b >= temp && !isdirsep(*b); b--);
      if (b < temp) break;
      a = b;
      start += 3;
    } else if (isdirsep(start[1])) {
      start += 2;
    } else if (!start[1]) {
      start ++; // Skip lone "."
      break;
    } else
      break;
  }

  *a++ = '/';
  strncpy(a,start,tolen - (a - temp) - 1);
  temp[tolen - 1] = '\0';

  strncpy(to, temp, tolen - 1);
  to[tolen - 1] = '\0';

  delete[] temp;

  return 1;
}

/*
 * 'filename_relative()' - Make a filename relative to the current working directory.
 */

int					// O - 0 if no change, 1 if changed
filename_relative(char       *to,	// O - Relative filename
                  int        tolen,	// I - Size of "to" buffer
                  const char *from) {	// I - Absolute filename
  const char	*newslash;		// Directory separator
  const char	*slash;			// Directory separator
  char		cwd[1024];		// Current directory


  if (from[0] == '\0' || !isdirsep(*from)) {
    strncpy(to, from, tolen - 1);
    to[tolen - 1] = '\0';
    return 0;
  }

  if (!getcwd(cwd, sizeof(cwd))) {
    strncpy(to, from, tolen - 1);
    to[tolen - 1] = '\0';
    return 0;
  }

  for (slash = from, newslash = cwd;
       *slash != '\0' && *newslash != '\0';
       slash ++, newslash ++)
    if (isdirsep(*slash) && isdirsep(*newslash)) continue;
#if defined(WIN32) || defined(__EMX__) || defined(__APPLE__)
    else if (tolower(*slash) != tolower(*newslash)) break;
#else
    else if (*slash != *newslash) break;
#endif // WIN32 || __EMX__ || __APPLE__

  while (!isdirsep(*slash) && slash > from) slash --;

  if (isdirsep(*slash)) slash ++;

#if defined(WIN32) || defined(__EMX__)
  if (isalpha(slash[0]) && slash[1] == ':') {
    strncpy(to, from, tolen - 1);
    to[tolen - 1] = '\0';
    return 0; /* Different drive letter... */
  }
#endif /* WIN32 || __EMX__ */

  if (*newslash != '\0')
    while (!isdirsep(*newslash) && newslash > cwd) newslash --;

  to[0]         = '\0';
  to[tolen - 1] = '\0';

  while (*newslash != '\0') {
    if (isdirsep(*newslash)) strncat(to, "../", tolen - 1);

    newslash ++;
  }

  strncat(to, slash, tolen - 1);

  return 1;
}


//
// End of "$Id: filename_absolute.cxx,v 1.5.2.4.2.5 2002/01/01 15:11:31 easysw Exp $".
//
