//
// "$Id: Fl_File_Browser.cxx,v 1.1.2.5 2001/11/25 16:38:11 easysw Exp $"
//
// Fl_File_Browser routines.
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
//   Fl_File_Browser::full_height()    - Return the height of the list.
//   Fl_File_Browser::item_height()    - Return the height of a list item.
//   Fl_File_Browser::item_width()     - Return the width of a list item.
//   Fl_File_Browser::item_draw()      - Draw a list item.
//   Fl_File_Browser::Fl_File_Browser() - Create a Fl_File_Browser widget.
//   Fl_File_Browser::load()           - Load a directory into the browser.
//   Fl_File_Browser::filter()         - Set the filename filter.
//

//
// Include necessary header files...
//

#include <FL/Fl_File_Browser.H>
#include <FL/fl_draw.H>
#include <FL/filename.H>
#include <stdio.h>
#include <stdlib.h>
#include "flstring.h"

#if defined(WIN32) && ! defined(__CYGWIN__)
#  include <windows.h>
#  include <direct.h>
#endif /* WIN32 */

#if defined(__EMX__)
#define  INCL_DOS
#define  INCL_DOSMISC
#include <os2.h>
#endif /* __EMX__ */


//
// FL_BLINE definition from "Fl_Browser.cxx"...
//

#define SELECTED 1
#define NOTDISPLAYED 2

struct FL_BLINE			// data is in a linked list of these
{
  FL_BLINE	*prev;		// Previous item in list
  FL_BLINE	*next;		// Next item in list
  void		*data;		// Pointer to data (function)
  short		length;		// sizeof(txt)-1, may be longer than string
  char		flags;		// selected, displayed
  char		txt[1];		// start of allocated array
};


//
// 'Fl_File_Browser::full_height()' - Return the height of the list.
//

int					// O - Height in pixels
Fl_File_Browser::full_height() const
{
  int	i,				// Looping var
	th;				// Total height of list.


  for (i = 0, th = 0; i < size(); i ++)
    th += item_height(find_line(i));

  return (th);
}


//
// 'Fl_File_Browser::item_height()' - Return the height of a list item.
//

int					// O - Height in pixels
Fl_File_Browser::item_height(void *p) const	// I - List item data
{
  FL_BLINE	*line;			// Pointer to line
  char		*text;			// Pointer into text
  int		height;			// Width of line
  int		textheight;		// Height of text


  // Figure out the standard text height...
  fl_font(textfont(), textsize());
  textheight = fl_height();

  // We always have at least 1 line...
  height = textheight;

  // Scan for newlines...
  line = (FL_BLINE *)p;

  if (line != NULL)
    for (text = line->txt; *text != '\0'; text ++)
      if (*text == '\n')
	height += textheight;

  // If we have enabled icons then add space for them...
  if (Fl_File_Icon::first() != NULL && height < iconsize_)
    height = iconsize_;

  // Add space for the selection border..
  height += 2;

  // Return the height
  return (height);
}


//
// 'Fl_File_Browser::item_width()' - Return the width of a list item.
//

int					// O - Width in pixels
Fl_File_Browser::item_width(void *p) const	// I - List item data
{
  int		i;			// Looping var
  FL_BLINE	*line;			// Pointer to line
  char		*text,			// Pointer into text
		*ptr,			// Pointer into fragment
		fragment[10240];	// Fragment of text
  int		width,			// Width of line
		tempwidth;		// Width of fragment
  int		column;			// Current column
  const int	*columns;		// Columns


  // Set the font and size...
  fl_font(textfont(), textsize());

  // Scan for newlines...
  line    = (FL_BLINE *)p;
  columns = column_widths();

  if (strchr(line->txt, '\n') == NULL &&
      strchr(line->txt, column_char()) == NULL)
  {
    // Do a fast width calculation...
    width = (int)fl_width(line->txt);
  }
  else
  {
    // More than 1 line or have columns; find the maximum width...
    width     = 0;
    tempwidth = 0;
    column    = 0;

    for (text = line->txt, ptr = fragment; *text != '\0'; text ++)
      if (*text == '\n')
      {
        // Newline - nul terminate this fragment and get the width...
        *ptr = '\0';

	tempwidth += (int)fl_width(fragment);

        // Update the max width as needed...
	if (tempwidth > width)
	  width = tempwidth;

        // Point back to the start of the fragment...
	ptr       = fragment;
	tempwidth = 0;
	column    = 0;
      }
      else if (*text == column_char())
      {
        // Advance to the next column...
        column ++;
	if (columns)
	{
	  for (i = 0, tempwidth = 0; i < column && columns[i]; i ++)
	    tempwidth += columns[i];
	}
	else
          tempwidth = column * (int)(fl_height() * 0.6 * 8.0);

        if (tempwidth > width)
	  width = tempwidth;

	ptr = fragment;
      }
      else
        *ptr++ = *text;

    if (ptr > fragment)
    {
      // Nul terminate this fragment and get the width...
      *ptr = '\0';

      tempwidth += (int)fl_width(fragment);

      // Update the max width as needed...
      if (tempwidth > width)
	width = tempwidth;
    }
  }

  // If we have enabled icons then add space for them...
  if (Fl_File_Icon::first() != NULL)
    width += iconsize_ + 8;

  // Add space for the selection border..
  width += 2;

  // Return the width
  return (width);
}


//
// 'Fl_File_Browser::item_draw()' - Draw a list item.
//

void
Fl_File_Browser::item_draw(void *p,	// I - List item data
                	  int  x,	// I - Upper-lefthand X coordinate
			  int  y,	// I - Upper-lefthand Y coordinate
			  int  w,	// I - Width of item
			  int  h) const	// I - Height of item
{
  int		i;			// Looping var
  FL_BLINE	*line;			// Pointer to line
  Fl_Color	c;			// Text color
  char		*text,			// Pointer into text
		*ptr,			// Pointer into fragment
		fragment[10240];	// Fragment of text
  int		width,			// Width of line
		height;			// Height of line
  int		column;			// Current column
  const int	*columns;		// Columns


  (void)h;

  // Draw the list item text...
  line = (FL_BLINE *)p;

  if (line->txt[strlen(line->txt) - 1] == '/')
    fl_font(textfont() | FL_BOLD, textsize());
  else
    fl_font(textfont(), textsize());

  if (line->flags & SELECTED)
    c = fl_contrast(textcolor(), selection_color());
  else
    c = textcolor();

  if (Fl_File_Icon::first() == NULL)
  {
    // No icons, just draw the text...
    x ++;
    w -= 2;
  }
  else
  {
    // Draw the icon if it is set...
    if (line->data)
      ((Fl_File_Icon *)line->data)->draw(x, y, iconsize_, iconsize_,
                                	(line->flags & SELECTED) ? FL_YELLOW :
				                                   FL_LIGHT2,
					active_r());

    // Draw the text offset to the right...
    x += iconsize_ + 9;
    w -= iconsize_ - 10;

    // Center the text vertically...
    height = fl_height();

    for (text = line->txt; *text != '\0'; text ++)
      if (*text == '\n')
	height += fl_height();

    if (height < iconsize_)
      y += (iconsize_ - height) / 2;
  }

  // Draw the text...
  line    = (FL_BLINE *)p;
  columns = column_widths();
  width   = 0;
  column  = 0;

  if (active_r())
    fl_color(c);
  else
    fl_color(fl_inactive(c));

  for (text = line->txt, ptr = fragment; *text != '\0'; text ++)
    if (*text == '\n')
    {
      // Newline - nul terminate this fragment and draw it...
      *ptr = '\0';

      fl_draw(fragment, x + width, y, w - width, fl_height(),
              (Fl_Align)(FL_ALIGN_LEFT | FL_ALIGN_CLIP), 0, 0);

      // Point back to the start of the fragment...
      ptr    = fragment;
      width  = 0;
      y      += fl_height();
      column = 0;
    }
    else if (*text == column_char())
    {
      // Tab - nul terminate this fragment and draw it...
      *ptr = '\0';

      fl_draw(fragment, x + width, y, w - width, fl_height(),
              (Fl_Align)(FL_ALIGN_LEFT | FL_ALIGN_CLIP), 0, 0);

      // Advance to the next column...
      column ++;
      if (columns)
      {
	for (i = 0, width = 0; i < column && columns[i]; i ++)
	  width += columns[i];
      }
      else
        width = column * (int)(fl_height() * 0.6 * 8.0);

      ptr = fragment;
    }
    else
      *ptr++ = *text;

  if (ptr > fragment)
  {
    // Nul terminate this fragment and draw it...
    *ptr = '\0';

    fl_draw(fragment, x + width, y, w - width, fl_height(),
            (Fl_Align)(FL_ALIGN_LEFT | FL_ALIGN_CLIP), 0, 0);
  }
}


//
// 'Fl_File_Browser::Fl_File_Browser()' - Create a Fl_File_Browser widget.
//

Fl_File_Browser::Fl_File_Browser(int        x,	// I - Upper-lefthand X coordinate
                         int        y,	// I - Upper-lefthand Y coordinate
			 int        w,	// I - Width in pixels
			 int        h,	// I - Height in pixels
			 const char *l)	// I - Label text
    : Fl_Browser(x, y, w, h, l)
{
  // Initialize the filter pattern, current directory, and icon size...
  pattern_   = "*";
  directory_ = "";
  iconsize_  = 3 * textsize() / 2;
  filetype_  = FILES;
}


//
// 'Fl_File_Browser::load()' - Load a directory into the browser.
//

int				// O - Number of files loaded
Fl_File_Browser::load(const char *directory)// I - Directory to load
{
  int		i;		// Looping var
  int		num_files;	// Number of files in directory
  int		num_dirs;	// Number of directories in list
  char		filename[4096];	// Current file
  Fl_File_Icon	*icon;		// Icon to use


//  printf("Fl_File_Browser::load(\"%s\")\n", directory);

  clear();
  directory_ = directory;

  if (directory_[0] == '\0')
  {
    //
    // No directory specified; for UNIX list all mount points.  For DOS
    // list all valid drive letters...
    //

    num_files = 0;
    if ((icon = Fl_File_Icon::find("any", Fl_File_Icon::DEVICE)) == NULL)
      icon = Fl_File_Icon::find("any", Fl_File_Icon::DIRECTORY);

#if defined(WIN32) && ! defined (__CYGWIN__)
    DWORD	drives;		// Drive available bits


    drives = GetLogicalDrives();
    for (i = 'A'; i <= 'Z'; i ++, drives >>= 1)
      if (drives & 1)
      {
        sprintf(filename, "%c:/", i);

	if (i < 'C')
	  add(filename, icon);
	else
	  add(filename, icon);

	num_files ++;
      }
#elif defined(__EMX__)
    ULONG	curdrive;	// Current drive
    ULONG	drives;		// Drive available bits
    int		start = 3;      // 'C' (MRS - dunno if this is correct!)


    DosQueryCurrentDisk(&curdrive, &drives);
    drives >>= start - 1;
    for (i = 'A'; i <= 'Z'; i ++, drives >>= 1)
      if (drives & 1)
      {
        sprintf(filename, "%c:/", i);
        add(filename, icon);

	num_files ++;
      }
#else
    FILE	*mtab;		// /etc/mtab or /etc/mnttab file
    char	line[1024];	// Input line


    //
    // Open the file that contains a list of mounted filesystems...
    //
#  if defined(hpux) || defined(__sun)
    mtab = fopen("/etc/mnttab", "r");	// Fairly standard
#  elif defined(__sgi) || defined(linux)
    mtab = fopen("/etc/mtab", "r");	// More standard
#  else
    mtab = fopen("/etc/fstab", "r");	// Otherwise fallback to full list
    if (mtab == NULL)
      mtab = fopen("/etc/vfstab", "r");
#  endif

    if (mtab != NULL)
    {
      while (fgets(line, sizeof(line), mtab) != NULL)
      {
        if (line[0] == '#' || line[0] == '\n')
	  continue;
        if (sscanf(line, "%*s%4095s", filename) != 1)
	  continue;

        strncat(filename, "/", sizeof(filename) - 1);

//        printf("Fl_File_Browser::load() - adding \"%s\" to list...\n", filename);
        add(filename, icon);
	num_files ++;
      }

      fclose(mtab);
    }
#endif // WIN32 || __EMX__
  }
  else
  {
    dirent	**files;	// Files in in directory


    //
    // Build the file list...
    //

#if (defined(WIN32) && ! defined(__CYGWIN__)) || defined(__EMX__)
    strncpy(filename, directory_, sizeof(filename) - 1);
    filename[sizeof(filename) - 1] = '\0';
    i = strlen(filename) - 1;

    if (i == 2 && filename[1] == ':' &&
        (filename[2] == '/' || filename[2] == '\\'))
      filename[2] = '/';
    else if (filename[i] != '/' && filename[i] != '\\')
      strcat(filename, "/");

    num_files = filename_list(filename, &files);
#else
    num_files = filename_list(directory_, &files);
#endif /* WIN32 || __EMX__ */

    if (num_files <= 0)
      return (0);

    for (i = 0, num_dirs = 0; i < num_files; i ++)
    {
      if (strcmp(files[i]->d_name, ".") != 0 &&
          strcmp(files[i]->d_name, "..") != 0)
      {
	snprintf(filename, sizeof(filename), "%s/%s", directory_,
	         files[i]->d_name);

	if (filename_isdir(filename))
	{
	  char name[1024]; // Temporary directory name

	  snprintf(name, sizeof(name), "%s/", files[i]->d_name);

          num_dirs ++;
          insert(num_dirs, name, Fl_File_Icon::find(filename));
	}
	else if (filetype_ == FILES &&
	         filename_match(files[i]->d_name, pattern_))
          add(files[i]->d_name, Fl_File_Icon::find(filename));
      }

      free(files[i]);
    }

    free(files);
  }

  return (num_files);
}


//
// 'Fl_File_Browser::filter()' - Set the filename filter.
//

void
Fl_File_Browser::filter(const char *pattern)	// I - Pattern string
{
  // If pattern is NULL set the pattern to "*"...
  if (pattern)
    pattern_ = pattern;
  else
    pattern_ = "*";

  // Reload the current directory...
  load(directory_);
}


//
// End of "$Id: Fl_File_Browser.cxx,v 1.1.2.5 2001/11/25 16:38:11 easysw Exp $".
//
