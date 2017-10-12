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

#include <FL/Fl.H>
#include <FL/Fl_System_Driver.H>
#include <FL/filename.H>
#include <stdarg.h>
#include <FL/fl_utf8.h>
#include "utf8_internal.h"

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


/**
  Converts UTF-8 string \p s to a local multi-byte character string.
*/
char * fl_utf2mbcs(const char *s)
{
  return Fl::system_driver()->utf2mbcs(s);
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
  return Fl::system_driver()->getenv(v);
}


/** Cross-platform function to open files with a UTF-8 encoded name.

 This function is especially useful under the MSWindows platform where the
 standard open() function fails with UTF-8 encoded non-ASCII filenames.
 \param f  the UTF-8 encoded filename
 \param oflags  other arguments are as in the standard open() function
 \return  a file descriptor upon successful completion, or -1 in case of error.
 \sa fl_fopen(), fl_open_ext().
 */
int fl_open(const char* f, int oflags, ...)
{
  int pmode;
  va_list ap;
  va_start(ap, oflags);
  pmode = va_arg (ap, int);
  va_end(ap);
  return Fl::system_driver()->open(f, oflags, pmode);
}

/** Cross-platform function to open files with a UTF-8 encoded name.
 In comparison with fl_open(), this function allows to control whether
 the file is opened in binary (a.k.a. untranslated) mode. This is especially
 useful under the MSWindows platform where files are by default opened in
 text (translated) mode.
 \param fname  the UTF-8 encoded filename
 \param translation if zero, the file is to be accessed in untranslated (a.k.a. binary)
 mode.
 \param oflags,...  these arguments are as in the standard open() function.
 Setting \p oflags to zero opens the file for reading.
 \return  a file descriptor upon successful completion, or -1 in case of error.
 */
int fl_open_ext(const char* fname, int translation, int oflags, ...)
{
  int pmode;
  va_list ap;
  va_start(ap, oflags);
  pmode = va_arg (ap, int);
  va_end(ap);
  return Fl::system_driver()->open_ext(fname, translation, oflags, pmode);
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
  return Fl::system_driver()->fopen(f, mode);
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
  return Fl::system_driver()->system(cmd);
}

int fl_execvp(const char *file, char *const *argv)
{
  return Fl::system_driver()->execvp(file, argv);
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
  return Fl::system_driver()->chmod(f, mode);
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
  return Fl::system_driver()->access(f, mode);
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
  return Fl::system_driver()->stat(f, b);
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
  return Fl::system_driver()->getcwd(b, l);
}

/** Cross-platform function to unlink() (that is, delete) a file using
    a UTF-8 encoded filename.

 This function is especially useful under the MSWindows platform where the
 standard function expects UTF-16 encoded non-ASCII filenames.

  \param     f the filename to unlink
  \return    the return value of _wunlink() on Windows or unlink() on other platforms.
*/
int fl_unlink(const char* f) {
  return Fl::system_driver()->unlink(f);
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
  return Fl::system_driver()->mkdir(f, mode);
}

/** Cross-platform function to remove a directory with a UTF-8 encoded
  name.

 This function is especially useful on the MSWindows platform where the
 standard _wrmdir() function expects UTF-16 encoded non-ASCII filenames.

  \param[in] f the UTF-8 encoded filename to remove
  \return    the return value of _wrmdir() on Windows or rmdir() on other platforms.
*/
int fl_rmdir(const char* f) {
  return Fl::system_driver()->rmdir(f);
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
  return Fl::system_driver()->rename(f, n);
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


//============================================================
// this part comes from file src/fl_utf.c of FLTK 1.3
//============================================================

/*!Set to 1 to turn bad UTF-8 bytes into ISO-8859-1. If this is zero
 they are instead turned into the Unicode REPLACEMENT CHARACTER, of
 value 0xfffd.
 If this is on fl_utf8decode() will correctly map most (perhaps all)
 human-readable text that is in ISO-8859-1. This may allow you
 to completely ignore character sets in your code because virtually
 everything is either ISO-8859-1 or UTF-8.
 */
#ifndef ERRORS_TO_ISO8859_1
# define ERRORS_TO_ISO8859_1 1
#endif

/*!Set to 1 to turn bad UTF-8 bytes in the 0x80-0x9f range into the
 Unicode index for Microsoft's CP1252 character set. You should
 also set ERRORS_TO_ISO8859_1. With this a huge amount of more
 available text (such as all web pages) are correctly converted
 to Unicode.
 */
#ifndef ERRORS_TO_CP1252
# define ERRORS_TO_CP1252 1
#endif

/*!A number of Unicode code points are in fact illegal and should not
 be produced by a UTF-8 converter. Turn this on will replace the
 bytes in those encodings with errors. If you do this then converting
 arbitrary 16-bit data to UTF-8 and then back is not an identity,
 which will probably break a lot of software.
 */
#ifndef STRICT_RFC3629
# define STRICT_RFC3629 0
#endif

#if ERRORS_TO_CP1252
/* Codes 0x80..0x9f from the Microsoft CP1252 character set, translated
 * to Unicode:
 */
static unsigned short cp1252[32] = {
  0x20ac, 0x0081, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
  0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008d, 0x017d, 0x008f,
  0x0090, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
  0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0x009d, 0x017e, 0x0178
};
#endif

/*! Decode a single UTF-8 encoded character starting at \e p. The
 resulting Unicode value (in the range 0-0x10ffff) is returned,
 and \e len is set to the number of bytes in the UTF-8 encoding
 (adding \e len to \e p will point at the next character).
 
 If \p p points at an illegal UTF-8 encoding, including one that
 would go past \e end, or where a code uses more bytes than
 necessary, then *(unsigned char*)p is translated as though it is
 in the Microsoft CP1252 character set and \e len is set to 1.
 Treating errors this way allows this to decode almost any
 ISO-8859-1 or CP1252 text that has been mistakenly placed where
 UTF-8 is expected, and has proven very useful.
 
 If you want errors to be converted to error characters (as the
 standards recommend), adding a test to see if the length is
 unexpectedly 1 will work:
 
 \code
 if (*p & 0x80) {              // what should be a multibyte encoding
 code = fl_utf8decode(p,end,&len);
 if (len<2) code = 0xFFFD;   // Turn errors into REPLACEMENT CHARACTER
 } else {                      // handle the 1-byte UTF-8 encoding:
 code = *p;
 len = 1;
 }
 \endcode
 
 Direct testing for the 1-byte case (as shown above) will also
 speed up the scanning of strings where the majority of characters
 are ASCII.
 */
unsigned fl_utf8decode(const char* p, const char* end, int* len)
{
  unsigned char c = *(const unsigned char*)p;
  if (c < 0x80) {
    if (len) *len = 1;
    return c;
#if ERRORS_TO_CP1252
  } else if (c < 0xa0) {
    if (len) *len = 1;
    return cp1252[c-0x80];
#endif
  } else if (c < 0xc2) {
    goto FAIL;
  }
  if ( (end && p+1 >= end) || (p[1]&0xc0) != 0x80) goto FAIL;
  if (c < 0xe0) {
    if (len) *len = 2;
    return
    ((p[0] & 0x1f) << 6) +
    ((p[1] & 0x3f));
  } else if (c == 0xe0) {
    if (((const unsigned char*)p)[1] < 0xa0) goto FAIL;
    goto UTF8_3;
#if STRICT_RFC3629
  } else if (c == 0xed) {
    /* RFC 3629 says surrogate chars are illegal. */
    if (((const unsigned char*)p)[1] >= 0xa0) goto FAIL;
    goto UTF8_3;
  } else if (c == 0xef) {
    /* 0xfffe and 0xffff are also illegal characters */
    if (((const unsigned char*)p)[1]==0xbf &&
        ((const unsigned char*)p)[2]>=0xbe) goto FAIL;
    goto UTF8_3;
#endif
  } else if (c < 0xf0) {
  UTF8_3:
    if ( (end && p+2 >= end) || (p[2]&0xc0) != 0x80) goto FAIL;
    if (len) *len = 3;
    return
    ((p[0] & 0x0f) << 12) +
    ((p[1] & 0x3f) << 6) +
    ((p[2] & 0x3f));
  } else if (c == 0xf0) {
    if (((const unsigned char*)p)[1] < 0x90) goto FAIL;
    goto UTF8_4;
  } else if (c < 0xf4) {
  UTF8_4:
    if ( (end && p+3 >= end) || (p[2]&0xc0) != 0x80 || (p[3]&0xc0) != 0x80) goto FAIL;
    if (len) *len = 4;
#if STRICT_RFC3629
    /* RFC 3629 says all codes ending in fffe or ffff are illegal: */
    if ((p[1]&0xf)==0xf &&
        ((const unsigned char*)p)[2] == 0xbf &&
        ((const unsigned char*)p)[3] >= 0xbe) goto FAIL;
#endif
    return
    ((p[0] & 0x07) << 18) +
    ((p[1] & 0x3f) << 12) +
    ((p[2] & 0x3f) << 6) +
    ((p[3] & 0x3f));
  } else if (c == 0xf4) {
    if (((const unsigned char*)p)[1] > 0x8f) goto FAIL; /* after 0x10ffff */
    goto UTF8_4;
  } else {
  FAIL:
    if (len) *len = 1;
#if ERRORS_TO_ISO8859_1
    return c;
#else
    return 0xfffd; /* Unicode REPLACEMENT CHARACTER */
#endif
  }
}

/*! Move \p p forward until it points to the start of a UTF-8
 character. If it already points at the start of one then it
 is returned unchanged. Any UTF-8 errors are treated as though each
 byte of the error is an individual character.
 
 \e start is the start of the string and is used to limit the
 backwards search for the start of a UTF-8 character.
 
 \e end is the end of the string and is assumed to be a break
 between characters. It is assumed to be greater than p.
 
 This function is for moving a pointer that was jumped to the
 middle of a string, such as when doing a binary search for
 a position. You should use either this or fl_utf8back() depending
 on which direction your algorithm can handle the pointer
 moving. Do not use this to scan strings, use fl_utf8decode()
 instead.
 */
const char* fl_utf8fwd(const char* p, const char* start, const char* end)
{
  const char* a;
  int len;
  /* if we are not pointing at a continuation character, we are done: */
  if ((*p&0xc0) != 0x80) return p;
  /* search backwards for a 0xc0 starting the character: */
  for (a = p-1; ; --a) {
    if (a < start) return p;
    if (!(a[0]&0x80)) return p;
    if ((a[0]&0x40)) break;
  }
  fl_utf8decode(a,end,&len);
  a += len;
  if (a > p) return a;
  return p;
}

/*! Move \p p backward until it points to the start of a UTF-8
 character. If it already points at the start of one then it
 is returned unchanged. Any UTF-8 errors are treated as though each
 byte of the error is an individual character.
 
 \e start is the start of the string and is used to limit the
 backwards search for the start of a UTF-8 character.
 
 \e end is the end of the string and is assumed to be a break
 between characters. It is assumed to be greater than p.
 
 If you wish to decrement a UTF-8 pointer, pass p-1 to this.
 */
const char* fl_utf8back(const char* p, const char* start, const char* end)
{
  const char* a;
  int len;
  /* if we are not pointing at a continuation character, we are done: */
  if ((*p&0xc0) != 0x80) return p;
  /* search backwards for a 0xc0 starting the character: */
  for (a = p-1; ; --a) {
    if (a < start) return p;
    if (!(a[0]&0x80)) return p;
    if ((a[0]&0x40)) break;
  }
  fl_utf8decode(a,end,&len);
  if (a+len > p) return a;
  return p;
}

/*! Returns number of bytes that utf8encode() will use to encode the
 character \p ucs. */
int fl_utf8bytes(unsigned ucs) {
  if (ucs < 0x000080U) {
    return 1;
  } else if (ucs < 0x000800U) {
    return 2;
  } else if (ucs < 0x010000U) {
    return 3;
  } else if (ucs <= 0x10ffffU) {
    return 4;
  } else {
    return 3; /* length of the illegal character encoding */
  }
}

/*! Write the UTF-8 encoding of \e ucs into \e buf and return the
 number of bytes written. Up to 4 bytes may be written. If you know
 that \p ucs is less than 0x10000 then at most 3 bytes will be written.
 If you wish to speed this up, remember that anything less than 0x80
 is written as a single byte.
 
 If ucs is greater than 0x10ffff this is an illegal character
 according to RFC 3629. These are converted as though they are
 0xFFFD (REPLACEMENT CHARACTER).
 
 RFC 3629 also says many other values for \p ucs are illegal (in
 the range 0xd800 to 0xdfff, or ending with 0xfffe or
 0xffff). However I encode these as though they are legal, so that
 utf8encode/fl_utf8decode will be the identity for all codes between 0
 and 0x10ffff.
 */
int fl_utf8encode(unsigned ucs, char* buf) {
  if (ucs < 0x000080U) {
    buf[0] = ucs;
    return 1;
  } else if (ucs < 0x000800U) {
    buf[0] = 0xc0 | (ucs >> 6);
    buf[1] = 0x80 | (ucs & 0x3F);
    return 2;
  } else if (ucs < 0x010000U) {
    buf[0] = 0xe0 | (ucs >> 12);
    buf[1] = 0x80 | ((ucs >> 6) & 0x3F);
    buf[2] = 0x80 | (ucs & 0x3F);
    return 3;
  } else if (ucs <= 0x0010ffffU) {
    buf[0] = 0xf0 | (ucs >> 18);
    buf[1] = 0x80 | ((ucs >> 12) & 0x3F);
    buf[2] = 0x80 | ((ucs >> 6) & 0x3F);
    buf[3] = 0x80 | (ucs & 0x3F);
    return 4;
  } else {
    /* encode 0xfffd: */
    buf[0] = (char)0xef;
    buf[1] = (char)0xbf;
    buf[2] = (char)0xbd;
    return 3;
  }
}

/*! Convert a single 32-bit Unicode codepoint into an array of 16-bit
 characters. These are used by some system calls, especially on Windows.
 
 \p ucs is the value to convert.
 
 \p dst points at an array to write, and \p dstlen is the number of
 locations in this array. At most \p dstlen words will be
 written, and a 0 terminating word will be added if \p dstlen is
 large enough. Thus this function will never overwrite the buffer
 and will attempt return a zero-terminated string if space permits.
 If \p dstlen is zero then \p dst can be set to NULL and no data
 is written, but the length is returned.
 
 The return value is the number of 16-bit words that \e would be written
 to \p dst if it is large enough, not counting any terminating
 zero.
 
 If the return value is greater than \p dstlen it indicates truncation,
 you should then allocate a new array of size return+1 and call this again.
 
 Unicode characters in the range 0x10000 to 0x10ffff are converted to
 "surrogate pairs" which take two words each (in UTF-16 encoding).
 Typically, setting \p dstlen to 2 will ensure that any valid Unicode
 value can be converted, and setting \p dstlen to 3 or more will allow
 a NULL terminated sequence to be returned.
 */
unsigned fl_ucs_to_Utf16(const unsigned ucs, unsigned short *dst, const unsigned dstlen)
{
  /* The rule for direct conversion from UCS to UTF16 is:
   * - if UCS >  0x0010FFFF then UCS is invalid
   * - if UCS >= 0xD800 && UCS <= 0xDFFF UCS is invalid
   * - if UCS <= 0x0000FFFF then U16 = UCS, len = 1
   * - else
   * -- U16[0] = ((UCS - 0x00010000) >> 10) & 0x3FF + 0xD800
   * -- U16[1] = (UCS & 0x3FF) + 0xDC00
   * -- len = 2;
   */
  unsigned count;        /* Count of converted UTF16 cells */
  unsigned short u16[4]; /* Alternate buffer if dst is not set */
  unsigned short *out;   /* points to the active buffer */
  /* Ensure we have a valid buffer to write to */
  if((!dstlen) || (!dst)) {
    out = u16;
  } else {
    out = dst;
  }
  /* Convert from UCS to UTF16 */
  if((ucs > 0x0010FFFF) || /* UCS is too large */
     ((ucs > 0xD7FF) && (ucs < 0xE000))) { /* UCS in invalid range */
    out[0] = 0xFFFD; /* REPLACEMENT CHARACTER */
    count = 1;
  } else if(ucs < 0x00010000) {
    out[0] = (unsigned short)ucs;
    count = 1;
  } else if(dstlen < 2) { /* dst is too small for the result */
    out[0] = 0xFFFD; /* REPLACEMENT CHARACTER */
    count = 2;
  } else {
    out[0] = (((ucs - 0x00010000) >> 10) & 0x3FF) + 0xD800;
    out[1] = (ucs & 0x3FF) + 0xDC00;
    count = 2;
  }
  /* NULL terminate the output, if there is space */
  if(count < dstlen) { out[count] = 0; }
  return count;
} /* fl_ucs_to_Utf16 */

/*! Convert a UTF-8 sequence into an array of 16-bit characters. These
 are used by some system calls, especially on Windows.
 
 \p src points at the UTF-8, and \p srclen is the number of bytes to
 convert.
 
 \p dst points at an array to write, and \p dstlen is the number of
 locations in this array. At most \p dstlen-1 words will be
 written there, plus a 0 terminating word. Thus this function
 will never overwrite the buffer and will always return a
 zero-terminated string. If \p dstlen is zero then \p dst can be
 null and no data is written, but the length is returned.
 
 The return value is the number of 16-bit words that \e would be written
 to \p dst if it were long enough, not counting the terminating
 zero. If the return value is greater or equal to \p dstlen it
 indicates truncation, you can then allocate a new array of size
 return+1 and call this again.
 
 Errors in the UTF-8 are converted as though each byte in the
 erroneous string is in the Microsoft CP1252 encoding. This allows
 ISO-8859-1 text mistakenly identified as UTF-8 to be printed
 correctly.
 
 Unicode characters in the range 0x10000 to 0x10ffff are converted to
 "surrogate pairs" which take two words each (this is called UTF-16
 encoding).
 */
unsigned fl_utf8toUtf16(const char* src, unsigned srclen,
                        unsigned short* dst, unsigned dstlen)
{
  const char* p = src;
  const char* e = src+srclen;
  unsigned count = 0;
  if (dstlen) for (;;) {
    if (p >= e) {dst[count] = 0; return count;}
    if (!(*p & 0x80)) { /* ascii */
      dst[count] = *p++;
    } else {
      int len; unsigned ucs = fl_utf8decode(p,e,&len);
      p += len;
      if (ucs < 0x10000) {
        dst[count] = ucs;
      } else {
        /* make a surrogate pair: */
        if (count+2 >= dstlen) {dst[count] = 0; count += 2; break;}
        dst[count] = (((ucs-0x10000u)>>10)&0x3ff) | 0xd800;
        dst[++count] = (ucs&0x3ff) | 0xdc00;
      }
    }
    if (++count == dstlen) {dst[count-1] = 0; break;}
  }
  /* we filled dst, measure the rest: */
  while (p < e) {
    if (!(*p & 0x80)) p++;
    else {
      int len; unsigned ucs = fl_utf8decode(p,e,&len);
      p += len;
      if (ucs >= 0x10000) ++count;
    }
    ++count;
  }
  return count;
}


/*! Convert a UTF-8 sequence into an array of 1-byte characters.
 
 If the UTF-8 decodes to a character greater than 0xff then it is
 replaced with '?'.
 
 Errors in the UTF-8 sequence are converted as individual bytes, same as
 fl_utf8decode() does. This allows ISO-8859-1 text mistakenly identified
 as UTF-8 to be printed correctly (and possibly CP1252 on Windows).
 
 \p src points at the UTF-8 sequence, and \p srclen is the number of
 bytes to convert.
 
 Up to \p dstlen bytes are written to \p dst, including a null
 terminator. The return value is the number of bytes that would be
 written, not counting the null terminator. If greater or equal to
 \p dstlen then if you malloc a new array of size n+1 you will have
 the space needed for the entire string. If \p dstlen is zero then
 nothing is written and this call just measures the storage space
 needed.
 */
unsigned fl_utf8toa(const char* src, unsigned srclen,
                    char* dst, unsigned dstlen)
{
  const char* p = src;
  const char* e = src+srclen;
  unsigned count = 0;
  if (dstlen) for (;;) {
    unsigned char c;
    if (p >= e) {dst[count] = 0; return count;}
    c = *(const unsigned char*)p;
    if (c < 0xC2) { /* ascii or bad code */
      dst[count] = c;
      p++;
    } else {
      int len; unsigned ucs = fl_utf8decode(p,e,&len);
      p += len;
      if (ucs < 0x100) dst[count] = ucs;
      else dst[count] = '?';
    }
    if (++count >= dstlen) {dst[count-1] = 0; break;}
  }
  /* we filled dst, measure the rest: */
  while (p < e) {
    if (!(*p & 0x80)) p++;
    else {
      int len;
      fl_utf8decode(p,e,&len);
      p += len;
    }
    ++count;
  }
  return count;
}


/*! Convert an ISO-8859-1 (ie normal c-string) byte stream to UTF-8.
 
 It is possible this should convert Microsoft's CP1252 to UTF-8
 instead. This would translate the codes in the range 0x80-0x9f
 to different characters. Currently it does not do this.
 
 Up to \p dstlen bytes are written to \p dst, including a null
 terminator. The return value is the number of bytes that would be
 written, not counting the null terminator. If greater or equal to
 \p dstlen then if you malloc a new array of size n+1 you will have
 the space needed for the entire string. If \p dstlen is zero then
 nothing is written and this call just measures the storage space
 needed.
 
 \p srclen is the number of bytes in \p src to convert.
 
 If the return value equals \p srclen then this indicates that
 no conversion is necessary, as only ASCII characters are in the
 string.
 */
unsigned fl_utf8froma(char* dst, unsigned dstlen,
                      const char* src, unsigned srclen) {
  const char* p = src;
  const char* e = src+srclen;
  unsigned count = 0;
  if (dstlen) for (;;) {
    unsigned char ucs;
    if (p >= e) {dst[count] = 0; return count;}
    ucs = *(const unsigned char*)p++;
    if (ucs < 0x80U) {
      dst[count++] = ucs;
      if (count >= dstlen) {dst[count-1] = 0; break;}
    } else { /* 2 bytes (note that CP1252 translate could make 3 bytes!) */
      if (count+2 >= dstlen) {dst[count] = 0; count += 2; break;}
      dst[count++] = 0xc0 | (ucs >> 6);
      dst[count++] = 0x80 | (ucs & 0x3F);
    }
  }
  /* we filled dst, measure the rest: */
  while (p < e) {
    unsigned char ucs = *(const unsigned char*)p++;
    if (ucs < 0x80U) {
      count++;
    } else {
      count += 2;
    }
  }
  return count;
}


/*! Examines the first \p srclen bytes in \p src and returns a verdict
 on whether it is UTF-8 or not.
 - Returns 0 if there is any illegal UTF-8 sequences, using the
 same rules as fl_utf8decode(). Note that some UCS values considered
 illegal by RFC 3629, such as 0xffff, are considered legal by this.
 - Returns 1 if there are only single-byte characters (ie no bytes
 have the high bit set). This is legal UTF-8, but also indicates
 plain ASCII. It also returns 1 if \p srclen is zero.
 - Returns 2 if there are only characters less than 0x800.
 - Returns 3 if there are only characters less than 0x10000.
 - Returns 4 if there are characters in the 0x10000 to 0x10ffff range.
 
 Because there are many illegal sequences in UTF-8, it is almost
 impossible for a string in another encoding to be confused with
 UTF-8. This is very useful for transitioning Unix to UTF-8
 filenames, you can simply test each filename with this to decide
 if it is UTF-8 or in the locale encoding. My hope is that if
 this is done we will be able to cleanly transition to a locale-less
 encoding.
 */
int fl_utf8test(const char* src, unsigned srclen) {
  int ret = 1;
  const char* p = src;
  const char* e = src+srclen;
  while (p < e) {
    if (*p & 0x80) {
      int len; fl_utf8decode(p,e,&len);
      if (len < 2) return 0;
      if (len > ret) ret = len;
      p += len;
    } else {
      p++;
    }
  }
  return ret;
}

/* forward declare mk_wcwidth() as static so the name is not visible.
 */
static int mk_wcwidth(unsigned int ucs);

/* include the c source directly so its contents are only visible here
 */
#include "xutf8/mk_wcwidth.c"

/** wrapper to adapt Markus Kuhn's implementation of wcwidth() for FLTK
 \param [in] ucs Unicode character value
 \returns width of character in columns
 
 See http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c for Markus Kuhn's
 original implementation of wcwidth() and wcswidth()
 (defined in IEEE Std 1002.1-2001) for Unicode.
 
 \b WARNING: this function returns widths for "raw" Unicode characters.
 It does not even try to map C1 control characters (0x80 to 0x9F) to
 CP1252, and C0/C1 control characters and DEL will return -1.
 You are advised to use fl_width(const char* src) instead.
 */
int fl_wcwidth_(unsigned int ucs) {
  return mk_wcwidth(ucs);
}

/** extended wrapper around  fl_wcwidth_(unsigned int ucs) function.
 \param[in] src pointer to start of UTF-8 byte sequence
 \returns width of character in columns
 
 Depending on build options, this function may map C1 control
 characters (0x80 to 0x9f) to CP1252, and return the width of
 that character instead. This is not the same behaviour as
 fl_wcwidth_(unsigned int ucs) .
 
 Note that other control characters and DEL will still return -1,
 so if you want different behaviour, you need to test for those
 characters before calling fl_wcwidth(), and handle them separately.
 */
int fl_wcwidth(const char* src) {
  int len = fl_utf8len(*src);
  int ret = 0;
  unsigned int ucs = fl_utf8decode(src, src+len, &ret);
  int width = fl_wcwidth_(ucs);
  return width;
}

/**
 Converts a UTF-8 string into a wide character string.
 
 This function generates 32-bit wchar_t (e.g. "ucs4" as it were) except
 on Windows where it is equivalent to fl_utf8toUtf16 and returns
 UTF-16.
 
 \p src points at the UTF-8, and \p srclen is the number of bytes to
 convert.
 
 \p dst points at an array to write, and \p dstlen is the number of
 locations in this array. At most \p dstlen-1 wchar_t will be
 written there, plus a 0 terminating wchar_t.
 
 The return value is the number of wchar_t that \e would be written
 to \p dst if it were long enough, not counting the terminating
 zero. If the return value is greater or equal to \p dstlen it
 indicates truncation, you can then allocate a new array of size
 return+1 and call this again.
 
 Notice that sizeof(wchar_t) is 2 on Windows and is 4 on Linux
 and most other systems. Where wchar_t is 16 bits, Unicode
 characters in the range 0x10000 to 0x10ffff are converted to
 "surrogate pairs" which take two words each (this is called UTF-16
 encoding). If wchar_t is 32 bits this rather nasty problem is
 avoided.
 
 Note that Windows includes Cygwin, i.e. compiled with Cygwin's POSIX
 layer (cygwin1.dll, --enable-cygwin), either native (GDI) or X11.
 */
unsigned fl_utf8towc(const char* src, unsigned srclen,
                     wchar_t* dst, unsigned dstlen)
{
  return Fl::system_driver()->utf8towc(src, srclen, dst, dstlen);
}


/*! Turn "wide characters" as returned by some system calls
 (especially on Windows) into UTF-8.
 
 Up to \p dstlen bytes are written to \p dst, including a null
 terminator. The return value is the number of bytes that would be
 written, not counting the null terminator. If greater or equal to
 \p dstlen then if you malloc a new array of size n+1 you will have
 the space needed for the entire string. If \p dstlen is zero then
 nothing is written and this call just measures the storage space
 needed.
 
 \p srclen is the number of words in \p src to convert. On Windows
 this is not necessarily the number of characters, due to there
 possibly being "surrogate pairs" in the UTF-16 encoding used.
 On Unix wchar_t is 32 bits and each location is a character.
 
 On Unix if a \p src word is greater than 0x10ffff then this is an
 illegal character according to RFC 3629. These are converted as
 though they are 0xFFFD (REPLACEMENT CHARACTER). Characters in the
 range 0xd800 to 0xdfff, or ending with 0xfffe or 0xffff are also
 illegal according to RFC 3629. However I encode these as though
 they are legal, so that fl_utf8towc will return the original data.
 
 On Windows "surrogate pairs" are converted to a single character
 and UTF-8 encoded (as 4 bytes). Mismatched halves of surrogate
 pairs are converted as though they are individual characters.
 */
unsigned fl_utf8fromwc(char* dst, unsigned dstlen, const wchar_t* src, unsigned srclen)
{
  return Fl::system_driver()->utf8fromwc(dst, dstlen, src, srclen);
}


/*! Return true if the "locale" seems to indicate that UTF-8 encoding
 is used. If true the fl_utf8to_mb and fl_utf8from_mb don't do anything
 useful.
 
 <i>It is highly recommended that you change your system so this
 does return true.</i> On Windows this is done by setting the
 "codepage" to CP_UTF8.  On Unix this is done by setting $LC_CTYPE
 to a string containing the letters "utf" or "UTF" in it, or by
 deleting all $LC* and $LANG environment variables. In the future
 it is likely that all non-Asian Unix systems will return true,
 due to the compatibility of UTF-8 with ISO-8859-1.
 */
int fl_utf8locale()
{
  return Fl::system_driver()->utf8locale();
}


/*! Convert the UTF-8 used by FLTK to the locale-specific encoding
 used for filenames (and sometimes used for data in files).
 Unfortunately due to stupid design you will have to do this as
 needed for filenames. This is a bug on both Unix and Windows.
 
 Up to \p dstlen bytes are written to \p dst, including a null
 terminator. The return value is the number of bytes that would be
 written, not counting the null terminator. If greater or equal to
 \p dstlen then if you malloc a new array of size n+1 you will have
 the space needed for the entire string. If \p dstlen is zero then
 nothing is written and this call just measures the storage space
 needed.
 
 If fl_utf8locale() returns true then this does not change the data.
 */
unsigned fl_utf8to_mb(const char* src, unsigned srclen, char* dst, unsigned dstlen) {
  if (fl_utf8locale()) {
    /* identity transform: */
    if (srclen < dstlen) {
      memcpy(dst, src, srclen);
      dst[srclen] = 0;
    } else {
      /* Buffer insufficent or buffer query */
    }
    return srclen;
  }
  return Fl::system_driver()->utf8to_mb(src, srclen, dst, dstlen);
}


/*! Convert a filename from the locale-specific multibyte encoding
 used by Windows to UTF-8 as used by FLTK.
 
 Up to \p dstlen bytes are written to \p dst, including a null
 terminator. The return value is the number of bytes that would be
 written, not counting the null terminator. If greater or equal to
 \p dstlen then if you malloc a new array of size n+1 you will have
 the space needed for the entire string. If \p dstlen is zero then
 nothing is written and this call just measures the storage space
 needed.
 
 On Unix or on Windows when a UTF-8 locale is in effect, this
 does not change the data.
 You may also want to check if fl_utf8test() returns non-zero, so that
 the filesystem can store filenames in UTF-8 encoding regardless of
 the locale.
 */
unsigned fl_utf8from_mb(char* dst, unsigned dstlen, const char* src, unsigned srclen) {
  if (fl_utf8locale()) {
    /* identity transform: */
    if (srclen < dstlen) {
      memcpy(dst, src, srclen);
      dst[srclen] = 0;
    } else {
      /* Buffer insufficent or buffer query */
    }
    return srclen;
  }
  return Fl::system_driver()->utf8from_mb(dst, dstlen, src, srclen);
}

//============================================================
// end of the part from file src/fl_utf.c of FLTK 1.3
//============================================================

/** @} */

//
// End of "$Id$".
//
