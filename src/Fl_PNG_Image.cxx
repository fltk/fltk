//
// "$Id: Fl_PNG_Image.cxx,v 1.1.2.1 2001/11/19 01:06:45 easysw Exp $"
//
// Fl_PNG_Image routines.
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

#include <FL/Fl_PNG_Image.H>
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <errno.h>

extern "C"
{
#ifdef HAVE_LIBPNG
#  include <zlib.h>
#  include <png.h>
#endif // HAVE_LIBPNG
}


#if 0
#ifdef HAVE_LIBPNG
//
// 'Fl_Help_View::load_png()' - Load a PNG image file.
//

int					// O - 0 = success, -1 = fail
Fl_Help_View::load_png(Fl_Help_Image *img,// I - Image pointer
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

  if (info->color_type & PNG_COLOR_MASK_COLOR)
    img->d = 3;
  else
    img->d = 1;

  if ((info->color_type & PNG_COLOR_MASK_ALPHA) || info->num_trans)
    img->d ++;

  if (info->bit_depth < 8)
  {
    png_set_packing(pp);
    png_set_expand(pp);
  }
  else if (info->bit_depth == 16)
    png_set_strip_16(pp);

#if defined(HAVE_PNG_GET_VALID) && defined(HAVE_SET_TRNS_TO_ALPHA)
  // Handle transparency...
  if (png_get_valid(pp, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(pp);
#endif // HAVE_PNG_GET_VALID && HAVE_SET_TRNS_TO_ALPHA

  img->w    = (int)info->width;
  img->h    = (int)info->height;
  img->data = (unsigned char *)malloc(img->w * img->h * img->d);

  // Background color...
  unsigned	rgba = fltk_colors[bgcolor_];

  bg.red   = 65535 * (rgba >> 24) / 255;
  bg.green = 65535 * ((rgba >> 16) & 255) / 255;
  bg.blue  = 65535 * ((rgba >> 8) & 255) / 255;

  png_set_background(pp, &bg, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

  // Allocate pointers...
  rows = (png_bytep *)calloc(info->height, sizeof(png_bytep));

  for (i = 0; i < (int)info->height; i ++)
    rows[i] = img->data + i * img->w * img->d;

  // Read the image, handling interlacing as needed...
  for (i = png_set_interlace_handling(pp); i > 0; i --)
    png_read_rows(pp, rows, NULL, img->h);

  // Free memory and return...
  free(rows);

  png_read_end(pp, info);
#  ifdef HAVE_PNG_READ_DESTROY
  png_read_destroy(pp, info, NULL);
#  else
  png_destroy_read_struct(&pp, &info, NULL);
#  endif // HAVE_PNG_READ_DESTROY

  return (1);
}
#endif // HAVE_LIBPNG
#endif // 0

//
// End of "$Id: Fl_PNG_Image.cxx,v 1.1.2.1 2001/11/19 01:06:45 easysw Exp $".
//
