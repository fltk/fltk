//
// Definition of Apple Darwin system driver.
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

#include "Fl_Darwin_System_Driver.H"
#include "../Cocoa/Fl_MacOS_Sys_Menu_Bar_Driver.H"
#include <FL/Fl.H>
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_Tree_Prefs.H>
#include <FL/Fl_Pixmap.H>
#include <FL/platform.H>
#include <FL/fl_draw.H>
#include "../../flstring.h"
#include <string.h>
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
#include <xlocale.h>
#endif
#include <locale.h>
#include <stdio.h>
#include <dlfcn.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <sys/stat.h>


const char *Fl_Darwin_System_Driver::shift_name() {
  return "⇧\\"; // "\xe2\x87\xa7\\"; // U+21E7 (upwards white arrow)
}
const char *Fl_Darwin_System_Driver::meta_name() {
  return "⌘\\"; // "\xe2\x8c\x98\\"; // U+2318 (place of interest sign)
}
const char *Fl_Darwin_System_Driver::alt_name() {
  return "⌥\\"; // "\xe2\x8c\xa5\\"; // U+2325 (option key)
}
const char *Fl_Darwin_System_Driver::control_name() {
  return "⌃\\"; // "\xe2\x8c\x83\\"; // U+2303 (up arrowhead)
}

Fl_Darwin_System_Driver::Fl_Darwin_System_Driver() : Fl_Posix_System_Driver() {
  if (fl_mac_os_version == 0) fl_mac_os_version = calc_mac_os_version();
  command_key = FL_META;
  control_key = FL_CTRL;
}

int Fl_Darwin_System_Driver::single_arg(const char *arg) {
  // The Finder application in MacOS X passes the "-psn_N_NNNNN" option to all apps.
  return (strncmp(arg, "psn_", 4) == 0);
}

int Fl_Darwin_System_Driver::arg_and_value(const char *name, const char *value) {
  // Xcode in MacOS X may pass "-NSDocumentRevisionsDebugMode YES"
  return strcmp(name, "NSDocumentRevisionsDebugMode") == 0;
}

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
static locale_t postscript_locale = NULL;
#endif

int Fl_Darwin_System_Driver::clocale_vprintf(FILE *output, const char *format, va_list args) {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (fl_mac_os_version >= 100400) {
    if (!postscript_locale)
      postscript_locale = newlocale(LC_NUMERIC_MASK, "C", (locale_t)0);
    return vfprintf_l(output, postscript_locale, format, args);
  }
#endif
  char *saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  int retval = vfprintf(output, format, args);
  setlocale(LC_NUMERIC, saved_locale);
  return retval;
}

int Fl_Darwin_System_Driver::clocale_vsnprintf(char *output, size_t output_size, const char *format, va_list args) {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (fl_mac_os_version >= 100400) {
    if (!postscript_locale)
      postscript_locale = newlocale(LC_NUMERIC_MASK, "C", (locale_t)0);
    return vsnprintf_l(output, output_size, postscript_locale, format, args);
  }
#endif
  char *saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  int retval = vsnprintf(output, output_size, format, args);
  setlocale(LC_NUMERIC, saved_locale);
  return retval;
}

int Fl_Darwin_System_Driver::clocale_vsscanf(const char *input, const char *format, va_list args) {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (fl_mac_os_version >= 100400) {
    if (!postscript_locale)
      postscript_locale = newlocale(LC_NUMERIC_MASK, "C", (locale_t)0);
    return vsscanf_l(input, postscript_locale, format, args);
  }
#endif
  char *saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  int retval = vsscanf(input, format, args);
  setlocale(LC_NUMERIC, saved_locale);
  return retval;
}


/* Returns the address of a Carbon function after dynamically loading the Carbon library if needed.
 Supports old Mac OS X versions that may use a couple of Carbon calls:
 GetKeys used by OS X 10.3 or before (in Fl::get_key())
 PMSessionPageSetupDialog and PMSessionPrintDialog used by 10.4 or before (in Fl_Printer::begin_job())
 */
void *Fl_Darwin_System_Driver::get_carbon_function(const char *function_name) {
  static void *carbon = ::dlopen("/System/Library/Frameworks/Carbon.framework/Carbon", RTLD_LAZY);
  return (carbon ? dlsym(carbon, function_name) : NULL);
}

int Fl_Darwin_System_Driver::filename_list(const char *d, dirent ***list,
                                           int (*sort)(struct dirent **, struct dirent **),
                                           char *errmsg, int errmsg_sz) {
  int dirlen;
  char *dirloc;
  // Assume that locale encoding is no less dense than UTF-8
  dirlen = (int)strlen(d);
  dirloc = (char *)d;
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_8
  int n = scandir(dirloc, list, 0, (int(*)(const struct dirent**,const struct dirent**))sort);
# else
  int n = scandir(dirloc, list, 0, (int(*)(const void*,const void*))sort);
# endif
  if (n==-1) {
    if (errmsg) fl_snprintf(errmsg, errmsg_sz, "%s", strerror(errno));
    return -1;
  }
  // convert every filename to UTF-8, and append a '/' to all
  // filenames that are directories
  int i;
  char *fullname = (char*)malloc(dirlen+FL_PATH_MAX+3); // Add enough extra for two /'s and a nul
  // Use memcpy for speed since we already know the length of the string...
  memcpy(fullname, d, dirlen+1);
  char *name = fullname + dirlen;
  if (name!=fullname && name[-1]!='/') *name++ = '/';
  for (i=0; i<n; i++) {
    int newlen;
    dirent *de = (*list)[i];
    int len = (int)strlen(de->d_name);
    newlen = len;
    dirent *newde = (dirent*)malloc(de->d_name - (char*)de + newlen + 2); // Add space for a / and a nul
    // Conversion to UTF-8
    memcpy(newde, de, de->d_name - (char*)de);
    strcpy(newde->d_name, de->d_name);
    // Check if dir (checks done on "old" name as we need to interact with
    // the underlying OS)
    if (de->d_name[len-1]!='/' && len<=FL_PATH_MAX) {
      // Use memcpy for speed since we already know the length of the string...
      memcpy(name, de->d_name, len+1);
      if (fl_filename_isdir(fullname)) {
        char *dst = newde->d_name + newlen;
        *dst++ = '/';
        *dst = 0;
      }
    }
    free(de);
    (*list)[i] = newde;
  }
  free(fullname);
  return n;
}


int Fl_Darwin_System_Driver::open_uri(const char *uri, char *msg, int msglen)
{
  char  *argv[3];                       // Command-line arguments
  argv[0] = (char*)"open";
  argv[1] = (char*)uri;
  argv[2] = (char*)0;
  if (msg) snprintf(msg, msglen, "open %s", uri);
  return run_program("/usr/bin/open", argv, msg, msglen) != 0;
}

int Fl_Darwin_System_Driver::file_browser_load_filesystem(Fl_File_Browser *browser, char *filename, int lname, Fl_File_Icon *icon)
{
  // MacOS X and Darwin use getfsstat() system call...
  int                   numfs;  // Number of file systems
  struct statfs *fs;    // Buffer for file system info
  int num_files = 0;

  // We always have the root filesystem.
  browser->add("/", icon);

  // Get the mounted filesystems...
  numfs = getfsstat(NULL, 0, MNT_NOWAIT);
  if (numfs > 0) {
    // We have file systems, get them...
    fs = new struct statfs[numfs];
    getfsstat(fs, sizeof(struct statfs) * numfs, MNT_NOWAIT);

    // Add filesystems to the list...
    for (int i = 0; i < numfs; i ++) {
      // Ignore "/", "/dev", and "/.vol"...
      if (fs[i].f_mntonname[1] && strcmp(fs[i].f_mntonname, "/dev") &&
          strcmp(fs[i].f_mntonname, "/.vol")) {
        snprintf(filename, lname, "%s/", fs[i].f_mntonname);
        browser->add(filename, icon);
      }
      num_files ++;
    }

    // Free the memory used for the file system info array...
    delete[] fs;
  }
  return num_files;
}

void Fl_Darwin_System_Driver::newUUID(char *uuidBuffer)
{
  CFUUIDRef theUUID = CFUUIDCreate(NULL);
  CFUUIDBytes b = CFUUIDGetUUIDBytes(theUUID);
  snprintf(uuidBuffer, 36+1, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
          b.byte0, b.byte1, b.byte2, b.byte3, b.byte4, b.byte5, b.byte6, b.byte7,
          b.byte8, b.byte9, b.byte10, b.byte11, b.byte12, b.byte13, b.byte14, b.byte15);
  CFRelease(theUUID);
}

/*
 * returns pointer to the filename, or null if name ends with ':'
 */
const char *Fl_Darwin_System_Driver::filename_name( const char *name )
{
  const char *p, *q;
  if (!name) return (0);
  for ( p = q = name ; *p ; ) {
    if ( ( p[0] == ':' ) && ( p[1] == ':' ) ) {
      q = p+2;
      p++;
    }
    else if (p[0] == '/') {
      q = p + 1;
    }
    p++;
  }
  return q;
}

// These function assume a western code page. If you need to support
// scripts that are not part of this code page, you might want to
// take a look at FLTK2, which uses utf8 for text encoding.
//
// By keeping these conversion tables in their own module, they will not
// be statically linked (by a smart linker) unless actually used.
//
// On MS-Windows, nothing need to be converted. We simply return the
// original pointer.
//
// Most X11 implementations seem to default to Latin-1 as a code since it
// is a superset of ISO 8859-1, the original Western codepage on X11.
//
// Apple's OS X however renders text in MacRoman for western settings. The
// lookup tables below will convert all common character codes and replace
// unknown characters with an upside-down question mark.

// This table converts Windows-1252/Latin 1 into MacRoman encoding
static uchar latin2roman[128] = {
0xdb, 0xc0, 0xe2, 0xc4, 0xe3, 0xc9, 0xa0, 0xe0, 0xf6, 0xe4, 0xc0, 0xdc, 0xce, 0xc0, 0xc0, 0xc0,
0xc0, 0xd4, 0xd5, 0xd2, 0xd3, 0xa5, 0xd0, 0xd1, 0xf7, 0xaa, 0xc0, 0xdd, 0xcf, 0xc0, 0xc0, 0xd9,
0xca, 0xc1, 0xa2, 0xa3, 0xc0, 0xb4, 0xc0, 0xa4, 0xac, 0xa9, 0xbb, 0xc7, 0xc2, 0xc0, 0xa8, 0xf8,
0xa1, 0xb1, 0xc0, 0xc0, 0xab, 0xb5, 0xa6, 0xe1, 0xfc, 0xc0, 0xbc, 0xc8, 0xc0, 0xc0, 0xc0, 0xc0,
0xcb, 0xe7, 0xe5, 0xcc, 0x80, 0x81, 0xae, 0x82, 0xe9, 0x83, 0xe6, 0xe8, 0xed, 0xea, 0xeb, 0xec,
0xc0, 0x84, 0xf1, 0xee, 0xef, 0xcd, 0x85, 0xc0, 0xaf, 0xf4, 0xf2, 0xf3, 0x86, 0xc0, 0xc0, 0xa7,
0x88, 0x87, 0x89, 0x8b, 0x8a, 0x8c, 0xbe, 0x8d, 0x8f, 0x8e, 0x90, 0x91, 0x93, 0x92, 0x94, 0x95,
0xc0, 0x96, 0x98, 0x97, 0x99, 0x9b, 0x9a, 0xd6, 0xbf, 0x9d, 0x9c, 0x9e, 0x9f, 0xc0, 0xc0, 0xd8
};

// This table converts MacRoman into Windows-1252/Latin 1
static uchar roman2latin[128] = {
0xc4, 0xc5, 0xc7, 0xc9, 0xd1, 0xd6, 0xdc, 0xe1, 0xe0, 0xe2, 0xe4, 0xe3, 0xe5, 0xe7, 0xe9, 0xe8,
0xea, 0xeb, 0xed, 0xec, 0xee, 0xef, 0xf1, 0xf3, 0xf2, 0xf4, 0xf6, 0xf5, 0xfa, 0xf9, 0xfb, 0xfc,
0x86, 0xb0, 0xa2, 0xa3, 0xa7, 0x95, 0xb6, 0xdf, 0xae, 0xa9, 0x99, 0xb4, 0xa8, 0xbf, 0xc6, 0xd8,
0xbf, 0xb1, 0xbf, 0xbf, 0xa5, 0xb5, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xaa, 0xba, 0xbf, 0xe6, 0xf8,
0xbf, 0xa1, 0xac, 0xbf, 0x83, 0xbf, 0xbf, 0xab, 0xbb, 0x85, 0xa0, 0xc0, 0xc3, 0xd5, 0x8c, 0x9c,
0x96, 0x97, 0x93, 0x94, 0x91, 0x92, 0xf7, 0xbf, 0xff, 0x9f, 0xbf, 0x80, 0x8b, 0x9b, 0xbf, 0xbf,
0x87, 0xb7, 0x82, 0x84, 0x89, 0xc2, 0xca, 0xc1, 0xcb, 0xc8, 0xcd, 0xce, 0xcf, 0xcc, 0xd3, 0xd4,
0xbf, 0xd2, 0xda, 0xdb, 0xd9, 0xbf, 0x88, 0x98, 0xaf, 0xbf, 0xbf, 0xbf, 0xb8, 0xbf, 0xbf, 0xbf
};

static char *buf = 0;
static int n_buf = 0;

const char *Fl_Darwin_System_Driver::latin1_to_local(const char *t, int n)
{
  if (n==-1) n = (int)strlen(t);
  if (n<=n_buf) {
    n_buf = (n + 257) & 0x7fffff00;
    if (buf) free(buf);
    buf = (char*)malloc(n_buf);
  }
  const uchar *src = (const uchar*)t;
  uchar *dst = (uchar*)buf;
  for ( ; n>0; n--) {
    uchar c = *src++;
    if (c>127)
      *dst = latin2roman[c-128];
    else
      *dst = c;
  }
  //*dst = 0; // this would be wrong!
  return buf;
}

const char *Fl_Darwin_System_Driver::local_to_latin1(const char *t, int n)
{
  if (n==-1) n = (int)strlen(t);
  if (n<=n_buf) {
    n_buf = (n + 257) & 0x7fffff00;
    if (buf) free(buf);
    buf = (char*)malloc(n_buf);
  }
  const uchar *src = (const uchar*)t;
  uchar *dst = (uchar*)buf;
  for ( ; n>0; n--) {
    uchar c = *src++;
    if (c>127)
      *dst++ = roman2latin[c-128];
    else
      *dst++ = c;
  }
  //*dst = 0; // this would be wrong
  return buf;
}

// On Mac OS X, nothing need to be converted. We simply return the
// original pointer.
const char *Fl_Darwin_System_Driver::mac_roman_to_local(const char *t, int)
{
  return t;
}

// On Mac OS X, nothing need to be converted. We simply return the
// original pointer.
const char *Fl_Darwin_System_Driver::local_to_mac_roman(const char *t, int)
{
  return t;
}

Fl_Sys_Menu_Bar_Driver *Fl_Darwin_System_Driver::sys_menu_bar_driver()
{
  return Fl_MacOS_Sys_Menu_Bar_Driver::driver();
}

// Draw Mac-specific Fl_Tree open/close icons
void Fl_Darwin_System_Driver::tree_draw_expando_button(int x, int y, bool state, bool active) {
  fl_color(active ? FL_FOREGROUND_COLOR : FL_INACTIVE_COLOR);
  if(state) fl_polygon(x + 3, y, x + 3, y + 11, x + 8, y + 5);   // right arrow: ▶
  else      fl_polygon(x, y + 3, x + 11, y + 3, x + 5, y + 8);   // down arrow: ▼
}
int Fl_Darwin_System_Driver::tree_connector_style() {
  return FL_TREE_CONNECTOR_NONE;
}
