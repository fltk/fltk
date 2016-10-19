//
// "$Id$"
//
// Fl_File_Browser routines.
//
// Copyright 1999-2010 by Michael Sweet.
// Copyright 2016 by Bill Spitzak and others.
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
// Contents:
//
//   Fl_File_Browser::full_height()     - Return the height of the list.
//   Fl_File_Browser::item_height()     - Return the height of a list item.
//   Fl_File_Browser::item_width()      - Return the width of a list item.
//   Fl_File_Browser::item_draw()       - Draw a list item.
//   Fl_File_Browser::Fl_File_Browser() - Create a Fl_File_Browser widget.
//   Fl_File_Browser::load()            - Load a directory into the browser.
//   Fl_File_Browser::filter()          - Set the filename filter.
//

//
// Include necessary header files...
//

#include <FL/Fl_File_Browser.H>
#include <FL/Fl.H>
#include <FL/Fl_System_Driver.H>
#include <FL/fl_draw.H>
#include <FL/filename.H>
#include <FL/Fl_Image.H>	// icon
#include <stdio.h>
#include <stdlib.h>
#include "flstring.h"

//
// FL_BLINE definition from "Fl_Browser.cxx"...
//

#define SELECTED 1
#define NOTDISPLAYED 2

// TODO -- Warning: The definition of FL_BLINE here is a hack.
//    Fl_File_Browser should not do this. PLEASE FIX.
//    FL_BLINE should be private to Fl_Browser, and not re-defined here.
//    For now, make sure this struct is precisely consistent with Fl_Browser.cxx.
//
struct FL_BLINE			// data is in a linked list of these
{
  FL_BLINE	*prev;		// Previous item in list
  FL_BLINE	*next;		// Next item in list
  void		*data;		// Pointer to data (function)
  Fl_Image      *icon;		// Pointer to optional icon
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
  char		*t;			// Pointer into text
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
    for (t = line->txt; *t != '\0'; t ++)
      if (*t == '\n')
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
  char		*t,			// Pointer into text
		*ptr,			// Pointer into fragment
		fragment[10240];	// Fragment of text
  int		width,			// Width of line
		tempwidth;		// Width of fragment
  int		column;			// Current column
  const int	*columns;		// Columns


  // Scan for newlines...
  line    = (FL_BLINE *)p;
  columns = column_widths();

  // Set the font and size...
  if (line->txt[strlen(line->txt) - 1] == '/')
    fl_font(textfont() | FL_BOLD, textsize());
  else
    fl_font(textfont(), textsize());

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

    for (t = line->txt, ptr = fragment; *t != '\0'; t ++)
      if (*t == '\n')
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
      else if (*t == column_char())
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
        *ptr++ = *t;

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
                 	   int  X,	// I - Upper-lefthand X coordinate
		 	   int  Y,	// I - Upper-lefthand Y coordinate
		 	   int  W,	// I - Width of item
			   int) const	// I - Height of item
{
  int		i;			// Looping var
  FL_BLINE	*line;			// Pointer to line
  Fl_Color	c;			// Text color
  char		*t,			// Pointer into text
		*ptr,			// Pointer into fragment
		fragment[10240];	// Fragment of text
  int		width,			// Width of line
		height;			// Height of line
  int		column;			// Current column
  const int	*columns;		// Columns


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
    X ++;
    W -= 2;
  }
  else
  {
    // Draw the icon if it is set...
    if (line->data)
      ((Fl_File_Icon *)line->data)->draw(X, Y, iconsize_, iconsize_,
                                	(line->flags & SELECTED) ? FL_YELLOW :
				                                   FL_LIGHT2,
					active_r());

    // Draw the text offset to the right...
    X += iconsize_ + 9;
    W -= iconsize_ - 10;

    // Center the text vertically...
    height = fl_height();

    for (t = line->txt; *t != '\0'; t ++)
      if (*t == '\n')
	height += fl_height();

    if (height < iconsize_)
      Y += (iconsize_ - height) / 2;
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

  for (t = line->txt, ptr = fragment; *t != '\0'; t ++) {
    if (*t == '\n') {
      // Newline - nul terminate this fragment and draw it...
      *ptr = '\0';

      fl_draw(fragment, X + width, Y, W - width, fl_height(),
              (Fl_Align)(FL_ALIGN_LEFT | FL_ALIGN_CLIP), 0, 0);

      // Point back to the start of the fragment...
      ptr    = fragment;
      width  = 0;
      Y      += fl_height();
      column = 0;
    } else if (*t == column_char()) {
      // Tab - nul terminate this fragment and draw it...
      *ptr = '\0';

      int cW = W - width; // Clip width...

      if (columns) {
	// Try clipping inside this column...
	for (i = 0; i < column && columns[i]; i ++) { ; }

	if (columns[i])
	  cW = columns[i];
      }

      fl_draw(fragment, X + width, Y, cW, fl_height(),
              (Fl_Align)(FL_ALIGN_LEFT | FL_ALIGN_CLIP), 0, 0);

      // Advance to the next column...
      column ++;
      if (columns) {
	for (i = 0, width = 0; i < column && columns[i]; i ++)
	  width += columns[i];
      }
      else
	width = column * (int)(fl_height() * 0.6 * 8.0);
      ptr = fragment;
    }
    else
      *ptr++ = *t;
  }
  if (ptr > fragment) {
    // Nul terminate this fragment and draw it...
    *ptr = '\0';

    fl_draw(fragment, X + width, Y, W - width, fl_height(),
            (Fl_Align)(FL_ALIGN_LEFT | FL_ALIGN_CLIP), 0, 0);
  }
}


//
// 'Fl_File_Browser::Fl_File_Browser()' - Create a Fl_File_Browser widget.
//

Fl_File_Browser::Fl_File_Browser(int        X,  // I - Upper-lefthand X coordinate
                        	 int        Y,  // I - Upper-lefthand Y coordinate
				 int        W,  // I - Width in pixels
				 int        H,  // I - Height in pixels
				 const char *l)	// I - Label text
    : Fl_Browser(X, Y, W, H, l)
{
  // Initialize the filter pattern, current directory, and icon size...
  pattern_   = "*";
  directory_ = "";
  iconsize_  = (uchar)(3 * textsize() / 2);
  filetype_  = FILES;
}


//
// 'Fl_File_Browser::load()' - Load a directory into the browser.
//

int						// O - Number of files loaded
Fl_File_Browser::load(const char     *directory,// I - Directory to load
                      Fl_File_Sort_F *sort)	// I - Sort function to use
{
  int		i;				// Looping var
  int		num_files;			// Number of files in directory
  int		num_dirs;			// Number of directories in list
  char		filename[4096];			// Current file
  Fl_File_Icon	*icon;				// Icon to use


//  printf("Fl_File_Browser::load(\"%s\")\n", directory);

  clear();

  directory_ = directory;

  if (!directory)
    return (0);

  if (directory_[0] == '\0')
  {
    //
    // No directory specified; for UNIX list all mount points.  For DOS
    // list all valid drive letters...
    //
    if ((icon = Fl_File_Icon::find("any", Fl_File_Icon::DEVICE)) == NULL)
      icon = Fl_File_Icon::find("any", Fl_File_Icon::DIRECTORY);
    num_files = Fl::system_driver()->file_browser_load_filesystem(this, filename, (int)sizeof(filename), icon);
  }
  else
  {
    dirent	**files;	// Files in in directory
    //
    // Build the file list...
    //
    num_files = Fl::system_driver()->file_browser_load_directory(directory_, filename, sizeof(filename), &files, sort);
    if (num_files <= 0)
      return (0);

    for (i = 0, num_dirs = 0; i < num_files; i ++) {
      if (strcmp(files[i]->d_name, "./")) {
	snprintf(filename, sizeof(filename), "%s/%s", directory_,
	         files[i]->d_name);

        icon = Fl_File_Icon::find(filename);
	if ((icon && icon->type() == Fl_File_Icon::DIRECTORY) ||
            Fl::system_driver()->filename_isdir_quick(filename)) {
          num_dirs ++;
          insert(num_dirs, files[i]->d_name, icon);
	} else if (filetype_ == FILES &&
	           fl_filename_match(files[i]->d_name, pattern_)) {
          add(files[i]->d_name, icon);
	}
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
}


//
// End of "$Id$".
//
