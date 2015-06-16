//
// "$Id$"
//
// Unicode to UTF-8 conversion functions.
//
// Author: Jean-Marc Lienher ( http://oksid.ch )
// Copyright 2000-2010 by O'ksi'D.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <config.h>
#include <FL/filename.H>
#include <stdarg.h>

#if defined(WIN32) || defined(__CYGWIN__)
# include <ctype.h>
# include <io.h>
# include <windows.h>
# include <winbase.h>
# include <process.h>
# ifdef __CYGWIN__
#  include  <wchar.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  include <unistd.h>
# else
#  include  <direct.h>
# endif
extern "C" {
  int XUtf8Tolower(int ucs);
  unsigned short XUtf8IsNonSpacing(unsigned int ucs);
};
#elif defined(__APPLE__)
# include <stdio.h>
# include <time.h>
//# include <unix.h>
# include <fcntl.h>
# include <unistd.h>
# include <wchar.h>
# include <stdlib.h>
#   include <sys/types.h>
# include <sys/stat.h>

extern "C" {
  int XUtf8Tolower(int ucs);
  unsigned short XUtf8IsNonSpacing(unsigned int ucs);
}

#else // X-windows platform

# include "Xutf8.h"
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>
#endif // WIN32

#include <FL/fl_utf8.h>
#include <string.h>
#include <stdlib.h>

#undef fl_open

/** \addtogroup fl_unicode
    @{
*/

// *** NOTE : All functions are LIMITED to 24 bits Unicode values !!! ***
// ***        But only 16 bits are really used under Linux and win32  ***


#define NBC 0xFFFF + 1

static int Toupper(int ucs) {
  long i;
  static unsigned short *table = NULL;

  if (!table) {
    table = (unsigned short*) malloc(
	    sizeof(unsigned short) * (NBC));
    for (i = 0; i < NBC; i++) {
      table[i] = (unsigned short) i;
    }
    for (i = 0; i < NBC; i++) {
      int l;
      l = XUtf8Tolower(i);
      if (l != i) table[l] = (unsigned short) i;
    }
  }
  if (ucs >= NBC || ucs < 0) return ucs;
  return table[ucs];
}

/**
  Returns the byte length of the UTF-8 sequence with first byte \p c,
  or -1 if \p c is not valid.

  This function is helpful for finding faulty UTF-8 sequences.
  \see fl_utf8len1
*/
int fl_utf8len(char c)
{
  if (!(c & 0x80)) return 1;
  if (c & 0x40) {
    if (c & 0x20) {
      if (c & 0x10) {
        if (c & 0x08) {
          if (c & 0x04) {
            return 6;
          }
          return 5;
        }
        return 4;
      }
      return 3;
    }
    return 2;
  }
  return -1;
} // fl_utf8len


/**
  Returns the byte length of the UTF-8 sequence with first byte \p c,
  or 1 if \p c is not valid.

  This function can be used to scan faulty UTF-8 sequences, albeit
  ignoring invalid codes.
  \see fl_utf8len
*/
int fl_utf8len1(char c)
{
  if (!(c & 0x80)) return 1;
  if (c & 0x40) {
    if (c & 0x20) {
      if (c & 0x10) {
        if (c & 0x08) {
          if (c & 0x04) {
            return 6;
          }
          return 5;
        }
        return 4;
      }
      return 3;
    }
    return 2;
  }
  return 1;
} // fl_utf8len1


/**
  Returns the number of Unicode chars in the UTF-8 string.
*/
int
fl_utf_nb_char(
	const unsigned char 	*buf,
	int 			len)
{
  int i = 0;
  int nbc = 0;
  while (i < len) {
    int cl = fl_utf8len((buf+i)[0]);
    if (cl < 1) cl = 1;
    nbc++;
    i += cl;
  }
  return nbc;
}


/**
  UTF-8 aware strncasecmp - converts to lower case Unicode and tests.

  \param s1, s2 the UTF-8 strings to compare
  \param n the maximum number of UTF-8 characters to compare
  \return result of comparison
  \retval 0 if the strings are equal
  \retval >0 if s1 is greater than s2
  \retval <0 if s1 is less than s2
*/
int fl_utf_strncasecmp(const char *s1, const char *s2, int n)
{
  int i;
  for (i = 0; i < n; i++) {
    int l1, l2;
    unsigned int u1, u2;

    if (*s1==0 && *s2==0) return 0; // all compared equal, return 0

    u1 = fl_utf8decode(s1, 0, &l1);
    u2 = fl_utf8decode(s2, 0, &l2);
    int res = XUtf8Tolower(u1) - XUtf8Tolower(u2);
    if (res) return res;
    s1 += l1;
    s2 += l2;
  }
  return 0;
}


/**
  UTF-8 aware strcasecmp - converts to Unicode and tests.

  \return result of comparison
  \retval 0 if the strings are equal
  \retval 1 if s1 is greater than s2
  \retval -1 if s1 is less than s2
*/
int fl_utf_strcasecmp(const char *s1, const char *s2)
{
  return fl_utf_strncasecmp(s1, s2, 0x7fffffff);
}

/**
  Returns the Unicode lower case value of \p ucs.
*/
int fl_tolower(unsigned int ucs)
{
  return XUtf8Tolower(ucs);
}

/**
  Returns the Unicode upper case value of \p ucs.
*/
int fl_toupper(unsigned int ucs)
{
  return Toupper(ucs);
}

/**
  Converts the string \p str to its lower case equivalent into buf.
  Warning: to be safe buf length must be at least 3 * len [for 16-bit Unicode]
*/
int fl_utf_tolower(const unsigned char *str, int len, char *buf)
{
  int i;
  int l = 0;
  char *end = (char *)&str[len];
  for (i = 0; i < len;) {
    int l1, l2;
    unsigned int u1;

    u1 = fl_utf8decode((const char*)(str + i), end, &l1);
    l2 = fl_utf8encode((unsigned int) XUtf8Tolower(u1), buf + l);
    if (l1 < 1) {
      i += 1;
    } else {
      i += l1;
    }
    if (l2 < 1) {
      l += 1;
    } else {
      l += l2;
    }
  }
  return l;
}

/**
  Converts the string \p str to its upper case equivalent into buf.
  Warning: to be safe buf length must be at least 3 * len [for 16-bit Unicode]
*/
int fl_utf_toupper(const unsigned char *str, int len, char *buf)
{
  int i;
  int l = 0;
  char *end = (char *)&str[len];
  for (i = 0; i < len;) {
    int l1, l2;
    unsigned int u1;

    u1 = fl_utf8decode((const char*)(str + i), end, &l1);
    l2 = fl_utf8encode((unsigned int) Toupper(u1), buf + l);
    if (l1 < 1) {
      i += 1;
    } else {
      i += l1;
    }
    if (l2 < 1) {
      l += 1;
    } else {
      l += l2;
    }
  }
  return l;
}


/**
  Returns true if the Unicode character \p ucs is non-spacing.

  Non-spacing characters in Unicode are typically combining marks like
  tilde (~), diaeresis (¨), or other marks that are added to a base
  character, for instance 'a' (base character) + '¨' (combining mark) = 'ä'
  (German Umlaut).

  - http://unicode.org/glossary/#base_character
  - http://unicode.org/glossary/#nonspacing_mark
  - http://unicode.org/glossary/#combining_character
*/
unsigned int fl_nonspacing(unsigned int ucs)
{
  return (unsigned int) XUtf8IsNonSpacing(ucs);
}

#ifdef WIN32
unsigned int fl_codepage = 0;
#endif

#if defined (WIN32) && !defined(__CYGWIN__)

// For Win32 platforms, we frequently need to translate between
// Windows 16-bit wide characters (usually UTF-16) and our
// native UTF-8 strings. To this end, we maintain a number of
// character buffers to support the conversions.
// NOTE: Our re-use of these buffers means this code is not
// going to be thread-safe.
static xchar *mbwbuf = NULL;
static xchar *wbuf = NULL;
static xchar *wbuf1 = NULL;
static char *buf = NULL;
static int buf_len = 0;
static unsigned short *wbufa = NULL;

// FIXME: This should *maybe* return 'const char *' instead of 'char *'
char *fl_utf8_to_locale(const char *s, int len, UINT codepage)
{
  if (!s) return (char *)"";
  int l = 0;
  unsigned wn = fl_utf8toUtf16(s, len, NULL, 0); // Query length
  wn = wn * 2 + 1;
  if (wn >= (unsigned)buf_len) {
    buf_len = wn;
    buf = (char*) realloc(buf, buf_len);
    wbufa = (unsigned short*) realloc(wbufa, buf_len * sizeof(short));
  }
  if (codepage < 1) codepage = fl_codepage;
  l = fl_utf8toUtf16(s, len, wbufa, wn); // Convert string
  wbufa[l] = 0;
  buf[l] = 0;
  l = WideCharToMultiByte(codepage, 0, (WCHAR*)wbufa, l, buf, buf_len, NULL, NULL);
  if (l < 0) l = 0;
  buf[l] = 0;
  return buf;
}

// FIXME: This should maybe return 'const char *' instead of 'char *'
char *fl_locale_to_utf8(const char *s, int len, UINT codepage)
{
  if (!s) return (char *)"";
  int l = 0;
  if (buf_len < len * 5 + 1) {
    buf_len = len * 5 + 1;
    buf = (char*) realloc(buf, buf_len);
    wbufa = (unsigned short*) realloc(wbufa, buf_len * sizeof(short));
  }
  if (codepage < 1) codepage = fl_codepage;
  buf[l] = 0;

  l = MultiByteToWideChar(codepage, 0, s, len, (WCHAR*)wbufa, buf_len);
  if (l < 0) l = 0;
  wbufa[l] = 0;
  l = fl_utf8fromwc(buf, buf_len, (xchar*)wbufa, l);
  buf[l] = 0;
  return buf;
}
#endif

/**
  Converts UTF-8 string \p s to a local multi-byte character string.
*/
char * fl_utf2mbcs(const char *s)
{
  if (!s) return NULL;

#if defined(WIN32) && !defined(__CYGWIN__)

  size_t l = strlen(s);
  static char *buf = NULL;

  unsigned wn = fl_utf8toUtf16(s, (unsigned) l, NULL, 0) + 7; // Query length
  mbwbuf = (xchar*)realloc(mbwbuf, sizeof(xchar)*wn);
  l = fl_utf8toUtf16(s, (unsigned) l, (unsigned short *)mbwbuf, wn); // Convert string
  mbwbuf[l] = 0;

  buf = (char*)realloc(buf, (unsigned) (l * 6 + 1));
  l = (unsigned) wcstombs(buf, mbwbuf, (unsigned) l * 6);
  buf[l] = 0;
  return buf;
#else
  return (char*) s;
#endif
}

/** Cross-platform function to get environment variables with a UTF-8 encoded
  name or value.

  This function is especially useful under the MSWindows platform where
  non-ASCII environment variables are encoded as wide characters.
  The returned value of the variable is encoded in UTF-8 as well.

  On platforms other than MSWindows this function calls getenv directly.
  The return value is returned as-is.

  \param[in] v the UTF-8 encoded environment variable
  \return  the environment variable in UTF-8 encoding, or NULL in case of error.
*/

char *fl_getenv(const char* v) {

#if defined (WIN32) && !defined(__CYGWIN__)

  size_t l =  strlen(v);
  unsigned wn = fl_utf8toUtf16(v, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
  wn = fl_utf8toUtf16(v, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  xchar *ret = _wgetenv(wbuf);
  static char *buf = NULL;
  if (ret) {
    l = (unsigned) wcslen(ret);
    wn = fl_utf8fromwc(NULL, 0, ret, (unsigned) l) + 1; // query length
    buf = (char*) realloc(buf, wn);
    wn = fl_utf8fromwc(buf, wn, ret, (unsigned) l); // convert string
    buf[wn] = 0;
    return buf;
  } else {
    return NULL;
  }

#else

  return getenv(v);

#endif

} // fl_getenv()


/** Cross-platform function to open files with a UTF-8 encoded name.

 This function is especially useful under the MSWindows platform where the
 standard open() function fails with UTF-8 encoded non-ASCII filenames.
 \param f  the UTF-8 encoded filename
 \param oflags  other arguments are as in the standard open() function
 \return  a file descriptor upon successful completion, or -1 in case of error.
 \sa fl_fopen().
 */
int fl_open(const char* f, int oflags, ...)
{
  int pmode;
  va_list ap;
  va_start(ap, oflags);
  pmode = va_arg (ap, int);
  va_end(ap);

#if defined (WIN32) && !defined(__CYGWIN__)

  unsigned l = (unsigned) strlen(f);
  unsigned wn = fl_utf8toUtf16(f, l, NULL, 0) + 1; // Query length
  wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
  wn = fl_utf8toUtf16(f, l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  if (pmode == -1) return _wopen(wbuf, oflags);
  else return _wopen(wbuf, oflags, pmode);

#else

  if (pmode == -1) return open(f, oflags);
  else return open(f, oflags, pmode);

#endif

} // fl_open()


/** Cross-platform function to open files with a UTF-8 encoded name.

  This function is especially useful under the MSWindows platform where the
  standard fopen() function fails with UTF-8 encoded non-ASCII filenames.
  \param f  the UTF-8 encoded filename
  \param mode  same as the second argument of the standard fopen() function
  \return  a FILE pointer upon successful completion, or NULL in case of error.
  \sa fl_open().
*/
FILE *fl_fopen(const char* f, const char *mode) {

#if  defined (WIN32) && !defined(__CYGWIN__)

  size_t l = strlen(f);
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  l = strlen(mode);
  wn = fl_utf8toUtf16(mode, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf1 = (xchar*)realloc(wbuf1, sizeof(xchar)*wn);
  wn = fl_utf8toUtf16(mode, (unsigned) l, (unsigned short *)wbuf1, wn); // Convert string
  wbuf1[wn] = 0;
  return _wfopen(wbuf, wbuf1);

#else

  return fopen(f, mode);

#endif

} // fl_fopen()


/** Cross-platform function to run a system command with a UTF-8 encoded string.

  This function is especially useful under the MSWindows platform where
  non-ASCII program (file) names must be encoded as wide characters.

  On platforms other than MSWindows this function calls system() directly.

  \param[in] cmd the UTF-8 encoded command string
  \return the return value of _wsystem() on Windows or system() on other platforms.
*/

int fl_system(const char* cmd)
{
#if defined(WIN32) && !defined(__CYGWIN__)

# ifdef __MINGW32__
  return system(fl_utf2mbcs(cmd));
# else
  size_t l = strlen(cmd);
  unsigned wn = fl_utf8toUtf16(cmd, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
  wn = fl_utf8toUtf16(cmd, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return _wsystem(wbuf);
# endif

#else
  return system(cmd);
#endif
}

int fl_execvp(const char *file, char *const *argv)
{
#if defined(WIN32) && !defined(__CYGWIN__) // Windows

# ifdef __MINGW32__
  return _execvp(fl_utf2mbcs(file), argv);
# else
  size_t l = strlen(file);
  int i, n;
  xchar **ar;
  unsigned wn = fl_utf8toUtf16(file, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
  wn = fl_utf8toUtf16(file, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;

  i = 0; n = 0;
  while (argv[i]) {i++; n++;}
  ar = (xchar**) malloc(sizeof(xchar*) * (n + 1));
  i = 0;
  while (i <= n) {
    unsigned wn;
    l = strlen(argv[i]);
    wn = fl_utf8toUtf16(argv[i], (unsigned) l, NULL, 0) + 1; // Query length
    ar[i] = (xchar *)malloc(sizeof(xchar)*wn);
    wn = fl_utf8toUtf16(argv[i], (unsigned) l, (unsigned short *)ar[i], wn); // Convert string
    ar[i][wn] = 0;
    i++;
  }
  ar[n] = NULL;
  _wexecvp(wbuf, ar);	// STR #3040
  i = 0;
  while (i <= n) {
    free(ar[i]);
    i++;
  }
  free(ar);
  return -1;		// STR #3040
# endif

#else			// other platforms
  return execvp(file, argv);
#endif

}

/** Cross-platform function to set a files mode() with a UTF-8 encoded
  name or value.

 This function is especially useful under the MSWindows platform where the
 standard chmod() function fails with UTF-8 encoded non-ASCII filenames.

  \param[in] f the UTF-8 encoded filename
  \param[in] mode the mode to set
  \return    the return value of _wchmod() on Windows or chmod() on other platforms.
*/
int fl_chmod(const char* f, int mode) {

#if defined(WIN32) && !defined(__CYGWIN__) // Windows

  size_t l = strlen(f);
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return _wchmod(wbuf, mode);

#else // other platforms

  return chmod(f, mode);

#endif

} // fl_chmod()


/** Cross-platform function to test a files access() with a UTF-8 encoded
  name or value.

 This function is especially useful under the MSWindows platform where the
 standard access() function fails with UTF-8 encoded non-ASCII filenames.

  \param[in] f the UTF-8 encoded filename
  \param[in] mode the mode to test
  \return    the return value of _waccess() on Windows or access() on other platforms.
*/
int fl_access(const char* f, int mode) {

#if defined (WIN32) && !defined(__CYGWIN__) // Windows

  size_t l = strlen(f);
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return _waccess(wbuf, mode);

#else // other platforms

  return access(f, mode);

#endif

} // fl_access()


/** Cross-platform function to stat() a file using a UTF-8 encoded
  name or value.

 This function is especially useful under the MSWindows platform where the
 standard stat() function fails with UTF-8 encoded non-ASCII filenames.

  \param[in] f the UTF-8 encoded filename
  \param     b the stat struct to populate
  \return    the return value of _wstat() on Windows or stat() on other platforms.
*/
int fl_stat(const char* f, struct stat *b) {

#if defined(WIN32) && !defined(__CYGWIN__) // Windows

  size_t l = strlen(f);
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return _wstat(wbuf, (struct _stat*)b);

#else // other platforms

  return stat(f, b);

#endif

} // fl_stat()


/** Cross-platform function to get the current working directory
    as a UTF-8 encoded value.

 This function is especially useful under the MSWindows platform where the
 standard _wgetcwd() function returns UTF-16 encoded non-ASCII filenames.

  \param     b the buffer to populate
  \param     l the length of the buffer
  \return    the CWD encoded as UTF-8.
*/
char *fl_getcwd(char* b, int l) {

  if (b == NULL) {
    b = (char*) malloc(l+1);
  }

#if defined(WIN32) && !defined(__CYGWIN__) // Windows

  static xchar *wbuf = NULL;
  wbuf = (xchar*)realloc(wbuf, sizeof(xchar) * (l+1));
  xchar *ret = _wgetcwd(wbuf, l);
  if (ret) {
    unsigned dstlen = l;
    l = (int) wcslen(wbuf);
    dstlen = fl_utf8fromwc(b, dstlen, wbuf, (unsigned) l);
    b[dstlen] = 0;
    return b;
  } else {
    return NULL;
  }

#else // other platforms

  return getcwd(b, l);

#endif

} // fl_getcwd()


/** Cross-platform function to unlink() (that is, delete) a file using
    a UTF-8 encoded filename.

 This function is especially useful under the MSWindows platform where the
 standard function expects UTF-16 encoded non-ASCII filenames.

  \param     f the filename to unlink
  \return    the return value of _wunlink() on Windows or unlink() on other platforms.
*/
int fl_unlink(const char* f) {

#if defined(WIN32) && !defined(__CYGWIN__) // Windows

  size_t l = strlen(f);
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return _wunlink(wbuf);

#else // other platforms

  return unlink(f);

#endif

} // fl_unlink()


/** Cross-platform function to create a directory with a UTF-8 encoded
  name.

 This function is especially useful on the MSWindows platform where the
 standard _wmkdir() function expects UTF-16 encoded non-ASCII filenames.

  \param[in] f the UTF-8 encoded filename
  \param[in] mode the mode of the directory
  \return    the return value of _wmkdir() on Windows or mkdir() on other platforms.
*/
int fl_mkdir(const char* f, int mode) {

#if defined(WIN32) && !defined(__CYGWIN__) // Windows

  size_t l = strlen(f);
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return _wmkdir(wbuf);

#else // other platforms

  return mkdir(f, mode);

#endif

} // fl_mkdir()


/** Cross-platform function to remove a directory with a UTF-8 encoded
  name.

 This function is especially useful on the MSWindows platform where the
 standard _wrmdir() function expects UTF-16 encoded non-ASCII filenames.

  \param[in] f the UTF-8 encoded filename to remove
  \return    the return value of _wrmdir() on Windows or rmdir() on other platforms.
*/
int fl_rmdir(const char* f) {

#if defined (WIN32) && !defined(__CYGWIN__) // Windows

  size_t l = strlen(f);
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return _wrmdir(wbuf);

#else

  return rmdir(f);

#endif

} // fl_rmdir()


/** Cross-platform function to rename a filesystem object using
    UTF-8 encoded names.

 This function is especially useful on the MSWindows platform where the
 standard _wrename() function expects UTF-16 encoded non-ASCII filenames.

  \param[in] f the UTF-8 encoded filename to change
  \param[in] n the new UTF-8 encoded filename to set
  \return    the return value of _wrename() on Windows or rename() on other platforms.
*/
int fl_rename(const char* f, const char *n) {

#if defined (WIN32) && !defined(__CYGWIN__) // Windows

  size_t l = strlen(f);
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (xchar*)realloc(wbuf, sizeof(xchar)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  l = strlen(n);
  wn = fl_utf8toUtf16(n, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf1 = (xchar*)realloc(wbuf1, sizeof(xchar)*wn);
  wn = fl_utf8toUtf16(n, (unsigned) l, (unsigned short *)wbuf1, wn); // Convert string
  wbuf1[wn] = 0;
  return _wrename(wbuf, wbuf1);

#else

  return rename(f, n);

#endif

} // fl_rename()


/** Cross-platform function to recursively create a path in the file system.

  This function creates a \p path in the file system by recursively creating
  all directories.
*/
char fl_make_path( const char *path ) {
  if (fl_access(path, 0)) {
    const char *s = strrchr( path, '/' );
    if ( !s ) return 0;
    size_t len = (size_t) (s-path);
    char *p = (char*)malloc( len+1 );
    memcpy( p, path, len );
    p[len] = 0;
    fl_make_path( p );
    free( p );
    fl_mkdir(path, 0700);
  }
  return 1;
}

/** Cross-platform function to create a path for the file in the file system.

  This function strips the filename from the given \p path and creates
  a path in the file system by recursively creating all directories.
*/

void fl_make_path_for_file( const char *path ) {

  const char *s = strrchr( path, '/' );
  if ( !s ) return;
  size_t len =  (s-path);
  char *p = (char*)malloc( len+1 );
  memcpy( p, path, len );
  p[len] = 0;
  fl_make_path( p );
  free( p );

} // fl_make_path_for_file()

/** @} */

//
// End of "$Id$".
//
