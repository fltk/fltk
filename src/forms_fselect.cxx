//
// "$Id$"
//
// Forms file selection routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

// Emulate the Forms file chooser using the fltk file chooser.

#include <FL/forms.H>
#include "flstring.h"

static char fl_directory[FL_PATH_MAX];
static const char *fl_pattern;  // assumed passed value is static
static char fl_filename[FL_PATH_MAX];

char* fl_show_file_selector(const char *message,const char *dir,
			    const char *pat,const char *fname) {
  if (dir && dir[0]) strlcpy(fl_directory,dir,sizeof(fl_directory));
  if (pat && pat[0]) fl_pattern = pat;
  if (fname && fname[0]) strlcpy(fl_filename,fname,sizeof(fl_filename));
  char *p = fl_directory+strlen(fl_directory);
  if (p > fl_directory && *(p-1)!='/'
#ifdef WIN32
      && *(p-1)!='\\' && *(p-1)!=':'
#endif
      ) *p++ = '/';
  strlcpy(p,fl_filename,sizeof(fl_directory) - (p - fl_directory));
  const char *q = fl_file_chooser(message,fl_pattern,fl_directory);
  if (!q) return 0;
  strlcpy(fl_directory, q, sizeof(fl_directory));
  p = (char *)fl_filename_name(fl_directory);
  strlcpy(fl_filename, p, sizeof(fl_filename));
  if (p > fl_directory+1) p--;
  *p = 0;
  return (char *)q;
}

char*	fl_get_directory() {return fl_directory;}

char*	fl_get_pattern() {return (char *)fl_pattern;}

char*	fl_get_filename() {return fl_filename;}

//
// End of "$Id$".
//
