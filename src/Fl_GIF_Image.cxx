//
// "$Id: Fl_GIF_Image.cxx,v 1.1.2.1 2001/11/19 01:06:45 easysw Exp $"
//
// Fl_GIF_Image routines.
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
//

//
// Include necessary header files...
//

#include <FL/Fl_GIF_Image.H>
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <errno.h>

#if defined(WIN32) && ! defined(__CYGWIN__)
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


//
// GIF definitions...
//

#define GIF_INTERLACE	0x40
#define GIF_COLORMAP	0x80

typedef unsigned char	gif_cmap_t[256][3];


//
// Local functions...
//

#if 0
static int	gif_read_cmap(FILE *fp, int ncolors, gif_cmap_t cmap);
static int	gif_get_block(FILE *fp, unsigned char *buffer);
static int	gif_get_code (FILE *fp, int code_size, int first_time);
static int	gif_read_lzw(FILE *fp, int first_time, int input_code_size);
static int	gif_read_image(FILE *fp, Fl_Help_Image *img, gif_cmap_t cmap,
		               int interlace);


//
// 'Fl_Help_View::load_gif()' - Load a GIF image file...
//

int					// O - 0 = success, -1 = fail
Fl_Help_View::load_gif(Fl_Help_Image *img,// I - Image pointer
        	       FILE         *fp)// I - File to load from
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
	       Fl_Help_Image  *img,	// I - Image pointer
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
#endif


//
// End of "$Id: Fl_GIF_Image.cxx,v 1.1.2.1 2001/11/19 01:06:45 easysw Exp $".
//
