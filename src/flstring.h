/*
 * Common string header file for the Fast Light Tool Kit (FLTK).
 * Internal use only (see "important note" below).
 *
 * Copyright 1998-2026 by Bill Spitzak and others.
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file.  If this
 * file is missing or damaged, see the license at:
 *
 *     https://www.fltk.org/COPYING.php
 *
 * Please see the following page on how to report bugs and issues:
 *
 *     https://www.fltk.org/bugs.php
 */

 /*
 * Important note: this header file includes '<config.h>' !
 *
 * This header MUST NOT be included in public headers (i.e. in 'FL/') and
 * SHOULD NOT be included in test and demo programs (i.e. in 'test/' or
 * 'examples/') because it includes '<config.h>'.
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
#  include <FL/fl_string_functions.h>

/*
 * Apparently Unixware defines "index" to strchr (!) rather than
 * providing a proper entry point or not providing the (obsolete)
 * BSD function.  Make sure index is not defined...
 */

#  ifdef index
#    undef index
#  endif /* index */

/*
 * Visual C++ 2005 incorrectly displays a warning about the use of
 * POSIX APIs on Windows, which is supposed to be POSIX compliant...
 * Some of these functions are also defined in ISO C99...
 */

#  if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
#    define strcasecmp(s,t)     _stricmp((s), (t))
#    define strncasecmp(s,t,n)  _strnicmp((s), (t), (n))
#  endif /* _WIN32 && ... */

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

/* promoted to <FL/fl_string_functions.h> */
/* FL_EXPORT extern size_t fl_strlcpy(char *, const char *, size_t); */
#  ifndef HAVE_STRLCPY
#    define strlcpy fl_strlcpy
#  endif /* !HAVE_STRLCPY */

/*
 * Locale independent ASCII string compare function,
 * does not introduce locale issues as with strcasecmp()
 */
FL_EXPORT extern int fl_ascii_strcasecmp(const char *s, const char *t);


// Special character class detection functions for FLTK:
//
// The following functions replace system functions which are locale dependent
// and/or work only with pure ASCII characters (range: 0x00 .. 0x7f).
// Using input (bytes) outside this range results in undefined behavior and may
// even crash (Visual Studio in Debug mode).
// Note that UTF-8 sequences using 'char' variables would be sign-extended to
// large negative values which are invalid.
//
// Note: These functions are not UTF-8 aware and are intended to fix bad behavior,
//   but they don't fix wrong semantics. In the future we should check all text
//   handling functions for UTF-8 awareness.
//
// These functions are intentionally declared and defined in this header in the
// `src` folder which is hidden from user code. They should not be used in demo
// programs in `test` and `examples` folders.
// These functions must not be "exported" (don't use FL_EXPORT).
//
// AlbrechtS, June 2026

/*
  This function can be used to replace isspace(int) in FLTK.

  This function is \b NOT UTF-8 aware and \b should only be used where only ASCII
  checks are needed.

  \note isspace() can only be used correctly on ASCII characters (bytes) in the
    range 0 .. 127. Everything else is locale dependent or results in undefined
    behavior.

  \param[in]  ch  input character
*/
inline int fl_ascii_isspace(int ch) {
  if (ch < 0 || ch > 0x7f) return 0;
  return isspace(ch);
}

/*
  This function can be used to replace toupper(int) in FLTK.

  This function is \b NOT UTF-8 aware and \b should only be used where only ASCII
  checks are needed.

  \note toupper() can only be used correctly on ASCII characters (bytes) in the
    range 0 .. 127. Everything else is locale dependent or results in undefined
    behavior.

  \param[in]  ch  input character
*/
inline int fl_ascii_toupper(int ch) {
  if (ch >= 'a' && ch <= 'z') return ch - ('a' - 'A');
  return ch;
}

/*
  This function can be used to replace tolower(int) in FLTK.

  This function is \b NOT UTF-8 aware and \b should only be used where only ASCII
  checks are needed.

  \note tolower() can only be used correctly on ASCII characters (bytes) in the
    range 0 .. 127. Everything else is locale dependent or results in undefined
    behavior.

  \param[in]  ch  input character
*/
inline int fl_ascii_tolower(int ch) {
  if (ch >= 'A' && ch <= 'Z') return ch + ('a' - 'A');
  return ch;
}
#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !flstring_h */
