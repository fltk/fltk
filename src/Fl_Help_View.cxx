//
// "$Id: Fl_Help_View.cxx,v 1.1.2.53 2004/07/27 16:02:20 easysw Exp $"
//
// Fl_Help_View widget routines.
//
// Copyright 1997-2004 by Easy Software Products.
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
// Contents:
//
//   Fl_Help_View::add_block()       - Add a text block to the list.
//   Fl_Help_View::add_link()        - Add a new link to the list.
//   Fl_Help_View::add_target()      - Add a new target to the list.
//   Fl_Help_View::compare_targets() - Compare two targets.
//   Fl_Help_View::do_align()        - Compute the alignment for a line in
//                                     a block.
//   Fl_Help_View::draw()            - Draw the Fl_Help_View widget.
//   Fl_Help_View::format()          - Format the help text.
//   Fl_Help_View::format_table()    - Format a table...
//   Fl_Help_View::get_align()       - Get an alignment attribute.
//   Fl_Help_View::get_attr()        - Get an attribute value from the string.
//   Fl_Help_View::get_color()       - Get an alignment attribute.
//   Fl_Help_View::handle()          - Handle events in the widget.
//   Fl_Help_View::Fl_Help_View()    - Build a Fl_Help_View widget.
//   Fl_Help_View::~Fl_Help_View()   - Destroy a Fl_Help_View widget.
//   Fl_Help_View::load()            - Load the specified file.
//   Fl_Help_View::resize()          - Resize the help widget.
//   Fl_Help_View::topline()         - Set the top line to the named target.
//   Fl_Help_View::topline()         - Set the top line by number.
//   Fl_Help_View::value()           - Set the help text directly.
//   scrollbar_callback()            - A callback for the scrollbar.
//

//
// Include necessary header files...
//

#include <FL/Fl_Help_View.H>
#include <FL/Fl_Pixmap.H>
#include <stdio.h>
#include <stdlib.h>
#include "flstring.h"
#include <ctype.h>
#include <errno.h>

#if defined(WIN32) && ! defined(__CYGWIN__)
#  include <io.h>
#  include <direct.h>
#else
#  include <unistd.h>
#endif // WIN32

#define MAX_COLUMNS	200


//
// Typedef the C API sort function type the only way I know how...
//

extern "C"
{
  typedef int (*compare_func_t)(const void *, const void *);
}


//
// Local functions...
//

static int	quote_char(const char *);
static void	scrollbar_callback(Fl_Widget *s, void *);
static void	hscrollbar_callback(Fl_Widget *s, void *);


//
// Broken image...
//

static const char *broken_xpm[] =
		{
		  "16 24 4 1",
		  "@ c #000000",
		  "  c #ffffff",
		  "+ c none",
		  "x c #ff0000",
		  // pixels
		  "@@@@@@@+++++++++",
		  "@    @++++++++++",
		  "@   @+++++++++++",
		  "@   @++@++++++++",
		  "@    @@+++++++++",
		  "@     @+++@+++++",
		  "@     @++@@++++@",
		  "@ xxx  @@  @++@@",
		  "@  xxx    xx@@ @",
		  "@   xxx  xxx   @",
		  "@    xxxxxx    @",
		  "@     xxxx     @",
		  "@    xxxxxx    @",
		  "@   xxx  xxx   @",
		  "@  xxx    xxx  @",
		  "@ xxx      xxx @",
		  "@              @",
		  "@              @",
		  "@              @",
		  "@              @",
		  "@              @",
		  "@              @",
		  "@              @",
		  "@@@@@@@@@@@@@@@@",
		  NULL
		};

static Fl_Pixmap broken_image(broken_xpm);


//
// 'Fl_Help_View::add_block()' - Add a text block to the list.
//

Fl_Help_Block *					// O - Pointer to new block
Fl_Help_View::add_block(const char   *s,	// I - Pointer to start of block text
                	int           xx,	// I - X position of block
			int           yy,	// I - Y position of block
			int           ww,	// I - Right margin of block
			int           hh,	// I - Height of block
			unsigned char border)	// I - Draw border?
{
  Fl_Help_Block	*temp;				// New block


//  printf("add_block(s = %p, xx = %d, yy = %d, ww = %d, hh = %d, border = %d)\n",
//         s, xx, yy, ww, hh, border);

  if (nblocks_ >= ablocks_)
  {
    ablocks_ += 16;

    if (ablocks_ == 16)
      blocks_ = (Fl_Help_Block *)malloc(sizeof(Fl_Help_Block) * ablocks_);
    else
      blocks_ = (Fl_Help_Block *)realloc(blocks_, sizeof(Fl_Help_Block) * ablocks_);
  }

  temp = blocks_ + nblocks_;
  memset(temp, 0, sizeof(Fl_Help_Block));
  temp->start   = s;
  temp->end     = s;
  temp->x       = xx;
  temp->y       = yy;
  temp->w       = ww;
  temp->h       = hh;
  temp->border  = border;
  temp->bgcolor = bgcolor_;
  nblocks_ ++;

  return (temp);
}


//
// 'Fl_Help_View::add_link()' - Add a new link to the list.
//

void
Fl_Help_View::add_link(const char *n,	// I - Name of link
                      int        xx,	// I - X position of link
		      int        yy,	// I - Y position of link
		      int        ww,	// I - Width of link text
		      int        hh)	// I - Height of link text
{
  Fl_Help_Link	*temp;			// New link
  char		*target;		// Pointer to target name


  if (nlinks_ >= alinks_)
  {
    alinks_ += 16;

    if (alinks_ == 16)
      links_ = (Fl_Help_Link *)malloc(sizeof(Fl_Help_Link) * alinks_);
    else
      links_ = (Fl_Help_Link *)realloc(links_, sizeof(Fl_Help_Link) * alinks_);
  }

  temp = links_ + nlinks_;

  temp->x       = xx;
  temp->y       = yy;
  temp->w       = xx + ww;
  temp->h       = yy + hh;

  strlcpy(temp->filename, n, sizeof(temp->filename));

  if ((target = strrchr(temp->filename, '#')) != NULL)
  {
    *target++ = '\0';
    strlcpy(temp->name, target, sizeof(temp->name));
  }
  else
    temp->name[0] = '\0';

  nlinks_ ++;
}


//
// 'Fl_Help_View::add_target()' - Add a new target to the list.
//

void
Fl_Help_View::add_target(const char *n,	// I - Name of target
                	int        yy)	// I - Y position of target
{
  Fl_Help_Target	*temp;			// New target


  if (ntargets_ >= atargets_)
  {
    atargets_ += 16;

    if (atargets_ == 16)
      targets_ = (Fl_Help_Target *)malloc(sizeof(Fl_Help_Target) * atargets_);
    else
      targets_ = (Fl_Help_Target *)realloc(targets_, sizeof(Fl_Help_Target) * atargets_);
  }

  temp = targets_ + ntargets_;

  temp->y = yy;
  strlcpy(temp->name, n, sizeof(temp->name));

  ntargets_ ++;
}


//
// 'Fl_Help_View::compare_targets()' - Compare two targets.
//

int							// O - Result of comparison
Fl_Help_View::compare_targets(const Fl_Help_Target *t0,	// I - First target
                             const Fl_Help_Target *t1)	// I - Second target
{
  return (strcasecmp(t0->name, t1->name));
}


//
// 'Fl_Help_View::do_align()' - Compute the alignment for a line in a block.
//

int						// O - New line
Fl_Help_View::do_align(Fl_Help_Block *block,	// I - Block to add to
                      int          line,	// I - Current line
		      int          xx,		// I - Current X position
		      int          a,		// I - Current alignment
		      int          &l)		// IO - Starting link
{
  int	offset;					// Alignment offset


  switch (a)
  {
    case RIGHT :	// Right align
	offset = block->w - xx;
	break;
    case CENTER :	// Center
	offset = (block->w - xx) / 2;
	break;
    default :		// Left align
	offset = 0;
	break;
  }

  block->line[line] = block->x + offset;

  if (line < 31)
    line ++;

  while (l < nlinks_)
  {
    links_[l].x += offset;
    links_[l].w += offset;
    l ++;
  }

  return (line);
}


//
// 'Fl_Help_View::draw()' - Draw the Fl_Help_View widget.
//

void
Fl_Help_View::draw()
{
  int			i;		// Looping var
  const Fl_Help_Block	*block;		// Pointer to current block
  const char		*ptr,		// Pointer to text in block
			*attrs;		// Pointer to start of element attributes
  char			*s,		// Pointer into buffer
			buf[1024],	// Text buffer
			attr[1024];	// Attribute buffer
  int			xx, yy, ww, hh;	// Current positions and sizes
  int			line;		// Current line
  unsigned char		font, fsize;	// Current font and size
  int			head, pre,	// Flags for text
			needspace;	// Do we need whitespace?
  Fl_Boxtype		b = box() ? box() : FL_DOWN_BOX;
					// Box to draw...


  // Draw the scrollbar(s) and box first...
  ww = w();
  hh = h();
  i  = 0;

  if (hscrollbar_.visible()) {
    draw_child(hscrollbar_);
    hh -= 17;
    i ++;
  }
  if (scrollbar_.visible()) {
    draw_child(scrollbar_);
    ww -= 17;
    i ++;
  }
  if (i == 2) {
    fl_color(FL_GRAY);
    fl_rectf(x() + ww, y() + hh, 17, 17);
  }

  draw_box(b, x(), y(), ww, hh, bgcolor_);

  if (!value_)
    return;

  // Clip the drawing to the inside of the box...
  fl_push_clip(x() + Fl::box_dx(b), y() + Fl::box_dy(b),
               ww - Fl::box_dw(b), hh - Fl::box_dh(b));
  fl_color(textcolor_);

  // Draw all visible blocks...
  for (i = 0, block = blocks_; i < nblocks_; i ++, block ++)
    if ((block->y + block->h) >= topline_ && block->y < (topline_ + h()))
    {
      line      = 0;
      xx        = block->line[line];
      yy        = block->y - topline_;
      hh        = 0;
      pre       = 0;
      head      = 0;
      needspace = 0;

      initfont(font, fsize);

      for (ptr = block->start, s = buf; ptr < block->end;)
      {
	if ((*ptr == '<' || isspace(*ptr)) && s > buf)
	{
	  if (!head && !pre)
	  {
            // Check width...
            *s = '\0';
            s  = buf;
            ww = (int)fl_width(buf);

            if (needspace && xx > block->x)
	      xx += (int)fl_width(' ');

            if ((xx + ww) > block->w)
	    {
	      if (line < 31)
	        line ++;
	      xx = block->line[line];
	      yy += hh;
	      hh = 0;
	    }

            fl_draw(buf, xx + x() - leftline_, yy + y());

            xx += ww;
	    if ((fsize + 2) > hh)
	      hh = fsize + 2;

	    needspace = 0;
	  }
	  else if (pre)
	  {
	    while (isspace(*ptr))
	    {
	      if (*ptr == '\n')
	      {
	        *s = '\0';
                s = buf;

                fl_draw(buf, xx + x() - leftline_, yy + y());

		if (line < 31)
	          line ++;
		xx = block->line[line];
		yy += hh;
		hh = fsize + 2;
	      }
	      else if (*ptr == '\t')
	      {
		// Do tabs every 8 columns...
		while (((s - buf) & 7))
	          *s++ = ' ';
	      }
	      else
	        *s++ = ' ';

              if ((fsize + 2) > hh)
	        hh = fsize + 2;

              ptr ++;
	    }

            if (s > buf)
	    {
	      *s = '\0';
	      s = buf;

              fl_draw(buf, xx + x() - leftline_, yy + y());
              xx += (int)fl_width(buf);
	    }

	    needspace = 0;
	  }
	  else
	  {
            s = buf;

	    while (isspace(*ptr))
              ptr ++;
	  }
	}

	if (*ptr == '<')
	{
	  ptr ++;

          if (strncmp(ptr, "!--", 3) == 0)
	  {
	    // Comment...
	    ptr += 3;
	    if ((ptr = strstr(ptr, "-->")) != NULL)
	    {
	      ptr += 3;
	      continue;
	    }
	    else
	      break;
	  }

	  while (*ptr && *ptr != '>' && !isspace(*ptr))
            if (s < (buf + sizeof(buf) - 1))
	      *s++ = *ptr++;
	    else
	      ptr ++;

	  *s = '\0';
	  s = buf;

	  attrs = ptr;
	  while (*ptr && *ptr != '>')
            ptr ++;

	  if (*ptr == '>')
            ptr ++;

	  if (strcasecmp(buf, "HEAD") == 0)
            head = 1;
	  else if (strcasecmp(buf, "BR") == 0)
	  {
	    if (line < 31)
	      line ++;
	    xx = block->line[line];
            yy += hh;
	    hh = 0;
	  }
	  else if (strcasecmp(buf, "HR") == 0)
	  {
	    fl_line(block->x + x(), yy + y(), block->w + x(),
	            yy + y());

	    if (line < 31)
	      line ++;
	    xx = block->line[line];
            yy += 2 * hh;
	    hh = 0;
	  }
	  else if (strcasecmp(buf, "CENTER") == 0 ||
        	   strcasecmp(buf, "P") == 0 ||
        	   strcasecmp(buf, "H1") == 0 ||
		   strcasecmp(buf, "H2") == 0 ||
		   strcasecmp(buf, "H3") == 0 ||
		   strcasecmp(buf, "H4") == 0 ||
		   strcasecmp(buf, "H5") == 0 ||
		   strcasecmp(buf, "H6") == 0 ||
		   strcasecmp(buf, "UL") == 0 ||
		   strcasecmp(buf, "OL") == 0 ||
		   strcasecmp(buf, "DL") == 0 ||
		   strcasecmp(buf, "LI") == 0 ||
		   strcasecmp(buf, "DD") == 0 ||
		   strcasecmp(buf, "DT") == 0 ||
		   strcasecmp(buf, "PRE") == 0)
	  {
            if (tolower(buf[0]) == 'h')
	    {
	      font  = FL_HELVETICA_BOLD;
	      fsize = (uchar)(textsize_ + '7' - buf[1]);
	    }
	    else if (strcasecmp(buf, "DT") == 0)
	    {
	      font  = (uchar)(textfont_ | FL_ITALIC);
	      fsize = textsize_;
	    }
	    else if (strcasecmp(buf, "PRE") == 0)
	    {
	      font  = FL_COURIER;
	      fsize = textsize_;
	      pre   = 1;
	    }

            if (strcasecmp(buf, "LI") == 0)
	    {
	      fl_font(FL_SYMBOL, fsize);
	      fl_draw("\267", xx - fsize + x() - leftline_, yy + y());
	    }

	    pushfont(font, fsize);
	  }
	  else if (strcasecmp(buf, "A") == 0 &&
	           get_attr(attrs, "HREF", attr, sizeof(attr)) != NULL)
	    fl_color(linkcolor_);
	  else if (strcasecmp(buf, "/A") == 0)
	    fl_color(textcolor_);
	  else if (strcasecmp(buf, "B") == 0 ||
	           strcasecmp(buf, "STRONG") == 0)
	    pushfont(font |= FL_BOLD, fsize);
	  else if (strcasecmp(buf, "TD") == 0 ||
	           strcasecmp(buf, "TH") == 0)
          {
	    int tx, ty, tw, th;

	    if (tolower(buf[1]) == 'h')
	      pushfont(font |= FL_BOLD, fsize);
	    else
	      pushfont(font = textfont_, fsize);

            tx = block->x - 4 - leftline_;
	    ty = block->y - topline_ - fsize - 3;
            tw = block->w - block->x + 7;
	    th = block->h + fsize - 5;

            if (tx < 0)
	    {
	      tw += tx;
	      tx  = 0;
	    }

	    if (ty < 0)
	    {
	      th += ty;
	      ty  = 0;
	    }

            tx += x();
	    ty += y();

            if (block->bgcolor != bgcolor_)
	    {
	      fl_color(block->bgcolor);
              fl_rectf(tx, ty, tw, th);
              fl_color(textcolor_);
	    }

            if (block->border)
              fl_rect(tx, ty, tw, th);
	  }
	  else if (strcasecmp(buf, "I") == 0 ||
                   strcasecmp(buf, "EM") == 0)
	    pushfont(font |= FL_ITALIC, fsize);
	  else if (strcasecmp(buf, "CODE") == 0 ||
	           strcasecmp(buf, "TT") == 0)
	    pushfont(font = FL_COURIER, fsize);
	  else if (strcasecmp(buf, "KBD") == 0)
	    pushfont(font = FL_COURIER_BOLD, fsize);
	  else if (strcasecmp(buf, "VAR") == 0)
	    pushfont(font = FL_COURIER_ITALIC, fsize);
	  else if (strcasecmp(buf, "/HEAD") == 0)
            head = 0;
	  else if (strcasecmp(buf, "/H1") == 0 ||
		   strcasecmp(buf, "/H2") == 0 ||
		   strcasecmp(buf, "/H3") == 0 ||
		   strcasecmp(buf, "/H4") == 0 ||
		   strcasecmp(buf, "/H5") == 0 ||
		   strcasecmp(buf, "/H6") == 0 ||
		   strcasecmp(buf, "/B") == 0 ||
		   strcasecmp(buf, "/STRONG") == 0 ||
		   strcasecmp(buf, "/I") == 0 ||
		   strcasecmp(buf, "/EM") == 0 ||
		   strcasecmp(buf, "/CODE") == 0 ||
		   strcasecmp(buf, "/TT") == 0 ||
		   strcasecmp(buf, "/KBD") == 0 ||
		   strcasecmp(buf, "/VAR") == 0)
	    popfont(font, fsize);
	  else if (strcasecmp(buf, "/PRE") == 0)
	  {
	    popfont(font, fsize);
	    pre = 0;
	  }
	  else if (strcasecmp(buf, "IMG") == 0)
	  {
	    Fl_Shared_Image *img = 0;
	    int		width, height;
	    char	wattr[8], hattr[8];


            get_attr(attrs, "WIDTH", wattr, sizeof(wattr));
            get_attr(attrs, "HEIGHT", hattr, sizeof(hattr));
	    width  = get_length(wattr);
	    height = get_length(hattr);

	    if (get_attr(attrs, "SRC", attr, sizeof(attr))) {
	      img = get_image(attr, width, height);
	      if (!width) width = img->w();
	      if (!height) height = img->h();
	    }

	    if (!width || !height) {
              if (get_attr(attrs, "ALT", attr, sizeof(attr)) == NULL) {
	        strcpy(attr, "IMG");
              }
	    }

	    ww = width;

	    if (needspace && xx > block->x)
	      xx += (int)fl_width(' ');

	    if ((xx + ww) > block->w)
	    {
	      if (line < 31)
		line ++;

	      xx = block->line[line];
	      yy += hh;
	      hh = 0;
	    }

	    if (img) 
	      img->draw(xx + x() - leftline_,
	                yy + y() - fl_height() + fl_descent() + 2);

	    xx += ww;
	    if ((height + 2) > hh)
	      hh = height + 2;

	    needspace = 0;
	  }
	}
	else if (*ptr == '\n' && pre)
	{
	  *s = '\0';
	  s = buf;

          fl_draw(buf, xx + x() - leftline_, yy + y());

	  if (line < 31)
	    line ++;
	  xx = block->line[line];
	  yy += hh;
	  hh = fsize + 2;
	  needspace = 0;

	  ptr ++;
	}
	else if (isspace(*ptr))
	{
	  if (pre)
	  {
	    if (*ptr == ' ')
	      *s++ = ' ';
	    else
	    {
	      // Do tabs every 8 columns...
	      while (((s - buf) & 7))
	        *s++ = ' ';
            }
	  }

          ptr ++;
	  needspace = 1;
	}
	else if (*ptr == '&')
	{
	  ptr ++;

          int qch = quote_char(ptr);

	  if (qch < 0)
	    *s++ = '&';
	  else {
	    *s++ = qch;
	    ptr = strchr(ptr, ';') + 1;
	  }

          if ((fsize + 2) > hh)
	    hh = fsize + 2;
	}
	else
	{
	  *s++ = *ptr++;

          if ((fsize + 2) > hh)
	    hh = fsize + 2;
        }
      }

      *s = '\0';

      if (s > buf && !pre && !head)
      {
	ww = (int)fl_width(buf);

        if (needspace && xx > block->x)
	  xx += (int)fl_width(' ');

	if ((xx + ww) > block->w)
	{
	  if (line < 31)
	    line ++;
	  xx = block->line[line];
	  yy += hh;
	  hh = 0;
	}
      }

      if (s > buf && !head)
        fl_draw(buf, xx + x() - leftline_, yy + y());
    }

  fl_pop_clip();
}


//
// 'Fl_Help_View::find()' - Find the specified string...
//

int						// O - Matching position or -1 if not found
Fl_Help_View::find(const char *s,		// I - String to find
                   int        p)		// I - Starting position
{
  int		i,				// Looping var
		c;				// Current character
  Fl_Help_Block	*b;				// Current block
  const char	*bp,				// Block matching pointer
		*bs,				// Start of current comparison
		*sp;				// Search string pointer


  // Range check input and value...
  if (!s || !value_) return -1;

  if (p < 0 || p >= (int)strlen(value_)) p = 0;
  else if (p > 0) p ++;

  // Look for the string...
  for (i = nblocks_, b = blocks_; i > 0; i --, b ++) {
    if (b->end < (value_ + p))
      continue;

    if (b->start < (value_ + p)) bp = value_ + p;
    else bp = b->start;

    for (sp = s, bs = bp; *sp && *bp && bp < b->end; bp ++) {
      if (*bp == '<') {
        // skip to end of element...
	while (*bp && bp < b->end && *bp != '>') bp ++;
	continue;
      } else if (*bp == '&') {
        // decode HTML entity...
	if ((c = quote_char(bp + 1)) < 0) c = '&';
	else bp = strchr(bp + 1, ';') + 1;
      } else c = *bp;

      if (tolower(*sp) == tolower(c)) sp ++;
      else {
        // No match, so reset to start of search...
	sp = s;
	bs ++;
	bp = bs;
      }
    }

    if (!*sp) {
      // Found a match!
      topline(b->y - b->h);
      return (b->end - value_);
    }
  }

  // No match!
  return (-1);
}


//
// 'Fl_Help_View::format()' - Format the help text.
//

void
Fl_Help_View::format()
{
  int		i;		// Looping var
  int		done;		// Are we done yet?
  Fl_Help_Block	*block,		// Current block
		*cell;		// Current table cell
  int		cells[MAX_COLUMNS],
				// Cells in the current row...
		row;		// Current table row (block number)
  const char	*ptr,		// Pointer into block
		*start,		// Pointer to start of element
		*attrs;		// Pointer to start of element attributes
  char		*s,		// Pointer into buffer
		buf[1024],	// Text buffer
		attr[1024],	// Attribute buffer
		wattr[1024],	// Width attribute buffer
		hattr[1024],	// Height attribute buffer
		linkdest[1024];	// Link destination
  int		xx, yy, ww, hh;	// Size of current text fragment
  int		line;		// Current line in block
  int		links;		// Links for current line
  unsigned char	font, fsize;	// Current font and size
  unsigned char	border;		// Draw border?
  int		talign,		// Current alignment
		newalign,	// New alignment
		head,		// In the <HEAD> section?
		pre,		// <PRE> text?
		needspace;	// Do we need whitespace?
  int		table_width,	// Width of table
		table_offset;	// Offset of table
  int		column,		// Current table column number
		columns[MAX_COLUMNS];
				// Column widths
  Fl_Color	tc, rc;		// Table/row background color


  // Reset document width...
  hsize_ = w() - 24;

  done = 0;
  while (!done)
  {
    // Reset state variables...
    done       = 1;
    nblocks_   = 0;
    nlinks_    = 0;
    ntargets_  = 0;
    size_      = 0;
    bgcolor_   = color();
    textcolor_ = textcolor();
    linkcolor_ = selection_color();

    tc = rc = bgcolor_;

    strcpy(title_, "Untitled");

    if (!value_)
      return;

    // Setup for formatting...
    initfont(font, fsize);

    line         = 0;
    links        = 0;
    xx           = 4;
    yy           = fsize + 2;
    ww           = 0;
    column       = 0;
    border       = 0;
    hh           = 0;
    block        = add_block(value_, xx, yy, hsize_, 0);
    row          = 0;
    head         = 0;
    pre          = 0;
    talign       = LEFT;
    newalign     = LEFT;
    needspace    = 0;
    linkdest[0]  = '\0';
    table_offset = 0;

    for (ptr = value_, s = buf; *ptr;)
    {
      if ((*ptr == '<' || isspace(*ptr)) && s > buf)
      {
        // Get width...
        *s = '\0';
        ww = (int)fl_width(buf);

	if (!head && !pre)
	{
          // Check width...
          if (ww > hsize_) {
	    hsize_ = ww;
	    done   = 0;
	    break;
	  }

          if (needspace && xx > block->x)
	    ww += (int)fl_width(' ');

  //        printf("line = %d, xx = %d, ww = %d, block->x = %d, block->w = %d\n",
  //	       line, xx, ww, block->x, block->w);

          if ((xx + ww) > block->w)
	  {
            line     = do_align(block, line, xx, newalign, links);
	    xx       = block->x;
	    yy       += hh;
	    block->h += hh;
	    hh       = 0;
	  }

          if (linkdest[0])
	    add_link(linkdest, xx, yy - fsize, ww, fsize);

	  xx += ww;
	  if ((fsize + 2) > hh)
	    hh = fsize + 2;

	  needspace = 0;
	}
	else if (pre)
	{
          // Add a link as needed...
          if (linkdest[0])
	    add_link(linkdest, xx, yy - hh, ww, hh);

	  xx += ww;
	  if ((fsize + 2) > hh)
	    hh = fsize + 2;

          // Handle preformatted text...
	  while (isspace(*ptr))
	  {
	    if (*ptr == '\n')
	    {
              if (xx > hsize_) break;

              line     = do_align(block, line, xx, newalign, links);
              xx       = block->x;
	      yy       += hh;
	      block->h += hh;
	      hh       = fsize + 2;
	    }
	    else
              xx += (int)fl_width(' ');

            if ((fsize + 2) > hh)
	      hh = fsize + 2;

            ptr ++;
	  }

          if (xx > hsize_) {
	    hsize_ = xx;
	    done   = 0;
	    break;
	  }

	  needspace = 0;
	}
	else
	{
          // Handle normal text or stuff in the <HEAD> section...
	  while (isspace(*ptr))
            ptr ++;
	}

	s = buf;
      }

      if (*ptr == '<')
      {
	start = ptr;
	ptr ++;

        if (strncmp(ptr, "!--", 3) == 0)
	{
	  // Comment...
	  ptr += 3;
	  if ((ptr = strstr(ptr, "-->")) != NULL)
	  {
	    ptr += 3;
	    continue;
	  }
	  else
	    break;
	}

	while (*ptr && *ptr != '>' && !isspace(*ptr))
          if (s < (buf + sizeof(buf) - 1))
            *s++ = *ptr++;
	  else
	    ptr ++;

	*s = '\0';
	s = buf;

//        puts(buf);

	attrs = ptr;
	while (*ptr && *ptr != '>')
          ptr ++;

	if (*ptr == '>')
          ptr ++;

	if (strcasecmp(buf, "HEAD") == 0)
          head = 1;
	else if (strcasecmp(buf, "/HEAD") == 0)
          head = 0;
	else if (strcasecmp(buf, "TITLE") == 0)
	{
          // Copy the title in the document...
          for (s = title_;
	       *ptr != '<' && *ptr && s < (title_ + sizeof(title_) - 1);
	       *s++ = *ptr++);

	  *s = '\0';
	  s = buf;
	}
	else if (strcasecmp(buf, "A") == 0)
	{
          if (get_attr(attrs, "NAME", attr, sizeof(attr)) != NULL)
	    add_target(attr, yy - fsize - 2);

	  if (get_attr(attrs, "HREF", attr, sizeof(attr)) != NULL)
	    strlcpy(linkdest, attr, sizeof(linkdest));
	}
	else if (strcasecmp(buf, "/A") == 0)
          linkdest[0] = '\0';
	else if (strcasecmp(buf, "BODY") == 0)
	{
          bgcolor_   = get_color(get_attr(attrs, "BGCOLOR", attr, sizeof(attr)),
	                	 color());
          textcolor_ = get_color(get_attr(attrs, "TEXT", attr, sizeof(attr)),
	                	 textcolor());
          linkcolor_ = get_color(get_attr(attrs, "LINK", attr, sizeof(attr)),
	                	 selection_color());
	}
	else if (strcasecmp(buf, "BR") == 0)
	{
          line     = do_align(block, line, xx, newalign, links);
          xx       = block->x;
	  block->h += hh;
          yy       += hh;
	  hh       = 0;
	}
	else if (strcasecmp(buf, "CENTER") == 0 ||
        	 strcasecmp(buf, "P") == 0 ||
        	 strcasecmp(buf, "H1") == 0 ||
		 strcasecmp(buf, "H2") == 0 ||
		 strcasecmp(buf, "H3") == 0 ||
		 strcasecmp(buf, "H4") == 0 ||
		 strcasecmp(buf, "H5") == 0 ||
		 strcasecmp(buf, "H6") == 0 ||
		 strcasecmp(buf, "UL") == 0 ||
		 strcasecmp(buf, "OL") == 0 ||
		 strcasecmp(buf, "DL") == 0 ||
		 strcasecmp(buf, "LI") == 0 ||
		 strcasecmp(buf, "DD") == 0 ||
		 strcasecmp(buf, "DT") == 0 ||
		 strcasecmp(buf, "HR") == 0 ||
		 strcasecmp(buf, "PRE") == 0 ||
		 strcasecmp(buf, "TABLE") == 0)
	{
          block->end = start;
          line       = do_align(block, line, xx, newalign, links);
          xx         = block->x;
          block->h   += hh;

          if (strcasecmp(buf, "UL") == 0 ||
	      strcasecmp(buf, "OL") == 0 ||
	      strcasecmp(buf, "DL") == 0)
          {
	    block->h += fsize + 2;
	    xx       += 4 * fsize;
	  }
          else if (strcasecmp(buf, "TABLE") == 0)
	  {
	    if (get_attr(attrs, "BORDER", attr, sizeof(attr)))
	      border = (uchar)atoi(attr);
	    else
	      border = 0;

            tc = rc = get_color(get_attr(attrs, "BGCOLOR", attr, sizeof(attr)), bgcolor_);

	    block->h += fsize + 2;

            format_table(&table_width, columns, start);

            if ((xx + table_width) > hsize_) {
#ifdef DEBUG
              printf("xx=%d, table_width=%d, hsize_=%d\n", xx, table_width,
	             hsize_);
#endif // DEBUG
	      hsize_ = xx + table_width;
	      done   = 0;
	      break;
	    }

            switch (get_align(attrs, talign))
	    {
	      default :
	          table_offset = 0;
	          break;

	      case CENTER :
	          table_offset = (hsize_ - table_width) / 2 - textsize_;
	          break;

	      case RIGHT :
	          table_offset = hsize_ - table_width - textsize_;
	          break;
	    }

	    column = 0;
	  }

          if (tolower(buf[0]) == 'h' && isdigit(buf[1]))
	  {
	    font  = FL_HELVETICA_BOLD;
	    fsize = (uchar)(textsize_ + '7' - buf[1]);
	  }
	  else if (strcasecmp(buf, "DT") == 0)
	  {
	    font  = (uchar)(textfont_ | FL_ITALIC);
	    fsize = textsize_;
	  }
	  else if (strcasecmp(buf, "PRE") == 0)
	  {
	    font  = FL_COURIER;
	    fsize = textsize_;
	    pre   = 1;
	  }
	  else
	  {
	    font  = textfont_;
	    fsize = textsize_;
	  }

	  pushfont(font, fsize);

          yy = block->y + block->h;
          hh = 0;

          if ((tolower(buf[0]) == 'h' && isdigit(buf[1])) ||
	      strcasecmp(buf, "DD") == 0 ||
	      strcasecmp(buf, "DT") == 0 ||
	      strcasecmp(buf, "P") == 0)
            yy += fsize + 2;
	  else if (strcasecmp(buf, "HR") == 0)
	  {
	    hh += 2 * fsize;
	    yy += fsize;
	  }

          if (row)
	    block = add_block(start, xx, yy, block->w, 0);
	  else
	    block = add_block(start, xx, yy, hsize_, 0);

	  needspace = 0;
	  line      = 0;

	  if (strcasecmp(buf, "CENTER") == 0)
	    newalign = talign = CENTER;
	  else
	    newalign = get_align(attrs, talign);
	}
	else if (strcasecmp(buf, "/CENTER") == 0 ||
		 strcasecmp(buf, "/P") == 0 ||
		 strcasecmp(buf, "/H1") == 0 ||
		 strcasecmp(buf, "/H2") == 0 ||
		 strcasecmp(buf, "/H3") == 0 ||
		 strcasecmp(buf, "/H4") == 0 ||
		 strcasecmp(buf, "/H5") == 0 ||
		 strcasecmp(buf, "/H6") == 0 ||
		 strcasecmp(buf, "/PRE") == 0 ||
		 strcasecmp(buf, "/UL") == 0 ||
		 strcasecmp(buf, "/OL") == 0 ||
		 strcasecmp(buf, "/DL") == 0 ||
		 strcasecmp(buf, "/TABLE") == 0)
	{
          line       = do_align(block, line, xx, newalign, links);
          xx         = block->x;
          block->end = ptr;

          if (strcasecmp(buf, "/UL") == 0 ||
	      strcasecmp(buf, "/OL") == 0 ||
	      strcasecmp(buf, "/DL") == 0)
	  {
	    xx       -= 4 * fsize;
	    block->h += fsize + 2;
	  }
	  else if (strcasecmp(buf, "/TABLE") == 0)
	    block->h += fsize + 2;
	  else if (strcasecmp(buf, "/PRE") == 0)
	  {
	    pre = 0;
	    hh  = 0;
	  }
	  else if (strcasecmp(buf, "/CENTER") == 0)
	    talign = LEFT;

          popfont(font, fsize);

          while (isspace(*ptr))
	    ptr ++;

          block->h += hh;
          yy       += hh;

          if (tolower(buf[2]) == 'l')
            yy += fsize + 2;

          if (row)
	    block = add_block(ptr, xx, yy, block->w, 0);
	  else
	    block = add_block(ptr, xx, yy, hsize_, 0);

	  needspace = 0;
	  hh        = 0;
	  line      = 0;
	  newalign  = talign;
	}
	else if (strcasecmp(buf, "TR") == 0)
	{
          block->end = start;
          line       = do_align(block, line, xx, newalign, links);
          xx         = block->x;
          block->h   += hh;

          if (row)
	  {
            yy = blocks_[row].y + blocks_[row].h;

	    for (cell = blocks_ + row + 1; cell <= block; cell ++)
	      if ((cell->y + cell->h) > yy)
		yy = cell->y + cell->h;

            block = blocks_ + row;

            block->h = yy - block->y + 2;

	    for (i = 0; i < column; i ++)
	      if (cells[i])
	      {
		cell = blocks_ + cells[i];
		cell->h = block->h;
	      }
	  }

          memset(cells, 0, sizeof(cells));

	  yy        = block->y + block->h - 4;
	  hh        = 0;
          block     = add_block(start, xx, yy, hsize_, 0);
	  row       = block - blocks_;
	  needspace = 0;
	  column    = 0;
	  line      = 0;

          rc = get_color(get_attr(attrs, "BGCOLOR", attr, sizeof(attr)), tc);
	}
	else if (strcasecmp(buf, "/TR") == 0 && row)
	{
          line       = do_align(block, line, xx, newalign, links);
          block->end = start;
	  block->h   += hh;

          xx = blocks_[row].x;

          yy = blocks_[row].y + blocks_[row].h;

	  for (cell = blocks_ + row + 1; cell <= block; cell ++)
	    if ((cell->y + cell->h) > yy)
	      yy = cell->y + cell->h;

          block = blocks_ + row;

          block->h = yy - block->y + 2;

	  for (i = 0; i < column; i ++)
	    if (cells[i])
	    {
	      cell = blocks_ + cells[i];
	      cell->h = block->h;
	    }

	  yy        = block->y + block->h - 4;
          block     = add_block(start, xx, yy, hsize_, 0);
	  needspace = 0;
	  row       = 0;
	  line      = 0;
	}
	else if ((strcasecmp(buf, "TD") == 0 ||
                  strcasecmp(buf, "TH") == 0) && row)
	{
          int	colspan;		// COLSPAN attribute


          line       = do_align(block, line, xx, newalign, links);
          block->end = start;
	  block->h   += hh;

          if (strcasecmp(buf, "TH") == 0)
	    font = (uchar)(textfont_ | FL_BOLD);
	  else
	    font = textfont_;

          fsize = textsize_;

          xx = blocks_[row].x + fsize + 3 + table_offset;
	  for (i = 0; i < column; i ++)
	    xx += columns[i] + 6;

          if (get_attr(attrs, "COLSPAN", attr, sizeof(attr)) != NULL)
	    colspan = atoi(attr);
	  else
	    colspan = 1;

          for (i = 0, ww = -6; i < colspan; i ++)
	    ww += columns[column + i] + 6;

          if (block->end == block->start && nblocks_ > 1)
	  {
	    nblocks_ --;
	    block --;
	  }

	  pushfont(font, fsize);

	  yy        = blocks_[row].y;
	  hh        = 0;
          block     = add_block(start, xx, yy, xx + ww, 0, border);
	  needspace = 0;
	  line      = 0;
	  newalign  = get_align(attrs, tolower(buf[1]) == 'h' ? CENTER : LEFT);
	  talign    = newalign;

          cells[column] = block - blocks_;

	  column += colspan;

          block->bgcolor = get_color(get_attr(attrs, "BGCOLOR", attr,
	                                      sizeof(attr)), rc);
	}
	else if ((strcasecmp(buf, "/TD") == 0 ||
                  strcasecmp(buf, "/TH") == 0) && row)
	{
          popfont(font, fsize);
	}
	else if (strcasecmp(buf, "B") == 0 ||
        	 strcasecmp(buf, "STRONG") == 0)
	  pushfont(font |= FL_BOLD, fsize);
	else if (strcasecmp(buf, "I") == 0 ||
        	 strcasecmp(buf, "EM") == 0)
	  pushfont(font |= FL_ITALIC, fsize);
	else if (strcasecmp(buf, "CODE") == 0 ||
	         strcasecmp(buf, "TT") == 0)
	  pushfont(font = FL_COURIER, fsize);
	else if (strcasecmp(buf, "KBD") == 0)
	  pushfont(font = FL_COURIER_BOLD, fsize);
	else if (strcasecmp(buf, "VAR") == 0)
	  pushfont(font = FL_COURIER_ITALIC, fsize);
	else if (strcasecmp(buf, "/B") == 0 ||
		 strcasecmp(buf, "/STRONG") == 0 ||
		 strcasecmp(buf, "/I") == 0 ||
		 strcasecmp(buf, "/EM") == 0 ||
		 strcasecmp(buf, "/CODE") == 0 ||
		 strcasecmp(buf, "/TT") == 0 ||
		 strcasecmp(buf, "/KBD") == 0 ||
		 strcasecmp(buf, "/VAR") == 0)
	  popfont(font, fsize);
	else if (strcasecmp(buf, "IMG") == 0)
	{
	  Fl_Shared_Image	*img = 0;
	  int		width;
	  int		height;


          get_attr(attrs, "WIDTH", wattr, sizeof(wattr));
          get_attr(attrs, "HEIGHT", hattr, sizeof(hattr));
	  width  = get_length(wattr);
	  height = get_length(hattr);

	  if (get_attr(attrs, "SRC", attr, sizeof(attr))) {
	    img    = get_image(attr, width, height);
	    width  = img->w();
	    height = img->h();
	  }

	  ww = width;

          if (ww > hsize_) {
	    hsize_ = ww;
	    done   = 0;
	    break;
	  }

	  if (needspace && xx > block->x)
	    ww += (int)fl_width(' ');

	  if ((xx + ww) > block->w)
	  {
	    line     = do_align(block, line, xx, newalign, links);
	    xx       = block->x;
	    yy       += hh;
	    block->h += hh;
	    hh       = 0;
	  }

	  if (linkdest[0])
	    add_link(linkdest, xx, yy - height, ww, height);

	  xx += ww;
	  if ((height + 2) > hh)
	    hh = height + 2;

	  needspace = 0;
	}
      }
      else if (*ptr == '\n' && pre)
      {
	if (linkdest[0])
	  add_link(linkdest, xx, yy - hh, ww, hh);

        if (xx > hsize_) {
	  hsize_ = xx;
          done   = 0;
	  break;
	}

	line      = do_align(block, line, xx, newalign, links);
	xx        = block->x;
	yy        += hh;
	block->h  += hh;
	needspace = 0;
	ptr ++;
      }
      else if (isspace(*ptr))
      {
	needspace = 1;

	ptr ++;
      }
      else if (*ptr == '&' && s < (buf + sizeof(buf) - 1))
      {
	ptr ++;

        int qch = quote_char(ptr);

	if (qch < 0)
	  *s++ = '&';
	else {
	  *s++ = qch;
	  ptr = strchr(ptr, ';') + 1;
	}

	if ((fsize + 2) > hh)
          hh = fsize + 2;
      }
      else
      {
	if (s < (buf + sizeof(buf) - 1))
          *s++ = *ptr++;
	else
          ptr ++;

	if ((fsize + 2) > hh)
          hh = fsize + 2;
      }
    }

    if (s > buf && !pre && !head)
    {
      *s = '\0';
      ww = (int)fl_width(buf);

  //    printf("line = %d, xx = %d, ww = %d, block->x = %d, block->w = %d\n",
  //	   line, xx, ww, block->x, block->w);

      if (ww > hsize_) {
	hsize_ = ww;
	done   = 0;
	break;
      }

      if (needspace && xx > block->x)
	ww += (int)fl_width(' ');

      if ((xx + ww) > block->w)
      {
	line     = do_align(block, line, xx, newalign, links);
	xx       = block->x;
	yy       += hh;
	block->h += hh;
	hh       = 0;
      }

      if (linkdest[0])
	add_link(linkdest, xx, yy - fsize, ww, fsize);

      xx += ww;
      if ((fsize + 2) > hh)
	hh = fsize + 2;

      needspace = 0;
    }

    block->end = ptr;
    size_      = yy + hh;
  }

  if (ntargets_ > 1)
    qsort(targets_, ntargets_, sizeof(Fl_Help_Target),
          (compare_func_t)compare_targets);

  if (hsize_ > (w() - 24)) {
    hscrollbar_.show();

    if (size_ < (h() - 24)) {
      scrollbar_.hide();
      hscrollbar_.resize(x(), y() + h() - 17, w(), 17);
    } else {
      scrollbar_.show();
      scrollbar_.resize(x() + w() - 17, y(), 17, h() - 17);
      hscrollbar_.resize(x(), y() + h() - 17, w() - 17, 17);
    }
  } else {
    hscrollbar_.hide();

    if (size_ < (h() - 8)) scrollbar_.hide();
    else {
      scrollbar_.resize(x() + w() - 17, y(), 17, h());
      scrollbar_.show();
    }
  }

  // Reset scrolling if it needs to be...
  if (scrollbar_.visible()) {
    int temph = h() - 8;
    if (hscrollbar_.visible()) temph -= 16;
    if ((topline_ + temph) > size_) topline(size_ - temph);
    else topline(topline_);
  } else topline(0);

  if (hscrollbar_.visible()) {
    int tempw = w() - 24;
    if ((leftline_ + tempw) > hsize_) leftline(hsize_ - tempw);
    else leftline(leftline_);
  } else leftline(0);
}


//
// 'Fl_Help_View::format_table()' - Format a table...
//

void
Fl_Help_View::format_table(int        *table_width,	// O - Total table width
                           int        *columns,		// O - Column widths
	                   const char *table)		// I - Pointer to start of table
{
  int		column,					// Current column
		num_columns,				// Number of columns
		colspan,				// COLSPAN attribute
		width,					// Current width
		temp_width,				// Temporary width
		max_width,				// Maximum width
		incell,					// In a table cell?
		pre,					// <PRE> text?
		needspace;				// Need whitespace?
  char		*s,					// Pointer into buffer
		buf[1024],				// Text buffer
		attr[1024],				// Other attribute
		wattr[1024],				// WIDTH attribute
		hattr[1024];				// HEIGHT attribute
  const char	*ptr,					// Pointer into table
		*attrs,					// Pointer to attributes
		*start;					// Start of element
  int		minwidths[MAX_COLUMNS];			// Minimum widths for each column
  unsigned char	font, fsize;				// Current font and size


  // Clear widths...
  *table_width = 0;
  for (column = 0; column < MAX_COLUMNS; column ++)
  {
    columns[column]   = 0;
    minwidths[column] = 0;
  }

  num_columns = 0;
  colspan     = 0;
  max_width   = 0;
  pre         = 0;
  needspace   = 0;

  // Scan the table...
  for (ptr = table, column = -1, width = 0, s = buf, incell = 0; *ptr;)
  {
    if ((*ptr == '<' || isspace(*ptr)) && s > buf && incell)
    {
      // Check width...
      if (needspace)
      {
        *s++      = ' ';
	needspace = 0;
      }

      *s         = '\0';
      temp_width = (int)fl_width(buf);
      s          = buf;

      if (temp_width > minwidths[column])
        minwidths[column] = temp_width;

      width += temp_width;

      if (width > max_width)
        max_width = width;
    }

    if (*ptr == '<')
    {
      start = ptr;

      for (s = buf, ptr ++; *ptr && *ptr != '>' && !isspace(*ptr);)
        if (s < (buf + sizeof(buf) - 1))
          *s++ = *ptr++;
	else
	  ptr ++;

      *s = '\0';
      s = buf;

      attrs = ptr;
      while (*ptr && *ptr != '>')
        ptr ++;

      if (*ptr == '>')
        ptr ++;

      if (strcasecmp(buf, "BR") == 0 ||
	  strcasecmp(buf, "HR") == 0)
      {
        width     = 0;
	needspace = 0;
      }
      else if (strcasecmp(buf, "TABLE") == 0 && start > table)
        break;
      else if (strcasecmp(buf, "CENTER") == 0 ||
               strcasecmp(buf, "P") == 0 ||
               strcasecmp(buf, "H1") == 0 ||
	       strcasecmp(buf, "H2") == 0 ||
	       strcasecmp(buf, "H3") == 0 ||
	       strcasecmp(buf, "H4") == 0 ||
	       strcasecmp(buf, "H5") == 0 ||
	       strcasecmp(buf, "H6") == 0 ||
	       strcasecmp(buf, "UL") == 0 ||
	       strcasecmp(buf, "OL") == 0 ||
	       strcasecmp(buf, "DL") == 0 ||
	       strcasecmp(buf, "LI") == 0 ||
	       strcasecmp(buf, "DD") == 0 ||
	       strcasecmp(buf, "DT") == 0 ||
	       strcasecmp(buf, "PRE") == 0)
      {
        width     = 0;
	needspace = 0;

        if (tolower(buf[0]) == 'h' && isdigit(buf[1]))
	{
	  font  = FL_HELVETICA_BOLD;
	  fsize = (uchar)(textsize_ + '7' - buf[1]);
	}
	else if (strcasecmp(buf, "DT") == 0)
	{
	  font  = (uchar)(textfont_ | FL_ITALIC);
	  fsize = textsize_;
	}
	else if (strcasecmp(buf, "PRE") == 0)
	{
	  font  = FL_COURIER;
	  fsize = textsize_;
	  pre   = 1;
	}
	else if (strcasecmp(buf, "LI") == 0)
	{
	  width  += 4 * fsize;
	  font   = textfont_;
	  fsize  = textsize_;
	}
	else
	{
	  font  = textfont_;
	  fsize = textsize_;
	}

	pushfont(font, fsize);
      }
      else if (strcasecmp(buf, "/CENTER") == 0 ||
	       strcasecmp(buf, "/P") == 0 ||
	       strcasecmp(buf, "/H1") == 0 ||
	       strcasecmp(buf, "/H2") == 0 ||
	       strcasecmp(buf, "/H3") == 0 ||
	       strcasecmp(buf, "/H4") == 0 ||
	       strcasecmp(buf, "/H5") == 0 ||
	       strcasecmp(buf, "/H6") == 0 ||
	       strcasecmp(buf, "/PRE") == 0 ||
	       strcasecmp(buf, "/UL") == 0 ||
	       strcasecmp(buf, "/OL") == 0 ||
	       strcasecmp(buf, "/DL") == 0)
      {
        width     = 0;
	needspace = 0;

        popfont(font, fsize);
      }
      else if (strcasecmp(buf, "TR") == 0 || strcasecmp(buf, "/TR") == 0 ||
               strcasecmp(buf, "/TABLE") == 0)
      {
//        printf("%s column = %d, colspan = %d, num_columns = %d\n",
//	       buf, column, colspan, num_columns);

        if (column >= 0)
	{
	  // This is a hack to support COLSPAN...
	  max_width /= colspan;

	  while (colspan > 0)
	  {
	    if (max_width > columns[column])
	      columns[column] = max_width;

	    column ++;
	    colspan --;
	  }
	}

	if (strcasecmp(buf, "/TABLE") == 0)
	  break;

	needspace = 0;
	column    = -1;
	width     = 0;
	max_width = 0;
	incell    = 0;
      }
      else if (strcasecmp(buf, "TD") == 0 ||
               strcasecmp(buf, "TH") == 0)
      {
//        printf("BEFORE column = %d, colspan = %d, num_columns = %d\n",
//	       column, colspan, num_columns);

        if (column >= 0)
	{
	  // This is a hack to support COLSPAN...
	  max_width /= colspan;

	  while (colspan > 0)
	  {
	    if (max_width > columns[column])
	      columns[column] = max_width;

	    column ++;
	    colspan --;
	  }
	}
	else
	  column ++;

        if (get_attr(attrs, "COLSPAN", attr, sizeof(attr)) != NULL)
	  colspan = atoi(attr);
	else
	  colspan = 1;

//        printf("AFTER column = %d, colspan = %d, num_columns = %d\n",
//	       column, colspan, num_columns);

        if ((column + colspan) >= num_columns)
	  num_columns = column + colspan;

	needspace = 0;
	width     = 0;
	incell    = 1;

        if (strcasecmp(buf, "TH") == 0)
	  font = (uchar)(textfont_ | FL_BOLD);
	else
	  font = textfont_;

        fsize = textsize_;

	pushfont(font, fsize);

        if (get_attr(attrs, "WIDTH", attr, sizeof(attr)) != NULL)
	  max_width = get_length(attr);
	else
	  max_width = 0;

//        printf("max_width = %d\n", max_width);
      }
      else if (strcasecmp(buf, "/TD") == 0 ||
               strcasecmp(buf, "/TH") == 0)
      {
	incell = 0;
        popfont(font, fsize);
      }
      else if (strcasecmp(buf, "B") == 0 ||
               strcasecmp(buf, "STRONG") == 0)
	pushfont(font |= FL_BOLD, fsize);
      else if (strcasecmp(buf, "I") == 0 ||
               strcasecmp(buf, "EM") == 0)
	pushfont(font |= FL_ITALIC, fsize);
      else if (strcasecmp(buf, "CODE") == 0 ||
               strcasecmp(buf, "TT") == 0)
	pushfont(font = FL_COURIER, fsize);
      else if (strcasecmp(buf, "KBD") == 0)
	pushfont(font = FL_COURIER_BOLD, fsize);
      else if (strcasecmp(buf, "VAR") == 0)
	pushfont(font = FL_COURIER_ITALIC, fsize);
      else if (strcasecmp(buf, "/B") == 0 ||
	       strcasecmp(buf, "/STRONG") == 0 ||
	       strcasecmp(buf, "/I") == 0 ||
	       strcasecmp(buf, "/EM") == 0 ||
	       strcasecmp(buf, "/CODE") == 0 ||
	       strcasecmp(buf, "/TT") == 0 ||
	       strcasecmp(buf, "/KBD") == 0 ||
	       strcasecmp(buf, "/VAR") == 0)
	popfont(font, fsize);
      else if (strcasecmp(buf, "IMG") == 0 && incell)
      {
	Fl_Shared_Image	*img = 0;
	int		iwidth, iheight;


        get_attr(attrs, "WIDTH", wattr, sizeof(wattr));
        get_attr(attrs, "HEIGHT", hattr, sizeof(hattr));
	iwidth  = get_length(wattr);
	iheight = get_length(hattr);

        if (get_attr(attrs, "SRC", attr, sizeof(attr))) {
	  img     = get_image(attr, iwidth, iheight);
	  iwidth  = img->w();
	  iheight = img->h();
	}

	if (iwidth > minwidths[column])
          minwidths[column] = iwidth;

        width += iwidth;
	if (needspace)
	  width += (int)fl_width(' ');

	if (width > max_width)
          max_width = width;

	needspace = 0;
      }
    }
    else if (*ptr == '\n' && pre)
    {
      width     = 0;
      needspace = 0;
      ptr ++;
    }
    else if (isspace(*ptr))
    {
      needspace = 1;

      ptr ++;
    }
    else if (*ptr == '&' && s < (buf + sizeof(buf) - 1))
    {
      ptr ++;

      int qch = quote_char(ptr);

      if (qch < 0)
	*s++ = '&';
      else {
	*s++ = qch;
	ptr = strchr(ptr, ';') + 1;
      }
    }
    else
    {
      if (s < (buf + sizeof(buf) - 1))
        *s++ = *ptr++;
      else
        ptr ++;
    }
  }

  // Now that we have scanned the entire table, adjust the table and
  // cell widths to fit on the screen...
  if (get_attr(table + 6, "WIDTH", attr, sizeof(attr)))
    *table_width = get_length(attr);
  else
    *table_width = 0;

#ifdef DEBUG
  printf("num_columns = %d, table_width = %d\n", num_columns, *table_width);
#endif // DEBUG

  if (num_columns == 0)
    return;

  // Add up the widths...
  for (column = 0, width = 0; column < num_columns; column ++)
    width += columns[column];

#ifdef DEBUG
  printf("width = %d, w() = %d\n", width, w());
  for (column = 0; column < num_columns; column ++)
    printf("    columns[%d] = %d, minwidths[%d] = %d\n", column, columns[column],
           column, minwidths[column]);
#endif // DEBUG

  // Adjust the width if needed...
  int scale_width = *table_width;

  if (scale_width == 0) {
    if (width > (hsize_ - 24)) scale_width = hsize_ - 24;
    else scale_width = width;
  }

  if (width < scale_width) {
#ifdef DEBUG
    printf("Scaling table up to %d from %d...\n", scale_width, width);
#endif // DEBUG

    *table_width = 0;

    scale_width = (scale_width - width) / num_columns;

#ifdef DEBUG
    printf("adjusted scale_width = %d\n", scale_width);
#endif // DEBUG

    for (column = 0; column < num_columns; column ++) {
      columns[column] += scale_width;

      (*table_width) += columns[column];
    }
  }
  else if (width > scale_width) {
#ifdef DEBUG
    printf("Scaling table down to %d from %d...\n", scale_width, width);
#endif // DEBUG

    for (column = 0; column < num_columns; column ++) {
      width       -= minwidths[column];
      scale_width -= minwidths[column];
    }

#ifdef DEBUG
    printf("adjusted width = %d, scale_width = %d\n", width, scale_width);
#endif // DEBUG

    if (width > 0) {
      for (column = 0; column < num_columns; column ++) {
	columns[column] -= minwidths[column];
	columns[column] = scale_width * columns[column] / width;
	columns[column] += minwidths[column];
      }
    }

    *table_width = 0;
    for (column = 0; column < num_columns; column ++) {
      (*table_width) += columns[column];
    }
  }
  else if (*table_width == 0)
    *table_width = width;

#ifdef DEBUG
  printf("FINAL table_width = %d\n", *table_width);
  for (column = 0; column < num_columns; column ++)
    printf("    columns[%d] = %d\n", column, columns[column]);
#endif // DEBUG
}


//
// 'Fl_Help_View::get_align()' - Get an alignment attribute.
//

int					// O - Alignment
Fl_Help_View::get_align(const char *p,	// I - Pointer to start of attrs
                        int        a)	// I - Default alignment
{
  char	buf[255];			// Alignment value


  if (get_attr(p, "ALIGN", buf, sizeof(buf)) == NULL)
    return (a);

  if (strcasecmp(buf, "CENTER") == 0)
    return (CENTER);
  else if (strcasecmp(buf, "RIGHT") == 0)
    return (RIGHT);
  else
    return (LEFT);
}


//
// 'Fl_Help_View::get_attr()' - Get an attribute value from the string.
//

const char *					// O - Pointer to buf or NULL
Fl_Help_View::get_attr(const char *p,		// I - Pointer to start of attributes
                      const char *n,		// I - Name of attribute
		      char       *buf,		// O - Buffer for attribute value
		      int        bufsize)	// I - Size of buffer
{
  char	name[255],				// Name from string
	*ptr,					// Pointer into name or value
	quote;					// Quote


  buf[0] = '\0';

  while (*p && *p != '>')
  {
    while (isspace(*p))
      p ++;

    if (*p == '>' || !*p)
      return (NULL);

    for (ptr = name; *p && !isspace(*p) && *p != '=' && *p != '>';)
      if (ptr < (name + sizeof(name) - 1))
        *ptr++ = *p++;
      else
        p ++;

    *ptr = '\0';

    if (isspace(*p) || !*p || *p == '>')
      buf[0] = '\0';
    else
    {
      if (*p == '=')
        p ++;

      for (ptr = buf; *p && !isspace(*p) && *p != '>';)
        if (*p == '\'' || *p == '\"')
	{
	  quote = *p++;

	  while (*p && *p != quote)
	    if ((ptr - buf + 1) < bufsize)
	      *ptr++ = *p++;
	    else
	      p ++;

          if (*p == quote)
	    p ++;
	}
	else if ((ptr - buf + 1) < bufsize)
	  *ptr++ = *p++;
	else
	  p ++;

      *ptr = '\0';
    }

    if (strcasecmp(n, name) == 0)
      return (buf);
    else
      buf[0] = '\0';

    if (*p == '>')
      return (NULL);
  }

  return (NULL);
}


//
// 'Fl_Help_View::get_color()' - Get an alignment attribute.
//

Fl_Color				// O - Color value
Fl_Help_View::get_color(const char *n,	// I - Color name
                        Fl_Color   c)	// I - Default color value
{
  int	rgb, r, g, b;			// RGB values


  if (!n || !n[0])
    return (c);

  if (n[0] == '#')
  {
    // Do hex color lookup
    rgb = strtol(n + 1, NULL, 16);

    r = rgb >> 16;
    g = (rgb >> 8) & 255;
    b = rgb & 255;

    return (fl_rgb_color((uchar)r, (uchar)g, (uchar)b));
  }
  else if (strcasecmp(n, "black") == 0)
    return (FL_BLACK);
  else if (strcasecmp(n, "red") == 0)
    return (FL_RED);
#ifdef __BORLANDC__ // Workaround for compiler bug...
  else if (strcasecmp(n, "green") == 0) {
    r = 0;
    g = 0x80;
    b = 0;
    return (fl_rgb_color(r, g, b));
  }
#else
  else if (strcasecmp(n, "green") == 0)
    return (fl_rgb_color(0, 0x80, 0));
#endif // __BORLANDC__
  else if (strcasecmp(n, "yellow") == 0)
    return (FL_YELLOW);
  else if (strcasecmp(n, "blue") == 0)
    return (FL_BLUE);
  else if (strcasecmp(n, "magenta") == 0 || strcasecmp(n, "fuchsia") == 0)
    return (FL_MAGENTA);
  else if (strcasecmp(n, "cyan") == 0 || strcasecmp(n, "aqua") == 0)
    return (FL_CYAN);
  else if (strcasecmp(n, "white") == 0)
    return (FL_WHITE);
  else if (strcasecmp(n, "gray") == 0 || strcasecmp(n, "grey") == 0)
    return (fl_rgb_color(0x80, 0x80, 0x80));
  else if (strcasecmp(n, "lime") == 0)
    return (FL_GREEN);
  else if (strcasecmp(n, "maroon") == 0)
    return (fl_rgb_color(0x80, 0, 0));
  else if (strcasecmp(n, "navy") == 0)
    return (fl_rgb_color(0, 0, 0x80));
  else if (strcasecmp(n, "olive") == 0)
    return (fl_rgb_color(0x80, 0x80, 0));
  else if (strcasecmp(n, "purple") == 0)
    return (fl_rgb_color(0x80, 0, 0x80));
  else if (strcasecmp(n, "silver") == 0)
    return (fl_rgb_color(0xc0, 0xc0, 0xc0));
  else if (strcasecmp(n, "teal") == 0)
    return (fl_rgb_color(0, 0x80, 0x80));
  else
    return (c);
}


//
// 'Fl_Help_View::get_image()' - Get an inline image.
//

Fl_Shared_Image *
Fl_Help_View::get_image(const char *name, int W, int H) {
  const char	*localname;		// Local filename
  char		dir[1024];		// Current directory
  char		temp[1024],		// Temporary filename
		*tempptr;		// Pointer into temporary name
  Fl_Shared_Image *ip;			// Image pointer...

  // See if the image can be found...
  if (strchr(directory_, ':') != NULL && strchr(name, ':') == NULL) {
    if (name[0] == '/') {
      strlcpy(temp, directory_, sizeof(temp));

      if ((tempptr = strrchr(strchr(directory_, ':') + 3, '/')) != NULL) {
        strlcpy(tempptr, name, sizeof(temp) - (tempptr - temp));
      } else {
        strlcat(temp, name, sizeof(temp));
      }
    } else {
      snprintf(temp, sizeof(temp), "%s/%s", directory_, name);
    }

    if (link_) localname = (*link_)(this, temp);
    else localname = temp;
  } else if (name[0] != '/' && strchr(name, ':') == NULL) {
    if (directory_[0]) snprintf(temp, sizeof(temp), "%s/%s", directory_, name);
    else {
      getcwd(dir, sizeof(dir));
      snprintf(temp, sizeof(temp), "file:%s/%s", dir, name);
    }

    if (link_) localname = (*link_)(this, temp);
    else localname = temp;
  } else if (link_) localname = (*link_)(this, name);
  else localname = name;

  if (!localname) return 0;

  if (strncmp(localname, "file:", 5) == 0) localname += 5;

  if ((ip = Fl_Shared_Image::get(localname, W, H)) == NULL)
    ip = (Fl_Shared_Image *)&broken_image;

  return ip;
}


//
// 'Fl_Help_View::get_length()' - Get a length value, either absolute or %.
//

int
Fl_Help_View::get_length(const char *l) {	// I - Value
  int	val;					// Integer value

  if (!l[0]) return 0;

  val = atoi(l);
  if (l[strlen(l) - 1] == '%') {
    if (val > 100) val = 100;
    else if (val < 0) val = 0;

    val = val * (hsize_ - 24) / 100;
  }

  return val;
}


//
// 'Fl_Help_View::handle()' - Handle events in the widget.
//

int				// O - 1 if we handled it, 0 otherwise
Fl_Help_View::handle(int event)	// I - Event to handle
{
  int		i;		// Looping var
  int		xx, yy;		// Adjusted mouse position
  Fl_Help_Link	*linkp;		// Current link
  char		target[32];	// Current target


  switch (event)
  {
    case FL_PUSH :
	if (Fl_Group::handle(event))
	  return (1);

    case FL_MOVE :
        xx = Fl::event_x() - x() + leftline_;
	yy = Fl::event_y() - y() + topline_;
	break;

    case FL_LEAVE :
        fl_cursor(FL_CURSOR_DEFAULT);

    default :
	return (Fl_Group::handle(event));
  }

  // Handle mouse clicks on links...
  for (i = nlinks_, linkp = links_; i > 0; i --, linkp ++)
    if (xx >= linkp->x && xx < linkp->w &&
        yy >= linkp->y && yy < linkp->h)
      break;

  if (!i)
  {
    fl_cursor(FL_CURSOR_DEFAULT);
    return (1);
  }

  // Change the cursor for FL_MOTION events, and go to the link for
  // clicks...
  if (event == FL_MOVE)
    fl_cursor(FL_CURSOR_HAND);
  else
  {
    fl_cursor(FL_CURSOR_DEFAULT);

    strlcpy(target, linkp->name, sizeof(target));

    set_changed();

    if (strcmp(linkp->filename, filename_) != 0 && linkp->filename[0])
    {
      char	dir[1024];	// Current directory
      char	temp[1024],	// Temporary filename
		*tempptr;	// Pointer into temporary filename


      if (strchr(directory_, ':') != NULL &&
          strchr(linkp->filename, ':') == NULL)
      {
	if (linkp->filename[0] == '/')
	{
          strlcpy(temp, directory_, sizeof(temp));
          if ((tempptr = strrchr(strchr(directory_, ':') + 3, '/')) != NULL)
	    strlcpy(tempptr, linkp->filename, sizeof(temp));
	  else
	    strlcat(temp, linkp->filename, sizeof(temp));
	}
	else
	  snprintf(temp, sizeof(temp), "%s/%s", directory_, linkp->filename);
      }
      else if (linkp->filename[0] != '/' && strchr(linkp->filename, ':') == NULL)
      {
	if (directory_[0])
	  snprintf(temp, sizeof(temp), "%s/%s", directory_, linkp->filename);
	else
	{
	  getcwd(dir, sizeof(dir));
	  snprintf(temp, sizeof(temp), "file:%s/%s", dir, linkp->filename);
	}
      }
      else
        strlcpy(temp, linkp->filename, sizeof(temp));

      if (linkp->name[0])
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "#%s",
	         linkp->name);

      load(temp);
    }
    else if (target[0])
      topline(target);
    else
      topline(0);

    leftline(0);
  }

  return (1);
}


//
// 'Fl_Help_View::Fl_Help_View()' - Build a Fl_Help_View widget.
//

Fl_Help_View::Fl_Help_View(int        xx,	// I - Left position
                	   int        yy,	// I - Top position
			   int        ww,	// I - Width in pixels
			   int        hh,	// I - Height in pixels
			   const char *l)
    : Fl_Group(xx, yy, ww, hh, l),
      scrollbar_(xx + ww - 17, yy, 17, hh - 17),
      hscrollbar_(xx, yy + hh - 17, ww - 17, 17)
{
  color(FL_BACKGROUND2_COLOR, FL_SELECTION_COLOR);

  title_[0]     = '\0';
  defcolor_     = FL_FOREGROUND_COLOR;
  bgcolor_      = FL_BACKGROUND_COLOR;
  textcolor_    = FL_FOREGROUND_COLOR;
  linkcolor_    = FL_SELECTION_COLOR;
  textfont_     = FL_TIMES;
  textsize_     = 12;
  value_        = NULL;

  ablocks_      = 0;
  nblocks_      = 0;
  blocks_       = (Fl_Help_Block *)0;

  nfonts_       = 0;

  link_         = (Fl_Help_Func *)0;

  alinks_       = 0;
  nlinks_       = 0;
  links_        = (Fl_Help_Link *)0;

  atargets_     = 0;
  ntargets_     = 0;
  targets_      = (Fl_Help_Target *)0;

  directory_[0] = '\0';
  filename_[0]  = '\0';

  topline_      = 0;
  leftline_     = 0;
  size_         = 0;
  hsize_        = 0;

  scrollbar_.value(0, hh, 0, 1);
  scrollbar_.step(8.0);
  scrollbar_.show();
  scrollbar_.callback(scrollbar_callback);

  hscrollbar_.value(0, ww, 0, 1);
  hscrollbar_.step(8.0);
  hscrollbar_.show();
  hscrollbar_.callback(hscrollbar_callback);
  hscrollbar_.type(FL_HORIZONTAL);
  end();
}


//
// 'Fl_Help_View::~Fl_Help_View()' - Destroy a Fl_Help_View widget.
//

Fl_Help_View::~Fl_Help_View()
{
  if (nblocks_)
    free(blocks_);
  if (nlinks_)
    free(links_);
  if (ntargets_)
    free(targets_);
  if (value_)
    free((void *)value_);
}


//
// 'Fl_Help_View::load()' - Load the specified file.
//

int				// O - 0 on success, -1 on error
Fl_Help_View::load(const char *f)// I - Filename to load (may also have target)
{
  FILE		*fp;		// File to read from
  long		len;		// Length of file
  char		*target;	// Target in file
  char		*slash;		// Directory separator
  const char	*localname;	// Local filename
  char		error[1024];	// Error buffer
  char		newname[1024];	// New filename buffer


  strlcpy(newname, f, sizeof(newname));
  if ((target = strrchr(newname, '#')) != NULL)
    *target++ = '\0';

  if (link_)
    localname = (*link_)(this, newname);
  else
    localname = filename_;

  if (!localname)
    return (0);

  strlcpy(filename_, newname, sizeof(filename_));
  strlcpy(directory_, newname, sizeof(directory_));

  // Note: We do not support Windows backslashes, since they are illegal
  //       in URLs...
  if ((slash = strrchr(directory_, '/')) == NULL)
    directory_[0] = '\0';
  else if (slash > directory_ && slash[-1] != '/')
    *slash = '\0';

  if (value_ != NULL)
  {
    free((void *)value_);
    value_ = NULL;
  }

  if (strncmp(localname, "ftp:", 4) == 0 ||
      strncmp(localname, "http:", 5) == 0 ||
      strncmp(localname, "https:", 6) == 0 ||
      strncmp(localname, "ipp:", 4) == 0 ||
      strncmp(localname, "mailto:", 7) == 0 ||
      strncmp(localname, "news:", 5) == 0)
  {
    // Remote link wasn't resolved...
    snprintf(error, sizeof(error),
             "<HTML><HEAD><TITLE>Error</TITLE></HEAD>"
             "<BODY><H1>Error</H1>"
	     "<P>Unable to follow the link \"%s\" - "
	     "no handler exists for this URI scheme.</P></BODY>",
	     localname);
    value_ = strdup(error);
  }
  else
  {
    if (strncmp(localname, "file:", 5) == 0)
      localname += 5;	// Adjust for local filename...

    if ((fp = fopen(localname, "rb")) != NULL)
    {
      fseek(fp, 0, SEEK_END);
      len = ftell(fp);
      rewind(fp);

      value_ = (const char *)calloc(len + 1, 1);
      fread((void *)value_, 1, len, fp);
      fclose(fp);
    }
    else
    {
      snprintf(error, sizeof(error),
               "<HTML><HEAD><TITLE>Error</TITLE></HEAD>"
               "<BODY><H1>Error</H1>"
	       "<P>Unable to follow the link \"%s\" - "
	       "%s.</P></BODY>",
	       localname, strerror(errno));
      value_ = strdup(error);
    }
  }

  format();

  if (target)
    topline(target);
  else
    topline(0);

  return (0);
}


//
// 'Fl_Help_View::resize()' - Resize the help widget.
//

void
Fl_Help_View::resize(int xx,	// I - New left position
                    int yy,	// I - New top position
		    int ww,	// I - New width
		    int hh)	// I - New height
{
  Fl_Widget::resize(xx, yy, ww, hh);

  scrollbar_.resize(x() + w() - 17, y(), 17, h() - 17);
  hscrollbar_.resize(x(), y() + h() - 17, w() - 17, 17);

  format();
}


//
// 'Fl_Help_View::topline()' - Set the top line to the named target.
//

void
Fl_Help_View::topline(const char *n)	// I - Target name
{
  Fl_Help_Target key,			// Target name key
		*target;		// Pointer to matching target


  if (ntargets_ == 0)
    return;

  strlcpy(key.name, n, sizeof(key.name));

  target = (Fl_Help_Target *)bsearch(&key, targets_, ntargets_, sizeof(Fl_Help_Target),
                                 (compare_func_t)compare_targets);

  if (target != NULL)
    topline(target->y);
}


//
// 'Fl_Help_View::topline()' - Set the top line by number.
//

void
Fl_Help_View::topline(int t)	// I - Top line number
{
  if (!value_)
    return;

  if (size_ < (h() - 24) || t < 0)
    t = 0;
  else if (t > size_)
    t = size_;

  topline_ = t;

  scrollbar_.value(topline_, h() - 24, 0, size_);

  set_changed();
  do_callback();

  redraw();
}


//
// 'Fl_Help_View::leftline()' - Set the left position.
//

void
Fl_Help_View::leftline(int l)	// I - Left position
{
  if (!value_)
    return;

  if (hsize_ < (w() - 24) || l < 0)
    l = 0;
  else if (l > hsize_)
    l = hsize_;

  leftline_ = l;

  hscrollbar_.value(leftline_, w() - 24, 0, hsize_);

  redraw();
}


//
// 'Fl_Help_View::value()' - Set the help text directly.
//

void
Fl_Help_View::value(const char *v)	// I - Text to view
{
  if (!v)
    return;

  if (value_ != NULL)
    free((void *)value_);

  value_ = strdup(v);

  format();

  set_changed();
  topline(0);
  leftline(0);
}


//
// 'quote_char()' - Return the character code associated with a quoted char.
//

static int			// O - Code or -1 on error
quote_char(const char *p) {	// I - Quoted string
  int	i;			// Looping var
  static struct {
    const char	*name;
    int		namelen;
    int		code;
  }	*nameptr,		// Pointer into name array
	names[] = {		// Quoting names
    { "Aacute;", 7, 193 },
    { "aacute;", 7, 225 },
    { "Acirc;",  6, 194 },
    { "acirc;",  6, 226 },
    { "acute;",  6, 180 },
    { "AElig;",  6, 198 },
    { "aelig;",  6, 230 },
    { "Agrave;", 7, 192 },
    { "agrave;", 7, 224 },
    { "amp;",    4, '&' },
    { "Aring;",  6, 197 },
    { "aring;",  6, 229 },
    { "Atilde;", 7, 195 },
    { "atilde;", 7, 227 },
    { "Auml;",   5, 196 },
    { "auml;",   5, 228 },
    { "brvbar;", 7, 166 },
    { "Ccedil;", 7, 199 },
    { "ccedil;", 7, 231 },
    { "cedil;",  6, 184 },
    { "cent;",   5, 162 },
    { "copy;",   5, 169 },
    { "curren;", 7, 164 },
    { "deg;",    4, 176 },
    { "divide;", 7, 247 },
    { "Eacute;", 7, 201 },
    { "eacute;", 7, 233 },
    { "Ecirc;",  6, 202 },
    { "ecirc;",  6, 234 },
    { "Egrave;", 7, 200 },
    { "egrave;", 7, 232 },
    { "ETH;",    4, 208 },
    { "eth;",    4, 240 },
    { "Euml;",   5, 203 },
    { "euml;",   5, 235 },
    { "frac12;", 7, 189 },
    { "frac14;", 7, 188 },
    { "frac34;", 7, 190 },
    { "gt;",     3, '>' },
    { "Iacute;", 7, 205 },
    { "iacute;", 7, 237 },
    { "Icirc;",  6, 206 },
    { "icirc;",  6, 238 },
    { "iexcl;",  6, 161 },
    { "Igrave;", 7, 204 },
    { "igrave;", 7, 236 },
    { "iquest;", 7, 191 },
    { "Iuml;",   5, 207 },
    { "iuml;",   5, 239 },
    { "laquo;",  6, 171 },
    { "lt;",     3, '<' },
    { "macr;",   5, 175 },
    { "micro;",  6, 181 },
    { "middot;", 7, 183 },
    { "nbsp;",   5, ' ' },
    { "not;",    4, 172 },
    { "Ntilde;", 7, 209 },
    { "ntilde;", 7, 241 },
    { "Oacute;", 7, 211 },
    { "oacute;", 7, 243 },
    { "Ocirc;",  6, 212 },
    { "ocirc;",  6, 244 },
    { "Ograve;", 7, 210 },
    { "ograve;", 7, 242 },
    { "ordf;",   5, 170 },
    { "ordm;",   5, 186 },
    { "Oslash;", 7, 216 },
    { "oslash;", 7, 248 },
    { "Otilde;", 7, 213 },
    { "otilde;", 7, 245 },
    { "Ouml;",   5, 214 },
    { "ouml;",   5, 246 },
    { "para;",   5, 182 },
    { "plusmn;", 7, 177 },
    { "pound;",  6, 163 },
    { "quot;",   5, '\"' },
    { "raquo;",  6, 187 },
    { "reg;",    4, 174 },
    { "sect;",   5, 167 },
    { "shy;",    4, 173 },
    { "sup1;",   5, 185 },
    { "sup2;",   5, 178 },
    { "sup3;",   5, 179 },
    { "szlig;",  6, 223 },
    { "THORN;",  6, 222 },
    { "thorn;",  6, 254 },
    { "times;",  6, 215 },
    { "Uacute;", 7, 218 },
    { "uacute;", 7, 250 },
    { "Ucirc;",  6, 219 },
    { "ucirc;",  6, 251 },
    { "Ugrave;", 7, 217 },
    { "ugrave;", 7, 249 },
    { "uml;",    4, 168 },
    { "Uuml;",   5, 220 },
    { "uuml;",   5, 252 },
    { "Yacute;", 7, 221 },
    { "yacute;", 7, 253 },
    { "yen;",    4, 165 },
    { "yuml;",   5, 255 }
  };

  if (!strchr(p, ';')) return -1;
  if (*p == '#') {
    if (*(p+1) == 'x' || *(p+1) == 'X') return strtol(p+2, NULL, 16);
    else return atoi(p+1);
  }
  for (i = (int)(sizeof(names) / sizeof(names[0])), nameptr = names; i > 0; i --, nameptr ++)
    if (strncmp(p, nameptr->name, nameptr->namelen) == 0)
      return nameptr->code;

  return -1;
}


//
// 'scrollbar_callback()' - A callback for the scrollbar.
//

static void
scrollbar_callback(Fl_Widget *s, void *)
{
  ((Fl_Help_View *)(s->parent()))->topline(int(((Fl_Scrollbar*)s)->value()));
}


//
// 'hscrollbar_callback()' - A callback for the horizontal scrollbar.
//

static void
hscrollbar_callback(Fl_Widget *s, void *)
{
  ((Fl_Help_View *)(s->parent()))->leftline(int(((Fl_Scrollbar*)s)->value()));
}


//
// End of "$Id: Fl_Help_View.cxx,v 1.1.2.53 2004/07/27 16:02:20 easysw Exp $".
//
