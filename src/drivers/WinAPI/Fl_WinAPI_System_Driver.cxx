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
#include <FL/filename.H>
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_File_Icon.H>
#include "../../flstring.h"
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <rpc.h>
#include <sys/types.h>
#include <sys/stat.h>
// function pointer for the UuidCreate Function
// RPC_STATUS RPC_ENTRY UuidCreate(UUID __RPC_FAR *Uuid);
typedef RPC_STATUS (WINAPI* uuid_func)(UUID __RPC_FAR *Uuid);
#include <shellapi.h>
#include <wchar.h>
#include <process.h>
#include <locale.h>
#include <time.h>
#include <direct.h>
#include <io.h>
// Apparently Borland C++ defines DIRECTORY in <direct.h>, which
// interfers with the Fl_File_Icon enumeration of the same name.
#  ifdef DIRECTORY
#    undef DIRECTORY
#  endif // DIRECTORY
#ifdef __CYGWIN__
#  include <mntent.h>
#endif

inline int isdirsep(char c) { return c == '/' || c == '\\'; }

#if !defined(FL_DOXYGEN)
const char* fl_local_alt   = "Alt";
const char* fl_local_ctrl  = "Ctrl";
const char* fl_local_meta  = "Meta";
const char* fl_local_shift = "Shift";
#endif

static wchar_t *mbwbuf = NULL;
static wchar_t *wbuf = NULL;
static wchar_t *wbuf1 = NULL;

extern "C" {
  int fl_scandir(const char *dirname, struct dirent ***namelist,
                 int (*select)(struct dirent *),
                 int (*compar)(struct dirent **, struct dirent **));
}

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

int Fl_WinAPI_System_Driver::filename_list(const char *d, dirent ***list, int (*sort)(struct dirent **, struct dirent **) ) {
  // For Windows we have a special scandir implementation that uses
  // the Win32 "wide" functions for lookup, avoiding the code page mess
  // entirely. It also fixes up the trailing '/'.
  return fl_scandir(d, list, 0, sort);
}

int Fl_WinAPI_System_Driver::filename_expand(char *to,int tolen, const char *from) {
  char *temp = new char[tolen];
  strlcpy(temp,from, tolen);
  char *start = temp;
  char *end = temp+strlen(temp);
  int ret = 0;
  for (char *a=temp; a<end; ) {	// for each slash component
    char *e; for (e=a; e<end && !isdirsep(*e); e++) {/*empty*/} // find next slash
    const char *value = 0; // this will point at substitute value
    switch (*a) {
      case '~':	// a home directory name
        if (e <= a+1) {	// current user's directory
          value = getenv("HOME");
        }
        break;
      case '$':		/* an environment variable */
      {char t = *e; *(char *)e = 0; value = getenv(a+1); *(char *)e = t;}
        break;
    }
    if (value) {
      // substitutions that start with slash delete everything before them:
      if (isdirsep(value[0])) start = a;
      // also if it starts with "A:"
      if (value[0] && value[1]==':') start = a;
      int t = (int) strlen(value); if (isdirsep(value[t-1])) t--;
      if ((end+1-e+t) >= tolen) end += tolen - (end+1-e+t);
      memmove(a+t, e, end+1-e);
      end = a+t+(end-e);
      *end = '\0';
      memcpy(a, value, t);
      ret++;
    } else {
      a = e+1;
      if (*e == '\\') {*e = '/'; ret++;} // ha ha!
    }
  }
  strlcpy(to, start, tolen);
  delete[] temp;
  return ret;
}

int                                                     // O - 0 if no change, 1 if changed
Fl_WinAPI_System_Driver::filename_relative(char *to,	// O - Relative filename
                                    int        tolen,   // I - Size of "to" buffer
                                    const char *from,   // I - Absolute filename
                                    const char *base)   // I - Find path relative to this path
{
  char          *newslash;		// Directory separator
  const char	*slash;			// Directory separator
  char          *cwd = 0L, *cwd_buf = 0L;
  if (base) cwd = cwd_buf = strdup(base);
  
  // return if "from" is not an absolute path
  if (from[0] == '\0' ||
      (!isdirsep(*from) && !isalpha(*from) && from[1] != ':' &&
       !isdirsep(from[2]))) {
        strlcpy(to, from, tolen);
        if (cwd_buf) free(cwd_buf);
        return 0;
      }
  
  // return if "cwd" is not an absolute path
  if (!cwd || cwd[0] == '\0' ||
      (!isdirsep(*cwd) && !isalpha(*cwd) && cwd[1] != ':' &&
       !isdirsep(cwd[2]))) {
        strlcpy(to, from, tolen);
        if (cwd_buf) free(cwd_buf);
        return 0;
      }
  
  // convert all backslashes into forward slashes
  for (newslash = strchr(cwd, '\\'); newslash; newslash = strchr(newslash + 1, '\\'))
    *newslash = '/';
  
  // test for the exact same string and return "." if so
  if (!strcasecmp(from, cwd)) {
    strlcpy(to, ".", tolen);
    free(cwd_buf);
    return (1);
  }
  
  // test for the same drive. Return the absolute path if not
  if (tolower(*from & 255) != tolower(*cwd & 255)) {
    // Not the same drive...
    strlcpy(to, from, tolen);
    free(cwd_buf);
    return 0;
  }
  
  // compare the path name without the drive prefix
  from += 2; cwd += 2;
  
  // compare both path names until we find a difference
  for (slash = from, newslash = cwd;
       *slash != '\0' && *newslash != '\0';
       slash ++, newslash ++)
    if (isdirsep(*slash) && isdirsep(*newslash)) continue;
    else if (tolower(*slash & 255) != tolower(*newslash & 255)) break;
  
  // skip over trailing slashes
  if ( *newslash == '\0' && *slash != '\0' && !isdirsep(*slash)
      &&(newslash==cwd || !isdirsep(newslash[-1])) )
    newslash--;
  
  // now go back to the first character of the first differing paths segment
  while (!isdirsep(*slash) && slash > from) slash --;
  if (isdirsep(*slash)) slash ++;
  
  // do the same for the current dir
  if (isdirsep(*newslash)) newslash --;
  if (*newslash != '\0')
    while (!isdirsep(*newslash) && newslash > cwd) newslash --;
  
  // prepare the destination buffer
  to[0]         = '\0';
  to[tolen - 1] = '\0';
  
  // now add a "previous dir" sequence for every following slash in the cwd
  while (*newslash != '\0') {
    if (isdirsep(*newslash)) strlcat(to, "../", tolen);
    newslash ++;
  }
  
  // finally add the differing path from "from"
  strlcat(to, slash, tolen);
  
  free(cwd_buf);
  return 1;
}

int Fl_WinAPI_System_Driver::filename_absolute(char *to, int tolen, const char *from) {
  if (isdirsep(*from) || *from == '|' || from[1]==':') {
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
  for (a = temp; *a; a++) if (*a=='\\') *a = '/'; // ha ha
  if (isdirsep(*(a-1))) a--;
  /* remove intermediate . and .. names: */
  while (*start == '.') {
    if (start[1]=='.' && isdirsep(start[2])) {
      char *b;
      for (b = a-1; b >= temp && !isdirsep(*b); b--) {/*empty*/}
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

int Fl_WinAPI_System_Driver::filename_isdir(const char* n)
{
  struct _stat	s;
  char		fn[FL_PATH_MAX];
  int		length;
  length = (int) strlen(n);
  // This workaround brought to you by the fine folks at Microsoft!
  // (read lots of sarcasm in that...)
  if (length < (int)(sizeof(fn) - 1)) {
    if (length < 4 && isalpha(n[0]) && n[1] == ':' &&
        (isdirsep(n[2]) || !n[2])) {
      // Always use D:/ for drive letters
      fn[0] = n[0];
      strcpy(fn + 1, ":/");
      n = fn;
    } else if (length > 0 && isdirsep(n[length - 1])) {
      // Strip trailing slash from name...
      length --;
      memcpy(fn, n, length);
      fn[length] = '\0';
      n = fn;
    }
  }
  return !_stat(n, &s) && (s.st_mode & _S_IFDIR);
}

int Fl_WinAPI_System_Driver::filename_isdir_quick(const char* n)
{
  // Do a quick optimization for filenames with a trailing slash...
  if (*n && isdirsep(n[strlen(n) - 1])) return 1;
  return filename_isdir(n);
}

const char *Fl_WinAPI_System_Driver::filename_ext(const char *buf) {
  const char *q = 0;
  const char *p = buf;
  for (p = buf; *p; p++) {
    if (isdirsep(*p) ) q = 0;
    else if (*p == '.') q = p;
  }
  return q ? q : p;
}

int Fl_WinAPI_System_Driver::open_uri(const char *uri, char *msg, int msglen)
{
  if (msg) snprintf(msg, msglen, "open %s", uri);  
  return (int)(ShellExecute(HWND_DESKTOP, "open", uri, NULL, NULL, SW_SHOW) > (void *)32);
}

int Fl_WinAPI_System_Driver::file_browser_load_filesystem(Fl_File_Browser *browser, char *filename, Fl_File_Icon *icon)
{
  int num_files = 0;
#  ifdef __CYGWIN__
  //
  // Cygwin provides an implementation of setmntent() to get the list
  // of available drives...
  //
  FILE          *m = setmntent("/-not-used-", "r");
  struct mntent *p;
  while ((p = getmntent (m)) != NULL) {
    browser->add(p->mnt_dir, icon);
    num_files ++;
  }
  endmntent(m);
#  else
  //
  // Normal WIN32 code uses drive bits...
  //
  DWORD	drives;		// Drive available bits
  drives = GetLogicalDrives();
  for (int i = 'A'; i <= 'Z'; i ++, drives >>= 1)
    if (drives & 1)
    {
      sprintf(filename, "%c:/", i);
      if (i < 'C') // see also: GetDriveType and GetVolumeInformation in WIN32
        browser->add(filename, icon);
      else
        browser->add(filename, icon);
      num_files ++;
    }
#  endif // __CYGWIN__
  return num_files;
}

int Fl_WinAPI_System_Driver::file_browser_load_directory(const char *directory, char *filename,
                                                         size_t name_size, dirent ***pfiles, Fl_File_Sort_F *sort)
{
  strlcpy(filename, directory, name_size);
  int i = (int) (strlen(filename) - 1);
  if (i == 2 && filename[1] == ':' &&
      (filename[2] == '/' || filename[2] == '\\'))
    filename[2] = '/';
  else if (filename[i] != '/' && filename[i] != '\\')
    strlcat(filename, "/", name_size);
  return filename_list(filename, pfiles, sort);
}

void Fl_WinAPI_System_Driver::newUUID(char *uuidBuffer)
{
  // First try and use the win API function UuidCreate(), but if that is not
  // available, fall back to making something up from scratch.
  // We do not want to link against the Rpcrt4.dll, as we will rarely use it,
  // so we load the DLL dynamically, if it is available, and work from there.
  static HMODULE hMod = NULL;
  UUID ud;
  UUID *pu = &ud;
  int got_uuid = 0;
  
  if (!hMod) {		// first time in?
    hMod = LoadLibrary("Rpcrt4.dll");
  }
  
  if (hMod) {		// do we have a usable handle to Rpcrt4.dll?
    uuid_func uuid_crt = (uuid_func)GetProcAddress(hMod, "UuidCreate");
    if (uuid_crt != NULL) {
      RPC_STATUS rpc_res = uuid_crt(pu);
      if ( // is the return status OK for our needs?
          (rpc_res == RPC_S_OK) ||		// all is well
          (rpc_res == RPC_S_UUID_LOCAL_ONLY) || // only unique to this machine
          (rpc_res == RPC_S_UUID_NO_ADDRESS)	// probably only locally unique
          ) {
        got_uuid = -1;
        sprintf(uuidBuffer, "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                pu->Data1, pu->Data2, pu->Data3, pu->Data4[0], pu->Data4[1],
                pu->Data4[2], pu->Data4[3], pu->Data4[4],
                pu->Data4[5], pu->Data4[6], pu->Data4[7]);
      }
    }
  }
  if (got_uuid == 0) {		// did not make a UUID - use fallback logic
    unsigned char b[16];
    time_t t = time(0);		// first 4 byte
    b[0] = (unsigned char)t;
    b[1] = (unsigned char)(t>>8);
    b[2] = (unsigned char)(t>>16);
    b[3] = (unsigned char)(t>>24);
    int r = rand();		// four more bytes
    b[4] = (unsigned char)r;
    b[5] = (unsigned char)(r>>8);
    b[6] = (unsigned char)(r>>16);
    b[7] = (unsigned char)(r>>24);
    // Now we try to find 4 more "random" bytes. We extract the
    // lower 4 bytes from the address of t - it is created on the
    // stack so *might* be in a different place each time...
    // This is now done via a union to make it compile OK on 64-bit systems.
    union { void *pv; unsigned char a[sizeof(void*)]; } v;
    v.pv = (void *)(&t);
    // NOTE: This assume that all WinXX systems are little-endian
    b[8] = v.a[0];
    b[9] = v.a[1];
    b[10] = v.a[2];
    b[11] = v.a[3];
    TCHAR name[MAX_COMPUTERNAME_LENGTH + 1]; // only used to make last four bytes
    DWORD nSize = MAX_COMPUTERNAME_LENGTH + 1;
    // GetComputerName() does not depend on any extra libs, and returns something
    // analogous to gethostname()
    GetComputerName(name, &nSize);
    //  use the first 4 TCHAR's of the name to create the last 4 bytes of our UUID
    for (int ii = 0; ii < 4; ii++) {
      b[12 + ii] = (unsigned char)name[ii];
    }
    sprintf(uuidBuffer, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7],
            b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
  }
}

char *Fl_WinAPI_System_Driver::preference_rootnode(Fl_Preferences *prefs, Fl_Preferences::Root root, const char *vendor,
                                                  const char *application)
{
#  define FLPREFS_RESOURCE	"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"
#  define FLPREFS_RESOURCEW	L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"
  static char filename[ FL_PATH_MAX ];
  filename[0] = 0;
  size_t appDataLen = strlen(vendor) + strlen(application) + 8;
  DWORD type, nn;
  LONG err;
  HKEY key;
  
  switch (root) {
    case Fl_Preferences::SYSTEM:
      err = RegOpenKeyW( HKEY_LOCAL_MACHINE, FLPREFS_RESOURCEW, &key );
      if (err == ERROR_SUCCESS) {
        nn = (DWORD) (FL_PATH_MAX - appDataLen);
        err = RegQueryValueExW( key, L"Common AppData", 0L, &type,
                               (BYTE*)filename, &nn );
        if ( ( err != ERROR_SUCCESS ) && ( type == REG_SZ ) ) {
          filename[0] = 0;
          filename[1] = 0;
        }
        RegCloseKey(key);
      }
      break;
    case Fl_Preferences::USER:
      err = RegOpenKeyW( HKEY_CURRENT_USER, FLPREFS_RESOURCEW, &key );
      if (err == ERROR_SUCCESS) {
        nn = (DWORD) (FL_PATH_MAX - appDataLen);
        err = RegQueryValueExW( key, L"AppData", 0L, &type,
                               (BYTE*)filename, &nn );
        if ( ( err != ERROR_SUCCESS ) && ( type == REG_SZ ) ) {
          filename[0] = 0;
          filename[1] = 0;
        }
        RegCloseKey(key);
      }
      break;
  }
  if (!filename[1] && !filename[0]) {
    strcpy(filename, "C:\\FLTK");
  } else {
#if 0
    wchar_t *b = (wchar_t*)_wcsdup((wchar_t *)filename);
#else
    // cygwin does not come with _wcsdup. Use malloc +  wcscpy.
    // For implementation of wcsdup functionality See
    // - http://linenum.info/p/glibc/2.7/wcsmbs/wcsdup.c
    wchar_t *b = (wchar_t*) malloc((wcslen((wchar_t *) filename) + 1) * sizeof(wchar_t));
    wcscpy(b, (wchar_t *) filename);
#endif
    //  filename[fl_unicode2utf(b, wcslen((wchar_t*)b), filename)] = 0;
    unsigned len = fl_utf8fromwc(filename, (FL_PATH_MAX-1), b, (unsigned) wcslen(b));
    filename[len] = 0;
    free(b);
  }
  snprintf(filename + strlen(filename), sizeof(filename) - strlen(filename),
           "/%s/%s.prefs", vendor, application);
  for (char *s = filename; *s; s++) if (*s == '\\') *s = '/';
  return filename;
}

void *Fl_WinAPI_System_Driver::dlopen(const char *filename)
{
  return LoadLibrary(filename);
}

void Fl_WinAPI_System_Driver::png_extra_rgba_processing(unsigned char *ptr, int w, int h)
{
  // Some Windows graphics drivers don't honor transparency when RGB == white
  // Convert RGB to 0 when alpha == 0...
  for (int i = w * h; i > 0; i --, ptr += 4) {
    if (!ptr[3]) ptr[0] = ptr[1] = ptr[2] = 0;
  }
}

const char *Fl_WinAPI_System_Driver::next_dir_sep(const char *start)
{
  const char *p = strchr(start, '/');
  if (!p) p = strchr(start, '\\');
  return p;
}

int Fl_WinAPI_System_Driver::file_type(const char *filename)
{
  int filetype;
  if (filename[strlen(filename) - 1] == '/')
    filetype = Fl_File_Icon::DIRECTORY;
  else if (filename_isdir(filename))
    filetype = Fl_File_Icon::DIRECTORY;
  else
    filetype = Fl_File_Icon::PLAIN;
  return filetype;
}

//
// End of "$Id$".
//
