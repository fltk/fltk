//
// "$Id: filename_expand.cxx,v 1.4.2.4 2001/01/22 15:13:40 easysw Exp $"
//
// Filename expansion routines for the Fast Light Tool Kit (FLTK).
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

/* expand a file name by substuting environment variables and
   home directories.  Returns true if any changes were made.
   to & from may be the same buffer.
*/

#include <FL/filename.H>
#include <stdlib.h>
#include <string.h>
#if defined(WIN32) && !defined(__CYGWIN__)
#else
# include <unistd.h>
# include <pwd.h>
#endif

#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
static inline int isdirsep(char c) {return c=='/' || c=='\\';}
#else
#define isdirsep(c) ((c)=='/')
#endif

int filename_expand(char *to,const char *from) {

  char temp[FL_PATH_MAX];
  strcpy(temp,from);
  const char *start = temp;
  const char *end = temp+strlen(temp);

  int ret = 0;

  for (char *a=temp; a<end; ) {	// for each slash component
    char *e; for (e=a; e<end && !isdirsep(*e); e++); // find next slash
    const char *value = 0; // this will point at substitute value
    switch (*a) {
    case '~':	// a home directory name
      if (e <= a+1) {	// current user's directory
	value = getenv("HOME");
#ifndef WIN32
      } else {	// another user's directory
	struct passwd *pwd;
	char t = *e; *(char *)e = 0; 
        pwd = getpwnam(a+1); 
        *(char *)e = t;
	    if (pwd) value = pwd->pw_dir;
#endif
      }
      break;
    case '$':		/* an environment variable */
      {char t = *e; *(char *)e = 0; value = getenv(a+1); *(char *)e = t;}
      break;
    }
    if (value) {
      // substitutions that start with slash delete everything before them:
      if (isdirsep(value[0])) start = a;
#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
      // also if it starts with "A:"
      if (value[0] && value[1]==':') start = a;
#endif
      int t = strlen(value); if (isdirsep(value[t-1])) t--;
      memmove(a+t, e, end+1-e);
      end = a+t+(end-e);
      memcpy(a, value, t);
      ret++;
    } else {
      a = e+1;
#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
      if (*e == '\\') {*e = '/'; ret++;} // ha ha!
#endif
    }
  }
  strcpy(to,start);
  return ret;
}

//
// End of "$Id: filename_expand.cxx,v 1.4.2.4 2001/01/22 15:13:40 easysw Exp $".
//
