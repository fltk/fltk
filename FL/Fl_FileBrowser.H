//
// "$Id: Fl_FileBrowser.H,v 1.4 2000/01/16 07:44:20 robertk Exp $"
//
// Fl_FileBrowser definitions for the Fast Light Tool Kit (FLTK).
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

//
// Include necessary header files...
//

#ifndef _FL_FILEBROWSER_H_
#  define _FL_FILEBROWSER_H_

#  include <FL/Fl_Browser.H>
#  include <FL/Fl_FileIcon.H>


//
// Fl_FileBrowser class...
//

class FL_API Fl_FileBrowser : public Fl_Browser
{
  const char	*directory_;
  uchar		iconsize_;
  const char	*pattern_;

  int		item_height(void *) const;
  int		item_width(void *) const;
  void		item_draw(void *, int, int, int, int) const;
  int		incr_height() const { return (item_height(0)); }

public:
  Fl_FileBrowser(int, int, int, int, const char * = 0);

  uchar		iconsize() const { return (iconsize_); };
  void		iconsize(uchar s) { iconsize_ = s; redraw(); };

  void		directory(const char *directory) { load(directory); }
  const char	*directory(void) const { return (directory_); }

  void		filter(const char *pattern);
  const char	*filter() const { return (pattern_); };

  int		load(const char *directory);

#ifdef FLTK_2
  unsigned	textsize() const { return (Fl_Browser::text_size()); };
  void		textsize(unsigned s) { Fl_Browser::text_size(s); };
#else
  uchar		textsize() const { return (Fl_Browser::textsize()); };
  void		textsize(uchar s) { Fl_Browser::textsize(s); };
#endif  

};

#endif // !_FL_FILEBROWSER_H_

//
// End of "$Id: Fl_FileBrowser.H,v 1.4 2000/01/16 07:44:20 robertk Exp $".
//
