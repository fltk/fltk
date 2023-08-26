//
// Filename expansion routines for the Fast Light Tool Kit (FLTK).
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

/* expand a file name by prepending current directory, deleting . and
   .. (not really correct for symbolic links) between the prepended
   current directory.  Use $PWD if it exists.
   Returns true if any changes were made.
*/

#include <FL/filename.H>
#include <FL/Fl.H>
#include <FL/Fl_String.H>
#include <FL/fl_string_functions.h>
#include "Fl_System_Driver.H"
#include <stdlib.h>
#include "flstring.h"

static inline int isdirsep(char c) {return c == '/';}

/** Makes a filename absolute from a relative filename.
    \code
    #include <FL/filename.H>
    [..]
    fl_chdir("/var/tmp");
    fl_filename_absolute(out, sizeof(out), "foo.txt");         // out="/var/tmp/foo.txt"
    fl_filename_absolute(out, sizeof(out), "./foo.txt");       // out="/var/tmp/foo.txt"
    fl_filename_absolute(out, sizeof(out), "../log/messages"); // out="/var/log/messages"
    \endcode
    \param[out] to resulting absolute filename
    \param[in]  tolen size of the absolute filename buffer
    \param[in]  from relative filename
    \return 0 if no change, non zero otherwise
 */
int fl_filename_absolute(char *to, int tolen, const char *from) {
  return Fl::system_driver()->filename_absolute(to, tolen, from);
}


/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

int Fl_System_Driver::filename_absolute(char *to, int tolen, const char *from) {
  if (isdirsep(*from) || *from == '|') {
    strlcpy(to, from, tolen);
    return 0;
  }
  char *a;
  char *temp = new char[tolen];
  const char *start = from;
  a = fl_getcwd(temp, tolen);
  if (!a) {
    strlcpy(to, from, tolen);
    delete[] temp;
    return 0;
  }
  a = temp+strlen(temp);
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
      if (start[2]==0)
        start += 2; // Skip to end of path
      else
        start += 3; // Skip over dir separator
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

/**
 \}
 \endcond
 */


/** Makes a filename relative to the current working directory.
    \code
    #include <FL/filename.H>
    [..]
    fl_chdir("/var/tmp/somedir");       // set cwd to /var/tmp/somedir
    [..]
    char out[FL_PATH_MAX];
    fl_filename_relative(out, sizeof(out), "/var/tmp/somedir/foo.txt");  // out="foo.txt",    return=1
    fl_filename_relative(out, sizeof(out), "/var/tmp/foo.txt");          // out="../foo.txt", return=1
    fl_filename_relative(out, sizeof(out), "foo.txt");                   // out="foo.txt",    return=0 (no change)
    fl_filename_relative(out, sizeof(out), "./foo.txt");                 // out="./foo.txt",  return=0 (no change)
    fl_filename_relative(out, sizeof(out), "../foo.txt");                // out="../foo.txt", return=0 (no change)
    \endcode
    \param[out] to resulting relative filename
    \param[in]  tolen size of the relative filename buffer
    \param[in]  from absolute filename
    \return 0 if no change, non zero otherwise
 */
int                                     // O - 0 if no change, 1 if changed
fl_filename_relative(char       *to,    // O - Relative filename
                     int        tolen,  // I - Size of "to" buffer
                     const char *from)  // I - Absolute filename
{
  char cwd_buf[FL_PATH_MAX];    // Current directory
  // get the current directory and return if we can't
  if (!fl_getcwd(cwd_buf, sizeof(cwd_buf))) {
    strlcpy(to, from, tolen);
    return 0;
  }
  return fl_filename_relative(to, tolen, from, cwd_buf);
}


/** Makes a filename relative to any other directory.
 \param[out] to resulting relative filename
 \param[in]  tolen size of the relative filename buffer
 \param[in]  from absolute filename
 \param[in]  base relative to this absolute path
 \return 0 if no change, non zero otherwise
 */
int                                     // O - 0 if no change, 1 if changed
fl_filename_relative(char       *to,    // O - Relative filename
                     int        tolen,  // I - Size of "to" buffer
                     const char *from,  // I - Absolute filename
                     const char *base) { // I - Find path relative to this path
  return Fl::system_driver()->filename_relative(to, tolen, from, base);
}


/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

int                                             // O - 0 if no change, 1 if changed
Fl_System_Driver::filename_relative(char *to,   // O - Relative filename
                     int        tolen,          // I - Size of "to" buffer
                     const char *from,          // I - Absolute filename
                     const char *base)          // I - Find path relative to this path
{
  char          *newslash;              // Directory separator
  const char    *slash;                 // Directory separator
  char          *cwd = 0L, *cwd_buf = 0L;
  if (base) cwd = cwd_buf = fl_strdup(base);

  // return if "from" is not an absolute path
  if (from[0] == '\0' || !isdirsep(*from)) {
    strlcpy(to, from, tolen);
    if (cwd_buf) free(cwd_buf);
    return 0;
  }

  // return if "cwd" is not an absolute path
  if (!cwd || cwd[0] == '\0' || !isdirsep(*cwd)) {
    strlcpy(to, from, tolen);
    if (cwd_buf) free(cwd_buf);
    return 0;
  }

  // test for the exact same string and return "." if so
  if (!strcmp(from, cwd)) {
    strlcpy(to, ".", tolen);
    free(cwd_buf);
    return (1);
  }

  // compare both path names until we find a difference
  for (slash = from, newslash = cwd;
       *slash != '\0' && *newslash != '\0';
       slash ++, newslash ++)
    if (isdirsep(*slash) && isdirsep(*newslash)) continue;
    else if (*slash != *newslash) break;

  // skip over trailing slashes
  if ( *newslash == '\0' && *slash != '\0' && !isdirsep(*slash)
      &&(newslash==cwd || !isdirsep(newslash[-1])) )
    newslash--;

  // now go back to the first character of the first differing paths segment
  while (!isdirsep(*slash) && slash > from) slash --;
  if (isdirsep(*slash)) slash ++;

  // do the same for the current dir
  if (isdirsep(*newslash)) newslash --;
  if (*newslash != '\0')
    while (!isdirsep(*newslash) && newslash > cwd) newslash --;

  // prepare the destination buffer
  to[0]         = '\0';
  to[tolen - 1] = '\0';

  // now add a "previous dir" sequence for every following slash in the cwd
  while (*newslash != '\0') {
    if (isdirsep(*newslash)) strlcat(to, "../", tolen);

    newslash ++;
  }

  // finally add the differing path from "from"
  strlcat(to, slash, tolen);

  free(cwd_buf);
  return 1;
}

/**
 \}
 \endcond
 */


/**
 Return a new string that contains the name part of the filename.
 \param[in] filename file path and name
 \return the name part of a filename
 \see fl_filename_name(const char *filename)
 */
Fl_String fl_filename_name(const Fl_String &filename) {
  return Fl_String(fl_filename_name(filename.c_str()));
}

/**
 Return a new string that contains the path part of the filename.
 \param[in] filename file path and name
 \return the path part of a filename without the name
 \see fl_filename_name(const char *filename)
 */
Fl_String fl_filename_path(const Fl_String &filename) {
  const char *base = filename.c_str();
  const char *name = fl_filename_name(base);
  if (name) {
    return Fl_String(base, (int)(name-base));
  } else {
    return Fl_String();
  }
}

/**
 Return a new string that contains the filename extension.
 \param[in] filename file path and name
 \return the filename extension including the prepending '.', or an empty
    string if the filename has no extension
 \see fl_filename_ext(const char *buf)
 */
Fl_String fl_filename_ext(const Fl_String &filename) {
  return Fl_String(fl_filename_ext(filename.c_str()));
}

/**
 Return a copy of the old filename with the new extension.
 \param[in] filename file path and name
 \param[in] new_extension new filename extension, starts with a '.'
 \return the new filename
 \see fl_filename_setext(char *to, int tolen, const char *ext)
 */
Fl_String fl_filename_setext(const Fl_String &filename, const Fl_String &new_extension) {
  char buffer[FL_PATH_MAX];
  fl_strlcpy(buffer, filename.c_str(), FL_PATH_MAX);
  fl_filename_setext(buffer, FL_PATH_MAX, new_extension.c_str());
  return Fl_String(buffer);
}

/**
 Expands a filename containing shell variables and tilde (~).
 \param[in] from file path and name
 \return the new, expanded filename
 \see fl_filename_expand(char *to, int tolen, const char *from)
*/
Fl_String fl_filename_expand(const Fl_String &from) {
  char buffer[FL_PATH_MAX];
  fl_filename_expand(buffer, FL_PATH_MAX, from.c_str());
  return Fl_String(buffer);
}

/**
 Makes a filename absolute from a filename relative to the current working directory.
 \param[in] from relative filename
 \return the new, absolute filename
 \see fl_filename_absolute(char *to, int tolen, const char *from)
 */
Fl_String fl_filename_absolute(const Fl_String &from) {
  char buffer[FL_PATH_MAX];
  fl_filename_absolute(buffer, FL_PATH_MAX, from.c_str());
  return Fl_String(buffer);
}

/**
 Makes a filename relative to the current working directory.
 \param[in] from file path and name
 \return the new, relative filename
 \see fl_filename_relative(char *to, int tolen, const char *from)
 */
Fl_String fl_filename_relative(const Fl_String &from) {
  char buffer[FL_PATH_MAX];
  fl_filename_relative(buffer, FL_PATH_MAX, from.c_str());
  return Fl_String(buffer);
}
