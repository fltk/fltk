/*
 * "$Id: flstring.h,v 1.1.2.5 2002/04/29 20:56:19 easysw Exp $"
 *
 * Common string header file for the Fast Light Tool Kit (FLTK).
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

#ifndef flstring_h
#  define flstring_h

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
#  endif // index

#  if defined(WIN32) && !defined(__CYGWIN__)
#    define strcasecmp(s,t)	stricmp((s), (t))
#    define strncasecmp(s,t,n)	strnicmp((s), (t), (n))
#  elif defined(__EMX__)
#    define strcasecmp(s,t)	stricmp((s), (t))
#    define strncasecmp(s,t,n)	strnicmp((s), (t), (n))
#  endif /* WIN32 */

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */

#  if !HAVE_SNPRINTF
extern int fl_snprintf(char *, size_t, const char *, ...);
#    define snprintf fl_snprintf
#  endif /* !HAVE_SNPRINTF */

#  if !HAVE_VSNPRINTF
extern int fl_vsnprintf(char *, size_t, const char *, va_list ap);
#    define vsnprintf fl_vsnprintf
#  endif /* !HAVE_VSNPRINTF */

#  ifdef __cplusplus
}
#  endif /* __cplusplus */
#endif /* !flstring_h */

/*
 * End of "$Id: flstring.h,v 1.1.2.5 2002/04/29 20:56:19 easysw Exp $".
 */
