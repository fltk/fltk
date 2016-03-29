//
// "$Id$"
//
// Unicode to UTF-8 conversion functions.
//
// Author: Jean-Marc Lienher ( http://oksid.ch )
// Copyright 2000-2010 by O'ksi'D.
// Copyright 2016 by Bill Spitzak and others.
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

#include <FL/Fl_System_Driver.H>
#include <FL/filename.H>
#include <stdarg.h>
#include <FL/fl_utf8.h>
#include <string.h>
#include <stdlib.h>

#undef fl_open
extern "C" {
  int XUtf8Tolower(int ucs); // in src/xutf8/case.c
  unsigned short XUtf8IsNonSpacing(unsigned int ucs); // in src/xutf8/is_spacing.c
}


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


/**
  Converts UTF-8 string \p s to a local multi-byte character string.
*/
char * fl_utf2mbcs(const char *s)
{
  return Fl_System_Driver::driver()->utf2mbcs(s);
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
  return Fl_System_Driver::driver()->getenv(v);
}


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
  return Fl_System_Driver::driver()->open(f, oflags, pmode);
}


/** Cross-platform function to open files with a UTF-8 encoded name.

  This function is especially useful under the MSWindows platform where the
  standard fopen() function fails with UTF-8 encoded non-ASCII filenames.
  \param f  the UTF-8 encoded filename
  \param mode  same as the second argument of the standard fopen() function
  \return  a FILE pointer upon successful completion, or NULL in case of error.
  \sa fl_open().
*/
FILE *fl_fopen(const char* f, const char *mode) {
  return Fl_System_Driver::driver()->fopen(f, mode);
}

/** Cross-platform function to run a system command with a UTF-8 encoded string.

  This function is especially useful under the MSWindows platform where
  non-ASCII program (file) names must be encoded as wide characters.

  On platforms other than MSWindows this function calls system() directly.

  \param[in] cmd the UTF-8 encoded command string
  \return the return value of _wsystem() on Windows or system() on other platforms.
*/

int fl_system(const char* cmd)
{
  return Fl_System_Driver::driver()->system(cmd);
}

int fl_execvp(const char *file, char *const *argv)
{
  return Fl_System_Driver::driver()->execvp(file, argv);
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
  return Fl_System_Driver::driver()->chmod(f, mode);
}

/** Cross-platform function to test a files access() with a UTF-8 encoded
  name or value.

 This function is especially useful under the MSWindows platform where the
 standard access() function fails with UTF-8 encoded non-ASCII filenames.

  \param[in] f the UTF-8 encoded filename
  \param[in] mode the mode to test
  \return    the return value of _waccess() on Windows or access() on other platforms.
*/
int fl_access(const char* f, int mode) {
  return Fl_System_Driver::driver()->access(f, mode);
}

/** Cross-platform function to stat() a file using a UTF-8 encoded
  name or value.

 This function is especially useful under the MSWindows platform where the
 standard stat() function fails with UTF-8 encoded non-ASCII filenames.

  \param[in] f the UTF-8 encoded filename
  \param     b the stat struct to populate
  \return    the return value of _wstat() on Windows or stat() on other platforms.
*/
int fl_stat(const char* f, struct stat *b) {
  return Fl_System_Driver::driver()->stat(f, b);
}

// TODO: add fl_chdir if we have fl_getcwd

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
  return Fl_System_Driver::driver()->getcwd(b, l);
}

/** Cross-platform function to unlink() (that is, delete) a file using
    a UTF-8 encoded filename.

 This function is especially useful under the MSWindows platform where the
 standard function expects UTF-16 encoded non-ASCII filenames.

  \param     f the filename to unlink
  \return    the return value of _wunlink() on Windows or unlink() on other platforms.
*/
int fl_unlink(const char* f) {
  return Fl_System_Driver::driver()->unlink(f);
}

/** Cross-platform function to create a directory with a UTF-8 encoded
  name.

 This function is especially useful on the MSWindows platform where the
 standard _wmkdir() function expects UTF-16 encoded non-ASCII filenames.

  \param[in] f the UTF-8 encoded filename
  \param[in] mode the mode of the directory
  \return    the return value of _wmkdir() on Windows or mkdir() on other platforms.
*/
int fl_mkdir(const char* f, int mode) {
  return Fl_System_Driver::driver()->mkdir(f, mode);
}

/** Cross-platform function to remove a directory with a UTF-8 encoded
  name.

 This function is especially useful on the MSWindows platform where the
 standard _wrmdir() function expects UTF-16 encoded non-ASCII filenames.

  \param[in] f the UTF-8 encoded filename to remove
  \return    the return value of _wrmdir() on Windows or rmdir() on other platforms.
*/
int fl_rmdir(const char* f) {
  return Fl_System_Driver::driver()->rmdir(f);
}

/** Cross-platform function to rename a filesystem object using
    UTF-8 encoded names.

 This function is especially useful on the MSWindows platform where the
 standard _wrename() function expects UTF-16 encoded non-ASCII filenames.

  \param[in] f the UTF-8 encoded filename to change
  \param[in] n the new UTF-8 encoded filename to set
  \return    the return value of _wrename() on Windows or rename() on other platforms.
*/
int fl_rename(const char* f, const char *n) {
  return Fl_System_Driver::driver()->rename(f, n);
}

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
