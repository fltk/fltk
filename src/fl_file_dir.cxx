//
// "$Id: fl_file_dir.cxx,v 1.1.2.9 2002/04/28 08:42:33 easysw Exp $"
//
// File chooser widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2002 by Bill Spitzak and others.
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

#include <config.h>
#include <FL/filename.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_File_Chooser.H>

static Fl_File_Chooser	*fc = (Fl_File_Chooser *)0;
static void		(*current_callback)(const char*) = 0;


static void callback(Fl_File_Chooser *, void*) {
  if (current_callback)
    (*current_callback)(fc->value(0));
}


void fl_file_chooser_callback(void (*cb)(const char*)) {
  current_callback = cb;
}


char* fl_file_chooser(const char* message, const char* pat, const char* fname)
{
  static char	retname[1024];

  if (!fc) {
    if (!fname || !*fname) fname = ".";

    fc = new Fl_File_Chooser(fname, pat, Fl_File_Chooser::CREATE, message);
    fc->callback(callback, 0);
  } else {
    fc->type(Fl_File_Chooser::CREATE);
    fc->filter(pat);
    fc->label(message);

    if (!fname || !*fname) {
      if (fc->filter() != pat && (!pat || !fc->filter() ||
          strcmp(pat, fc->filter())) && fc->value()) {
	// if pattern is different, remove name but leave old directory:
	strncpy(retname, fc->value(), sizeof(retname) - 1);
	retname[sizeof(retname) - 1] = '\0';

	char *p = strrchr(retname, '/');

        if (p) {
	  // If the filename is "/foo", then the directory will be "/", not
	  // ""...
	  if (p == retname)
	    retname[1] = '\0';
	  else
	    *p = '\0';
	}

	// Set the directory...
	fc->directory(retname);
      }
    }
    else
      fc->value(fname);
  }

  fc->show();

  while (fc->shown())
    Fl::wait();

  if (fc->value()) {
    fl_filename_relative(retname, sizeof(retname), fc->value());

    return retname;
  } else return 0;
}


char* fl_dir_chooser(const char* message, const char* fname)
{
  static char	retname[1024];

  if (!fname || !*fname) fname = ".";

  if (!fc) {
    fc = new Fl_File_Chooser(fname, "*", Fl_File_Chooser::CREATE |
                                         Fl_File_Chooser::DIRECTORY, message);
    fc->callback(callback, 0);
  } else {
    fc->type(Fl_File_Chooser::CREATE | Fl_File_Chooser::DIRECTORY);
    fc->filter("*");
    fc->value(fname);
    fc->label(message);
  }

  fc->show();

  while (fc->shown())
    Fl::wait();

  if (fc->value()) {
    fl_filename_relative(retname, sizeof(retname), fc->value());

    return retname;
  } else return 0;
}


//
// End of "$Id: fl_file_dir.cxx,v 1.1.2.9 2002/04/28 08:42:33 easysw Exp $".
//
