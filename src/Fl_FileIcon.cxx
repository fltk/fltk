//
// "$Id: Fl_FileIcon.cxx,v 1.10 2001/07/29 22:04:43 spitzak Exp $"
//
// Fl_FileIcon routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1997-1999 by Easy Software Products.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//
// Contents:
//
//   Fl_FileIcon::Fl_FileIcon()       - Create a new file icon.
//   Fl_FileIcon::~Fl_FileIcon()      - Remove a file icon.
//   Fl_FileIcon::add()               - Add data to an icon.
//   Fl_FileIcon::find()              - Find an icon based upon a given file.
//   Fl_FileIcon::draw()              - Draw an icon.
//

//
// Include necessary header files...
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
# include <io.h>
# define F_OK 0
#else
# include <unistd.h>
#endif

#include <fltk/Fl_FileIcon.h>
#include <fltk/Fl_Widget.h>
#include <fltk/fl_draw.h>
#include <fltk/filename.h>


//
// Define missing POSIX/XPG4 macros as needed...
//

#ifndef S_ISDIR
#  define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#  define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#  define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#  define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif /* !S_ISDIR */


//
// Icon cache...
//

Fl_FileIcon	*Fl_FileIcon::first_ = (Fl_FileIcon *)0;


//
// 'Fl_FileIcon::Fl_FileIcon()' - Create a new file icon.
//

Fl_FileIcon::Fl_FileIcon(const char *p,	/* I - Filename pattern */
                         int        t,	/* I - File type */
		         int        nd,	/* I - Number of data values */
		         short      *d)	/* I - Data values */
{
  // Initialize the pattern and type...
  pattern_ = p;
  type_    = t;

  // Copy icon data as needed...
  if (nd)
  {
    num_data_   = nd;
    alloc_data_ = nd + 1;
    data_       = (short *)calloc(sizeof(short), nd + 1);
    memcpy(data_, d, nd * sizeof(short));
  }
  else
  {
    num_data_   = 0;
    alloc_data_ = 0;
  }

  // And add the icon to the list of icons...
  next_  = first_;
  first_ = this;
}


//
// 'Fl_FileIcon::~Fl_FileIcon()' - Remove a file icon.
//

Fl_FileIcon::~Fl_FileIcon()
{
  Fl_FileIcon	*current,	// Current icon in list
		*prev;		// Previous icon in list


  // Find the icon in the list...
  for (current = first_, prev = (Fl_FileIcon *)0;
       current != this && current != (Fl_FileIcon *)0;
       prev = current, current = current->next_);

  // Remove the icon from the list as needed...
  if (current)
  {
    if (prev)
      prev->next_ = current->next_;
    else
      first_ = current->next_;
  }

  // Free any memory used...
  if (alloc_data_)
    free(data_);
}


//
// 'Fl_FileIcon::add()' - Add data to an icon.
//

short *			// O - Pointer to new data value
Fl_FileIcon::add(short d)	// I - Data to add
{
  short	*dptr;		// Pointer to new data value


  // Allocate/reallocate memory as needed
  if ((num_data_ + 1) >= alloc_data_)
  {
    alloc_data_ += 128;

    if (alloc_data_ == 128)
      dptr = (short *)malloc(sizeof(short) * alloc_data_);
    else
      dptr = (short *)realloc(data_, sizeof(short) * alloc_data_);

    if (dptr == NULL)
      return (NULL);

    data_ = dptr;
  }

  // Store the new data value and return
  data_[num_data_++] = d;
  data_[num_data_]   = END;

  return (data_ + num_data_ - 1);
}


//
// 'Fl_FileIcon::find()' - Find an icon based upon a given file.
//

Fl_FileIcon *				// O - Matching file icon or NULL
Fl_FileIcon::find(const char *filename,	// I - Name of file */
                  int        filetype)	// I - Enumerated file type
{
  Fl_FileIcon	*current;		// Current file in list
  struct stat	fileinfo;		// Information on file


  // Get file information if needed...
  if (filetype == ANY)
    if (!stat(filename, &fileinfo))
    {
      if (S_ISDIR(fileinfo.st_mode))
        filetype = DIR;
#ifdef S_IFIFO
      else if (S_ISFIFO(fileinfo.st_mode))
        filetype = FIFO;
#endif // S_IFIFO
#if defined(S_ICHR) && defined(S_IBLK)
      else if (S_ISCHR(fileinfo.st_mode) || S_ISBLK(fileinfo.st_mode))
        filetype = DEVICE;
#endif // S_ICHR && S_IBLK
#ifdef S_ILNK
      else if (S_ISLNK(fileinfo.st_mode))
        filetype = LINK;
#endif // S_ILNK
      else
        filetype = PLAIN;
    }

  // Loop through the available file types and return any match that
  // is found...
  for (current = first_; current != (Fl_FileIcon *)0; current = current->next_)
    if ((current->type_ == filetype || current->type_ == ANY) &&
        filename_match(filename, current->pattern_))
      break;

  // Return the match (if any)...
  return (current);
}


//
// 'Fl_FileIcon::draw()' - Draw an icon.
//

void
Fl_FileIcon::draw(int      x,		// I - Upper-lefthand X
                  int      y,		// I - Upper-lefthand Y
	          int      w,		// I - Width of bounding box
	          int      h,		// I - Height of bounding box
                  Fl_Color ic,		// I - Icon color...
		  int      active)	// I - Active or inactive?
{
  Fl_Color	c;		// Current color
  short		*d;		// Pointer to data
  short		*prim;		// Pointer to start of primitive...
  double	scale;		// Scale of icon


  // Don't try to draw a NULL array!
  if (num_data_ == 0)
    return;

  // Setup the transform matrix as needed...
  scale = w < h ? w : h;

  fl_push_matrix();
  fl_translate((float)x + 0.5 * ((float)w - scale),
               (float)y + 0.5 * ((float)h + scale));
  fl_scale(scale, -scale);

  // Loop through the array until we see an unmatched END...
  d    = data_;
  prim = NULL;
  c    = ic;

  if (active)
    fl_color(c);
  else
    fl_color(fl_inactive(c));

  while (*d != END || prim)
    switch (*d)
    {
      case END :
          switch (*prim)
	  {
	    case LINE :
		fl_stroke();
		break;

	    case CLOSEDLINE :
		fl_closepath();
		fl_stroke();
		break;

	    case POLYGON :
		fl_fill();
		break;

	    case OUTLINEPOLYGON : {
		Fl_Color color = prim[1]==256 ? ic : (Fl_Color)prim[1];
		if (!active) color = fl_inactive(color);
		fl_fill_stroke(color);
		break;}
	  }

          prim = NULL;
	  d ++;
	  break;

      case COLOR :
          if (d[1] == 256)
	    c = ic;
	  else
	    c = (Fl_Color)d[1];

          if (!active)
	    c = fl_inactive(c);

          fl_color(c);
	  d += 2;
	  break;

      case LINE :
      case CLOSEDLINE :
      case POLYGON :
          prim = d;
	  d ++;
	  break;

      case OUTLINEPOLYGON :
          prim = d;
	  d += 2;
	  break;

      case VERTEX :
          if (prim)
	    fl_vertex(d[1] * 0.0001, d[2] * 0.0001);
	  d += 3;
	  break;
    }

  // Restore the transform matrix
  fl_pop_matrix();
}


//
// End of "$Id: Fl_FileIcon.cxx,v 1.10 2001/07/29 22:04:43 spitzak Exp $".
//
