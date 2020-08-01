//
// Definition of Apple Darwin system driver.
//
// Copyright 1998-2017 by Bill Spitzak and others.
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
#include "Fl_Posix_System_Driver.H"
#include "../../flstring.h"
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_File_Icon.H>
#include <FL/filename.H>
#include <FL/fl_string.h>
#include <FL/Fl.H>
#include <locale.h>
#include <stdio.h>
#if HAVE_DLFCN_H
#  include <dlfcn.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pwd.h>
#include <unistd.h>
#include <time.h>

//
// Define missing POSIX/XPG4 macros as needed...
//
#ifndef S_ISDIR
#  define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#  define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#  define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#  define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif /* !S_ISDIR */


#if HAVE_DLFCN_H
static void* double_dlopen(const char *filename1)
{
  void *ptr = ::dlopen(filename1, RTLD_LAZY | RTLD_GLOBAL);
  if (!ptr) {
    char filename2[FL_PATH_MAX];
    sprintf(filename2, "%s.0", filename1);
    ptr = dlopen(filename2, RTLD_LAZY | RTLD_GLOBAL);
  }
  return ptr;
}
#endif

void *Fl_Posix_System_Driver::dlopen(const char *filename)
{
  void *ptr = NULL;
#if HAVE_DLFCN_H
  ptr = double_dlopen(filename);
#  ifdef __APPLE_CC__ // allows testing on Darwin + XQuartz + fink
  if (!ptr) {
    char *f_dylib = fl_strdup(filename);
    strcpy(strrchr(f_dylib, '.'), ".dylib");
    char path[FL_PATH_MAX];
    sprintf(path, "/sw/lib/%s", f_dylib);
    ptr = ::dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
    if (!ptr) {
      sprintf(path, "/opt/sw/lib/%s", f_dylib);
      ptr = ::dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
    }
    if (!ptr) {
      sprintf(path, "/opt/X11/lib/%s", f_dylib);
      ptr = ::dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
    }
    free(f_dylib);
  }
#  endif // __APPLE_CC__
#endif // HAVE_DLFCN_H
  return ptr;
}

int Fl_Posix_System_Driver::file_type(const char *filename)
{
  int filetype;
  struct stat fileinfo;         // Information on file
  if (!::stat(filename, &fileinfo))
  {
    if (S_ISDIR(fileinfo.st_mode))
      filetype = Fl_File_Icon::DIRECTORY;
#  ifdef S_ISFIFO
    else if (S_ISFIFO(fileinfo.st_mode))
      filetype = Fl_File_Icon::FIFO;
#  endif // S_ISFIFO
#  if defined(S_ISCHR) && defined(S_ISBLK)
    else if (S_ISCHR(fileinfo.st_mode) || S_ISBLK(fileinfo.st_mode))
      filetype = Fl_File_Icon::DEVICE;
#  endif // S_ISCHR && S_ISBLK
#  ifdef S_ISLNK
    else if (S_ISLNK(fileinfo.st_mode))
      filetype = Fl_File_Icon::LINK;
#  endif // S_ISLNK
    else
      filetype = Fl_File_Icon::PLAIN;
  }
  else
    filetype = Fl_File_Icon::PLAIN;
  return filetype;
}

const char *Fl_Posix_System_Driver::getpwnam(const char *login) {
  struct passwd *pwd;
  pwd = ::getpwnam(login);
  return pwd ? pwd->pw_dir : NULL;
}


void Fl_Posix_System_Driver::gettime(time_t *sec, int *usec) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  *sec = tv.tv_sec;
  *usec = tv.tv_usec;
}
