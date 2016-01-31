/*
 * "$Id$"
 *
 * Common string header file for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2016 by Bill Spitzak and others.
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file.  If this
 * file is missing or damaged, see the license at:
 *
 *     http://www.fltk.org/COPYING.php
 *
 * Please report all bugs and problems on the following page:
 *
 *     http://www.fltk.org/str.php
 */

#ifndef flstring_h
#  define flstring_h

#  include <FL/Fl_Export.H>
#  include <config.h>
#  include <stdio.h>
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

#  if defined(WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
#    define strcasecmp(s,t)	_stricmp((s), (t))
#    define strncasecmp(s,t,n)	_strnicmp((s), (t), (n))
/* Visual C++ 2005 incorrectly displays a warning about the use of POSIX APIs
 * on Windows, which is supposed to be POSIX compliant...  Some of these
 * functions are also defined in ISO C99...
 */
#    ifndef __WATCOMC__
#      define strdup _strdup
#      define unlink _unlink
#    endif /* !__WATCOMC__ */
#  elif defined(__EMX__)
#    define strcasecmp(s,t)	stricmp((s), (t))
#    define strncasecmp(s,t,n)	strnicmp((s), (t), (n))
#  endif /* WIN32 */

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */

FL_EXPORT extern int fl_snprintf(char *, size_t, const char *, ...);
#  ifndef HAVE_SNPRINTF
#    define snprintf fl_snprintf
#  endif /* !HAVE_SNPRINTF */

FL_EXPORT extern int fl_vsnprintf(char *, size_t, const char *, va_list ap);
#  ifndef HAVE_VSNPRINTF
#    define vsnprintf fl_vsnprintf
#  endif /* !HAVE_VSNPRINTF */

/*
 * strlcpy() and strlcat() are some really useful BSD string functions
 * that work the way strncpy() and strncat() *should* have worked.
 */

FL_EXPORT extern size_t fl_strlcat(char *, const char *, size_t);
#  ifndef HAVE_STRLCAT
#    define strlcat fl_strlcat
#  endif /* !HAVE_STRLCAT */

FL_EXPORT extern size_t fl_strlcpy(char *, const char *, size_t);
#  ifndef HAVE_STRLCPY
#    define strlcpy fl_strlcpy
#  endif /* !HAVE_STRLCPY */

/*
 * locale independent ascii compare, does not introduce locale 
 * pbs as w/  case cmp 
 */
FL_EXPORT extern int fl_ascii_strcasecmp(const char *s, const char *t);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !flstring_h */

/*
 * End of "$Id$".
 */
