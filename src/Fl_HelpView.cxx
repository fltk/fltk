//
// "$Id: Fl_HelpView.cxx,v 1.1.2.5 2001/09/10 03:09:43 easysw Exp $"
//
// Fl_HelpView widget routines.
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
// Contents:
//
//   Fl_HelpView::add_block()       - Add a text block to the list.
//   Fl_HelpView::add_image()       - Add an image to the image cache.
//   Fl_HelpView::add_link()        - Add a new link to the list.
//   Fl_HelpView::add_target()      - Add a new target to the list.
//   Fl_HelpView::compare_targets() - Compare two targets.
//   Fl_HelpView::do_align()        - Compute the alignment for a line in
//                                    a block.
//   Fl_HelpView::draw()            - Draw the Fl_HelpView widget.
//   Fl_HelpView::find_image()      - Find an image by name 
//   Fl_HelpView::format()          - Format the help text.
//   Fl_HelpView::format_table()    - Format a table...
//   Fl_HelpView::get_align()       - Get an alignment attribute.
//   Fl_HelpView::get_attr()        - Get an attribute value from the string.
//   Fl_HelpView::get_color()       - Get an alignment attribute.
//   Fl_HelpView::handle()          - Handle events in the widget.
//   Fl_HelpView::Fl_HelpView()     - Build a Fl_HelpView widget.
//   Fl_HelpView::~Fl_HelpView()    - Destroy a Fl_HelpView widget.
//   Fl_HelpView::load()            - Load the specified file.
//   Fl_HelpView::load_gif()        - Load a GIF image file...
//   Fl_HelpView::load_jpeg()       - Load a JPEG image file.
//   Fl_HelpView::load_png()        - Load a PNG image file.
//   Fl_HelpView::resize()          - Resize the help widget.
//   Fl_HelpView::topline()         - Set the top line to the named target.
//   Fl_HelpView::topline()         - Set the top line by number.
//   Fl_HelpView::value()           - Set the help text directly.
//   Fl_HelpView::compare_blocks()  - Compare two blocks.
//   gif_read_cmap()                - Read the colormap from a GIF file...
//   gif_get_block()                - Read a GIF data block...
//   gif_get_code()                 - Get a LZW code from the file...
//   gif_read_lzw()                 - Read a byte from the LZW stream...
//   gif_read_image()               - Read a GIF image stream...
//   scrollbar_callback()           - A callback for the scrollbar.
//

//
// Include necessary header files...
//

#include <FL/Fl_HelpView.H>
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <errno.h>

#include <FL/Fl_Image.H>
#include <FL/Fl_Pixmap.H>

#if defined(WIN32)
#  include <io.h>
#  include <direct.h>
#  define strcasecmp(s,t)	stricmp((s), (t))
#  define strncasecmp(s,t,n)	strnicmp((s), (t), (n))
#elif defined(__EMX__)
#  define strcasecmp(s,t)	stricmp((s), (t))
#  define strncasecmp(s,t,n)	strnicmp((s), (t), (n))
#else
#  include <unistd.h>
#endif // WIN32

extern "C"
{
#ifdef HAVE_LIBPNG
#  include <zlib.h>
#  include <png.h>
#endif // HAVE_LIBPNG

#ifdef HAVE_LIBJPEG
#  include <jpeglib.h>
#endif // HAVE_LIBJPEG
}

#define MAX_COLUMNS	200


//
// Typedef the C API sort function type the only way I know how...
//

extern "C"
{
  typedef int (*compare_func_t)(const void *, const void *);
}

//
// GIF definitions...
//

#define GIF_INTERLACE	0x40
#define GIF_COLORMAP	0x80

typedef unsigned char	gif_cmap_t[256][3];


//
// Local globals...
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

static Fl_Pixmap *broken_image = (Fl_Pixmap *)0;
static int	gif_eof = 0;		// Did we hit EOF?
static unsigned	fltk_colors[] =
		{
		  0x00000000,
		  0xff000000,
		  0x00ff0000,
		  0xffff0000,
		  0x0000ff00,
		  0xff00ff00,
		  0x00ffff00,
		  0xffffff00,
		  0x55555500,
		  0xc6717100,
		  0x71c67100,
		  0x8e8e3800,
		  0x7171c600,
		  0x8e388e00,
		  0x388e8e00,
		  0xaaaaaa00,
		  0x55555500,
		  0x55555500,
		  0x55555500,
		  0x55555500,
		  0x55555500,
		  0x55555500,
		  0x55555500,
		  0x55555500,
		  0x55555500,
		  0x55555500,
		  0x55555500,
		  0x55555500,
		  0x55555500,
		  0x55555500,
		  0x55555500,
		  0x55555500,
		  0x00000000,
		  0x0d0d0d00,
		  0x1a1a1a00,
		  0x26262600,
		  0x31313100,
		  0x3d3d3d00,
		  0x48484800,
		  0x55555500,
		  0x5f5f5f00,
		  0x6a6a6a00,
		  0x75757500,
		  0x80808000,
		  0x8a8a8a00,
		  0x95959500,
		  0xa0a0a000,
		  0xaaaaaa00,
		  0xb5b5b500,
		  0xc0c0c000,
		  0xcbcbcb00,
		  0xd5d5d500,
		  0xe0e0e000,
		  0xeaeaea00,
		  0xf5f5f500,
		  0xffffff00,
		  0x00000000,
		  0x00240000,
		  0x00480000,
		  0x006d0000,
		  0x00910000,
		  0x00b60000,
		  0x00da0000,
		  0x00ff0000,
		  0x3f000000,
		  0x3f240000,
		  0x3f480000,
		  0x3f6d0000,
		  0x3f910000,
		  0x3fb60000,
		  0x3fda0000,
		  0x3fff0000,
		  0x7f000000,
		  0x7f240000,
		  0x7f480000,
		  0x7f6d0000,
		  0x7f910000,
		  0x7fb60000,
		  0x7fda0000,
		  0x7fff0000,
		  0xbf000000,
		  0xbf240000,
		  0xbf480000,
		  0xbf6d0000,
		  0xbf910000,
		  0xbfb60000,
		  0xbfda0000,
		  0xbfff0000,
		  0xff000000,
		  0xff240000,
		  0xff480000,
		  0xff6d0000,
		  0xff910000,
		  0xffb60000,
		  0xffda0000,
		  0xffff0000,
		  0x00003f00,
		  0x00243f00,
		  0x00483f00,
		  0x006d3f00,
		  0x00913f00,
		  0x00b63f00,
		  0x00da3f00,
		  0x00ff3f00,
		  0x3f003f00,
		  0x3f243f00,
		  0x3f483f00,
		  0x3f6d3f00,
		  0x3f913f00,
		  0x3fb63f00,
		  0x3fda3f00,
		  0x3fff3f00,
		  0x7f003f00,
		  0x7f243f00,
		  0x7f483f00,
		  0x7f6d3f00,
		  0x7f913f00,
		  0x7fb63f00,
		  0x7fda3f00,
		  0x7fff3f00,
		  0xbf003f00,
		  0xbf243f00,
		  0xbf483f00,
		  0xbf6d3f00,
		  0xbf913f00,
		  0xbfb63f00,
		  0xbfda3f00,
		  0xbfff3f00,
		  0xff003f00,
		  0xff243f00,
		  0xff483f00,
		  0xff6d3f00,
		  0xff913f00,
		  0xffb63f00,
		  0xffda3f00,
		  0xffff3f00,
		  0x00007f00,
		  0x00247f00,
		  0x00487f00,
		  0x006d7f00,
		  0x00917f00,
		  0x00b67f00,
		  0x00da7f00,
		  0x00ff7f00,
		  0x3f007f00,
		  0x3f247f00,
		  0x3f487f00,
		  0x3f6d7f00,
		  0x3f917f00,
		  0x3fb67f00,
		  0x3fda7f00,
		  0x3fff7f00,
		  0x7f007f00,
		  0x7f247f00,
		  0x7f487f00,
		  0x7f6d7f00,
		  0x7f917f00,
		  0x7fb67f00,
		  0x7fda7f00,
		  0x7fff7f00,
		  0xbf007f00,
		  0xbf247f00,
		  0xbf487f00,
		  0xbf6d7f00,
		  0xbf917f00,
		  0xbfb67f00,
		  0xbfda7f00,
		  0xbfff7f00,
		  0xff007f00,
		  0xff247f00,
		  0xff487f00,
		  0xff6d7f00,
		  0xff917f00,
		  0xffb67f00,
		  0xffda7f00,
		  0xffff7f00,
		  0x0000bf00,
		  0x0024bf00,
		  0x0048bf00,
		  0x006dbf00,
		  0x0091bf00,
		  0x00b6bf00,
		  0x00dabf00,
		  0x00ffbf00,
		  0x3f00bf00,
		  0x3f24bf00,
		  0x3f48bf00,
		  0x3f6dbf00,
		  0x3f91bf00,
		  0x3fb6bf00,
		  0x3fdabf00,
		  0x3fffbf00,
		  0x7f00bf00,
		  0x7f24bf00,
		  0x7f48bf00,
		  0x7f6dbf00,
		  0x7f91bf00,
		  0x7fb6bf00,
		  0x7fdabf00,
		  0x7fffbf00,
		  0xbf00bf00,
		  0xbf24bf00,
		  0xbf48bf00,
		  0xbf6dbf00,
		  0xbf91bf00,
		  0xbfb6bf00,
		  0xbfdabf00,
		  0xbfffbf00,
		  0xff00bf00,
		  0xff24bf00,
		  0xff48bf00,
		  0xff6dbf00,
		  0xff91bf00,
		  0xffb6bf00,
		  0xffdabf00,
		  0xffffbf00,
		  0x0000ff00,
		  0x0024ff00,
		  0x0048ff00,
		  0x006dff00,
		  0x0091ff00,
		  0x00b6ff00,
		  0x00daff00,
		  0x00ffff00,
		  0x3f00ff00,
		  0x3f24ff00,
		  0x3f48ff00,
		  0x3f6dff00,
		  0x3f91ff00,
		  0x3fb6ff00,
		  0x3fdaff00,
		  0x3fffff00,
		  0x7f00ff00,
		  0x7f24ff00,
		  0x7f48ff00,
		  0x7f6dff00,
		  0x7f91ff00,
		  0x7fb6ff00,
		  0x7fdaff00,
		  0x7fffff00,
		  0xbf00ff00,
		  0xbf24ff00,
		  0xbf48ff00,
		  0xbf6dff00,
		  0xbf91ff00,
		  0xbfb6ff00,
		  0xbfdaff00,
		  0xbfffff00,
		  0xff00ff00,
		  0xff24ff00,
		  0xff48ff00,
		  0xff6dff00,
		  0xff91ff00,
		  0xffb6ff00,
		  0xffdaff00,
		  0xffffff00
		};


//
// Local functions...
//

static int	gif_read_cmap(FILE *fp, int ncolors, gif_cmap_t cmap);
static int	gif_get_block(FILE *fp, unsigned char *buffer);
static int	gif_get_code (FILE *fp, int code_size, int first_time);
static int	gif_read_lzw(FILE *fp, int first_time, int input_code_size);
static int	gif_read_image(FILE *fp, Fl_HelpImage *img, gif_cmap_t cmap,
		               int interlace);
static void	scrollbar_callback(Fl_Widget *s, void *);


//
// 'Fl_HelpView::add_block()' - Add a text block to the list.
//

Fl_HelpBlock *					// O - Pointer to new block
Fl_HelpView::add_block(const char    *s,	// I - Pointer to start of block text
                       int           xx,	// I - X position of block
		       int           yy,	// I - Y position of block
		       int           ww,	// I - Right margin of block
		       int           hh,	// I - Height of block
		       unsigned char border)	// I - Draw border?
{
  Fl_HelpBlock	*temp;				// New block


//  printf("add_block(s = %p, xx = %d, yy = %d, ww = %d, hh = %d, border = %d)\n",
//         s, xx, yy, ww, hh, border);

  if (nblocks_ >= ablocks_)
  {
    ablocks_ += 16;

    if (ablocks_ == 16)
      blocks_ = (Fl_HelpBlock *)malloc(sizeof(Fl_HelpBlock) * ablocks_);
    else
      blocks_ = (Fl_HelpBlock *)realloc(blocks_, sizeof(Fl_HelpBlock) * ablocks_);
  }

  temp = blocks_ + nblocks_;
  temp->start  = s;
  temp->x      = xx;
  temp->y      = yy;
  temp->w      = ww;
  temp->h      = hh;
  temp->border = border;
  nblocks_ ++;

  return (temp);
}


//
// 'Fl_HelpView::add_image()' - Add an image to the image cache.
//

Fl_HelpImage *					// O - Image or NULL if not found
Fl_HelpView::add_image(const char *name,	// I - Path of image
                       const char *wattr,	// I - Width attribute
		       const char *hattr,	// I - Height attribute
                       int        make)		// I - Make the image?
{
  Fl_HelpImage	*img,				// New image
		*orig;				// Original image
  FILE		*fp;				// File pointer
  unsigned char	header[16];			// First 16 bytes of file
  int		status;				// Status of load...
  const char	*localname;			// Local filename
  char		dir[1024];			// Current directory
  char		temp[1024],			// Temporary filename
		*tempptr;			// Pointer into temporary name
  int		width,				// Desired width of image
		height;				// Desired height of image


  // See if the image has already been loaded...
  if ((img = find_image(name, wattr, hattr)) != (Fl_HelpImage *)0)
  {
    // Make the image if needed...
    if (!img->image)
      img->image = new Fl_RGB_Image(img->data, img->w, img->h, img->d);

    return (img);
  }

  // See if the image exists with the default size info...
  orig = find_image(name, "", "");

  // Allocate memory as needed...
  if (aimage_ == nimage_)
  {
    aimage_ += 16;

    if (aimage_ == 16)
      image_ = (Fl_HelpImage *)malloc(sizeof(Fl_HelpImage) * aimage_);
    else
      image_ = (Fl_HelpImage *)realloc(image_, sizeof(Fl_HelpImage) * aimage_);
  }

  img       = image_ + nimage_;
  img->name = strdup(name);
  img->copy = 0;

  if (!orig)
  {
    // See if the image can be found...
    if (strchr(directory_, ':') != NULL && strchr(name, ':') == NULL)
    {
      if (name[0] == '/')
      {
        strcpy(temp, directory_);
        if ((tempptr = strrchr(strchr(directory_, ':') + 3, '/')) != NULL)
	  strcpy(tempptr, name);
	else
	  strcat(temp, name);
      }
      else
	sprintf(temp, "%s/%s", directory_, name);

      if (link_)
	localname = (*link_)(temp);
      else
	localname = temp;
    }
    else if (name[0] != '/' && strchr(name, ':') == NULL)
    {
      if (directory_[0])
	sprintf(temp, "%s/%s", directory_, name);
      else
      {
	getcwd(dir, sizeof(dir));
	sprintf(temp, "file:%s/%s", dir, name);
      }

      if (link_)
	localname = (*link_)(temp);
      else
	localname = temp;
    }
    else if (link_)
      localname = (*link_)(name);
    else
      localname = name;

    if (!localname)
      return ((Fl_HelpImage *)0);

    if (strncmp(localname, "file:", 5) == 0)
      localname += 5;

    // Figure out the file type...
    if ((fp = fopen(localname, "rb")) == NULL)
      return ((Fl_HelpImage *)0);

    if (fread(header, 1, sizeof(header), fp) == 0)
      return ((Fl_HelpImage *)0);

    rewind(fp);

    // Load the image as appropriate...
    if (memcmp(header, "GIF87a", 6) == 0 ||
	memcmp(header, "GIF89a", 6) == 0)
      status = load_gif(img,  fp);
  #ifdef HAVE_LIBPNG
    else if (memcmp(header, "\211PNG", 4) == 0)
      status = load_png(img, fp);
  #endif // HAVE_LIBPNG
  #ifdef HAVE_LIBJPEG
    else if (memcmp(header, "\377\330\377", 3) == 0 &&	// Start-of-Image
	     header[3] >= 0xe0 && header[3] <= 0xef)	// APPn
      status = load_jpeg(img, fp);
  #endif // HAVE_LIBJPEG
    else
      status = 0;

    fclose(fp);

    if (!status)
    {
      free(img->name);
      return ((Fl_HelpImage *)0);
    }

    img->wattr[0] = '\0';
    img->hattr[0] = '\0';

    nimage_ ++;

    // Allocate memory as needed for the new copy...
    if (aimage_ == nimage_)
    {
      aimage_ += 16;
      image_  = (Fl_HelpImage *)realloc(image_, sizeof(Fl_HelpImage) * aimage_);
    }

    orig      = image_ + nimage_ - 1;
    img       = image_ + nimage_;
    img->name = strdup(name);
  }

//  printf("orig->data = %p, width = %d, height = %d\n", orig->data,
//         orig->w, orig->h);

  // Copy image data from original image...
  img->data = orig->data;
  img->w    = orig->w;
  img->h    = orig->h;
  img->d    = orig->d;
  img->copy = 1;

  // Figure out the size of the image...
  if (wattr[0])
  {
    if (wattr[strlen(wattr) - 1] == '%')
      width = atoi(wattr) * (w() - 24) / 100;
    else
      width = atoi(wattr);
  }
  else
    width = 0;

  if (hattr[0])
  {
    if (hattr[strlen(hattr) - 1] == '%')
      height = atoi(hattr) * h() / 100;
    else
      height = atoi(hattr);
  }
  else
    height = 0;

  if (width == 0 && height == 0)
  {
    // Use image size...
    width  = img->w;
    height = img->h;
  }
  else if (width == 0)
    // Scale width to height
    width = img->w * height / img->h;
  else if (height == 0)
    // Scale height to width
    height = img->h * width / img->w;

  // Scale the image as needed...
  if (width != img->w && height != img->h)
  {
    unsigned char	*scaled,	// Scaled image data
			*sptr,		// Source image data pointer
			*dptr;		// Destination image data pointer
    int			sy,		// Source coordinates
			dx, dy,		// Destination coordinates
			xerr, yerr,	// X & Y errors
			xmod, ymod,	// X & Y moduli
			xstep, ystep;	// X & Y step increments


     xmod   = img->w % width;
     xstep  = (img->w / width) * img->d;
     ymod   = img->h % height;
     ystep  = img->h / height;

     if ((scaled = (unsigned char *)malloc(width * height * img->d)) != NULL)
     {
       img->copy = 0;

       // Scale the image...
       for (dy = height, sy = 0, yerr = height / 2, dptr = scaled; dy > 0; dy --)
       {
         for (dx = width, xerr = width / 2,
	          sptr = img->data + sy * img->w * img->d;
	      dx > 0;
	      dx --)
         {
	   *dptr++ = sptr[0];
	   if (img->d > 1)
	   {
	     *dptr++ = sptr[1];
	     *dptr++ = sptr[2];
	   }

           sptr += xstep;
	   xerr -= xmod;
	   if (xerr <= 0)
	   {
	     xerr += width;
	     sptr += img->d;
	   }
	 }

         sy   += ystep;
	 yerr -= ymod;
	 if (yerr <= 0)
	 {
	   yerr += height;
	   sy ++;
	 }
       }

       // Finally, copy the new size and data to the image structure...
       if (!orig)
         free(img->data);

       img->w    = width;
       img->h    = height;
       img->data = scaled;
     }
  }

  strncpy(img->wattr, wattr, sizeof(img->wattr) - 1);
  img->wattr[sizeof(img->wattr) - 1] = '\0';
  strncpy(img->hattr, hattr, sizeof(img->hattr) - 1);
  img->hattr[sizeof(img->hattr) - 1] = '\0';

  if (make)
    img->image = new Fl_RGB_Image(img->data, img->w, img->h, img->d);
  else
    img->image = (Fl_Image *)0;

  nimage_ ++;

//  printf("img->data = %p, width = %d, height = %d\n", img->data,
//         img->w, img->h);

  return (img);
}


//
// 'Fl_HelpView::add_link()' - Add a new link to the list.
//

void
Fl_HelpView::add_link(const char *n,	// I - Name of link
                      int        xx,	// I - X position of link
		      int        yy,	// I - Y position of link
		      int        ww,	// I - Width of link text
		      int        hh)	// I - Height of link text
{
  Fl_HelpLink	*temp;			// New link
  char		*target;		// Pointer to target name


  if (nlinks_ >= alinks_)
  {
    alinks_ += 16;

    if (alinks_ == 16)
      links_ = (Fl_HelpLink *)malloc(sizeof(Fl_HelpLink) * alinks_);
    else
      links_ = (Fl_HelpLink *)realloc(links_, sizeof(Fl_HelpLink) * alinks_);
  }

  temp = links_ + nlinks_;

  temp->x       = xx;
  temp->y       = yy;
  temp->w       = xx + ww;
  temp->h       = yy + hh;

  strncpy(temp->filename, n, sizeof(temp->filename));
  temp->filename[sizeof(temp->filename) - 1] = '\0';

  if ((target = strrchr(temp->filename, '#')) != NULL)
  {
    *target++ = '\0';
    strncpy(temp->name, target, sizeof(temp->name));
    temp->name[sizeof(temp->name) - 1] = '\0';
  }
  else
    temp->name[0] = '\0';

  nlinks_ ++;
}


//
// 'Fl_HelpView::add_target()' - Add a new target to the list.
//

void
Fl_HelpView::add_target(const char *n,	// I - Name of target
                	int        yy)	// I - Y position of target
{
  Fl_HelpTarget	*temp;			// New target


  if (ntargets_ >= atargets_)
  {
    atargets_ += 16;

    if (atargets_ == 16)
      targets_ = (Fl_HelpTarget *)malloc(sizeof(Fl_HelpTarget) * atargets_);
    else
      targets_ = (Fl_HelpTarget *)realloc(targets_, sizeof(Fl_HelpTarget) * atargets_);
  }

  temp = targets_ + ntargets_;

  temp->y = yy;
  strncpy(temp->name, n, sizeof(temp->name));
  temp->name[sizeof(temp->name) - 1] = '\0';

  ntargets_ ++;
}


//
// 'Fl_HelpView::compare_targets()' - Compare two targets.
//

int							// O - Result of comparison
Fl_HelpView::compare_targets(const Fl_HelpTarget *t0,	// I - First target
                             const Fl_HelpTarget *t1)	// I - Second target
{
  return (strcasecmp(t0->name, t1->name));
}


//
// 'Fl_HelpView::do_align()' - Compute the alignment for a line in a block.
//

int						// O - New line
Fl_HelpView::do_align(Fl_HelpBlock *block,	// I - Block to add to
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
// 'Fl_HelpView::draw()' - Draw the Fl_HelpView widget.
//

void
Fl_HelpView::draw()
{
  int			i;		// Looping var
  const Fl_HelpBlock	*block;		// Pointer to current block
  const char		*ptr,		// Pointer to text in block
			*attrs;		// Pointer to start of element attributes
  char			*s,		// Pointer into buffer
			buf[1024],	// Text buffer
			attr[1024];	// Attribute buffer
  int			xx, yy, ww, hh;	// Current positions and sizes
  int			line;		// Current line
  unsigned char		font, size;	// Current font and size
  int			head, pre,	// Flags for text
			needspace;	// Do we need whitespace?
  Fl_Boxtype		b = box() ? box() : FL_DOWN_BOX;
					// Box to draw...
  Fl_Color		tc, c;		// Table/cell background color


  // Draw the scrollbar and box first...
  if (scrollbar_.visible())
  {
    draw_child(scrollbar_);
    draw_box(b, x(), y(), w() - 17, h(), bgcolor_);
  }
  else
    draw_box(b, x(), y(), w(), h(), bgcolor_);

  if (!value_)
    return;

  // Clip the drawing to the inside of the box...
  fl_push_clip(x() + 4, y() + 4, w() - 28, h() - 8);
  fl_color(textcolor_);

  tc = c = bgcolor_;

  // Draw all visible blocks...
  for (i = 0, block = blocks_; i < nblocks_ && (block->y - topline_) < h(); i ++, block ++)
    if ((block->y + block->h) >= topline_)
    {
      line      = 0;
      xx        = block->line[line];
      yy        = block->y - topline_;
      hh        = 0;
      pre       = 0;
      head      = 0;
      needspace = 0;

      initfont(font, size);

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

            fl_draw(buf, xx + x(), yy + y());

            xx += ww;
	    if ((size + 2) > hh)
	      hh = size + 2;

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

                fl_draw(buf, xx + x(), yy + y());

		if (line < 31)
	          line ++;
		xx = block->line[line];
		yy += hh;
		hh = size + 2;
	      }
	      else if (*ptr == '\t')
	      {
		// Do tabs every 8 columns...
		while (((s - buf) & 7))
	          *s++ = ' ';
	      }
	      else
	        *s++ = ' ';

              if ((size + 2) > hh)
	        hh = size + 2;

              ptr ++;
	    }

            if (s > buf)
	    {
	      *s = '\0';
	      s = buf;

              fl_draw(buf, xx + x(), yy + y());
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
	      font = FL_HELVETICA_BOLD;
	      size = textsize_ + '7' - buf[1];
	    }
	    else if (strcasecmp(buf, "DT") == 0)
	    {
	      font = textfont_ | FL_ITALIC;
	      size = textsize_;
	    }
	    else if (strcasecmp(buf, "PRE") == 0)
	    {
	      font = FL_COURIER;
	      size = textsize_;
	      pre  = 1;
	    }

            if (strcasecmp(buf, "LI") == 0)
	    {
	      fl_font(FL_SYMBOL, size);
	      fl_draw("\267", xx - size + x(), yy + y());
	    }

	    pushfont(font, size);

            if (c != bgcolor_)
	    {
	      fl_color(c);
              fl_rectf(block->x + x() - 4,
	               block->y - topline_ + y() - size - 3,
		       block->w - block->x + 7, block->h + size - 5);
              fl_color(textcolor_);
	    }
	  }
	  else if (strcasecmp(buf, "A") == 0 &&
	           get_attr(attrs, "HREF", attr, sizeof(attr)) != NULL)
	    fl_color(linkcolor_);
	  else if (strcasecmp(buf, "/A") == 0)
	    fl_color(textcolor_);
	  else if (strcasecmp(buf, "B") == 0)
	    pushfont(font |= FL_BOLD, size);
	  else if (strcasecmp(buf, "TABLE") == 0)
            tc = get_color(get_attr(attrs, "BGCOLOR", attr, sizeof(attr)), bgcolor_);
	  else if (strcasecmp(buf, "TD") == 0 ||
	           strcasecmp(buf, "TH") == 0)
          {
	    if (tolower(buf[1]) == 'h')
	      pushfont(font |= FL_BOLD, size);
	    else
	      pushfont(font = textfont_, size);

            c = get_color(get_attr(attrs, "BGCOLOR", attr, sizeof(attr)), tc);

            if (c != bgcolor_)
	    {
	      fl_color(c);
              fl_rectf(block->x + x() - 4,
	               block->y - topline_ + y() - size - 3,
		       block->w - block->x + 7, block->h + size - 5);
              fl_color(textcolor_);
	    }

            if (block->border)
              fl_rect(block->x + x() - 4,
	              block->y - topline_ + y() - size - 3,
		      block->w - block->x + 7, block->h + size - 5);
	  }
	  else if (strcasecmp(buf, "I") == 0)
	    pushfont(font |= FL_ITALIC, size);
	  else if (strcasecmp(buf, "CODE") == 0)
	    pushfont(font = FL_COURIER, size);
	  else if (strcasecmp(buf, "KBD") == 0)
	    pushfont(font = FL_COURIER_BOLD, size);
	  else if (strcasecmp(buf, "VAR") == 0)
	    pushfont(font = FL_COURIER_ITALIC, size);
	  else if (strcasecmp(buf, "/HEAD") == 0)
            head = 0;
	  else if (strcasecmp(buf, "/H1") == 0 ||
		   strcasecmp(buf, "/H2") == 0 ||
		   strcasecmp(buf, "/H3") == 0 ||
		   strcasecmp(buf, "/H4") == 0 ||
		   strcasecmp(buf, "/H5") == 0 ||
		   strcasecmp(buf, "/H6") == 0 ||
		   strcasecmp(buf, "/B") == 0 ||
		   strcasecmp(buf, "/I") == 0 ||
		   strcasecmp(buf, "/CODE") == 0 ||
		   strcasecmp(buf, "/KBD") == 0 ||
		   strcasecmp(buf, "/VAR") == 0)
	    popfont(font, size);
	  else if (strcasecmp(buf, "/TABLE") == 0)
	    tc = c = bgcolor_;
	  else if (strcasecmp(buf, "/TD") == 0 ||
	           strcasecmp(buf, "/TH") == 0)
	    c = tc;
	  else if (strcasecmp(buf, "/PRE") == 0)
	  {
	    popfont(font, size);
	    pre = 0;
	  }
	  else if (strcasecmp(buf, "IMG") == 0)
	  {
	    Fl_HelpImage	*img = (Fl_HelpImage *)0;
	    int		width = 16;
	    int		height = 24;
	    char	wattr[8], hattr[8];


            get_attr(attrs, "WIDTH", wattr, sizeof(wattr));
            get_attr(attrs, "HEIGHT", hattr, sizeof(hattr));

	    if (get_attr(attrs, "SRC", attr, sizeof(attr))) 
	      if ((img = add_image(attr, wattr, hattr)) != NULL)
	      {
	        if (!img->image)
	          img = (Fl_HelpImage *)0;
              }

	    if (img)
	    {
	      width  = img->w;
	      height = img->h;
	    }
	    else if (get_attr(attrs, "ALT", attr, sizeof(attr)) == NULL)
	      strcpy(attr, "IMG");

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
	      img->image->draw(xx + x(),
	                       yy + y() - fl_height() + fl_descent() + 2);
	    else
	      broken_image->draw(xx + x(),
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

          fl_draw(buf, xx + x(), yy + y());

	  if (line < 31)
	    line ++;
	  xx = block->line[line];
	  yy += hh;
	  hh = size + 2;
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

	  if (strncasecmp(ptr, "amp;", 4) == 0)
	  {
            *s++ = '&';
	    ptr += 4;
	  }
	  else if (strncasecmp(ptr, "lt;", 3) == 0)
	  {
            *s++ = '<';
	    ptr += 3;
	  }
	  else if (strncasecmp(ptr, "gt;", 3) == 0)
	  {
            *s++ = '>';
	    ptr += 3;
	  }
	  else if (strncasecmp(ptr, "nbsp;", 5) == 0)
	  {
            *s++ = ' ';
	    ptr += 5;
	  }
	  else if (strncasecmp(ptr, "copy;", 5) == 0)
	  {
            *s++ = '\251';
	    ptr += 5;
	  }
	  else if (strncasecmp(ptr, "reg;", 4) == 0)
	  {
            *s++ = '\256';
	    ptr += 4;
	  }
	  else if (strncasecmp(ptr, "quot;", 5) == 0)
	  {
            *s++ = '\"';
	    ptr += 5;
	  }

          if ((size + 2) > hh)
	    hh = size + 2;
	}
	else
	{
	  *s++ = *ptr++;

          if ((size + 2) > hh)
	    hh = size + 2;
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
        fl_draw(buf, xx + x(), yy + y());
    }

  fl_pop_clip();
}


//
// 'Fl_HelpView::find_image()' - Find an image by name 
//

Fl_HelpImage *					// O - Image or NULL if not found
Fl_HelpView::find_image(const char *name,	// I - Path and name of image
                	const char *wattr,	// I - Width attribute of image
			const char *hattr)	// I - Height attribute of image
{
  int		i;				// Looping var
  Fl_HelpImage	*img;				// Current image


  for (i = nimage_, img = image_; i > 0; i --, img ++) 
    if (strcmp(img->name, name) == 0 &&
        strcmp(img->wattr, wattr) == 0 &&
        strcmp(img->hattr, hattr) == 0)
      return (img);

  return ((Fl_HelpImage *)0);
}


//
// 'Fl_HelpView::format()' - Format the help text.
//

void
Fl_HelpView::format()
{
  int		i;		// Looping var
  Fl_HelpBlock	*block,		// Current block
		*cell;		// Current table cell
  int		row;		// Current table row (block number)
  const char	*ptr,		// Pointer into block
		*start,		// Pointer to start of element
		*attrs;		// Pointer to start of element attributes
  char		*s,		// Pointer into buffer
		buf[1024],	// Text buffer
		attr[1024],	// Attribute buffer
		wattr[1024],	// Width attribute buffer
		hattr[1024],	// Height attribute buffer
		link[1024];	// Link destination
  int		xx, yy, ww, hh;	// Size of current text fragment
  int		line;		// Current line in block
  int		links;		// Links for current line
  unsigned char	font, size;	// Current font and size
  unsigned char	border;		// Draw border?
  int		align,		// Current alignment
		newalign,	// New alignment
		head,		// In the <HEAD> section?
		pre,		// <PRE> text?
		needspace;	// Do we need whitespace?
  int		table_width;	// Width of table
  int		column,		// Current table column number
		columns[MAX_COLUMNS];
				// Column widths


  // Reset state variables...
  nblocks_   = 0;
  nlinks_    = 0;
  ntargets_  = 0;
  size_      = 0;
  bgcolor_   = color();
  textcolor_ = textcolor();
  linkcolor_ = selection_color();

  strcpy(title_, "Untitled");

  if (!value_)
    return;

  // Flush images that are scaled by percentage...
  for (i = 0; i < nimage_; i ++)
    if (strchr(image_[i].wattr, '%') != NULL ||
        strchr(image_[i].hattr, '%') != NULL)
    {
      // Flush this one...
      free(image_[i].name);
      free(image_[i].data);
      delete image_[i].image;
      nimage_ --;
      if (i < nimage_)
        memcpy(image_ + i, image_ + i + 1, (nimage_ - i) * sizeof(Fl_HelpImage));
      i --;
    }

  // Setup for formatting...
  initfont(font, size);

  line      = 0;
  links     = 0;
  xx        = 4;
  yy        = size + 2;
  ww        = 0;
  column    = 0;
  border    = 0;
  hh        = 0;
  block     = add_block(value_, xx, yy, w() - 24, 0);
  row       = 0;
  head      = 0;
  pre       = 0;
  align     = LEFT;
  newalign  = LEFT;
  needspace = 0;
  link[0]   = '\0';

  for (ptr = value_, s = buf; *ptr;)
  {
    if ((*ptr == '<' || isspace(*ptr)) && s > buf)
    {
      if (!head && !pre)
      {
        // Check width...
        *s = '\0';
        ww = (int)fl_width(buf);

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

        if (link[0])
	  add_link(link, xx, yy - size, ww, size);

	xx += ww;
	if ((size + 2) > hh)
	  hh = size + 2;

	needspace = 0;
      }
      else if (pre)
      {
        // Handle preformatted text...
	while (isspace(*ptr))
	{
	  if (*ptr == '\n')
	  {
            if (link[0])
	      add_link(link, xx, yy - hh, ww, hh);

            line     = do_align(block, line, xx, newalign, links);
            xx       = block->x;
	    yy       += hh;
	    block->h += hh;
	    hh       = size + 2;
	  }

          if ((size + 2) > hh)
	    hh = size + 2;

          ptr ++;
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
      while (*ptr && *ptr != '>' && !isspace(*ptr))
        if (s < (buf + sizeof(buf) - 1))
          *s++ = *ptr++;
	else
	  ptr ++;

      *s = '\0';
      s = buf;

//      puts(buf);

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
	  add_target(attr, yy - size - 2);
	else if (get_attr(attrs, "HREF", attr, sizeof(attr)) != NULL)
	{
	  strncpy(link, attr, sizeof(link) - 1);
	  link[sizeof(link) - 1] = '\0';
	}
      }
      else if (strcasecmp(buf, "/A") == 0)
        link[0] = '\0';
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
	  block->h += size + 2;
	  xx       += 4 * size;
	}
        else if (strcasecmp(buf, "TABLE") == 0)
	{
	  if (get_attr(attrs, "BORDER", attr, sizeof(attr)))
	    border = atoi(attr);
	  else
	    border = 0;

	  block->h += size + 2;

          format_table(&table_width, columns, start);

	  column = 0;
	}

        if (tolower(buf[0]) == 'h' && isdigit(buf[1]))
	{
	  font = FL_HELVETICA_BOLD;
	  size = textsize_ + '7' - buf[1];
	}
	else if (strcasecmp(buf, "DT") == 0)
	{
	  font = textfont_ | FL_ITALIC;
	  size = textsize_;
	}
	else if (strcasecmp(buf, "PRE") == 0)
	{
	  font = FL_COURIER;
	  size = textsize_;
	  pre  = 1;
	}
	else
	{
	  font = textfont_;
	  size = textsize_;
	}

	pushfont(font, size);

        yy = block->y + block->h;
        hh = 0;

        if ((tolower(buf[0]) == 'h' && isdigit(buf[1])) ||
	    strcasecmp(buf, "DD") == 0 ||
	    strcasecmp(buf, "DT") == 0 ||
	    strcasecmp(buf, "P") == 0)
          yy += size + 2;
	else if (strcasecmp(buf, "HR") == 0)
	{
	  hh += 2 * size;
	  yy += size;
	}

        if (row)
	  block = add_block(start, xx, yy, block->w, 0);
	else
	  block = add_block(start, xx, yy, w() - 24, 0);

	needspace = 0;
	line      = 0;

	if (strcasecmp(buf, "CENTER") == 0)
	  newalign = align = CENTER;
	else
	  newalign = get_align(attrs, align);
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
	  xx       -= 4 * size;
	  block->h += size + 2;
	}
	else if (strcasecmp(buf, "/TABLE") == 0)
	  block->h += size + 2;
	else if (strcasecmp(buf, "/PRE") == 0)
	{
	  pre = 0;
	  hh  = 0;
	}
	else if (strcasecmp(buf, "/CENTER") == 0)
	  align = LEFT;

        popfont(font, size);

        while (isspace(*ptr))
	  ptr ++;

        block->h += hh;
        yy       += hh;

        if (tolower(buf[2]) == 'l')
          yy += size + 2;

        if (row)
	  block = add_block(ptr, xx, yy, block->w, 0);
	else
	  block = add_block(ptr, xx, yy, w() - 24, 0);

	needspace = 0;
	hh        = 0;
	line      = 0;
	newalign  = align;
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

          block->h = yy - block->y + 2;

	  for (cell = blocks_ + row + 1; cell < block; cell ++)
	    cell->h = block->h;
	}

	yy        = block->y + block->h - 4;
	hh        = 0;
        block     = add_block(start, xx, yy, w() - 24, 0);
	row       = block - blocks_;
	needspace = 0;
	column    = 0;
	line      = 0;
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

        block->h = yy - block->y + 2;

	for (cell = blocks_ + row + 1; cell < block; cell ++)
	  cell->h = block->h;

	yy        = block->y + block->h - 4;
        block     = add_block(start, xx, yy, w() - 24, 0);
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
	  font = textfont_ | FL_BOLD;
	else
	  font = textfont_;

        size = textsize_;

        xx = blocks_[row].x + size + 3;
	for (i = 0; i < column; i ++)
	  xx += columns[i] + 6;

        if (get_attr(attrs, "COLSPAN", attr, sizeof(attr)) != NULL)
	  colspan = atoi(attr);
	else
	  colspan = 1;

        for (i = 0, ww = 0; i < colspan; i ++)
	  ww += columns[column + i];

        if (block->end == block->start && nblocks_ > 1)
	{
	  nblocks_ --;
	  block --;
	}

	pushfont(font, size);

	yy        = blocks_[row].y;
	hh        = 0;
        block     = add_block(start, xx, yy, xx + ww, 0, border);
	needspace = 0;
	line      = 0;
	newalign  = get_align(attrs, tolower(buf[1]) == 'h' ? CENTER : LEFT);

	column ++;
      }
      else if ((strcasecmp(buf, "/TD") == 0 ||
                strcasecmp(buf, "/TH") == 0) && row)
        popfont(font, size);
      else if (strcasecmp(buf, "B") == 0)
	pushfont(font |= FL_BOLD, size);
      else if (strcasecmp(buf, "I") == 0)
	pushfont(font |= FL_ITALIC, size);
      else if (strcasecmp(buf, "CODE") == 0)
	pushfont(font = FL_COURIER, size);
      else if (strcasecmp(buf, "KBD") == 0)
	pushfont(font = FL_COURIER_BOLD, size);
      else if (strcasecmp(buf, "VAR") == 0)
	pushfont(font = FL_COURIER_ITALIC, size);
      else if (strcasecmp(buf, "/B") == 0 ||
	       strcasecmp(buf, "/I") == 0 ||
	       strcasecmp(buf, "/CODE") == 0 ||
	       strcasecmp(buf, "/KBD") == 0 ||
	       strcasecmp(buf, "/VAR") == 0)
	popfont(font, size);
      else if (strcasecmp(buf, "IMG") == 0)
      {
	Fl_HelpImage	*img = (Fl_HelpImage *)0;
	int		width = 16;
	int		height = 24;


        get_attr(attrs, "WIDTH", wattr, sizeof(wattr));
        get_attr(attrs, "HEIGHT", hattr, sizeof(hattr));

	if (get_attr(attrs, "SRC", attr, sizeof(attr))) 
	  if ((img = add_image(attr, wattr, hattr)) != (Fl_HelpImage *)0 &&
	      img->image == NULL)
	    img = (Fl_HelpImage *)0;

	if (img)
	{
	  width  = img->w;
	  height = img->h;
	}
	else if (get_attr(attrs, "ALT", attr, sizeof(attr)) == NULL)
	  strcpy(attr, "IMG");

	ww = width;

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

	if (link[0])
	  add_link(link, xx, yy - height, ww, height);

	xx += ww;
	if ((height + 2) > hh)
	  hh = height + 2;

	needspace = 0;
      }
    }
    else if (*ptr == '\n' && pre)
    {
      if (link[0])
	add_link(link, xx, yy - hh, ww, hh);

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

      if (strncasecmp(ptr, "amp;", 4) == 0)
      {
        *s++ = '&';
	ptr += 4;
      }
      else if (strncasecmp(ptr, "lt;", 3) == 0)
      {
        *s++ = '<';
	ptr += 3;
      }
      else if (strncasecmp(ptr, "gt;", 3) == 0)
      {
        *s++ = '>';
	ptr += 3;
      }
      else if (strncasecmp(ptr, "nbsp;", 5) == 0)
      {
        *s++ = '\240';
	ptr += 5;
      }
      else if (strncasecmp(ptr, "copy;", 5) == 0)
      {
        *s++ = '\251';
	ptr += 5;
      }
      else if (strncasecmp(ptr, "reg;", 4) == 0)
      {
        *s++ = '\256';
	ptr += 4;
      }
      else if (strncasecmp(ptr, "quot;", 5) == 0)
      {
        *s++ = '\"';
	ptr += 5;
      }

      if ((size + 2) > hh)
        hh = size + 2;
    }
    else
    {
      if (s < (buf + sizeof(buf) - 1))
        *s++ = *ptr++;
      else
        ptr ++;

      if ((size + 2) > hh)
        hh = size + 2;
    }
  }

  if (s > buf && !pre && !head)
  {
    *s = '\0';
    ww = (int)fl_width(buf);

//    printf("line = %d, xx = %d, ww = %d, block->x = %d, block->w = %d\n",
//	   line, xx, ww, block->x, block->w);

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

    if (link[0])
      add_link(link, xx, yy - size, ww, size);

    xx += ww;
    if ((size + 2) > hh)
      hh = size + 2;

    needspace = 0;
  }

  block->end = ptr;
  size_      = yy + hh;

  if (ntargets_ > 1)
    qsort(targets_, ntargets_, sizeof(Fl_HelpTarget),
          (compare_func_t)compare_targets);

  if (nblocks_ > 1)
    qsort(blocks_, nblocks_, sizeof(Fl_HelpBlock),
          (compare_func_t)compare_blocks);

  if (size_ < (h() - 8))
    scrollbar_.hide();
  else
    scrollbar_.show();

  topline(topline_);
}


//
// 'Fl_HelpView::format_table()' - Format a table...
//

void
Fl_HelpView::format_table(int        *table_width,	// O - Total table width
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
  unsigned char	font, size;				// Current font and size


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
	  font = FL_HELVETICA_BOLD;
	  size = textsize_ + '7' - buf[1];
	}
	else if (strcasecmp(buf, "DT") == 0)
	{
	  font = textfont_ | FL_ITALIC;
	  size = textsize_;
	}
	else if (strcasecmp(buf, "PRE") == 0)
	{
	  font = FL_COURIER;
	  size = textsize_;
	  pre  = 1;
	}
	else if (strcasecmp(buf, "LI") == 0)
	{
	  width += 4 * size;
	  font  = textfont_;
	  size  = textsize_;
	}
	else
	{
	  font = textfont_;
	  size = textsize_;
	}

	pushfont(font, size);
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

        popfont(font, size);
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
	  font = textfont_ | FL_BOLD;
	else
	  font = textfont_;

        size = textsize_;

	pushfont(font, size);

        if (get_attr(attrs, "WIDTH", attr, sizeof(attr)) != NULL)
	{
	  max_width = atoi(attr);

	  if (attr[strlen(attr) - 1] == '%')
	    max_width = max_width * w() / 100;
	}
	else
	  max_width = 0;

//        printf("max_width = %d\n", max_width);
      }
      else if (strcasecmp(buf, "/TD") == 0 ||
               strcasecmp(buf, "/TH") == 0)
      {
	incell = 0;
        popfont(font, size);
      }
      else if (strcasecmp(buf, "B") == 0)
	pushfont(font |= FL_BOLD, size);
      else if (strcasecmp(buf, "I") == 0)
	pushfont(font |= FL_ITALIC, size);
      else if (strcasecmp(buf, "CODE") == 0)
	pushfont(font = FL_COURIER, size);
      else if (strcasecmp(buf, "KBD") == 0)
	pushfont(font = FL_COURIER_BOLD, size);
      else if (strcasecmp(buf, "VAR") == 0)
	pushfont(font = FL_COURIER_ITALIC, size);
      else if (strcasecmp(buf, "/B") == 0 ||
	       strcasecmp(buf, "/I") == 0 ||
	       strcasecmp(buf, "/CODE") == 0 ||
	       strcasecmp(buf, "/KBD") == 0 ||
	       strcasecmp(buf, "/VAR") == 0)
	popfont(font, size);
      else if (strcasecmp(buf, "IMG") == 0 && incell)
      {
	Fl_HelpImage	*img = (Fl_HelpImage *)0;


        get_attr(attrs, "WIDTH", wattr, sizeof(wattr));
        get_attr(attrs, "HEIGHT", hattr, sizeof(hattr));

        if (get_attr(attrs, "SRC", attr, sizeof(attr))) 
	  if ((img = add_image(attr, wattr, hattr)) != (Fl_HelpImage *)0 &&
	      img->image == NULL)
	    img = (Fl_HelpImage *)0;

	if (img)
	  temp_width = img->w;
	else
	  temp_width = 16;

	if (temp_width > minwidths[column])
          minwidths[column] = temp_width;

        width += temp_width;
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

      if (strncasecmp(ptr, "amp;", 4) == 0)
      {
        *s++ = '&';
	ptr += 4;
      }
      else if (strncasecmp(ptr, "lt;", 3) == 0)
      {
        *s++ = '<';
	ptr += 3;
      }
      else if (strncasecmp(ptr, "gt;", 3) == 0)
      {
        *s++ = '>';
	ptr += 3;
      }
      else if (strncasecmp(ptr, "nbsp;", 5) == 0)
      {
        *s++ = '\240';
	ptr += 5;
      }
      else if (strncasecmp(ptr, "copy;", 5) == 0)
      {
        *s++ = '\251';
	ptr += 5;
      }
      else if (strncasecmp(ptr, "reg;", 4) == 0)
      {
        *s++ = '\256';
	ptr += 4;
      }
      else if (strncasecmp(ptr, "quot;", 5) == 0)
      {
        *s++ = '\"';
	ptr += 5;
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
  {
    if (attr[strlen(attr) - 1] == '%')
      *table_width = atoi(attr) * w() / 100;
    else
      *table_width = atoi(attr);
  }
  else
    *table_width = 0;

//  printf("num_columns = %d, table_width = %d\n", num_columns, *table_width);

  if (num_columns == 0)
    return;

  // Add up the widths...
  for (column = 0, width = 0; column < num_columns; column ++)
    width += columns[column];

//  printf("width = %d, w() = %d\n", width, w());
//  for (column = 0; column < num_columns; column ++)
//    printf("    columns[%d] = %d, minwidths[%d] = %d\n", column, columns[column],
//           column, minwidths[column]);

  // Adjust the width if needed...
  int scale_width = *table_width;

  if (scale_width == 0 && width > w())
    scale_width = width;

  if (width > scale_width)
  {
    *table_width = 0;

    for (column = 0; column < num_columns; column ++)
    {
      if (width > 0)
      {
        temp_width = scale_width * columns[column] / width;

	if (temp_width < minwidths[column])
	  temp_width = minwidths[column];
      }
      else
        temp_width = minwidths[column];

      width           -= columns[column];
      scale_width     -= temp_width;
      columns[column] = temp_width;
      (*table_width)  += temp_width;
    }
  }
  else if (*table_width == 0)
    *table_width = width;

//  printf("FINAL table_width = %d\n", *table_width);
//  for (column = 0; column < num_columns; column ++)
//    printf("    columns[%d] = %d\n", column, columns[column]);
}


//
// 'Fl_HelpView::get_align()' - Get an alignment attribute.
//

int					// O - Alignment
Fl_HelpView::get_align(const char *p,	// I - Pointer to start of attrs
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
// 'Fl_HelpView::get_attr()' - Get an attribute value from the string.
//

const char *					// O - Pointer to buf or NULL
Fl_HelpView::get_attr(const char *p,		// I - Pointer to start of attributes
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
// 'Fl_HelpView::get_color()' - Get an alignment attribute.
//

Fl_Color				// O - Color value
Fl_HelpView::get_color(const char *n,	// I - Color name
                       Fl_Color   c)	// I - Default color value
{
  int	rgb, r, g, b;			// RGB values


  if (!n)
    return (c);

  if (n[0] == '#')
  {
    // Do hex color lookup
    rgb = strtol(n + 1, NULL, 16);

    r = rgb >> 16;
    g = (rgb >> 8) & 255;
    b = rgb & 255;

    if (r == g && g == b)
      return (fl_gray_ramp(FL_NUM_GRAY * r / 256));
    else
      return (fl_color_cube((FL_NUM_RED - 1) * r / 255,
                            (FL_NUM_GREEN - 1) * g / 255,
			    (FL_NUM_BLUE - 1) * b / 255));
  }
  else if (strcasecmp(n, "black") == 0)
    return (FL_BLACK);
  else if (strcasecmp(n, "red") == 0)
    return (FL_RED);
  else if (strcasecmp(n, "green") == 0)
    return (fl_color_cube(0, 4, 0));
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
    return (FL_GRAY);
  else if (strcasecmp(n, "lime") == 0)
    return (FL_GREEN);
  else if (strcasecmp(n, "maroon") == 0)
    return (fl_color_cube(2, 0, 0));
  else if (strcasecmp(n, "navy") == 0)
    return (fl_color_cube(0, 0, 2));
  else if (strcasecmp(n, "olive") == 0)
    return (fl_color_cube(2, 4, 0));
  else if (strcasecmp(n, "purple") == 0)
    return (fl_color_cube(2, 0, 2));
  else if (strcasecmp(n, "silver") == 0)
    return (FL_LIGHT2);
  else if (strcasecmp(n, "teal") == 0)
    return (fl_color_cube(0, 4, 2));
  else
    return (c);
}


//
// 'Fl_HelpView::handle()' - Handle events in the widget.
//

int				// O - 1 if we handled it, 0 otherwise
Fl_HelpView::handle(int event)	// I - Event to handle
{
  int		i;		// Looping var
  int		xx, yy;		// Adjusted mouse position
  Fl_HelpLink	*link;		// Current link
  char		target[32];	// Current target


  switch (event)
  {
    case FL_PUSH :
	if (Fl_Group::handle(event))
	  return (1);

    case FL_MOVE :
        xx = Fl::event_x() - x();
	yy = Fl::event_y() - y() + topline_;
	break;

    default :
	return (Fl_Group::handle(event));
  }

  // Handle mouse clicks on links...
  for (i = nlinks_, link = links_; i > 0; i --, link ++)
    if (xx >= link->x && xx < link->w &&
        yy >= link->y && yy < link->h)
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

    strncpy(target, link->name, sizeof(target) - 1);
    target[sizeof(target) - 1] = '\0';

    set_changed();

    if (strcmp(link->filename, filename_) != 0 && link->filename[0])
    {
      char	dir[1024];	// Current directory
      char	temp[1024],	// Temporary filename
		*tempptr;	// Pointer into temporary filename


      if (strchr(directory_, ':') != NULL && strchr(link->filename, ':') == NULL)
      {
	if (link->filename[0] == '/')
	{
          strcpy(temp, directory_);
          if ((tempptr = strrchr(strchr(directory_, ':') + 3, '/')) != NULL)
	    strcpy(tempptr, link->filename);
	  else
	    strcat(temp, link->filename);
	}
	else
	  sprintf(temp, "%s/%s", directory_, link->filename);

	load(temp);
      }
      else if (link->filename[0] != '/' && strchr(link->filename, ':') == NULL)
      {
	if (directory_[0])
	  sprintf(temp, "%s/%s", directory_, link->filename);
	else
	{
	  getcwd(dir, sizeof(dir));
	  sprintf(temp, "file:%s/%s", dir, link->filename);
	}

        load(temp);
      }
      else
        load(link->filename);
    }
    else if (target[0])
      topline(target);
    else
      topline(0);
  }

  return (1);
}


//
// 'Fl_HelpView::Fl_HelpView()' - Build a Fl_HelpView widget.
//

Fl_HelpView::Fl_HelpView(int        xx,	// I - Left position
                	 int        yy,	// I - Top position
			 int        ww,	// I - Width in pixels
			 int        hh,	// I - Height in pixels
			 const char *l)
    : Fl_Group(xx, yy, ww, hh, l),
      scrollbar_(xx + ww - 17, yy, 17, hh)
{
  link_        = (Fl_HelpFunc *)0;

  filename_[0] = '\0';
  value_       = NULL;

  ablocks_     = 0;
  nblocks_     = 0;
  blocks_      = (Fl_HelpBlock *)0;

  nimage_      = 0;
  aimage_      = 0;
  image_       = (Fl_HelpImage *)0;

  if (!broken_image)
    broken_image = new Fl_Pixmap((char **)broken_xpm);

  alinks_      = 0;
  nlinks_      = 0;
  links_       = (Fl_HelpLink *)0;

  atargets_    = 0;
  ntargets_    = 0;
  targets_     = (Fl_HelpTarget *)0;

  nfonts_      = 0;
  textfont_    = FL_TIMES;
  textsize_    = 12;

  topline_     = 0;
  size_        = 0;

  color(FL_WHITE);
  textcolor(FL_BLACK);
  selection_color(FL_BLUE);

  scrollbar_.value(0, hh, 0, 1);
  scrollbar_.step(8.0);
  scrollbar_.show();
  scrollbar_.callback(scrollbar_callback);

  end();
}


//
// 'Fl_HelpView::~Fl_HelpView()' - Destroy a Fl_HelpView widget.
//

Fl_HelpView::~Fl_HelpView()
{
  int		i;		// Looping var
  Fl_HelpImage	*img;		// Current image


  if (nblocks_)
    free(blocks_);
  if (nlinks_)
    free(links_);
  if (ntargets_)
    free(targets_);
  if (value_)
    free((void *)value_);
  if (image_)
  {
    for (i = nimage_, img = image_; i > 0; i --, img ++)
    {
      delete img->image;
      if (!img->copy)
        free(img->data);
      free(img->name);
    }
  }
}


//
// 'Fl_HelpView::load()' - Load the specified file.
//

int				// O - 0 on success, -1 on error
Fl_HelpView::load(const char *f)// I - Filename to load (may also have target)
{
  FILE		*fp;		// File to read from
  long		len;		// Length of file
  char		*target;	// Target in file
  char		*slash;		// Directory separator
  const char	*localname;	// Local filename
  char		error[1024];	// Error buffer


  strcpy(filename_, f);
  strcpy(directory_, filename_);

  if ((slash = strrchr(directory_, '/')) == NULL)
    directory_[0] = '\0';
  else if (slash > directory_ && slash[-1] != '/')
    *slash = '\0';

  if ((target = strrchr(filename_, '#')) != NULL)
    *target++ = '\0';

  if (link_)
    localname = (*link_)(filename_);
  else
    localname = filename_;

  if (localname != NULL &&
      (strncmp(localname, "ftp:", 4) == 0 ||
       strncmp(localname, "http:", 5) == 0 ||
       strncmp(localname, "https:", 6) == 0 ||
       strncmp(localname, "ipp:", 4) == 0 ||
       strncmp(localname, "mailto:", 7) == 0 ||
       strncmp(localname, "news:", 5) == 0))
    localname = NULL;	// Remote link wasn't resolved...
  else if (localname != NULL &&
           strncmp(localname, "file:", 5) == 0)
    localname += 5;	// Adjust for local filename...
      
  if (value_ != NULL)
  {
    free((void *)value_);
    value_ = NULL;
  }

  if (localname)
  {
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
      sprintf(error, "%s: %s\n", localname, strerror(errno));
      value_ = strdup(error);
    }
  }
  else
  {
    sprintf(error, "%s: %s\n", filename_, strerror(errno));
    value_ = strdup(error);
  }

  format();

  if (target)
    topline(target);
  else
    topline(0);

  return (0);
}


//
// 'Fl_HelpView::load_gif()' - Load a GIF image file...
//

int					// O - 0 = success, -1 = fail
Fl_HelpView::load_gif(Fl_HelpImage *img,// I - Image pointer
        	      FILE         *fp)	// I - File to load from
{
  unsigned char	buf[1024];		// Input buffer
  gif_cmap_t	cmap;			// Colormap
  int		ncolors,		// Bits per pixel
		transparent;		// Transparent color index


  // Read the header; we already know it is a GIF file...
  fread(buf, 13, 1, fp);

  img->w  = (buf[7] << 8) | buf[6];
  img->h  = (buf[9] << 8) | buf[8];
  ncolors = 2 << (buf[10] & 0x07);

  if (buf[10] & GIF_COLORMAP)
    if (!gif_read_cmap(fp, ncolors, cmap))
      return (0);

  transparent = -1;

  for (;;)
  {
    switch (getc(fp))
    {
      case ';' :	// End of image
          return (0);	// Early end of file

      case '!' :	// Extension record
          buf[0] = getc(fp);
          if (buf[0] == 0xf9)	// Graphic Control Extension
          {
            gif_get_block(fp, buf);
            if (buf[0] & 1)	// Get transparent color index
              transparent = buf[3];
          }

          while (gif_get_block(fp, buf) != 0);
          break;

      case ',' :	// Image data
          fread(buf, 9, 1, fp);

          if (buf[8] & GIF_COLORMAP)
          {
            ncolors = 2 << (buf[8] & 0x07);

	    if (!gif_read_cmap(fp, ncolors, cmap))
	      return (0);
	  }

          if (transparent >= 0)
          {
	    unsigned	rgba = fltk_colors[bgcolor_];


            // Map transparent color to background color...
	    cmap[transparent][0] = rgba >> 24;
            cmap[transparent][1] = rgba >> 16;
            cmap[transparent][2] = rgba >> 8;
          }

          img->w    = (buf[5] << 8) | buf[4];
          img->h    = (buf[7] << 8) | buf[6];
          img->d    = 3;
          img->data = (unsigned char *)malloc(img->w * img->h * img->d);
          if (img->data == NULL)
            return (0);

	  return (gif_read_image(fp, img, cmap, buf[8] & GIF_INTERLACE));
    }
  }
}


#ifdef HAVE_LIBJPEG
//
// 'Fl_HelpView::load_jpeg()' - Load a JPEG image file.
//

int						// O - 0 = success, -1 = fail
Fl_HelpView::load_jpeg(Fl_HelpImage *img,	// I - Image pointer
                       FILE         *fp)	// I - File to load from
{
  struct jpeg_decompress_struct	cinfo;		// Decompressor info
  struct jpeg_error_mgr		jerr;		// Error handler info
  JSAMPROW			row;		// Sample row pointer


  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, fp);
  jpeg_read_header(&cinfo, 1);

  cinfo.quantize_colors      = 0;
  cinfo.out_color_space      = JCS_RGB;
  cinfo.out_color_components = 3;
  cinfo.output_components    = 3;

  jpeg_calc_output_dimensions(&cinfo);

  img->w  = cinfo.output_width;
  img->h = cinfo.output_height;
  img->d  = cinfo.output_components;
  img->data = (unsigned char *)malloc(img->w * img->h * img->d);

  if (img->data == NULL)
  {
    jpeg_destroy_decompress(&cinfo);
    return (0);
  }

  jpeg_start_decompress(&cinfo);

  while (cinfo.output_scanline < cinfo.output_height)
  {
    row = (JSAMPROW)(img->data +
                     cinfo.output_scanline * cinfo.output_width *
                     cinfo.output_components);
    jpeg_read_scanlines(&cinfo, &row, (JDIMENSION)1);
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  return (1);
}
#endif // HAVE_LIBJPEG


#ifdef HAVE_LIBPNG
//
// 'Fl_HelpView::load_png()' - Load a PNG image file.
//

int					// O - 0 = success, -1 = fail
Fl_HelpView::load_png(Fl_HelpImage *img,// I - Image pointer
        	      FILE         *fp)	// I - File to read from
{
  int		i;			// Looping var
  png_structp	pp;			// PNG read pointer
  png_infop	info;			// PNG info pointers
  png_bytep	*rows;			// PNG row pointers
  png_color_16	bg;			// Background color


  // Setup the PNG data structures...
  pp   = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info = png_create_info_struct(pp);

  // Initialize the PNG read "engine"...
  png_init_io(pp, fp);

  // Get the image dimensions and convert to grayscale or RGB...
  png_read_info(pp, info);

  if (info->color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_expand(pp);

  if (info->color_type == PNG_COLOR_TYPE_GRAY)
    img->d = 1;
  else
    img->d = 3;

  img->w    = (int)info->width;
  img->h    = (int)info->height;
  img->data = (unsigned char *)malloc(img->w * img->h * 3);

  if (info->bit_depth < 8)
  {
    png_set_packing(pp);
    png_set_expand(pp);

    if (info->valid & PNG_INFO_sBIT)
      png_set_shift(pp, &(info->sig_bit));
  }
  else if (info->bit_depth == 16)
    png_set_strip_16(pp);

#ifdef HAVE_PNG_GET_VALID
  // Handle transparency...
  if (png_get_valid(pp, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(pp);
#endif // HAVE_PNG_GET_VALID

  // Background color...
  unsigned	rgba = fltk_colors[bgcolor_];

  bg.red   = 65535 * (rgba >> 24) / 255;
  bg.green = 65535 * ((rgba >> 16) & 255) / 255;
  bg.blue  = 65535 * ((rgba >> 8) & 255) / 255;

  png_set_background(pp, &bg, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

  // Allocate pointers...
  rows = (png_bytep *)calloc(info->height, sizeof(png_bytep));

  for (i = 0; i < (int)info->height; i ++)
    if (info->color_type == PNG_COLOR_TYPE_GRAY)
      rows[i] = img->data + i * img->w;
    else
      rows[i] = img->data + i * img->w * 3;

  // Read the image, handling interlacing as needed...
  for (i = png_set_interlace_handling(pp); i > 0; i --)
    png_read_rows(pp, rows, NULL, img->h);

  // Free memory and return...
  free(rows);

  png_read_end(pp, info);
  png_read_destroy(pp, info, NULL);

  return (1);
}
#endif // HAVE_LIBPNG


//
// 'Fl_HelpView::resize()' - Resize the help widget.
//

void
Fl_HelpView::resize(int xx,	// I - New left position
                    int yy,	// I - New top position
		    int ww,	// I - New width
		    int hh)	// I - New height
{
  Fl_Widget::resize(xx, yy, ww, hh);
  scrollbar_.resize(xx + ww - 17, yy, 17, hh);

  format();
}


//
// 'Fl_HelpView::topline()' - Set the top line to the named target.
//

void
Fl_HelpView::topline(const char *n)	// I - Target name
{
  Fl_HelpTarget	key,			// Target name key
		*target;		// Pointer to matching target


  if (ntargets_ == 0)
    return;

  strncpy(key.name, n, sizeof(key.name) - 1);
  key.name[sizeof(key.name) - 1] = '\0';

  target = (Fl_HelpTarget *)bsearch(&key, targets_, ntargets_, sizeof(Fl_HelpTarget),
                                 (compare_func_t)compare_targets);

  if (target != NULL)
    topline(target->y);
}


//
// 'Fl_HelpView::topline()' - Set the top line by number.
//

void
Fl_HelpView::topline(int t)	// I - Top line number
{
  if (!value_)
    return;

  if (size_ < (h() - 8) || t < 0)
    t = 0;
  else if (t > size_)
    t = size_;

  topline_ = t;

  scrollbar_.value(topline_, h(), 0, size_);

  do_callback();
  clear_changed();

  redraw();
}


//
// 'Fl_HelpView::value()' - Set the help text directly.
//

void
Fl_HelpView::value(const char *v)	// I - Text to view
{
  if (!v)
    return;

  if (value_ != NULL)
    free((void *)value_);

  value_ = strdup(v);

  format();

  set_changed();
  topline(0);
}


//
// 'Fl_HelpView::compare_blocks()' - Compare two blocks.
//

int						// O - Result of comparison
Fl_HelpView::compare_blocks(const void *a,	// I - First block
                            const void *b)	// I - Second block
{
  return (((Fl_HelpBlock *)a)->y - ((Fl_HelpBlock *)b)->y);
}


//
// 'gif_read_cmap()' - Read the colormap from a GIF file...
//

static int				// O - -1 = error, 0 = success
gif_read_cmap(FILE       *fp,		// I - File to read from
  	      int        ncolors,	// I - Number of colors
	      gif_cmap_t cmap)		// O - Colormap
{
  // Read the colormap...
  if (fread(cmap, 3, ncolors, fp) < (size_t)ncolors)
    return (0);

  return (1);
}


//
// 'gif_get_block()' - Read a GIF data block...
//

static int				// O - Number characters read
gif_get_block(FILE  *fp,		// I - File to read from
	      unsigned char *buf)	// I - Input buffer
{
  int	count;				// Number of character to read


  // Read the count byte followed by the data from the file...
  if ((count = getc(fp)) == EOF)
  {
    gif_eof = 1;
    return (-1);
  }
  else if (count == 0)
    gif_eof = 1;
  else if (fread(buf, 1, count, fp) < (size_t)count)
  {
    gif_eof = 1;
    return (-1);
  }
  else
    gif_eof = 0;

  return (count);
}


//
// 'gif_get_code()' - Get a LZW code from the file...
//

static int				// O - LZW code
gif_get_code(FILE *fp,			// I - File to read from
	     int  code_size,		// I - Size of code in bits
	     int  first_time)		// I - 1 = first time, 0 = not first time
{
  unsigned		i, j,		// Looping vars
			ret;		// Return value
  int			count;		// Number of bytes read
  static unsigned char	buf[280];	// Input buffer
  static unsigned	curbit,		// Current bit
			lastbit,	// Last bit in buffer
			done,		// Done with this buffer?
			last_byte;	// Last byte in buffer
  static unsigned	bits[8] =	// Bit masks for codes
			{
			  0x01, 0x02, 0x04, 0x08,
			  0x10, 0x20, 0x40, 0x80
			};


  if (first_time)
  {
    // Just initialize the input buffer...
    curbit  = 0;
    lastbit = 0;
    done    = 0;

    return (0);
  }


  if ((curbit + code_size) >= lastbit)
  {
    // Don't have enough bits to hold the code...
    if (done)
      return (-1);	// Sorry, no more...

    // Move last two bytes to front of buffer...
    if (last_byte > 1)
    {
      buf[0]    = buf[last_byte - 2];
      buf[1]    = buf[last_byte - 1];
      last_byte = 2;
    }
    else if (last_byte == 1)
    {
      buf[0]    = buf[last_byte - 1];
      last_byte = 1;
    }

    // Read in another buffer...
    if ((count = gif_get_block (fp, buf + last_byte)) <= 0)
    {
      // Whoops, no more data!
      done = 1;
      return (-1);
    }

    // Update buffer state...
    curbit    = (curbit - lastbit) + 8 * last_byte;
    last_byte += count;
    lastbit   = last_byte * 8;
  }

  ret = 0;
  for (ret = 0, i = curbit + code_size - 1, j = code_size;
       j > 0;
       i --, j --)
    ret = (ret << 1) | ((buf[i / 8] & bits[i & 7]) != 0);

  curbit += code_size;

  return ret;
}


//
// 'gif_read_lzw()' - Read a byte from the LZW stream...
//

static int				// I - Byte from stream
gif_read_lzw(FILE *fp,			// I - File to read from
	     int  first_time,		// I - 1 = first time, 0 = not first time
 	     int  input_code_size)	// I - Code size in bits
{
  int		i,			// Looping var
		code,			// Current code
		incode;			// Input code
  static short	fresh = 0,		// 1 = empty buffers
		code_size,		// Current code size
		set_code_size,		// Initial code size set
		max_code,		// Maximum code used
		max_code_size,		// Maximum code size
		firstcode,		// First code read
		oldcode,		// Last code read
		clear_code,		// Clear code for LZW input
		end_code,		// End code for LZW input
		table[2][4096],		// String table
		stack[8192],		// Output stack
		*sp;			// Current stack pointer


  if (first_time)
  {
    // Setup LZW state...
    set_code_size = input_code_size;
    code_size     = set_code_size + 1;
    clear_code    = 1 << set_code_size;
    end_code      = clear_code + 1;
    max_code_size = 2 * clear_code;
    max_code      = clear_code + 2;

    // Initialize input buffers...
    gif_get_code(fp, 0, 1);

    // Wipe the decompressor table...
    fresh = 1;

    for (i = 0; i < clear_code; i ++)
    {
      table[0][i] = 0;
      table[1][i] = i;
    }

    for (; i < 4096; i ++)
      table[0][i] = table[1][0] = 0;

    sp = stack;

    return (0);
  }
  else if (fresh)
  {
    fresh = 0;

    do
      firstcode = oldcode = gif_get_code(fp, code_size, 0);
    while (firstcode == clear_code);

    return (firstcode);
  }

  if (sp > stack)
    return (*--sp);

  while ((code = gif_get_code (fp, code_size, 0)) >= 0)
  {
    if (code == clear_code)
    {
      for (i = 0; i < clear_code; i ++)
      {
	table[0][i] = 0;
	table[1][i] = i;
      }

      for (; i < 4096; i ++)
	table[0][i] = table[1][i] = 0;

      code_size     = set_code_size + 1;
      max_code_size = 2 * clear_code;
      max_code      = clear_code + 2;

      sp = stack;

      firstcode = oldcode = gif_get_code(fp, code_size, 0);

      return (firstcode);
    }
    else if (code == end_code)
    {
      unsigned char	buf[260];


      if (!gif_eof)
        while (gif_get_block(fp, buf) > 0);

      return (-2);
    }

    incode = code;

    if (code >= max_code)
    {
      *sp++ = firstcode;
      code  = oldcode;
    }

    while (code >= clear_code)
    {
      *sp++ = table[1][code];
      if (code == table[0][code])
	return (255);

      code = table[0][code];
    }

    *sp++ = firstcode = table[1][code];
    code  = max_code;

    if (code < 4096)
    {
      table[0][code] = oldcode;
      table[1][code] = firstcode;
      max_code ++;

      if (max_code >= max_code_size && max_code_size < 4096)
      {
	max_code_size *= 2;
	code_size ++;
      }
    }

    oldcode = incode;

    if (sp > stack)
      return (*--sp);
  }

  return (code);
}


//
// 'gif_read_image()' - Read a GIF image stream...
//

static int				// I - 0 = success, -1 = failure
gif_read_image(FILE          *fp,	// I - Input file
	       Fl_HelpImage  *img,	// I - Image pointer
	       gif_cmap_t    cmap,	// I - Colormap
	       int           interlace)	// I - Non-zero = interlaced image
{
  unsigned char	code_size,		// Code size
		*temp;			// Current pixel
  int		xpos,			// Current X position
		ypos,			// Current Y position
		pass;			// Current pass
  int		pixel;			// Current pixel
  static int	xpasses[4] = { 8, 8, 4, 2 },
		ypasses[5] = { 0, 4, 2, 1, 999999 };


  xpos      = 0;
  ypos      = 0;
  pass      = 0;
  code_size = getc(fp);

  if (gif_read_lzw(fp, 1, code_size) < 0)
    return (0);

  temp = img->data;

  while ((pixel = gif_read_lzw(fp, 0, code_size)) >= 0)
  {
    temp[0] = cmap[pixel][0];

    if (img->d > 1)
    {
      temp[1] = cmap[pixel][1];
      temp[2] = cmap[pixel][2];
    }

    xpos ++;
    temp += img->d;
    if (xpos == img->w)
    {
      xpos = 0;

      if (interlace)
      {
        ypos += xpasses[pass];
        temp += (xpasses[pass] - 1) * img->w * img->d;

        if (ypos >= img->h)
	{
	  pass ++;

          ypos = ypasses[pass];
          temp = img->data + ypos * img->w * img->d;
	}
      }
      else
	ypos ++;
    }

    if (ypos >= img->h)
      break;
  }

  return (1);
}


//
// 'scrollbar_callback()' - A callback for the scrollbar.
//

static void
scrollbar_callback(Fl_Widget *s, void *)
{
  ((Fl_HelpView *)(s->parent()))->topline(int(((Fl_Scrollbar*)s)->value()));
}


//
// End of "$Id: Fl_HelpView.cxx,v 1.1.2.5 2001/09/10 03:09:43 easysw Exp $".
//
