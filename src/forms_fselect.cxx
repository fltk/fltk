//
// "$Id: forms_fselect.cxx,v 1.4.2.3 2001/01/22 15:13:41 easysw Exp $"
//
// Forms file selection routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

// Emulate the Forms file chooser using the fltk file chooser.

#include <FL/forms.H>
#include <string.h>

static char fl_directory[1024];
static const char *fl_pattern;  // assummed passed value is static
static char fl_filename[256];

char* fl_show_file_selector(const char *message,const char *dir,
			    const char *pat,const char *fname) {
  if (dir && dir[0]) strncpy(fl_directory,dir,1023);
  if (pat && pat[0]) fl_pattern = pat;
  if (fname && fname[0]) strncpy(fl_filename,fname,255);
  char *p = fl_directory+strlen(fl_directory);
  if (p > fl_directory && *(p-1)!='/'
#ifdef WIN32
      && *(p-1)!='\\' && *(p-1)!=':'
#endif
      ) *p++ = '/';
  strcpy(p,fl_filename);
  const char *q = fl_file_chooser(message,fl_pattern,fl_directory);
  if (!q) return 0;
  strcpy(fl_directory, q);
  p = (char *)filename_name(fl_directory);
  strcpy(fl_filename, p);
  if (p > fl_directory+1) p--;
  *p = 0;
  return (char *)q;
}

char*	fl_get_directory() {return fl_directory;}

char*	fl_get_pattern() {return (char *)fl_pattern;}

char*	fl_get_filename() {return fl_filename;}

//
// End of "$Id: forms_fselect.cxx,v 1.4.2.3 2001/01/22 15:13:41 easysw Exp $".
//
