//
// Definition of Posix system driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2022 by Bill Spitzak and others.
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

#include "Fl_X11_System_Driver.H"
#include <FL/Fl_File_Browser.H>
#include <FL/fl_string_functions.h>  // fl_strdup
#include <FL/platform.H>
#include "../../flstring.h"
#include "../../Fl_Timeout.h"

#include <X11/Xlib.h>
#include <locale.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>     // strerror(errno)
#include <errno.h>      // errno
#if HAVE_DLSYM && HAVE_DLFCN_H
#include <dlfcn.h>   // for dlsym
#endif


#if defined(_AIX)
extern "C" {
#  include <sys/vmount.h>
#  include <sys/mntctl.h>
  // Older AIX versions don't expose this prototype
  int mntctl(int, int, char *);
}
#endif  // _AIX

#if defined(__NetBSD__)
extern "C" {
#  include <sys/param.h>  // For '__NetBSD_Version__' definition
#  if defined(__NetBSD_Version__) && (__NetBSD_Version__ >= 300000000)
#    include <sys/types.h>
#    include <sys/statvfs.h>
#    if defined(HAVE_PTHREAD) && defined(HAVE_PTHREAD_H)
#      include <pthread.h>
#    endif  // HAVE_PTHREAD && HAVE_PTHREAD_H
#    ifdef HAVE_PTHREAD
  static pthread_mutex_t getvfsstat_mutex = PTHREAD_MUTEX_INITIALIZER;
#    endif  // HAVE_PTHREAD/
#  endif  // __NetBSD_Version__
}
#endif  // __NetBSD__

#ifndef HAVE_SCANDIR
extern "C" {
  int fl_scandir(const char *dirname, struct dirent ***namelist,
                 int (*select)(struct dirent *),
                 int (*compar)(struct dirent **, struct dirent **),
                 char *errmsg, int errmsg_sz);
}
#endif


void Fl_X11_System_Driver::display_arg(const char *arg) {
  Fl::display(arg);
}

int Fl_X11_System_Driver::XParseGeometry(const char* string, int* x, int* y,
                                         unsigned int* width, unsigned int* height) {
  return ::XParseGeometry(string, x, y, width, height);
}


#if !defined(FL_DOXYGEN)

const char *Fl_X11_System_Driver::shortcut_add_key_name(unsigned key, char *p, char *buf, const char **eom)
{
  const char* q;
  if (key == FL_Enter || key == '\r') q = "Enter";  // don't use Xlib's "Return":
  else if (key > 32 && key < 0x100) q = 0;
  else q = XKeysymToString(key);
  if (!q) {
    p += fl_utf8encode(fl_toupper(key), p);
    *p = 0;
    return buf;
  }
  if (p > buf) {
    strcpy(p,q);
    return buf;
  } else {
    if (eom) *eom = q;
    return q;
  }
}

void Fl_X11_System_Driver::own_colormap() {
  fl_open_display();
#if USE_COLORMAP
  switch (fl_visual->c_class) {
  case GrayScale :
  case PseudoColor :
  case DirectColor :
    break;
  default:
    return; // don't do anything for non-colormapped visuals
  }
  int i;
  XColor colors[16];
  // Get the first 16 colors from the default colormap...
  for (i = 0; i < 16; i ++) colors[i].pixel = i;
  XQueryColors(fl_display, fl_colormap, colors, 16);
  // Create a new colormap...
  fl_colormap = XCreateColormap(fl_display,
                                RootWindow(fl_display,fl_screen),
                                fl_visual->visual, AllocNone);
  // Copy those first 16 colors to our own colormap:
  for (i = 0; i < 16; i ++)
    XAllocColor(fl_display, fl_colormap, colors + i);
#endif // USE_COLORMAP
}

#endif // !defined(FL_DOXYGEN)
