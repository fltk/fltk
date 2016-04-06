//
// "$Id$"
//
// Definition of Apple Darwin system driver.
//
// Copyright 1998-2016 by Bill Spitzak and others.
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


#include "../../config_lib.h"
#include "Fl_WinAPI_System_Driver.H"
#include <FL/Fl.H>
#include <FL/fl_utf8.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <wchar.h>
#include <process.h>
#include <locale.h>

#if !defined(FL_DOXYGEN)
const char* fl_local_alt   = "Alt";
const char* fl_local_ctrl  = "Ctrl";
const char* fl_local_meta  = "Meta";
const char* fl_local_shift = "Shift";
#endif

static wchar_t *mbwbuf = NULL;
static wchar_t *wbuf = NULL;
static wchar_t *wbuf1 = NULL;

/**
 Creates a driver that manages all screen and display related calls.
 
 This function must be implemented once for every platform.
 */
Fl_System_Driver *Fl_System_Driver::newSystemDriver()
{
  return new Fl_WinAPI_System_Driver();
}

void Fl_WinAPI_System_Driver::warning(const char *format, va_list args) {
  // Show nothing for warnings under WIN32...
}

void Fl_WinAPI_System_Driver::error(const char *format, va_list args) {
  char buf[1024];
  vsnprintf(buf, 1024, format, args);
  MessageBox(0,buf,"Error",MB_ICONEXCLAMATION|MB_SYSTEMMODAL);
}

void Fl_WinAPI_System_Driver::fatal(const char *format, va_list args) {
  char buf[1024];
  vsnprintf(buf, 1024, format, args);
  MessageBox(0,buf,"Error",MB_ICONSTOP|MB_SYSTEMMODAL);
  ::exit(1);
}

char *Fl_WinAPI_System_Driver::utf2mbcs(const char *s) {
  if (!s) return NULL;
  size_t l = strlen(s);
  static char *buf = NULL;
  
  unsigned wn = fl_utf8toUtf16(s, (unsigned) l, NULL, 0) + 7; // Query length
  mbwbuf = (wchar_t*)realloc(mbwbuf, sizeof(wchar_t)*wn);
  l = fl_utf8toUtf16(s, (unsigned) l, (unsigned short *)mbwbuf, wn); // Convert string
  mbwbuf[l] = 0;
  
  buf = (char*)realloc(buf, (unsigned) (l * 6 + 1));
  l = (unsigned) wcstombs(buf, mbwbuf, (unsigned) l * 6);
  buf[l] = 0;
  return buf;
}

char *Fl_WinAPI_System_Driver::getenv(const char* v) {
  size_t l =  strlen(v);
  unsigned wn = fl_utf8toUtf16(v, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (wchar_t*)realloc(wbuf, sizeof(wchar_t)*wn);
  wn = fl_utf8toUtf16(v, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  wchar_t *ret = _wgetenv(wbuf);
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
}

int Fl_WinAPI_System_Driver::open(const char* f, int oflags, int pmode) {
  unsigned l = (unsigned) strlen(f);
  unsigned wn = fl_utf8toUtf16(f, l, NULL, 0) + 1; // Query length
  wbuf = (wchar_t*)realloc(wbuf, sizeof(wchar_t)*wn);
  wn = fl_utf8toUtf16(f, l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  if (pmode == -1) return _wopen(wbuf, oflags);
  else return _wopen(wbuf, oflags, pmode);
}

FILE *Fl_WinAPI_System_Driver::fopen(const char* f, const char *mode) {
  size_t l = strlen(f);
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (wchar_t*)realloc(wbuf, sizeof(wchar_t)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  l = strlen(mode);
  wn = fl_utf8toUtf16(mode, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf1 = (wchar_t*)realloc(wbuf1, sizeof(wchar_t)*wn);
  wn = fl_utf8toUtf16(mode, (unsigned) l, (unsigned short *)wbuf1, wn); // Convert string
  wbuf1[wn] = 0;
  return _wfopen(wbuf, wbuf1);
}

int Fl_WinAPI_System_Driver::system(const char* cmd) {
# ifdef __MINGW32__
  return system(fl_utf2mbcs(cmd));
# else
  size_t l = strlen(cmd);
  unsigned wn = fl_utf8toUtf16(cmd, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (wchar_t*)realloc(wbuf, sizeof(wchar_t)*wn);
  wn = fl_utf8toUtf16(cmd, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return _wsystem(wbuf);
# endif
}

int Fl_WinAPI_System_Driver::execvp(const char *file, char *const *argv) {
# ifdef __MINGW32__
  return _execvp(fl_utf2mbcs(file), argv);
# else
  size_t l = strlen(file);
  int i, n;
  wchar_t **ar;
  unsigned wn = fl_utf8toUtf16(file, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (wchar_t*)realloc(wbuf, sizeof(wchar_t)*wn);
  wn = fl_utf8toUtf16(file, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  
  i = 0; n = 0;
  while (argv[i]) {i++; n++;}
  ar = (wchar_t**) malloc(sizeof(wchar_t*) * (n + 1));
  i = 0;
  while (i <= n) {
    unsigned wn;
    l = strlen(argv[i]);
    wn = fl_utf8toUtf16(argv[i], (unsigned) l, NULL, 0) + 1; // Query length
    ar[i] = (wchar_t *)malloc(sizeof(wchar_t)*wn);
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
#endif
}

int Fl_WinAPI_System_Driver::chmod(const char* f, int mode) {
  size_t l = strlen(f);
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (wchar_t*)realloc(wbuf, sizeof(wchar_t)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return _wchmod(wbuf, mode);
}

int Fl_WinAPI_System_Driver::access(const char* f, int mode) {
  size_t l = strlen(f);
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (wchar_t*)realloc(wbuf, sizeof(wchar_t)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return _waccess(wbuf, mode);
}

int Fl_WinAPI_System_Driver::stat(const char* f, struct stat *b) {
  size_t l = strlen(f);
  if (f[l-1] == '/') l--; // must remove trailing /
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (wchar_t*)realloc(wbuf, sizeof(wchar_t)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return _wstat(wbuf, (struct _stat*)b);
}

char *Fl_WinAPI_System_Driver::getcwd(char* b, int l) {
  static wchar_t *wbuf = NULL;
  wbuf = (wchar_t*)realloc(wbuf, sizeof(wchar_t) * (l+1));
  wchar_t *ret = _wgetcwd(wbuf, l);
  if (ret) {
    unsigned dstlen = l;
    l = (int) wcslen(wbuf);
    dstlen = fl_utf8fromwc(b, dstlen, wbuf, (unsigned) l);
    b[dstlen] = 0;
    return b;
  } else {
    return NULL;
  }
}

int Fl_WinAPI_System_Driver::unlink(const char* f) {
  size_t l = strlen(f);
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (wchar_t*)realloc(wbuf, sizeof(wchar_t)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return _wunlink(wbuf);
}

int Fl_WinAPI_System_Driver::mkdir(const char* f, int mode) {
  size_t l = strlen(f);
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (wchar_t*)realloc(wbuf, sizeof(wchar_t)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return _wmkdir(wbuf);
}

int Fl_WinAPI_System_Driver::rmdir(const char* f) {
  size_t l = strlen(f);
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (wchar_t*)realloc(wbuf, sizeof(wchar_t)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return _wrmdir(wbuf);
}

int Fl_WinAPI_System_Driver::rename(const char* f, const char *n) {
  size_t l = strlen(f);
  unsigned wn = fl_utf8toUtf16(f, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf = (wchar_t*)realloc(wbuf, sizeof(wchar_t)*wn);
  wn = fl_utf8toUtf16(f, (unsigned) l, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  l = strlen(n);
  wn = fl_utf8toUtf16(n, (unsigned) l, NULL, 0) + 1; // Query length
  wbuf1 = (wchar_t*)realloc(wbuf1, sizeof(wchar_t)*wn);
  wn = fl_utf8toUtf16(n, (unsigned) l, (unsigned short *)wbuf1, wn); // Convert string
  wbuf1[wn] = 0;
  return _wrename(wbuf, wbuf1);
}

// Two WIN32-specific functions fl_utf8_to_locale() and fl_locale_to_utf8()
// from file fl_utf8.cxx are put here for API compatibility

static char *buf = NULL;
static int buf_len = 0;
static unsigned short *wbufa = NULL;
unsigned int fl_codepage = 0;


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
  l = fl_utf8fromwc(buf, buf_len, (wchar_t*)wbufa, l);
  buf[l] = 0;
  return buf;
}

///////////////////////////////////

unsigned Fl_WinAPI_System_Driver::utf8towc(const char* src, unsigned srclen, wchar_t* dst, unsigned dstlen) {
  return fl_utf8toUtf16(src, srclen, (unsigned short*)dst, dstlen);
}

unsigned Fl_WinAPI_System_Driver::utf8fromwc(char* dst, unsigned dstlen, const wchar_t* src, unsigned srclen)
{
  unsigned i = 0;
  unsigned count = 0;
  if (dstlen) for (;;) {
    unsigned ucs;
    if (i >= srclen) {
      dst[count] = 0;
      return count;
    }
    ucs = src[i++];
    if (ucs < 0x80U) {
      dst[count++] = ucs;
      if (count >= dstlen) {dst[count-1] = 0; break;}
    } else if (ucs < 0x800U) { /* 2 bytes */
      if (count+2 >= dstlen) {dst[count] = 0; count += 2; break;}
      dst[count++] = 0xc0 | (ucs >> 6);
      dst[count++] = 0x80 | (ucs & 0x3F);
    } else if (ucs >= 0xd800 && ucs <= 0xdbff && i < srclen &&
               src[i] >= 0xdc00 && src[i] <= 0xdfff) {
      /* surrogate pair */
      unsigned ucs2 = src[i++];
      ucs = 0x10000U + ((ucs&0x3ff)<<10) + (ucs2&0x3ff);
      /* all surrogate pairs turn into 4-byte UTF-8 */
      if (count+4 >= dstlen) {dst[count] = 0; count += 4; break;}
      dst[count++] = 0xf0 | (ucs >> 18);
      dst[count++] = 0x80 | ((ucs >> 12) & 0x3F);
      dst[count++] = 0x80 | ((ucs >> 6) & 0x3F);
      dst[count++] = 0x80 | (ucs & 0x3F);
    } else {
      /* all others are 3 bytes: */
      if (count+3 >= dstlen) {dst[count] = 0; count += 3; break;}
      dst[count++] = 0xe0 | (ucs >> 12);
      dst[count++] = 0x80 | ((ucs >> 6) & 0x3F);
      dst[count++] = 0x80 | (ucs & 0x3F);
    }
  }
  /* we filled dst, measure the rest: */
  while (i < srclen) {
    unsigned ucs = src[i++];
    if (ucs < 0x80U) {
      count++;
    } else if (ucs < 0x800U) { /* 2 bytes */
      count += 2;
    } else if (ucs >= 0xd800 && ucs <= 0xdbff && i < srclen-1 &&
               src[i+1] >= 0xdc00 && src[i+1] <= 0xdfff) {
      /* surrogate pair */
      ++i;
      count += 4;
    } else {
      count += 3;
    }
  }
  return count;
}

int Fl_WinAPI_System_Driver::utf8locale()
{
  static int ret = 2;
  if (ret == 2) {
    ret = GetACP() == CP_UTF8;
  }
  return ret;
}

unsigned Fl_WinAPI_System_Driver::utf8to_mb(const char* src, unsigned srclen, char* dst, unsigned dstlen) {
  wchar_t lbuf[1024];
  wchar_t* buf = lbuf;
  unsigned length = fl_utf8towc(src, srclen, buf, 1024);
  unsigned ret;
  if (length >= 1024) {
    buf = (wchar_t*)(malloc((length+1)*sizeof(wchar_t)));
    fl_utf8towc(src, srclen, buf, length+1);
  }
  if (dstlen) {
    // apparently this does not null-terminate, even though msdn documentation claims it does:
    ret =
    WideCharToMultiByte(GetACP(), 0, buf, length, dst, dstlen, 0, 0);
    dst[ret] = 0;
  }
  // if it overflows or measuring length, get the actual length:
  if (dstlen==0 || ret >= dstlen-1)
    ret = WideCharToMultiByte(GetACP(), 0, buf, length, 0, 0, 0, 0);
  if (buf != lbuf) free(buf);
  return ret;
}

unsigned Fl_WinAPI_System_Driver::utf8from_mb(char* dst, unsigned dstlen, const char* src, unsigned srclen) {
  wchar_t lbuf[1024];
  wchar_t* buf = lbuf;
  unsigned length;
  unsigned ret;
  length = MultiByteToWideChar(GetACP(), 0, src, srclen, buf, 1024);
  if ((length == 0)&&(GetLastError()==ERROR_INSUFFICIENT_BUFFER)) {
    length = MultiByteToWideChar(GetACP(), 0, src, srclen, 0, 0);
    buf = (wchar_t*)(malloc(length*sizeof(wchar_t)));
    MultiByteToWideChar(GetACP(), 0, src, srclen, buf, length);
  }
  ret = fl_utf8fromwc(dst, dstlen, buf, length);
  if (buf != lbuf) free((void*)buf);
  return ret;
}

int Fl_WinAPI_System_Driver::clocale_printf(FILE *output, const char *format, va_list args) {
  char *saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  int retval = vfprintf(output, format, args);
  setlocale(LC_NUMERIC, saved_locale);
  return retval;
}

//
// End of "$Id$".
//
