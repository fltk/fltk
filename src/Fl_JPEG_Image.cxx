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
// Custom JPEG error handling structure...
//

struct fl_jpeg_error_mgr {
  jpeg_error_mgr	pub_;		// Destination manager...
  int			err_;		// Error flag
};


//
// Error handler for JPEG files...
//

#ifdef HAVE_LIBJPEG
static void
fl_jpeg_error_handler(j_common_ptr dinfo) {	// I - Decompressor info
  ((fl_jpeg_error_mgr *)(dinfo->err))->err_ = 1;
  return;
}

static void
fl_jpeg_output_handler(j_common_ptr dinfo) {	// I - Decompressor info
  return;
}
#endif // HAVE_LIBJPEG


//
// 'Fl_JPEG_Image::Fl_JPEG_Image()' - Load a JPEG image file.
//

Fl_JPEG_Image::Fl_JPEG_Image(const char *jpeg)	// I - File to load
  : Fl_RGB_Image(0,0,0) {
#ifdef HAVE_LIBJPEG
  FILE				*fp;	// File pointer
  jpeg_decompress_struct	dinfo;	// Decompressor info
  fl_jpeg_error_mgr		jerr;	// Error handler info
  JSAMPROW			row;	// Sample row pointer


  // Clear data...
  alloc_array = 0;
  array = (uchar *)0;

  // Open the image file...
  if ((fp = fopen(jpeg, "rb")) == NULL) return;

  // Setup the decompressor info and read the header...
  dinfo.err                = jpeg_std_error((jpeg_error_mgr *)&jerr);
  jerr.pub_.error_exit     = fl_jpeg_error_handler;
  jerr.pub_.output_message = fl_jpeg_output_handler;
  jerr.err_                = 0;

  jpeg_create_decompress(&dinfo);
  jpeg_stdio_src(&dinfo, fp);
  jpeg_read_header(&dinfo, 1);

  if (jerr.err_) goto error_return;

  dinfo.quantize_colors      = (boolean)FALSE;
  dinfo.out_color_space      = JCS_RGB;
  dinfo.out_color_components = 3;
  dinfo.output_components    = 3;

  jpeg_calc_output_dimensions(&dinfo);

  w(dinfo.output_width);
  h(dinfo.output_height);
  d(dinfo.output_components);

  if (!w() || !h() || !d() || jerr.err_) goto error_return;

  array = new uchar[w() * h() * d()];
  alloc_array = 1;

  jpeg_start_decompress(&dinfo);

  while (dinfo.output_scanline < dinfo.output_height) {
    if (jerr.err_) goto error_return;

    row = (JSAMPROW)(array +
                     dinfo.output_scanline * dinfo.output_width *
                     dinfo.output_components);
    jpeg_read_scanlines(&dinfo, &row, (JDIMENSION)1);
  }

  jpeg_finish_decompress(&dinfo);
  jpeg_destroy_decompress(&dinfo);

  fclose(fp);

  return;

  // JPEG error handling...
  error_return:

  if (array) jpeg_finish_decompress(&dinfo);
  jpeg_destroy_decompress(&dinfo);

  fclose(fp);

  w(0);
  h(0);
  d(0);

  if (array) {
    delete[] (uchar *)array;
    array = 0;
    alloc_array = 0;
  }
#endif // HAVE_LIBJPEG
}

//
// End of "$Id$".
//
