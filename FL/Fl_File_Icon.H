//
// "$Id: Fl_File_Icon.H,v 1.1.2.6 2002/05/10 00:18:37 easysw Exp $"
//
// Fl_File_Icon definitions.
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

//
// Include necessary header files...
//

#ifndef _Fl_Fl_File_Icon_H_
#  define _Fl_Fl_File_Icon_H_

#  include "Fl.H"


//
// Special color value for the icon color.
//

#  define FL_ICON_COLOR (Fl_Color)0xffffffff


//
// Fl_File_Icon class...
//

class Fl_File_Icon			//// Icon data
{
  FL_EXPORT static Fl_File_Icon *first_;	// Pointer to first icon/filetype
  Fl_File_Icon	*next_;		// Pointer to next icon/filetype
  const char	*pattern_;	// Pattern string
  int		type_;		// Match only if directory or file?
  int		num_data_;	// Number of data elements
  int		alloc_data_;	// Number of allocated elements
  short		*data_;		// Icon data

  public:

  enum				// File types
  {
    ANY,			// Any kind of file
    PLAIN,			// Only plain files
    FIFO,			// Only named pipes
    DEVICE,			// Only character and block devices
    LINK,			// Only symbolic links
    DIRECTORY			// Only directories
  };

  enum				// Data opcodes
  {
    END,			// End of primitive/icon
    COLOR,			// Followed by color value (2 shorts)
    LINE,			// Start of line
    CLOSEDLINE,			// Start of closed line
    POLYGON,			// Start of polygon
    OUTLINEPOLYGON,		// Followed by outline color (2 shorts)
    VERTEX			// Followed by scaled X,Y
  };

  FL_EXPORT Fl_File_Icon(const char *p, int t, int nd = 0, short *d = 0);
  FL_EXPORT ~Fl_File_Icon();

  FL_EXPORT short	*add(short d);
  short			*add_color(Fl_Color c)
			{ short *d = add(COLOR); add(c >> 16); add(c); return (d); }
  short			*add_vertex(int x, int y)
			{ short *d = add(VERTEX); add(x); add(y); return (d); }
  short			*add_vertex(float x, float y)
			{ short *d = add(VERTEX); add((int)(x * 10000.0));
			  add((int)(y * 10000.0)); return (d); }
  void			clear() { num_data_ = 0; }
  FL_EXPORT void	draw(int x, int y, int w, int h, Fl_Color ic, int active = 1);
  FL_EXPORT void	label(Fl_Widget *w);
  FL_EXPORT static void	labeltype(const Fl_Label *o, int x, int y, int w, int h, Fl_Align a);
  FL_EXPORT void	load(const char *f);
  FL_EXPORT int		load_fti(const char *fti);
  FL_EXPORT int		load_image(const char *i);
  Fl_File_Icon		*next() { return (next_); }
  const char		*pattern() { return (pattern_); }
  int			size() { return (num_data_); }
  int			type() { return (type_); }
  short			*value() { return (data_); }

  FL_EXPORT static Fl_File_Icon *find(const char *filename, int filetype = ANY);
  static Fl_File_Icon *first() { return (first_); }
  FL_EXPORT static void	load_system_icons(void);
};

#endif // !_Fl_Fl_File_Icon_H_

//
// End of "$Id: Fl_File_Icon.H,v 1.1.2.6 2002/05/10 00:18:37 easysw Exp $".
//
