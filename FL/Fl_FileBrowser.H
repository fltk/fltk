//
// "$Id: Fl_FileBrowser.H,v 1.4.2.2 2001/08/11 14:49:51 easysw Exp $"
//
// FileBrowser definitions.
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

//
// Include necessary header files...
//

#ifndef _Fl_FileBrowser_H_
#  define _Fl_FileBrowser_H_

#  include "Fl_Browser.H"
#  include "Fl_FileIcon.H"


//
// Fl_FileBrowser class...
//

class Fl_FileBrowser : public Fl_Browser
{
  const char	*directory_;
  uchar		iconsize_;
  const char	*pattern_;

  int		full_height() const;
  int		item_height(void *) const;
  int		item_width(void *) const;
  void		item_draw(void *, int, int, int, int) const;
  int		incr_height() const { return (item_height(0)); }

public:
  FL_EXPORT Fl_FileBrowser(int, int, int, int, const char * = 0);

  uchar			iconsize() const { return (iconsize_); };
  void			iconsize(uchar s) { iconsize_ = s; redraw(); };

  FL_EXPORT void	filter(const char *pattern);
  const char		*filter() const { return (pattern_); };

  FL_EXPORT int		load(const char *directory);

  uchar			textsize() const { return (Fl_Browser::textsize()); };
  void			textsize(uchar s) { Fl_Browser::textsize(s); iconsize_ = 3 * s / 2; };
};

#endif // !_Fl_FileBrowser_H_

//
// End of "$Id: Fl_FileBrowser.H,v 1.4.2.2 2001/08/11 14:49:51 easysw Exp $".
//
