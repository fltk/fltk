//
// "$Id: Fl_File_Chooser2.cxx,v 1.1.2.14 2002/06/06 21:26:12 easysw Exp $"
//
// More Fl_File_Chooser routines.
//
// Copyright 1999-2002 by Michael Sweet.
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
// Contents:
//
//   Fl_File_Chooser::directory()  - Set the directory in the file chooser.
//   Fl_File_Chooser::count()      - Return the number of selected files.
//   Fl_File_Chooser::value()      - Return a selected filename.
//   Fl_File_Chooser::up()         - Go up one directory.
//   Fl_File_Chooser::newdir()     - Make a new directory.
//   Fl_File_Chooser::rescan()     - Rescan the current directory.
//   Fl_File_Chooser::fileListCB() - Handle clicks (and double-clicks) in the
//                                   FileBrowser.
//   Fl_File_Chooser::fileNameCB() - Handle text entry in the FileBrowser.
//

//
// Include necessary headers.
//

#include <FL/Fl_File_Chooser.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>
#include <FL/x.H>

#include <stdio.h>
#include <stdlib.h>
#include "flstring.h"
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(WIN32) && ! defined (__CYGWIN__)
#  include <direct.h>
#  include <io.h>
#else
#  include <unistd.h>
#  include <pwd.h>
#endif /* WIN32 */


//
// File chooser label strings...
//

const char	*Fl_File_Chooser::directory_label = "Directory:";
const char	*Fl_File_Chooser::filename_label = "Filename:";
const char	*Fl_File_Chooser::filter_label = "New Filter?";
Fl_File_Sort_F	*Fl_File_Chooser::sort = fl_numericsort;


//
// 'Fl_File_Chooser::directory()' - Set the directory in the file chooser.
//

void
Fl_File_Chooser::directory(const char *d)// I - Directory to change to
{
  char	pathname[1024],			// Full path of directory
	*pathptr,			// Pointer into full path
	*dirptr;			// Pointer into directory
  int	levels;				// Number of levels in directory


//  printf("Fl_File_Chooser::directory(\"%s\")\n", d == NULL ? "(null)" : d);

  // NULL == current directory
  if (d == NULL)
    d = ".";

  if (d[0] != '\0')
  {
    // Make the directory absolute...
#if (defined(WIN32) && ! defined(__CYGWIN__))|| defined(__EMX__)
    if (d[0] != '/' && d[0] != '\\' && d[1] != ':')
#else
    if (d[0] != '/' && d[0] != '\\')
#endif /* WIN32 || __EMX__ */
      fl_filename_absolute(directory_, d);
    else
      strlcpy(directory_, d, sizeof(directory_));

    // Strip any trailing slash and/or period...
    dirptr = directory_ + strlen(directory_) - 1;
    if (*dirptr == '.')
      *dirptr-- = '\0';
    if ((*dirptr == '/' || *dirptr == '\\') && dirptr > directory_)
      *dirptr = '\0';
  }
  else
    directory_[0] = '\0';

  // Clear the directory menu and fill it as needed...
  dirMenu->clear();
#if (defined(WIN32) && ! defined(__CYGWIN__)) || defined(__EMX__)
  dirMenu->add("My Computer");
#else
  dirMenu->add("File Systems");
#endif /* WIN32 || __EMX__ */

  levels = 0;
  for (dirptr = directory_, pathptr = pathname; *dirptr != '\0';)
  {
    if (*dirptr == '/' || *dirptr == '\\')
    {
      // Need to quote the slash first, and then add it to the menu...
      *pathptr++ = '\\';
      *pathptr++ = '/';
      *pathptr++ = '\0';
      dirptr ++;

      dirMenu->add(pathname);
      levels ++;
      pathptr = pathname;
    }
    else
      *pathptr++ = *dirptr++;
  }

  if (pathptr > pathname)
  {
    *pathptr = '\0';
    dirMenu->add(pathname);
    levels ++;
  }

  dirMenu->value(levels);

  // Rescan the directory...
  rescan();
}


//
// 'Fl_File_Chooser::count()' - Return the number of selected files.
//

int				// O - Number of selected files
Fl_File_Chooser::count()
{
  int		i;		// Looping var
  int		count;		// Number of selected files
  const char	*filename;	// Filename in input field or list
  char		pathname[1024];	// Full path to file


  if (!(type_ & MULTI))
  {
    // Check to see if the file name input field is blank...
    filename = fileName->value();
    if (filename == NULL || filename[0] == '\0')
      return (0);

    // Is the file name just the current directory?
    return (strcmp(filename, directory_) != 0);
  }

  for (i = 1, count = 0; i <= fileList->size(); i ++)
    if (fileList->selected(i))
    {
      // See if this file is a directory...
      filename = (char *)fileList->text(i);
      if (directory_[0] != '\0')
	snprintf(pathname, sizeof(pathname), "%s/%s", directory_, filename);
      else
	strlcpy(pathname, filename, sizeof(pathname));

      if (!fl_filename_isdir(pathname))
	count ++;
    }

  return (count);
}


//
// 'Fl_File_Chooser::value()' - Return a selected filename.
//

const char *			// O - Filename or NULL
Fl_File_Chooser::value(int f)	// I - File number
{
  int		i;		// Looping var
  int		count;		// Number of selected files
  const char	*name;		// Current filename
  static char	pathname[1024];	// Filename + directory


  if (!(type_ & MULTI))
  {
    name = fileName->value();
    if (name[0] == '\0') return NULL;
    else if (fl_filename_isdir(name)) {
      if (type_ & DIRECTORY) return name;
      else return NULL;
    } else return name;
  }

  for (i = 1, count = 0; i <= fileList->size(); i ++)
    if (fileList->selected(i))
    {
      // See if this file is a directory...
      name = fileList->text(i);

      if (directory_[0]) {
	snprintf(pathname, sizeof(pathname), "%s/%s", directory_, name);
      } else {
	strlcpy(pathname, name, sizeof(pathname));
      }

      if (!fl_filename_isdir(pathname))
      {
        // Nope, see if this this is "the one"...
	count ++;
	if (count == f)
          return ((const char *)pathname);
      }
    }

  return (NULL);
}


//
// 'Fl_File_Chooser::value()' - Set the current filename.
//

void
Fl_File_Chooser::value(const char *filename)	// I - Filename + directory
{
  int	i,					// Looping var
  	count;					// Number of items in list
  char	*slash;					// Directory separator
  char	pathname[1024];				// Local copy of filename


//  printf("Fl_File_Chooser::value(\"%s\")\n", filename == NULL ? "(null)" : filename);

  // See if the filename is the "My System" directory...
  if (filename == NULL || !filename[0]) {
    // Yes, just change the current directory...
    directory(filename);
    fileName->value("");
    okButton->deactivate();
    return;
  }

  // Switch to single-selection mode as needed
  if (type_ & MULTI)
    type(SINGLE);

  // See if there is a directory in there...
  fl_filename_absolute(pathname, sizeof(pathname), filename);

  if ((slash = strrchr(pathname, '/')) == NULL)
    slash = strrchr(pathname, '\\');

  if (slash != NULL)
  {
    // Yes, change the display to the directory... 
    *slash++ = '\0';
    directory(pathname);
  }
  else
  {
    directory(".");
    slash = pathname;
  }

  // Set the input field to the absolute path...
  if (slash > pathname) slash[-1] = '/';

  fileName->value(pathname);
  fileName->position(0, strlen(pathname));
  okButton->activate();

  // Then find the file in the file list and select it...
  count = fileList->size();

  fileList->deselect(0);
  fileList->redraw();

  for (i = 1; i <= count; i ++)
#if defined(WIN32) || defined(__EMX__)
    if (strcasecmp(fileList->text(i), slash) == 0) {
#else
    if (strcmp(fileList->text(i), slash) == 0) {
#endif // WIN32 || __EMX__
//      printf("Selecting line %d...\n", i);
      fileList->topline(i);
      fileList->select(i);
      break;
    }
}


//
// 'Fl_File_Chooser::up()' - Go up one directory.
//

void
Fl_File_Chooser::up()
{
  char *slash;		// Trailing slash


  if ((slash = strrchr(directory_, '/')) == NULL)
    slash = strrchr(directory_, '\\');

  if (directory_[0] != '\0')
    dirMenu->value(dirMenu->value() - 1);

  if (slash != NULL)
    *slash = '\0';
  else
  {
    upButton->deactivate();
    directory_[0] = '\0';
  }

  rescan();
}


//
// 'Fl_File_Chooser::newdir()' - Make a new directory.
//

void
Fl_File_Chooser::newdir()
{
  const char	*dir;		// New directory name
  char		pathname[1024];	// Full path of directory


  // Get a directory name from the user
  if ((dir = fl_input("New Directory?", NULL)) == NULL)
    return;

  // Make it relative to the current directory as needed...
#if (defined(WIN32) && ! defined (__CYGWIN__)) || defined(__EMX__)
  if (dir[0] != '/' && dir[0] != '\\' && dir[1] != ':')
#else
  if (dir[0] != '/' && dir[0] != '\\')
#endif /* WIN32 || __EMX__ */
    snprintf(pathname, sizeof(pathname), "%s/%s", directory_, dir);
  else
    strlcpy(pathname, dir, sizeof(pathname));

  // Create the directory; ignore EEXIST errors...
#if defined(WIN32) && ! defined (__CYGWIN__)
  if (mkdir(pathname))
#else
  if (mkdir(pathname, 0777))
#endif /* WIN32 */
    if (errno != EEXIST)
    {
      fl_alert("Unable to create directory!");
      return;
    }

  // Show the new directory...
  directory(pathname);
}


//
// 'Fl_File_Chooser::rescan()' - Rescan the current directory.
//

void
Fl_File_Chooser::rescan()
{
  char	pathname[1024];		// New pathname for filename field

//  printf("Fl_File_Chooser::rescan(); directory = \"%s\"\n", directory_);

  // Clear the current filename
  strlcpy(pathname, directory_, sizeof(pathname));
  if (pathname[strlen(pathname) - 1] != '/') {
    strlcat(pathname, "/", sizeof(pathname));
  }
  fileName->value(pathname);
  okButton->deactivate();

  // Build the file list...
  fileList->load(directory_, sort);
}


//
// 'Fl_File_Chooser::fileListCB()' - Handle clicks (and double-clicks) in the
//                               FileBrowser.
//

void
Fl_File_Chooser::fileListCB()
{
  char	*filename,		// New filename
	pathname[1024];		// Full pathname to file


  filename = (char *)fileList->text(fileList->value());
  if (!filename)
    return;

  if (directory_[0] != '\0')
    snprintf(pathname, sizeof(pathname), "%s/%s", directory_, filename);
  else
    strlcpy(pathname, filename, sizeof(pathname));

  if (Fl::event_clicks())
  {
#if (defined(WIN32) && ! defined(__CYGWIN__)) || defined(__EMX__)
    if ((strlen(pathname) == 2 && pathname[1] == ':') ||
        fl_filename_isdir(pathname))
#else
    if (fl_filename_isdir(pathname))
#endif /* WIN32 || __EMX__ */
    {
      directory(pathname);
      upButton->activate();
    }
    else
    {
      // Do any callback that is registered...
      if (callback_)
        (*callback_)(this, data_);

      // Hide the window...
      window->hide();
    }
  }
  else
  {
    // Strip any trailing slash from the directory name...
    filename = pathname + strlen(pathname) - 1;
    if (*filename == '/') *filename = '\0';

    fileName->value(pathname);

    if (!fl_filename_isdir(pathname) || (type_ & DIRECTORY))
      okButton->activate();
  }
}


//
// 'Fl_File_Chooser::fileNameCB()' - Handle text entry in the FileBrowser.
//

void
Fl_File_Chooser::fileNameCB()
{
  char		*filename,	// New filename
		*slash,		// Pointer to trailing slash
		pathname[1024],	// Full pathname to file
		matchname[256];	// Matching filename
  int		i,		// Looping var
		min_match,	// Minimum number of matching chars
		max_match,	// Maximum number of matching chars
		num_files,	// Number of files in directory
		first_line;	// First matching line
  const char	*file;		// File from directory


  // Get the filename from the text field...
  filename = (char *)fileName->value();

  if (filename == NULL || filename[0] == '\0')
  {
    okButton->deactivate();
    return;
  }

  // Expand ~ and $ variables as needed...
  if (strchr(filename, '~') || strchr(filename, '$')) {
    fl_filename_expand(pathname, sizeof(pathname), filename);
    filename = pathname;
    value(pathname);
  }

  // Make sure we have an absolute path...
#if (defined(WIN32) && !defined(__CYGWIN__)) || defined(__EMX__)
  if (directory_[0] != '\0' && filename[0] != '/' &&
      filename[0] != '\\' &&
      !(isalpha(filename[0]) && filename[1] == ':')) {
#else
  if (directory_[0] != '\0' && filename[0] != '/') {
#endif /* WIN32 || __EMX__ */
    fl_filename_absolute(pathname, sizeof(pathname), filename);
    value(pathname);
  } else if (filename != pathname) {
    // Finally, make sure that we have a writable copy...
    strlcpy(pathname, filename, sizeof(pathname));
  }

  filename = pathname;

  // Now process things according to the key pressed...
  if (Fl::event_key() == FL_Enter)
  {
    // Enter pressed - select or change directory...
#if (defined(WIN32) && ! defined(__CYGWIN__)) || defined(__EMX__)
    if ((strlen(pathname) == 2 && pathname[1] == ':') ||
        fl_filename_isdir(pathname))
#else
    if (fl_filename_isdir(pathname))
#endif /* WIN32 || __EMX__ */
      directory(pathname);
    else if ((type_ & CREATE) || access(pathname, 0) == 0)
    {
      // New file or file exists...  If we are in multiple selection mode,
      // switch to single selection mode...
      if (type_ & MULTI)
        type(SINGLE);

      // Do any callback that is registered...
      if (callback_)
        (*callback_)(this, data_);

      // Hide the window to signal things are done...
      window->hide();
    }
    else
    {
      // File doesn't exist, so beep at and alert the user...
      fl_alert("Please choose an existing file!");
    }
  }
  else if (Fl::event_key() != FL_Delete &&
           Fl::event_key() != FL_BackSpace)
  {
    // Check to see if the user has entered a directory...
    if ((slash = strrchr(pathname, '/')) == NULL)
      slash = strrchr(pathname, '\\');

    if (slash != NULL)
    {
      // Yes, change directories if necessary...
      if (slash > pathname)		// Special case for "/"
        *slash++ = '\0';
      else
        slash++;

      filename = slash;

#if defined(WIN32) || defined(__EMX__)
      if (strcasecmp(pathname, directory_)) {
#else
      if (strcmp(pathname, directory_)) {
#endif // WIN32 || __EMX__
        int p = fileName->position();
	int m = fileName->mark();

        directory(pathname);

	fileName->position(p, m);
      }
    }

    // Other key pressed - do filename completion as possible...
    num_files  = fileList->size();
    min_match  = strlen(filename);
    max_match  = 100000;
    first_line = 0;

    for (i = 1; i <= num_files && max_match > min_match; i ++)
    {
      file = fileList->text(i);

#if (defined(WIN32) && ! defined(__CYGWIN__)) || defined(__EMX__)
      if (strnicmp(filename, file, min_match) == 0)
#else
      if (strncmp(filename, file, min_match) == 0)
#endif // WIN32 || __EMX__
      {
        // OK, this one matches; check against the previous match
	if (max_match == 100000)
	{
	  // First match; copy stuff over...
	  strlcpy(matchname, file, sizeof(matchname));
	  max_match = strlen(matchname);

          // Strip trailing /, if any...
	  if (matchname[max_match - 1] == '/')
	  {
	    max_match --;
	    matchname[max_match] = '\0';
	  }

	  // And then make sure that the item is visible
          fileList->topline(i);
	  first_line = i;
	}
	else
	{
	  // Succeeding match; compare to find maximum string match...
	  while (max_match > min_match)
#if (defined(WIN32) && ! defined(__CYGWIN__)) || defined(__EMX__)
	    if (strnicmp(file, matchname, max_match) == 0)
#else
	    if (strncmp(file, matchname, max_match) == 0)
#endif // WIN32 || __EMX__
	      break;
	    else
	      max_match --;

          // Truncate the string as needed...
          matchname[max_match] = '\0';
	}
      }
    }

    // If we have any matches, add them to the input field...
    if (first_line > 0 && min_match == max_match &&
        max_match == (int)strlen(fileList->text(first_line))) {
      // This is the only possible match...
      fileList->deselect(0);
      fileList->select(first_line);
      fileList->redraw();
    }
    else if (max_match > min_match && max_match != 100000)
    {
      // Add the matching portion...
      fileName->replace(filename - pathname, filename - pathname + min_match,
                        matchname);

      // Highlight it with the cursor at the end of the selection so
      // s/he can press the right arrow to accept the selection
      // (Tab and End also do this for both cases.)
      fileName->position(filename - pathname + max_match,
	                 filename - pathname + min_match);
    }
    else if (max_match == 0) {
      fileList->deselect(0);
      fileList->redraw();
    }

    // See if we need to enable the OK button...
    if ((type_ & CREATE || access(fileName->value(), 0) == 0) &&
        (!fl_filename_isdir(fileName->value()) || type_ & DIRECTORY))
      okButton->activate();
    else
      okButton->deactivate();
  } else {
    // FL_Delete or FL_BackSpace
    fileList->deselect(0);
    fileList->redraw();
    okButton->deactivate();
  }
}


//
// End of "$Id: Fl_File_Chooser2.cxx,v 1.1.2.14 2002/06/06 21:26:12 easysw Exp $".
//
