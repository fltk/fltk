//
// "$Id: Fl_JPEG_Image.cxx,v 1.1.2.1 2001/11/19 01:06:45 easysw Exp $"
//
// Fl_JPEG_Image routines.
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

#include <FL/Fl_JPEG_Image.H>
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

extern "C"
{
#ifdef HAVE_LIBJPEG
#  include <jpeglib.h>
#endif // HAVE_LIBJPEG
}

#define MAX_COLUMNS	200


#if 0
#ifdef HAVE_LIBJPEG
//
// 'Fl_Help_View::load_jpeg()' - Load a JPEG image file.
//

int						// O - 0 = success, -1 = fail
Fl_Help_View::load_jpeg(Fl_Help_Image *img,	// I - Image pointer
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

#endif // 0

//
// End of "$Id: Fl_JPEG_Image.cxx,v 1.1.2.1 2001/11/19 01:06:45 easysw Exp $".
//
