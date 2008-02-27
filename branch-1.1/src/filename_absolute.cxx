//
// "$Id$"
//
// Filename expansion routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2005 by Bill Spitzak and others.
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
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
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
// Visual C++ 2005 incorrectly displays a warning about the use of POSIX APIs
// on Windows, which is supposed to be POSIX compliant...
#  define getcwd _getcwd
#else
#  include <unistd.h>
#  ifdef __EMX__
#    define getcwd _getcwd2
#  endif
#endif

#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
inline int isdirsep(char c) {return c=='/' || c=='\\';}
#else
#define isdirsep(c) ((c)=='/')
#endif

int fl_filename_absolute(char *to, int tolen, const char *from) {
  if (isdirsep(*from) || *from == '|'
#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
      || from[1]==':'
#endif
      ) {
    strlcpy(to, from, tolen);
    return 0;
  }

  char *a;
  char *temp = new char[tolen];
  const char *start = from;

  a = getcwd(temp, tolen);
  if (!a) {
    strlcpy(to, from, tolen);
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
  strlcpy(a,start,tolen - (a - temp));

  strlcpy(to, temp, tolen);

  delete[] temp;

  return 1;
}

/*
 * 'fl_filename_relative()' - Make a filename relative to the current working directory.
 */

int					// O - 0 if no change, 1 if changed
fl_filename_relative(char       *to,	// O - Relative filename
                     int        tolen,	// I - Size of "to" buffer
                     const char *from) {// I - Absolute filename
  char		*newslash;		// Directory separator
  const char	*slash;			// Directory separator
  char		cwd[1024];		// Current directory


#if defined(WIN32) || defined(__EMX__)
  if (from[0] == '\0' ||
      (!isdirsep(*from) && !isalpha(*from) && from[1] != ':' &&
       !isdirsep(from[2]))) {
#else
  if (from[0] == '\0' || !isdirsep(*from)) {
#endif // WIN32 || __EMX__
    strlcpy(to, from, tolen);
    return 0;
  }

  if (!getcwd(cwd, sizeof(cwd))) {
    strlcpy(to, from, tolen);
    return 0;
  }

#if defined(WIN32) || defined(__EMX__)
  for (newslash = strchr(cwd, '\\'); newslash; newslash = strchr(newslash + 1, '\\'))
    *newslash = '/';

  if (!strcasecmp(from, cwd)) {
    strlcpy(to, ".", tolen);
    return (1);
  }

  if (tolower(*from & 255) != tolower(*cwd & 255)) {
    // Not the same drive...
    strlcpy(to, from, tolen);
    return 0;
  }
  for (slash = from + 2, newslash = cwd + 2;
#else
  if (!strcmp(from, cwd)) {
    strlcpy(to, ".", tolen);
    return (1);
  }

  for (slash = from, newslash = cwd;
#endif // WIN32 || __EMX__
       *slash != '\0' && *newslash != '\0';
       slash ++, newslash ++)
    if (isdirsep(*slash) && isdirsep(*newslash)) continue;
#if defined(WIN32) || defined(__EMX__) || defined(__APPLE__)
    else if (tolower(*slash & 255) != tolower(*newslash & 255)) break;
#else
    else if (*slash != *newslash) break;
#endif // WIN32 || __EMX__ || __APPLE__

  if (*newslash == '\0' && *slash != '\0' && !isdirsep(*slash))
    newslash--;

  while (!isdirsep(*slash) && slash > from) slash --;

  if (isdirsep(*slash)) slash ++;

  if (*newslash != '\0')
    while (!isdirsep(*newslash) && newslash > cwd) newslash --;

  to[0]         = '\0';
  to[tolen - 1] = '\0';

  while (*newslash != '\0') {
    if (isdirsep(*newslash)) strlcat(to, "../", tolen);

    newslash ++;
  }

  strlcat(to, slash, tolen);

  return 1;
}


//
// End of "$Id$".
//
