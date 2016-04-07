//
// "$Id$"
//
// Filename expansion routines for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl_System_Driver.H>
#include <FL/Fl.H>

/** Expands a filename containing shell variables and tilde (~).
    Currently handles these variants:
    \code
    "~username"               // if 'username' does not exist, result will be unchanged
    "~/file"
    "$VARNAME"                // does NOT handle ${VARNAME}
    \endcode

    \b Examples:
    \code
    #include <FL/filename.H>
    [..]
    putenv("TMPDIR=/var/tmp");
    fl_filename_expand(out, sizeof(out), "~fred/.cshrc");     // out="/usr/fred/.cshrc"
    fl_filename_expand(out, sizeof(out), "~/.cshrc");         // out="/usr/<yourname>/.cshrc"
    fl_filename_expand(out, sizeof(out), "$TMPDIR/foo.txt");  // out="/var/tmp/foo.txt"
    \endcode
    \param[out] to resulting expanded filename
    \param[in]  tolen size of the expanded filename buffer 
    \param[in]  from filename containing shell variables
    \return 0 if no change, non zero otherwise
 */
int fl_filename_expand(char *to,int tolen, const char *from) {
  return Fl::system_driver()->filename_expand(to, tolen, from);
}

//
// End of "$Id$".
//
