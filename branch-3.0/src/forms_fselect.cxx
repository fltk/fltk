//
// "$Id$"
//
// Forms file selection routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

// Emulate the Forms file chooser using the fltk file chooser.

#include <FL/forms.H>
#include "flstring.h"

static char fl_directory[1024];
static const char *fl_pattern;  // assumed passed value is static
static char fl_filename[1024];

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
