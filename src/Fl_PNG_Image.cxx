//
// "$Id: Fl_PNG_Image.cxx,v 1.1.2.2 2001/11/23 12:06:36 easysw Exp $"
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
#include <config.h>
#include <stdio.h>
#include <stdlib.h>

extern "C"
{
#ifdef HAVE_LIBPNG
#  include <zlib.h>
#  include <png.h>
#endif // HAVE_LIBPNG
}


//
// 'Fl_PNG_Image::Fl_PNG_Image()' - Load a PNG image file.
//

Fl_PNG_Image::Fl_PNG_Image(const char *png) // I - File to read
  : Fl_RGB_Image(0,0,0) {
#ifdef HAVE_LIBPNG
  int		i;			// Looping var
  FILE		*fp;			// File pointer
  int		channels;		// Number of color channels
  png_structp	pp;			// PNG read pointer
  png_infop	info;			// PNG info pointers
  png_bytep	*rows;			// PNG row pointers


  // Open the PNG file...
  if ((fp = fopen(png, "rb")) == NULL) return;

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
    channels = 3;
  else
    channels = 1;

  if ((info->color_type & PNG_COLOR_MASK_ALPHA) || info->num_trans)
    channels ++;

  w(info->width);
  h(info->height);
  d(channels);

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

  array = new uchar[w() * h() * d()];

  // Allocate pointers...
  rows = new png_bytep[h()];

  for (i = 0; i < h(); i ++)
    rows[i] = (png_bytep)(array + i * w() * d());

  // Read the image, handling interlacing as needed...
  for (i = png_set_interlace_handling(pp); i > 0; i --)
    png_read_rows(pp, rows, NULL, h());

  // Free memory and return...
  delete rows;

  png_read_end(pp, info);
#  ifdef HAVE_PNG_READ_DESTROY
  png_read_destroy(pp, info, NULL);
#  else
  png_destroy_read_struct(&pp, &info, NULL);
#  endif // HAVE_PNG_READ_DESTROY

  fclose(fp);
#endif // HAVE_LIBPNG
}


//
// End of "$Id: Fl_PNG_Image.cxx,v 1.1.2.2 2001/11/23 12:06:36 easysw Exp $".
//
