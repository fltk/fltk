//
// "$Id: Fl_Help_View.H,v 1.1.2.3 2001/10/29 03:44:31 easysw Exp $"
//
// Help Viewer widget definitions.
//
// Copyright 1997-2001 by Easy Software Products.
// Image support donated by Matthias Melcher, Copyright 2000.
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

#ifndef _Fl_Help_View_H_
#  define _Fl_Help_View_H_

//
// Include necessary header files...
//

#  include <stdio.h>
#  include <FL/Fl.H>
#  include <FL/Fl_Group.H>
#  include <FL/Fl_Scrollbar.H>
#  include <FL/fl_draw.H>


//
// Fl_Help_Func type - link callback function for files...
//


typedef const char *(Fl_Help_Func)(Fl_Widget *, const char *);


//
// Fl_Help_Block structure...
//

struct Fl_Help_Block
{
  const char	*start,		// Start of text
		*end;		// End of text
  uchar		font,		// Text font
		size,		// Text size
		border;		// Draw border?
  int		x,		// Indentation/starting X coordinate
		y,		// Starting Y coordinate
		w,		// Width
		h;		// Height
  int		line[32];	// Left starting position for each line
};

//
// Fl_Help_Link structure...
//

struct Fl_Help_Link
{
  char		filename[192],	// Reference filename
		name[32];	// Link target (blank if none)
  int		x,		// X offset of link text
		y,		// Y offset of link text
		w,		// Width of link text
		h;		// Height of link text
};

//
// Fl_Help_Target structure...
//

struct Fl_Help_Target
{
  char		name[32];	// Target name
  int		y;		// Y offset of target
};

//
// Fl_Help_Image structure...
//

class Fl_Image;

struct Fl_Help_Image 
{
  char		*name,		// Path and name of the image
		wattr[8],	// Width attribute
		hattr[8];	// Height attribute
  Fl_Image	*image;		// FLTK image representation
  unsigned char	*data;		// Raw image data
  int		copy;		// Data is a copy?
  int		w, h, d;	// Image size & depth
};

//
// Fl_Help_View class...
//

class Fl_Help_View : public Fl_Group	//// Help viewer widget
{
  enum { RIGHT = -1, CENTER, LEFT };	// Alignments

  char		title_[1024];		// Title string
  Fl_Color	defcolor_,		// Default text color
		bgcolor_,		// Background color
		textcolor_,		// Text color
		linkcolor_;		// Link color
  uchar		textfont_,		// Default font for text
		textsize_;		// Default font size
  const char	*value_;		// HTML text value

  int		nblocks_,		// Number of blocks/paragraphs
		ablocks_;		// Allocated blocks
  Fl_Help_Block	*blocks_;		// Blocks

  int		nfonts_;		// Number of fonts in stack
  uchar		fonts_[100][2];		// Font stack

  Fl_Help_Func	*link_;			// Link transform function

  int		nlinks_,		// Number of links
		alinks_;		// Allocated links
  Fl_Help_Link	*links_;		// Links

  int		ntargets_,		// Number of targets
		atargets_;		// Allocated targets
  Fl_Help_Target	*targets_;		// Targets

  char		directory_[1024];	// Directory for current file
  char		filename_[1024];	// Current filename
  int		topline_,		// Top line in document
		size_;			// Total document length
  Fl_Scrollbar	scrollbar_;		// Vertical scrollbar for document

  int		nimage_,		// Number of images in a page
		aimage_;		// Allocated blocks
  Fl_Help_Image	*image_;		// list of image descriptors

  Fl_Help_Image	*add_image(const char *name, const char *wattr,
		           const char *hattr, int make = 1);
  Fl_Help_Image	*find_image(const char *name, const char *wattr,
		           const char *hattr);
  int		load_gif(Fl_Help_Image *img, FILE *fp);
  int		load_jpeg(Fl_Help_Image *img, FILE *fp);
  int		load_png(Fl_Help_Image *img, FILE *fp);

  Fl_Help_Block	*add_block(const char *s, int xx, int yy, int ww, int hh, uchar border = 0);
  static int	compare_blocks(const void *a, const void *b);
  void		add_link(const char *n, int xx, int yy, int ww, int hh);
  void		add_target(const char *n, int yy);
  static int	compare_targets(const Fl_Help_Target *t0, const Fl_Help_Target *t1);
  int		do_align(Fl_Help_Block *block, int line, int xx, int a, int &l);
  void		draw();
  void		format();
  void		format_table(int *table_width, int *columns, const char *table);
  int		get_align(const char *p, int a);
  const char	*get_attr(const char *p, const char *n, char *buf, int bufsize);
  Fl_Color	get_color(const char *n, Fl_Color c);
  int		handle(int);

  void		initfont(uchar &f, uchar &s) { nfonts_ = 0;
			fl_font(f = fonts_[0][0] = textfont_,
			        s = fonts_[0][1] = textsize_); }
  void		pushfont(uchar f, uchar s) { if (nfonts_ < 99) nfonts_ ++;
			fl_font(fonts_[nfonts_][0] = f,
			        fonts_[nfonts_][1] = s); }
  void		popfont(uchar &f, uchar &s) { if (nfonts_ > 0) nfonts_ --;
			fl_font(f = fonts_[nfonts_][0],
			        s = fonts_[nfonts_][1]); }

  public:

  Fl_Help_View(int xx, int yy, int ww, int hh, const char *l = 0);
  ~Fl_Help_View();
  const char	*directory() const { if (directory_[0]) return (directory_);
  					else return ((const char *)0); }
  const char	*filename() const { if (filename_[0]) return (filename_);
  					else return ((const char *)0); }
  void		link(Fl_Help_Func *fn) { link_ = fn; }
  int		load(const char *f);
  void		resize(int,int,int,int);
  int		size() const { return (size_); }
  void		textcolor(Fl_Color c) { if (textcolor_ == defcolor_) textcolor_ = c; defcolor_ = c; }
  Fl_Color	textcolor() const { return (defcolor_); }
  void		textfont(uchar f) { textfont_ = f; format(); }
  uchar		textfont() const { return (textfont_); }
  void		textsize(uchar s) { textsize_ = s; format(); }
  uchar		textsize() const { return (textsize_); }
  const char	*title() { return (title_); }
  void		topline(const char *n);
  void		topline(int);
  int		topline() const { return (topline_); }
  void		value(const char *v);
  const char	*value() const { return (value_); }
};

#endif // !_Fl_Help_View_H_

//
// End of "$Id: Fl_Help_View.H,v 1.1.2.3 2001/10/29 03:44:31 easysw Exp $".
//
