//
// "$Id$"
//
// Fl_PNG_Image routines.
//
// Copyright 1997-2009 by Easy Software Products.
// Image support by Matthias Melcher, Copyright 2000-2009.
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
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//
// Contents:

//
//   Fl_PNG_Image::Fl_PNG_Image() - Load a PNG image file.
//

//
// Include necessary header files...
//

#include <FL/Fl.H>
#include <FL/Fl_PNG_Image.H>
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <FL/fl_utf8.h>

extern "C"
{
#if defined(HAVE_LIBPNG) && defined(HAVE_LIBZ)
#  include <zlib.h>
#  ifdef HAVE_PNG_H
#    include <png.h>
#  else
#    include <libpng/png.h>
#  endif // HAVE_PNG_H
#endif // HAVE_LIBPNG && HAVE_LIBZ
}


/**
  The constructor loads the named PNG image from the given png filename.
  <P>The destructor free all memory and server resources that are used by
  the image.
*/
Fl_PNG_Image::Fl_PNG_Image(const char *png) // I - File to read
  : Fl_RGB_Image(0,0,0) {
#if defined(HAVE_LIBPNG) && defined(HAVE_LIBZ)
  int		i;			// Looping var
  FILE		*fp;			// File pointer
  int		channels;		// Number of color channels
  png_structp	pp;			// PNG read pointer
  png_infop	info;			// PNG info pointers
  png_bytep	*rows;			// PNG row pointers


  // Open the PNG file...
  if ((fp = fl_fopen(png, "rb")) == NULL) return;

  // Setup the PNG data structures...
  pp   = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info = png_create_info_struct(pp);

  if (setjmp(png_jmpbuf(pp)))
  {
    Fl::warning("PNG file \"%s\" contains errors!\n", png);
    return;
  }

  // Initialize the PNG read "engine"...
  png_init_io(pp, fp);

  // Get the image dimensions and convert to grayscale or RGB...
  png_read_info(pp, info);

  if (png_get_color_type(pp, info) == PNG_COLOR_TYPE_PALETTE)
    png_set_expand(pp);

  if (png_get_color_type(pp, info) & PNG_COLOR_MASK_COLOR)
    channels = 3;
  else
    channels = 1;

  int num_trans = 0;
  png_get_tRNS(pp, info, 0, &num_trans, 0);
  if ((png_get_color_type(pp, info) & PNG_COLOR_MASK_ALPHA) || (num_trans != 0))
      channels ++;

  w((int)(png_get_image_width(pp, info)));
  h((int)(png_get_image_height(pp, info)));
  d(channels);

  if (png_get_bit_depth(pp, info) < 8)
  {
    png_set_packing(pp);
    png_set_expand(pp);
  }
  else if (png_get_bit_depth(pp, info) == 16)
    png_set_strip_16(pp);

#  if defined(HAVE_PNG_GET_VALID) && defined(HAVE_PNG_SET_TRNS_TO_ALPHA)
  // Handle transparency...
  if (png_get_valid(pp, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(pp);
#  endif // HAVE_PNG_GET_VALID && HAVE_PNG_SET_TRNS_TO_ALPHA

  array = new uchar[w() * h() * d()];
  alloc_array = 1;

  // Allocate pointers...
  rows = new png_bytep[h()];

  for (i = 0; i < h(); i ++)
    rows[i] = (png_bytep)(array + i * w() * d());

  // Read the image, handling interlacing as needed...
  for (i = png_set_interlace_handling(pp); i > 0; i --)
    png_read_rows(pp, rows, NULL, h());

#ifdef WIN32
  // Some Windows graphics drivers don't honor transparency when RGB == white
  if (channels == 4) {
    // Convert RGB to 0 when alpha == 0...
    uchar *ptr = (uchar *)array;
    for (i = w() * h(); i > 0; i --, ptr += 4)
      if (!ptr[3]) ptr[0] = ptr[1] = ptr[2] = 0;
  }
#endif // WIN32

  // Free memory and return...
  delete[] rows;

  png_read_end(pp, info);
  png_destroy_read_struct(&pp, &info, NULL);

  fclose(fp);
#endif // HAVE_LIBPNG && HAVE_LIBZ
}


//
// End of "$Id$".
//
