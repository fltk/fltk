//
// "$Id: Fl_FileChooser2.cxx,v 1.15.2.2 2001/08/04 12:21:33 easysw Exp $"
//
// More Fl_FileChooser routines.
//
// Copyright 1999-2001 by Michael Sweet.
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
//   Fl_FileChooser::directory()  - Set the directory in the file chooser.
//   Fl_FileChooser::count()      - Return the number of selected files.
//   Fl_FileChooser::value()      - Return a selected filename.
//   Fl_FileChooser::up()         - Go up one directory.
//   Fl_FileChooser::newdir()     - Make a new directory.
//   Fl_FileChooser::rescan()     - Rescan the current directory.
//   Fl_FileChooser::fileListCB() - Handle clicks (and double-clicks) in the
//                               FileBrowser.
//   Fl_FileChooser::fileNameCB() - Handle text entry in the FileBrowser.
//

//
// Include necessary headers.
//

#include <FL/Fl_FileChooser.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>
#include <FL/x.H>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

#if defined(WIN32)
#  include <direct.h>
#  include <io.h>
#else
#  include <unistd.h>
#  include <pwd.h>
#endif /* WIN32 */


//
// 'Fl_FileChooser::directory()' - Set the directory in the file chooser.
//

void
Fl_FileChooser::directory(const char *d)	// I - Directory to change to
{
  char	pathname[1024],			// Full path of directory
	*pathptr,			// Pointer into full path
	*dirptr;			// Pointer into directory
  int	levels;				// Number of levels in directory


//  printf("Fl_FileChooser::directory(\"%s\")\n", d == NULL ? "(null)" : d);

  // NULL == current directory
  if (d == NULL)
    d = ".";

  if (d[0] != '\0')
  {
    // Make the directory absolute...
#if defined(WIN32) || defined(__EMX__)
    if (d[0] != '/' && d[0] != '\\' && d[1] != ':')
#else
    if (d[0] != '/' && d[0] != '\\')
#endif /* WIN32 || __EMX__ */
      filename_absolute(directory_, d);
    else
    {
      strncpy(directory_, d, sizeof(directory_) - 1);
      directory_[sizeof(directory_) - 1] = '\0';
    }

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
#if defined(WIN32) || defined(__EMX__)
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
// 'Fl_FileChooser::count()' - Return the number of selected files.
//

int				// O - Number of selected files
Fl_FileChooser::count()
{
  int		i;		// Looping var
  int		count;		// Number of selected files
  const char	*filename;	// Filename in input field or list
  char		pathname[1024];	// Full path to file


  if (type_ != MULTI)
  {
    // Check to see if the file name input field is blank...
    filename = fileName->value();
    if (filename == NULL || filename[0] == '\0')
      return (0);

    // Is the file name a directory?
    if (directory_[0] != '\0')
      sprintf(pathname, "%s/%s", directory_, filename);
    else
    {
      strncpy(pathname, filename, sizeof(pathname) - 1);
      pathname[sizeof(pathname) - 1] = '\0';
    }

    if (filename_isdir(pathname))
      return (0);
    else
      return (1);
  }

  for (i = 1, count = 0; i <= fileList->size(); i ++)
    if (fileList->selected(i))
    {
      // See if this file is a directory...
      filename = (char *)fileList->text(i);
      if (directory_[0] != '\0')
	sprintf(pathname, "%s/%s", directory_, filename);
      else
      {
	strncpy(pathname, filename, sizeof(pathname) - 1);
	pathname[sizeof(pathname) - 1] = '\0';
      }

      if (!filename_isdir(pathname))
	count ++;
    }

  return (count);
}


//
// 'Fl_FileChooser::value()' - Return a selected filename.
//

const char *			// O - Filename or NULL
Fl_FileChooser::value(int f)	// I - File number
{
  int		i;		// Looping var
  int		count;		// Number of selected files
  const char	*name;		// Current filename
  static char	pathname[1024];	// Filename + directory


  if (type_ != MULTI)
  {
    name = fileName->value();
    if (name[0] == '\0')
      return (NULL);

    sprintf(pathname, "%s/%s", directory_, name);
    return ((const char *)pathname);
  }

  for (i = 1, count = 0; i <= fileList->size(); i ++)
    if (fileList->selected(i))
    {
      // See if this file is a directory...
      name = fileList->text(i);
      sprintf(pathname, "%s/%s", directory_, name);

      if (!filename_isdir(pathname))
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
// 'Fl_FileChooser::value()' - Set the current filename.
//

void
Fl_FileChooser::value(const char *filename)	// I - Filename + directory
{
  int	i,					// Looping var
  	count;					// Number of items in list
  char	*slash;					// Directory separator
  char	pathname[1024];				// Local copy of filename


//  printf("Fl_FileChooser::value(\"%s\")\n", filename == NULL ? "(null)" : filename);

  // See if the filename is actually a directory...
  if (filename == NULL || !filename[0] || filename_isdir(filename))
  {
    // Yes, just change the current directory...
    directory(filename);
    return;
  }

  // Switch to single-selection mode as needed
  if (type_ == MULTI)
    type(SINGLE);

  // See if there is a directory in there...
  strncpy(pathname, filename, sizeof(pathname) - 1);
  pathname[sizeof(pathname) - 1] = '\0';

  if ((slash = strrchr(pathname, '/')) == NULL)
    slash = strrchr(pathname, '\\');

  if (slash != NULL)
  {
    // Yes, change the display to the directory... 
    *slash++ = '\0';
    directory(pathname);
  }
  else
    slash = pathname;

  // Set the input field to the remaining portion
  fileName->value(slash);
  fileName->position(0, strlen(slash));
  okButton->activate();

  // Then find the file in the file list and select it...
  count = fileList->size();

  for (i = 1; i <= count; i ++)
    if (strcmp(fileList->text(i), slash) == 0)
    {
      fileList->select(i);
      break;
    }
}


//
// 'Fl_FileChooser::up()' - Go up one directory.
//

void
Fl_FileChooser::up()
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
// 'Fl_FileChooser::newdir()' - Make a new directory.
//

void
Fl_FileChooser::newdir()
{
  const char	*dir;		// New directory name
  char		pathname[1024];	// Full path of directory


  // Get a directory name from the user
  if ((dir = fl_input("New Directory?", NULL)) == NULL)
    return;

  // Make it relative to the current directory as needed...
#if defined(WIN32) || defined(__EMX__)
  if (dir[0] != '/' && dir[0] != '\\' && dir[1] != ':')
#else
  if (dir[0] != '/' && dir[0] != '\\')
#endif /* WIN32 || __EMX__ */
    sprintf(pathname, "%s/%s", directory_, dir);
  else
  {
    strncpy(pathname, dir, sizeof(pathname) - 1);
    pathname[sizeof(pathname) - 1] = '\0';
  }

  // Create the directory; ignore EEXIST errors...
#if defined(WIN32)
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
// 'Fl_FileChooser::rescan()' - Rescan the current directory.
//

void
Fl_FileChooser::rescan()
{
//  printf("Fl_FileChooser::rescan(); directory = \"%s\"\n", directory_);

  // Clear the current filename
  fileName->value("");
  okButton->deactivate();

  // Build the file list...
  fileList->load(directory_);
}


//
// 'Fl_FileChooser::fileListCB()' - Handle clicks (and double-clicks) in the
//                               FileBrowser.
//

void
Fl_FileChooser::fileListCB()
{
  char	*filename,		// New filename
	pathname[1024];		// Full pathname to file


  filename = (char *)fileList->text(fileList->value());
  if (directory_[0] != '\0')
    sprintf(pathname, "%s/%s", directory_, filename);
  else
  {
    strncpy(pathname, filename, sizeof(pathname) - 1);
    pathname[sizeof(pathname) - 1] = '\0';
  }

  if (Fl::event_clicks())
  {
#if defined(WIN32) || defined(__EMX__)
    if ((strlen(pathname) == 2 && pathname[1] == ':') ||
        filename_isdir(pathname))
#else
    if (filename_isdir(pathname))
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
    fileName->value(filename);

    if (!filename_isdir(pathname))
      okButton->activate();
  }
}


//
// 'Fl_FileChooser::fileNameCB()' - Handle text entry in the FileBrowser.
//

void
Fl_FileChooser::fileNameCB()
{
  char		*filename,	// New filename
		*slash,		// Pointer to trailing slash
		pathname[1024];	// Full pathname to file
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

#if defined(WIN32) || defined(__EMX__)
  if (directory_[0] != '\0' &&
      filename[0] != '/' &&
      filename[0] != '\\' &&
      !(isalpha(filename[0]) && filename[1] == ':'))
    sprintf(pathname, "%s/%s", directory_, filename);
  else
  {
    strncpy(pathname, filename, sizeof(pathname) - 1);
    pathname[sizeof(pathname) - 1] = '\0';
  }
#else
  if (filename[0] == '~')
  {
    // Lookup user...
    struct passwd	*pwd;

    if (!filename[1] || filename[1] == '/')
      pwd = getpwuid(getuid());
    else
    {
      strncpy(pathname, filename + 1, sizeof(pathname) - 1);
      pathname[sizeof(pathname) - 1] = '\0';

      i = strlen(pathname) - 1;
      if (pathname[i] == '/')
        pathname[i] = '\0';

      pwd = getpwnam(pathname);
    }

    if (pwd)
    {
      strncpy(pathname, pwd->pw_dir, sizeof(pathname) - 1);
      pathname[sizeof(pathname) - 1] = '\0';

      if (filename[strlen(filename) - 1] == '/')
        strncat(pathname, "/", sizeof(pathname) - 1);
    }
    else
      sprintf(pathname, "%s/%s", directory_, filename);

    endpwent();
  }
  else if (directory_[0] != '\0' &&
           filename[0] != '/')
    sprintf(pathname, "%s/%s", directory_, filename);
  else
  {
    strncpy(pathname, filename, sizeof(pathname) - 1);
    pathname[sizeof(pathname) - 1] = '\0';
  }
#endif /* WIN32 || __EMX__ */

  if (Fl::event_key() == FL_Enter)
  {
    // Enter pressed - select or change directory...

#if defined(WIN32) || defined(__EMX__)
    if ((strlen(pathname) == 2 && pathname[1] == ':') ||
        filename_isdir(pathname))
#else
    if (filename_isdir(pathname))
#endif /* WIN32 || __EMX__ */
      directory(pathname);
    else if (type_ == CREATE || access(pathname, 0) == 0)
    {
      // New file or file exists...  If we are in multiple selection mode,
      // switch to single selection mode...
      if (type_ == MULTI)
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
      // TODO: NEED TO ADD fl_beep() FUNCTION TO 2.0!
#ifdef WIN32
      MessageBeep(MB_ICONEXCLAMATION);
#else
      XBell(fl_display, 100);
#endif // WIN32

      fl_alert("Please choose an existing file!");
    }
  }
  else if (Fl::event_key() != FL_Delete)
  {
    // Check to see if the user has entered a directory...
    if ((slash = strrchr(filename, '/')) == NULL)
      slash = strrchr(filename, '\\');

    if (slash != NULL)
    {
      // Yes, change directories and update the file name field...
      if ((slash = strrchr(pathname, '/')) == NULL)
	slash = strrchr(pathname, '\\');

      if (slash > pathname)		// Special case for "/"
        *slash++ = '\0';
      else
        slash++;

      if (strcmp(filename, "../") == 0)	// Special case for "../"
        up();
      else
        directory(pathname);

      // If the string ended after the slash, we're done for now...
      if (*slash == '\0')
        return;

      // Otherwise copy the remainder and proceed...
      fileName->value(slash);
      fileName->position(strlen(slash));
      filename = slash;
    }

    // Other key pressed - do filename completion as possible...
    num_files  = fileList->size();
    min_match  = strlen(filename);
    max_match  = 100000;
    first_line = 0;

    for (i = 1; i <= num_files && max_match > min_match; i ++)
    {
      file = fileList->text(i);

#if defined(WIN32) || defined(__EMX__)
      if (strnicmp(filename, file, min_match) == 0)
#else
      if (strncmp(filename, file, min_match) == 0)
#endif // WIN32 || __EMX__
      {
        // OK, this one matches; check against the previous match
	if (max_match == 100000)
	{
	  // First match; copy stuff over...
	  strncpy(pathname, file, sizeof(pathname) - 1);
	  pathname[sizeof(pathname) - 1] = '\0';
	  max_match = strlen(pathname);

          // Strip trailing /, if any...
	  if (pathname[max_match - 1] == '/')
	  {
	    max_match --;
	    pathname[max_match] = '\0';
	  }

	  // And then make sure that the item is visible
          fileList->topline(i);
	  first_line = i;
	}
	else
	{
	  // Succeeding match; compare to find maximum string match...
	  while (max_match > min_match)
#if defined(WIN32) || defined(__EMX__)
	    if (strnicmp(file, pathname, max_match) == 0)
#else
	    if (strncmp(file, pathname, max_match) == 0)
#endif // WIN32 || __EMX__
	      break;
	    else
	      max_match --;

          // Truncate the string as needed...
          pathname[max_match] = '\0';
	}
      }
    }

    fileList->deselect(0);
    fileList->redraw();

    // If we have any matches, add them to the input field...
    if (first_line > 0 && min_match == max_match &&
        max_match == (int)strlen(fileList->text(first_line)))
      fileList->select(first_line);
    else if (max_match > min_match && max_match != 100000)
    {
      // Add the matching portion...
      fileName->replace(0, min_match, pathname);

      // Highlight it; if the user just pressed the backspace
      // key, position the cursor at the start of the selection.
      // Otherwise, put the cursor at the end of the selection so
      // s/he can press the right arrow to accept the selection
      // (Tab and End also do this for both cases.)
      if (Fl::event_key() == FL_BackSpace)
        fileName->position(min_match - 1, max_match);
      else
        fileName->position(max_match, min_match);
    }

    // See if we need to enable the OK button...
    sprintf(pathname, "%s/%s", directory_, fileName->value());

    if ((type_ == CREATE || access(pathname, 0) == 0) &&
        !filename_isdir(pathname))
      okButton->activate();
    else
      okButton->deactivate();
  }
}


//
// End of "$Id: Fl_FileChooser2.cxx,v 1.15.2.2 2001/08/04 12:21:33 easysw Exp $".
//
