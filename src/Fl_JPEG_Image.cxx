//
// "$Id$"
//
// Fl_JPEG_Image routines.
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
//   Fl_JPEG_Image::Fl_JPEG_Image() - Load a JPEG image file.
//

//
// Include necessary header files...
//

#include <FL/Fl_JPEG_Image.H>
#include <config.h>
#include <stdio.h>
#include <stdlib.h>


// Some releases of the Cygwin JPEG libraries don't have a correctly
// updated header file for the INT32 data type; the following define
// from Shane Hill seems to be a usable workaround...

#if defined(WIN32) && defined(__CYGWIN__)
#  define XMD_H
#endif // WIN32 && __CYGWIN__


extern "C"
{
#ifdef HAVE_LIBJPEG
#  include <jpeglib.h>
#endif // HAVE_LIBJPEG
}


//
// Error handler for JPEG files...
//

#ifdef HAVE_LIBJPEG
static void
jpeg_error_handler(j_common_ptr)
{
  return;
}
#endif // HAVE_LIBJPEG


//
// 'Fl_JPEG_Image::Fl_JPEG_Image()' - Load a JPEG image file.
//

Fl_JPEG_Image::Fl_JPEG_Image(const char *jpeg)	// I - File to load
  : Fl_RGB_Image(0,0,0) {
#ifdef HAVE_LIBJPEG
  FILE				*fp;		// File pointer
  struct jpeg_decompress_struct	cinfo;		// Decompressor info
  struct jpeg_error_mgr		jerr;		// Error handler info
  JSAMPROW			row;		// Sample row pointer


  if ((fp = fopen(jpeg, "rb")) == NULL) return;

  cinfo.err = jpeg_std_error(&jerr);
  jerr.error_exit = jpeg_error_handler;
  jerr.output_message = jpeg_error_handler;

  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, fp);
  jpeg_read_header(&cinfo, 1);

  cinfo.quantize_colors      = (boolean)FALSE;
  cinfo.out_color_space      = JCS_RGB;
  cinfo.out_color_components = 3;
  cinfo.output_components    = 3;

  jpeg_calc_output_dimensions(&cinfo);

  w(cinfo.output_width);
  h(cinfo.output_height);
  d(cinfo.output_components);

  array = new uchar[w() * h() * d()];
  alloc_array = 1;

  jpeg_start_decompress(&cinfo);

  while (cinfo.output_scanline < cinfo.output_height)
  {
    row = (JSAMPROW)(array +
                     cinfo.output_scanline * cinfo.output_width *
                     cinfo.output_components);
    jpeg_read_scanlines(&cinfo, &row, (JDIMENSION)1);
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  fclose(fp);

#  if 0
  // JPEG error handling...
  error_return:

  if (array) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  fclose(fp);

  if (array) {
    w(0);
    h(0);
    d(0);

    delete[] (uchar *)array;
    array = 0;
    alloc_array = 0;
  }
#  endif // 0
#endif // HAVE_LIBJPEG
}

//
// End of "$Id$".
//
