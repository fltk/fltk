/*
 * "$Id: flstring.h,v 1.1.2.9 2002/07/17 06:09:26 matthiaswm Exp $"
 *
 * Common string header file for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2002 by Bill Spitzak and others.
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

#ifndef flstring_h
#  define flstring_h

#  include <FL/Fl_Export.H>
#  include <config.h>
#  include <stdarg.h>
#  include <string.h>
#  ifdef HAVE_STRINGS_H
#    include <strings.h>
#  endif /* HAVE_STRINGS_H */
#  include <ctype.h>

/*
 * Apparently Unixware defines "index" to strchr (!) rather than
 * providing a proper entry point or not providing the (obsolete)
 * BSD function.  Make sure index is not defined...
 */

#  ifdef index
#    undef index
#  endif /* index */

#  if defined(WIN32) && !defined(__CYGWIN__)
#    define strcasecmp(s,t)	stricmp((s), (t))
#    define strncasecmp(s,t,n)	strnicmp((s), (t), (n))
#  elif defined(__EMX__)
#    define strcasecmp(s,t)	stricmp((s), (t))
#    define strncasecmp(s,t,n)	strnicmp((s), (t), (n))
#  endif /* WIN32 */

// MetroWerks' CodeWarrior put thes "non-standard" functions int <extras.h>
// which unfortunatly does not play well otherwise when included - to be resolved
#  if defined __APPLE__ && defined __MWERKS__
     extern "C" {
       int strcasecmp(const char*,const char*);
       int strncasecmp(const char*,const char*,int);
       char *strdup(const char*);
     }
#  endif

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */

#  if !HAVE_SNPRINTF
FL_EXPORT extern int fl_snprintf(char *, size_t, const char *, ...);
#    define snprintf fl_snprintf
#  endif /* !HAVE_SNPRINTF */

#  if !HAVE_VSNPRINTF
FL_EXPORT extern int fl_vsnprintf(char *, size_t, const char *, va_list ap);
#    define vsnprintf fl_vsnprintf
#  endif /* !HAVE_VSNPRINTF */

/*
 * strlcpy() and strlcat() are some really useful BSD string functions
 * that work the way strncpy() and strncat() *should* have worked.
 */

#  if !HAVE_STRLCAT
FL_EXPORT extern size_t fl_strlcat(char *, const char *, size_t);
#    define strlcat fl_strlcat
#  endif /* !HAVE_STRLCAT */

#  if !HAVE_STRLCPY
FL_EXPORT extern size_t fl_strlcpy(char *, const char *, size_t);
#    define strlcpy fl_strlcpy
#  endif /* !HAVE_STRLCPY */

#  ifdef __cplusplus
}
#  endif /* __cplusplus */
#endif /* !flstring_h */

/*
 * End of "$Id: flstring.h,v 1.1.2.9 2002/07/17 06:09:26 matthiaswm Exp $".
 */
