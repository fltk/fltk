//
// Definition of Windows system driver for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include <config.h>
#include <FL/platform.H>
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
#include <sys/timeb.h>
#include <shellapi.h>
#include <wchar.h>
#include <process.h>
#include <locale.h>
#include <time.h>
#include <direct.h>
#include <io.h>
#include <fcntl.h>
#include <string>

// We must define _WIN32_IE at least to 0x0500 before inclusion of 'shlobj.h' to enable
// the declaration of SHGFP_TYPE_CURRENT for some older versions of MinGW, notably
// header versions 5.3.0 and earlier, whereas 5.4.2 seems to define _WIN32_IE as needed.
#if !(defined _WIN32_IE) || (_WIN32_IE < 0x0500)
#  undef _WIN32_IE
#  define _WIN32_IE  0x0500
#endif /* _WIN32_WINNT checks */

#include <shlobj.h>

// function pointer for the UuidCreate Function
// RPC_STATUS RPC_ENTRY UuidCreate(UUID __RPC_FAR *Uuid);
typedef RPC_STATUS (WINAPI *uuid_func)(UUID __RPC_FAR *Uuid);

// Apparently Borland C++ defines DIRECTORY in <direct.h>, which
// interferes with the Fl_File_Icon enumeration of the same name.
#  ifdef DIRECTORY
#    undef DIRECTORY
#  endif // DIRECTORY

#ifdef __CYGWIN__
#  include <mntent.h>
#endif

// Optional helper function to debug Fl_WinAPI_System_Driver::home_directory_name()
#ifndef DEBUG_HOME_DIRECTORY_NAME
#define DEBUG_HOME_DIRECTORY_NAME 0
#endif
#if DEBUG_HOME_DIRECTORY_NAME
static void print_env(const char *ev) {
  const char *val = getenv(ev);
  printf("%-30.30s = \"%s\"\n", ev, val ? val : "<null>");
  fflush(stdout);
}
#endif // DEBUG_HOME_DIRECTORY_NAME

static inline int isdirsep(char c) { return c == '/' || c == '\\'; }

static wchar_t *mbwbuf = NULL;
static wchar_t *wbuf = NULL;
static wchar_t *wbuf1 = NULL;

extern "C" {
  int fl_scandir(const char *dirname, struct dirent ***namelist,
                 int (*select)(struct dirent *),
                 int (*compar)(struct dirent **, struct dirent **),
                 char *errmsg, int errmsg_len);
}

/*
  Convert UTF-8 string to Windows wide character encoding (UTF-16).

  This helper function is used throughout this file to convert UTF-8
  strings to Windows specific UTF-16 encoding for filenames, paths, or
  other strings to be used by system functions.

  The input string can be a null-terminated string or its length can be
  provided by the optional argument 'lg'. If 'lg' is omitted or less than 0
  (default = -1) the string length is determined with strlen(), otherwise
  'lg' takes precedence. Zero (0) is a valid string length (an empty string).

  The argument 'wbuf' must have been initialized with NULL or a previous
  call to malloc() or realloc().

  If the converted string doesn't fit into the allocated size of 'wbuf' or if
  'wbuf' is NULL a new buffer is allocated with realloc(). Hence the pointer
  'wbuf' can be shared among multiple calls to this function if it has been
  initialized with NULL (or malloc or realloc) before the first call.

  The return value is either the old value of 'wbuf' (if the string fits)
  or a pointer to the (re)allocated buffer.

  Pseudo doxygen docs (static function intentionally not documented):

  param[in]     utf8    input string (UTF-8)
  param[in,out] wbuf    in:  pointer to output string buffer or NULL
                        out: new string (the pointer may be changed)
  param[in]     lg      optional: input string length (default = -1)

  returns       pointer to string buffer
*/
static wchar_t *utf8_to_wchar(const char *utf8, wchar_t *&wbuf, int lg = -1) {
  unsigned len = (lg >= 0) ? (unsigned)lg : (unsigned)strlen(utf8);
  unsigned wn = fl_utf8toUtf16(utf8, len, NULL, 0) + 1; // Query length
  wbuf = (wchar_t *)realloc(wbuf, sizeof(wchar_t) * wn);
  wn = fl_utf8toUtf16(utf8, len, (unsigned short *)wbuf, wn); // Convert string
  wbuf[wn] = 0;
  return wbuf;
}

/*
  Convert a Windows wide character (UTF-16) string to UTF-8 encoding.

  This helper function is used throughout this file to convert Windows
  wide character strings as returned by system functions to UTF-8
  encoding for internal usage.

  The argument 'utf8' must have been initialized with NULL or a previous
  call to malloc() or realloc().

  If the converted string doesn't fit into the allocated size of 'utf8' or if
  'utf8' is NULL a new buffer is allocated with realloc(). Hence the pointer
  'utf8' can be shared among multiple calls to this function if it has been
  initialized with NULL (or malloc or realloc) before the first call.
  Ideally every call to this function has its own static pointer though.

  The return value is either the old value of 'utf8' (if the string fits)
  or a pointer at the (re)allocated buffer.

  Pseudo doxygen docs (static function intentionally not documented):

  param[in]     wstr    input string (wide character, UTF-16)
  param[in,out] utf8    in:  pointer to output string buffer
                        out: new string (pointer may be changed)

  returns       pointer to string buffer
*/
static char *wchar_to_utf8(const wchar_t *wstr, char *&utf8) {
  unsigned len = (unsigned)wcslen(wstr);
  unsigned wn = fl_utf8fromwc(NULL, 0, wstr, len) + 1; // query length
  utf8 = (char *)realloc(utf8, wn);
  wn = fl_utf8fromwc(utf8, wn, wstr, len); // convert string
  utf8[wn] = 0;
  return utf8;
}

void Fl_WinAPI_System_Driver::warning(const char *format, va_list args) {
  // Show nothing for warnings under Windows...
}

void Fl_WinAPI_System_Driver::error(const char *format, va_list args) {
  char buf[1024];
  vsnprintf(buf, 1024, format, args);
  MessageBox(0, buf, "Error", MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
}

void Fl_WinAPI_System_Driver::fatal(const char *format, va_list args) {
  char buf[1024];
  vsnprintf(buf, 1024, format, args);
  MessageBox(0, buf, "Error", MB_ICONSTOP | MB_SYSTEMMODAL);
  ::exit(1);
}

char *Fl_WinAPI_System_Driver::utf2mbcs(const char *utf8) {
  static char *buf = NULL;
  if (!utf8) return NULL;

  unsigned len = (unsigned)strlen(utf8);

  unsigned wn = fl_utf8toUtf16(utf8, len, NULL, 0) + 7; // Query length
  mbwbuf = (wchar_t *)realloc(mbwbuf, sizeof(wchar_t) * wn);
  len = fl_utf8toUtf16(utf8, len, (unsigned short *)mbwbuf, wn); // Convert string
  mbwbuf[len] = 0;

  buf = (char*)realloc(buf, len * 6 + 1);
  len = (unsigned)wcstombs(buf, mbwbuf, len * 6);
  buf[len] = 0;
  return buf;
}

char *Fl_WinAPI_System_Driver::getenv(const char *var) {
  static char *buf = NULL;
  wchar_t *ret = _wgetenv(utf8_to_wchar(var, wbuf));
  if (!ret) return NULL;
  return wchar_to_utf8(ret, buf);
}

int Fl_WinAPI_System_Driver::putenv(const char *var) {
  unsigned len = (unsigned)strlen(var);
  unsigned wn = fl_utf8toUtf16(var, len, NULL, 0) + 1; // Query length
  wchar_t *wbuf = (wchar_t *)malloc(sizeof(wchar_t) * wn);
  wn = fl_utf8toUtf16(var, len, (unsigned short *)wbuf, wn);
  wbuf[wn] = 0;
  int ret = _wputenv(wbuf);
  free(wbuf);
  return ret;
}

int Fl_WinAPI_System_Driver::open(const char *fnam, int oflags, int pmode) {
  utf8_to_wchar(fnam, wbuf);
  if (pmode == -1) return _wopen(wbuf, oflags);
  else return _wopen(wbuf, oflags, pmode);
}

int Fl_WinAPI_System_Driver::open_ext(const char *fnam, int binary, int oflags, int pmode) {
  if (oflags == 0) oflags = _O_RDONLY;
  oflags |= (binary ? _O_BINARY : _O_TEXT);
  return this->open(fnam, oflags, pmode);
}

FILE *Fl_WinAPI_System_Driver::fopen(const char *fnam, const char *mode) {
  utf8_to_wchar(fnam, wbuf);
  utf8_to_wchar(mode, wbuf1);
  return _wfopen(wbuf, wbuf1);
}

int Fl_WinAPI_System_Driver::system(const char *cmd) {
  return _wsystem(utf8_to_wchar(cmd, wbuf));
}

int Fl_WinAPI_System_Driver::execvp(const char *file, char *const *argv) {
  int n = 0;
  while (argv[n]) n++; // count args
  wchar_t **ar = (wchar_t **)calloc(sizeof(wchar_t *), n + 1);
  // convert arguments first; trailing NULL provided by calloc()
  for (int i = 0; i < n; i++)
    ar[i] = utf8_to_wchar(argv[i], ar[i]); // alloc and assign
  // convert executable file and execute it ...
  utf8_to_wchar(file, wbuf);
  _wexecvp(wbuf, ar);   // STR #3040
  // clean up (reached only if _wexecvp() failed)
  for (int i = 0; i < n; i++)
    free(ar[i]);
  free(ar);
  return -1;            // STR #3040
}

int Fl_WinAPI_System_Driver::chmod(const char *fnam, int mode) {
  return _wchmod(utf8_to_wchar(fnam, wbuf), mode);
}

int Fl_WinAPI_System_Driver::access(const char *fnam, int mode) {
  return _waccess(utf8_to_wchar(fnam, wbuf), mode);
}

int Fl_WinAPI_System_Driver::flstat(const char *fnam, struct stat *b) {

  // remove trailing '/' or '\'
  unsigned len = (unsigned)strlen(fnam);
  if (len > 0 && (fnam[len-1] == '/' || fnam[len-1] == '\\'))
    len--;
  // convert filename and execute _wstat()
  return _wstat(utf8_to_wchar(fnam, wbuf, len), (struct _stat *)b);
}

char *Fl_WinAPI_System_Driver::getcwd(char *buf, int len) {

  static wchar_t *wbuf = NULL;
  wbuf = (wchar_t *)realloc(wbuf, sizeof(wchar_t) * (len + 1));
  wchar_t *ret = _wgetcwd(wbuf, len);
  if (!ret) return NULL;

  unsigned dstlen = (unsigned)len;
  len = (int)wcslen(wbuf);
  dstlen = fl_utf8fromwc(buf, dstlen, wbuf, (unsigned)len);
  buf[dstlen] = 0;
  return buf;
}

int Fl_WinAPI_System_Driver::chdir(const char *path) {
  return _wchdir(utf8_to_wchar(path, wbuf));
}

int Fl_WinAPI_System_Driver::unlink(const char *fnam) {
  return _wunlink(utf8_to_wchar(fnam, wbuf));
}

int Fl_WinAPI_System_Driver::mkdir(const char *fnam, int mode) {
  return _wmkdir(utf8_to_wchar(fnam, wbuf));
}

int Fl_WinAPI_System_Driver::rmdir(const char *fnam) {
  return _wrmdir(utf8_to_wchar(fnam, wbuf));
}

int Fl_WinAPI_System_Driver::rename(const char *fnam, const char *newnam) {
  utf8_to_wchar(fnam, wbuf);
  utf8_to_wchar(newnam, wbuf1);
  return _wrename(wbuf, wbuf1);
}

// See Fl::args_to_utf8()
int Fl_WinAPI_System_Driver::args_to_utf8(int argc, char ** &argv) {
  int i;

  // Convert the command line arguments to UTF-8
  LPWSTR *wideArgv = CommandLineToArgvW(GetCommandLineW(), &argc);
  argv = (char **)malloc((argc + 1) * sizeof(char *));
  for (i = 0; i < argc; i++) {
    // find the required size of the buffer
    int u8size = WideCharToMultiByte(CP_UTF8,     // CodePage
                                  0,              // dwFlags
                                  wideArgv[i],    // lpWideCharStr
                                  -1,             // cchWideChar
                                  NULL,           // lpMultiByteStr
                                  0,              // cbMultiByte
                                  NULL,           // lpDefaultChar
                                  NULL);          // lpUsedDefaultChar
    if (u8size > 0) {
      char *strbuf = (char*)::malloc(u8size);
      int ret = WideCharToMultiByte(CP_UTF8,        // CodePage
                                    0,              // dwFlags
                                    wideArgv[i],    // lpWideCharStr
                                    -1,             // cchWideChar
                                    strbuf,         // lpMultiByteStr
                                    u8size,         // cbMultiByte
                                    NULL,           // lpDefaultChar
                                    NULL);          // lpUsedDefaultChar

      if (ret) {
        argv[i] = strbuf;
      } else {
        argv[i] = _strdup("");
        ::free(strbuf);
      }
    } else {
      argv[i] = _strdup("");
    }
  }
  argv[argc] = NULL; // required NULL pointer at end of list

  // Free the wide character string array
  LocalFree(wideArgv);

  // Note: the allocated memory or argv[] will not be free'd by the system
  // on exit. This does not constitute a memory leak.

  return argc;
}


// Two Windows-specific functions fl_utf8_to_locale() and fl_locale_to_utf8()
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

unsigned Fl_WinAPI_System_Driver::utf8towc(const char *src, unsigned srclen, wchar_t *dst, unsigned dstlen) {
  return fl_utf8toUtf16(src, srclen, (unsigned short*)dst, dstlen);
}

unsigned Fl_WinAPI_System_Driver::utf8fromwc(char *dst, unsigned dstlen, const wchar_t *src, unsigned srclen) {
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
  static int ret = (GetACP() == CP_UTF8);
  return ret;
}

unsigned Fl_WinAPI_System_Driver::utf8to_mb(const char *src, unsigned srclen, char *dst, unsigned dstlen) {
  wchar_t lbuf[1024];
  wchar_t *buf = lbuf;
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

unsigned Fl_WinAPI_System_Driver::utf8from_mb(char *dst, unsigned dstlen, const char *src, unsigned srclen) {
  wchar_t lbuf[1024];
  wchar_t *buf = lbuf;
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

#if defined(_MSC_VER) && (_MSC_VER >= 1400 /*Visual Studio 2005*/)
static _locale_t c_locale = NULL;
#endif

int Fl_WinAPI_System_Driver::clocale_vprintf(FILE *output, const char *format, va_list args) {
#if defined(_MSC_VER) && (_MSC_VER >= 1400 /*Visual Studio 2005*/)
  if (!c_locale)
    c_locale = _create_locale(LC_NUMERIC, "C");
  int retval = _vfprintf_l(output, format, c_locale, args);
#else
  char *saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  int retval = vfprintf(output, format, args);
  setlocale(LC_NUMERIC, saved_locale);
#endif
  return retval;
}

int Fl_WinAPI_System_Driver::clocale_vsnprintf(char *output, size_t output_size, const char *format, va_list args) {
#if defined(_MSC_VER) && (_MSC_VER >= 1400 /*Visual Studio 2005*/)
  if (!c_locale)
    c_locale = _create_locale(LC_NUMERIC, "C");
  int retval = _vsnprintf_l(output, output_size, format, c_locale, args);
#else
  char *saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  int retval = vsnprintf(output, output_size, format, args);
  setlocale(LC_NUMERIC, saved_locale);
#endif
  return retval;
}

int Fl_WinAPI_System_Driver::clocale_vsscanf(const char *input, const char *format, va_list args) {
  char *saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  int retval = vsscanf(input, format, args);
  setlocale(LC_NUMERIC, saved_locale);
  return retval;
}


int Fl_WinAPI_System_Driver::filename_list(const char *d, dirent ***list,
                                           int (*sort)(struct dirent **, struct dirent **),
                                           char *errmsg, int errmsg_sz) {
  // For Windows we have a special scandir implementation that uses
  // the Win32 "wide" functions for lookup, avoiding the code page mess
  // entirely. It also fixes up the trailing '/'.
  return fl_scandir(d, list, 0, sort, errmsg, errmsg_sz);
}

int Fl_WinAPI_System_Driver::filename_expand(char *to, int tolen, const char *from) {
  char *temp = new char[tolen];
  strlcpy(temp,from, tolen);
  char *start = temp;
  char *end = temp+strlen(temp);
  int ret = 0;
  for (char *a=temp; a<end; ) { // for each slash component
    char *e; for (e=a; e<end && !isdirsep(*e); e++) {/*empty*/} // find next slash
    const char *value = 0; // this will point at substitute value
    switch (*a) {
      case '~': // a home directory name
        if (e <= a+1) { // current user's directory
          value = home_directory_name();
        }
        break;
      case '$':         /* an environment variable */
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
Fl_WinAPI_System_Driver::filename_relative(char *to,    // O - Relative filename
                                           int        tolen,   // I - Size of "to" buffer
                                           const char *dest_dir,   // I - Absolute filename
                                           const char *base_dir)   // I - Find path relative to this path
{
  // Find the relative path from base_dir to dest_dir.
  // Both paths must be absolute and well formed (contain no /../ and /./ segments).

  // return if any of the pointers is NULL
  if (!to || !dest_dir || !base_dir) {
    return 0;
  }

  // if there is a drive letter, make sure both paths use the same drive
  if (   (unsigned)base_dir[0] < 128 && isalpha(base_dir[0]) && base_dir[1] == ':'
      && (unsigned)dest_dir[0] < 128 && isalpha(dest_dir[0]) && dest_dir[1] == ':') {
    if (tolower(base_dir[0]) != tolower(dest_dir[0])) {
      strlcpy(to, dest_dir, tolen);
      return 0;
    }
    // same drive, so skip to the start of the path
    base_dir += 2;
    dest_dir += 2;
  }

  // return if `base_dir` or `dest_dir` is not an absolute path
  if (!isdirsep(*base_dir) || !isdirsep(*dest_dir)) {
    strlcpy(to, dest_dir, tolen);
    return 0;
  }

  const char *base_i = base_dir; // iterator through the base directory string
  const char *base_s = base_dir; // pointer to the last dir separator found
  const char *dest_i = dest_dir; // iterator through the destination directory
  const char *dest_s = dest_dir; // pointer to the last dir separator found

  // compare both path names until we find a difference
  for (;;) {
#if 0 // case sensitive
    base_i++;
    dest_i++;
    char b = *base_i, d = *dest_i;
#else // case insensitive
    base_i += fl_utf8len1(*base_i);
    int b = fl_tolower(fl_utf8decode(base_i, NULL, NULL));
    dest_i += fl_utf8len1(*dest_i);
    int d = fl_tolower(fl_utf8decode(dest_i, NULL, NULL));
#endif
    int b0 = (b == 0) || (isdirsep(b));
    int d0 = (d == 0) || (isdirsep(d));
    if (b0 && d0) {
      base_s = base_i;
      dest_s = dest_i;
    }
    if (b == 0 || d == 0)
      break;
    if (b != d)
      break;
  }
  // base_s and dest_s point at the last separator we found
  // base_i and dest_i point at the first character that differs

  // test for the exact same string and return "." if so
  if (   (base_i[0] == 0 || (isdirsep(base_i[0]) && base_i[1] == 0))
      && (dest_i[0] == 0 || (isdirsep(dest_i[0]) && dest_i[1] == 0))) {
    strlcpy(to, ".", tolen);
    return 0;
  }

  // prepare the destination buffer
  to[0] = '\0';
  to[tolen - 1] = '\0';

  // count the directory segments remaining in `base_dir`
  int n_up = 0;
  for (;;) {
    char b = *base_s++;
    if (b == 0)
      break;
    if (isdirsep(b) && *base_s)
      n_up++;
  }

  // now add a "previous dir" sequence for every following slash in the cwd
  if (n_up > 0)
    strlcat(to, "..", tolen);
  for (; n_up > 1; --n_up)
    strlcat(to, "/..", tolen);

  // finally add the differing path from "from"
  if (*dest_s) {
    if (n_up)
      strlcat(to, "/", tolen);
    strlcat(to, dest_s + 1, tolen);
  }

  return 1;
}

int Fl_WinAPI_System_Driver::filename_absolute(char *to, int tolen, const char *from, const char *base) {
  if (isdirsep(*from) || *from == '|' || from[1]==':' || !base) {
    strlcpy(to, from, tolen);
    return 0;
  }
  char *a;
  char *temp = new char[tolen];
  const char *start = from;
  strlcpy(temp, base, tolen);
  for (a = temp; *a; a++) if (*a=='\\') *a = '/'; // ha ha
  /* remove trailing '/' in current working directory */
  if (isdirsep(*(a-1))) a--;
  /* remove intermediate . and .. names: */
  while (*start == '.') {
    if (start[1]=='.' && (isdirsep(start[2]) || start[2]==0) ) {
      // found "..", remove the last directory segment form cwd
      char *b;
      for (b = a-1; b >= temp && !isdirsep(*b); b--) {/*empty*/}
      if (b < temp) break;
      a = b;
      if (start[2] == 0)
        start += 2;
      else
        start += 3;
    } else if (isdirsep(start[1])) {
      // found "./" in path, just skip it
      start += 2;
    } else if (!start[1]) {
      // found "." at end of path, just skip it
      start ++;
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

int Fl_WinAPI_System_Driver::filename_isdir(const char *n) {
  char fn[4]; // used for drive letter only: "X:/"
  int length = (int)strlen(n);
  // Strip trailing slash from name...
  if (length > 0 && isdirsep(n[length - 1]))
    length --;
  if (length < 1)
    return 0;

  // This workaround brought to you by the fine folks at Microsoft!
  // (read lots of sarcasm in that...)

  if (length == 2 && isalpha(n[0]) && n[1] == ':') { // trailing '/' already "removed"
    // Always use "X:/" for drive letters
    fn[0] = n[0];
    strcpy(fn + 1, ":/");
    n = fn;
    length = 3;
  }

  // convert filename to wide chars using *length*
  utf8_to_wchar(n, wbuf, length);

  DWORD fa = GetFileAttributesW(wbuf);
  return (fa != INVALID_FILE_ATTRIBUTES) && (fa & FILE_ATTRIBUTE_DIRECTORY);
}

int Fl_WinAPI_System_Driver::filename_isdir_quick(const char *n) {
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

int Fl_WinAPI_System_Driver::open_uri(const char *uri, char *msg, int msglen) {
  if (msg) snprintf(msg, msglen, "open %s", uri);
  return (int)(ShellExecute(HWND_DESKTOP, "open", uri, NULL, NULL, SW_SHOW) > (void *)32);
}

int Fl_WinAPI_System_Driver::file_browser_load_filesystem(Fl_File_Browser *browser, char *filename,
                                                          int lname, Fl_File_Icon *icon) {
  int num_files = 0;
# ifdef __CYGWIN__
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
# else
  //
  // Normal Windows code uses drive bits...
  //
  DWORD drives;         // Drive available bits
  drives = GetLogicalDrives();
  for (int i = 'A'; i <= 'Z'; i ++, drives >>= 1) {
    if (drives & 1) {
      snprintf(filename, lname, "%c:/", i);
      if (i < 'C') // see also: GetDriveType and GetVolumeInformation in Windows
        browser->add(filename, icon);
      else
        browser->add(filename, icon);
      num_files ++;
    }
  }
# endif // __CYGWIN__
  return num_files;
}

int Fl_WinAPI_System_Driver::file_browser_load_directory(const char *directory, char *filename,
                                                         size_t name_size, dirent ***pfiles,
                                                         Fl_File_Sort_F *sort,
                                                         char *errmsg, int errmsg_sz)
{
  strlcpy(filename, directory, name_size);
  int i = (int) (strlen(filename) - 1);
  if (i == 2 && filename[1] == ':' &&
      (filename[2] == '/' || filename[2] == '\\'))
    filename[2] = '/';
  else if (filename[i] != '/' && filename[i] != '\\')
    strlcat(filename, "/", name_size);
  return filename_list(filename, pfiles, sort, errmsg, errmsg_sz);
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

  if (!hMod) {          // first time in?
    hMod = LoadLibrary("Rpcrt4.dll");
  }

  if (hMod) {           // do we have a usable handle to Rpcrt4.dll?
    uuid_func uuid_crt = (uuid_func)GetProcAddress(hMod, "UuidCreate");
    if (uuid_crt != NULL) {
      RPC_STATUS rpc_res = uuid_crt(pu);
      if ( // is the return status OK for our needs?
          (rpc_res == RPC_S_OK) ||              // all is well
          (rpc_res == RPC_S_UUID_LOCAL_ONLY) || // only unique to this machine
          (rpc_res == RPC_S_UUID_NO_ADDRESS)    // probably only locally unique
          ) {
        got_uuid = -1;
        snprintf(uuidBuffer, 36+1, "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                pu->Data1, pu->Data2, pu->Data3, pu->Data4[0], pu->Data4[1],
                pu->Data4[2], pu->Data4[3], pu->Data4[4],
                pu->Data4[5], pu->Data4[6], pu->Data4[7]);
      }
    }
  }
  if (got_uuid == 0) {          // did not make a UUID - use fallback logic
    unsigned char b[16];
    time_t t = time(0);         // first 4 byte
    b[0] = (unsigned char)t;
    b[1] = (unsigned char)(t>>8);
    b[2] = (unsigned char)(t>>16);
    b[3] = (unsigned char)(t>>24);
    int r = rand();             // four more bytes
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
    snprintf(uuidBuffer, 36+1, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7],
            b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
  }
}

/*
 Note: `prefs` can be NULL!
 */
char *Fl_WinAPI_System_Driver::preference_rootnode(Fl_Preferences * /*prefs*/, Fl_Preferences::Root root, const char *vendor,
                                                  const char *application)
{
  static char *filename = 0L;
  // make enough room for a UTF-16 pathname
  if (!filename) filename = (char*)::malloc(2 * FL_PATH_MAX);
  HRESULT res;

  // https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetfolderpathw

  int appdata = CSIDL_APPDATA;              // assume user preferences
  if ((root & Fl_Preferences::ROOT_MASK) == Fl_Preferences::SYSTEM)
    appdata = CSIDL_COMMON_APPDATA;         // use system preferences

  res = SHGetFolderPathW(NULL,               // hwnd: Reserved!
                         appdata,            // csidl: User or common Application Data (Roaming)
                         NULL,               // hToken (unused)
                         SHGFP_TYPE_CURRENT, // dwFlags: use current, potentially redirected path
                         (LPWSTR)filename);  // out: filename in Windows wide string encoding
  if (res != S_OK) {
    // don't write data into some arbitrary directory! Just return NULL.
    return 0L;
  }

  // convert the path from Windows wide character (UTF-16) to UTF-8
  // FIXME: can this be simplified? Don't allocate/copy/move/free more than necessary!
  char *buf = NULL;
  wchar_to_utf8((wchar_t *)filename, buf);   // allocates buf for conversion
  strcpy(filename, buf);
  free(buf);

  // Make sure that the parameters are not NULL
  if ( (vendor==0L) || (vendor[0]==0) )
    vendor = "unknown";
  if ( (application==0L) || (application[0]==0) )
    application = "unknown";

  // append vendor, application, and ".prefs", and convert '\' to '/'
  snprintf(filename + strlen(filename), FL_PATH_MAX - strlen(filename),
           "/%s/%s.prefs", vendor, application);
  for (char *s = filename; *s; s++) if (*s == '\\') *s = '/';
  return filename;
}

void *Fl_WinAPI_System_Driver::load(const char *filename) {
  return LoadLibraryW(utf8_to_wchar(filename, wbuf));
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

// Note: the result is cached in a static variable
const char *Fl_WinAPI_System_Driver::home_directory_name()
{
  static std::string home;
  if (!home.empty())
    return home.c_str();

#if (DEBUG_HOME_DIRECTORY_NAME)
  print_env("HOMEDRIVE");
  print_env("HOMEPATH");
  print_env("UserProfile");
  print_env("HOME");
#endif

  // Implement various ways to retrieve the HOME path.
  // Note, from `man getenv`:
  //  "The implementation of getenv() is not required to be reentrant.
  //   The string pointed to by the return value of getenv() may be statically
  //   allocated, and can be modified by a subsequent call to getenv()...".
  // Tests show that this is the case in some MinGW implementations.

  if (home.empty()) {
    const char *home_drive = getenv("HOMEDRIVE");
    if (home_drive) {
      home = home_drive; // copy *before* calling getenv() again, see above
      const char *home_path = getenv("HOMEPATH");
      if (home_path) {
        home.append(home_path);
      } else {
        home.clear(); // reset
      } // home_path
    } // home_drive
  } // empty()

  if (home.empty()) {
    const char *h = getenv("UserProfile");
    if (h)
      home = h;
  }

  if (home.empty()) {
    const char *h = getenv("HOME");
    if (h)
      home = h;
  }
  if (home.empty()) {
    home = "~/"; // last resort
  }
  // Make path canonical.
  for (char& c : home) {
    if (c == '\\')
      c = '/';
  }
#if (DEBUG_HOME_DIRECTORY_NAME)
  printf("home_directory_name() returns \"%s\"\n", home.c_str());
  fflush(stdout);
#endif
  return home.c_str();
}

void Fl_WinAPI_System_Driver::gettime(time_t *sec, int *usec) {
  struct _timeb t;
  _ftime(&t);
  *sec = t.time;
  *usec = t.millitm * 1000;
}

//
// Code for lock support
//

// These pointers are in Fl_win32.cxx:
extern void (*fl_lock_function)();
extern void (*fl_unlock_function)();

// The main thread's ID
static DWORD main_thread;

// Microsoft's version of a MUTEX...
static CRITICAL_SECTION cs;
static CRITICAL_SECTION *cs_ring;

void Fl_WinAPI_System_Driver::unlock_ring() {
  LeaveCriticalSection(cs_ring);
}

void Fl_WinAPI_System_Driver::lock_ring() {
  if (!cs_ring) {
    cs_ring = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION));
    InitializeCriticalSection(cs_ring);
  }
  EnterCriticalSection(cs_ring);
}

//
// 'unlock_function()' - Release the lock.
//

static void unlock_function() {
  LeaveCriticalSection(&cs);
}

//
// 'lock_function()' - Get the lock.
//

static void lock_function() {
  EnterCriticalSection(&cs);
}

int Fl_WinAPI_System_Driver::lock() {
  if (!main_thread) InitializeCriticalSection(&cs);

  lock_function();

  if (!main_thread) {
    fl_lock_function   = lock_function;
    fl_unlock_function = unlock_function;
    main_thread        = GetCurrentThreadId();
  }
  return 0;
}

void Fl_WinAPI_System_Driver::unlock() {
  unlock_function();
}

void Fl_WinAPI_System_Driver::awake(void* msg) {
  PostThreadMessage( main_thread, fl_wake_msg, (WPARAM)msg, 0);
}

int Fl_WinAPI_System_Driver::close_fd(int fd) {
  return _close(fd);
}
