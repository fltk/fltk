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

#include <config.h>
#include "Fl_Posix_System_Driver.H"
#include <FL/filename.H>
#include <FL/Fl.H>
#include <X11/Xlib.h>
#include <locale.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>

// Pointers you can use to change FLTK to a foreign language.
// Note: Similar pointers are defined in FL/fl_ask.H and src/fl_ask.cxx
const char* fl_local_alt   = "Alt";
const char* fl_local_ctrl  = "Ctrl";
const char* fl_local_meta  = "Meta";
const char* fl_local_shift = "Shift";

#ifndef HAVE_SCANDIR
extern "C" {
  int fl_scandir(const char *dirname, struct dirent ***namelist,
                 int (*select)(struct dirent *),
                 int (*compar)(struct dirent **, struct dirent **));
}
#endif

/**
 Creates a driver that manages all screen and display related calls.
 
 This function must be implemented once for every platform.
 */
Fl_System_Driver *Fl_System_Driver::newSystemDriver()
{
  return new Fl_Posix_System_Driver();
}

void Fl_Posix_System_Driver::display_arg(const char *arg) {
  Fl::display(arg);
}

int Fl_Posix_System_Driver::XParseGeometry(const char* string, int* x, int* y,
                                           unsigned int* width, unsigned int* height) {
  return ::XParseGeometry(string, x, y, width, height);
}

int Fl_Posix_System_Driver::clocale_printf(FILE *output, const char *format, va_list args) {
  char *saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  int retval = vfprintf(output, format, args);
  setlocale(LC_NUMERIC, saved_locale);
  return retval;
}

int Fl_Posix_System_Driver::filename_list(const char *d, dirent ***list, int (*sort)(struct dirent **, struct dirent **) ) {
  int dirlen;
  char *dirloc;
  
  // Assume that locale encoding is no less dense than UTF-8
  dirlen = strlen(d);
  dirloc = (char *)malloc(dirlen + 1);
  fl_utf8to_mb(d, dirlen, dirloc, dirlen + 1);
  
#ifndef HAVE_SCANDIR
  // This version is when we define our own scandir
  int n = fl_scandir(dirloc, list, 0, sort);
#elif defined(HAVE_SCANDIR_POSIX)
  // POSIX (2008) defines the comparison function like this:
  int n = scandir(dirloc, list, 0, (int(*)(const dirent **, const dirent **))sort);
#elif defined(__osf__)
  // OSF, DU 4.0x
  int n = scandir(dirloc, list, 0, (int(*)(dirent **, dirent **))sort);
#elif defined(_AIX)
  // AIX is almost standard...
  int n = scandir(dirloc, list, 0, (int(*)(void*, void*))sort);
#elif defined(__sgi)
  int n = scandir(dirloc, list, 0, sort);
#else
  // The vast majority of UNIX systems want the sort function to have this
  // prototype, most likely so that it can be passed to qsort without any
  // changes:
  int n = scandir(dirloc, list, 0, (int(*)(const void*,const void*))sort);
#endif
  
  free(dirloc);
  
  // convert every filename to UTF-8, and append a '/' to all
  // filenames that are directories
  int i;
  char *fullname = (char*)malloc(dirlen+FL_PATH_MAX+3); // Add enough extra for two /'s and a nul
  // Use memcpy for speed since we already know the length of the string...
  memcpy(fullname, d, dirlen+1);
  
  char *name = fullname + dirlen;
  if (name!=fullname && name[-1]!='/')
    *name++ = '/';
  
  for (i=0; i<n; i++) {
    int newlen;
    dirent *de = (*list)[i];
    int len = strlen(de->d_name);
    newlen = fl_utf8from_mb(NULL, 0, de->d_name, len);
    dirent *newde = (dirent*)malloc(de->d_name - (char*)de + newlen + 2); // Add space for a / and a nul
    
    // Conversion to UTF-8
    memcpy(newde, de, de->d_name - (char*)de);
    fl_utf8from_mb(newde->d_name, newlen + 1, de->d_name, len);
    
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

const char *Fl_Posix_System_Driver::getpwnam(const char *login) {
  struct passwd *pwd;
  pwd = ::getpwnam(login);
  return pwd ? pwd->pw_dir : NULL;
}

//
// End of "$Id$".
//
