//
// Fl_PNG_Image support functions for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include <config.h>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/fl_string.h>
#include <FL/fl_utf8.h> // fl_fopen()
#include <stdio.h>

// PNG library include files

extern "C" {
#if defined(HAVE_LIBPNG) && defined(HAVE_LIBZ)
#include <zlib.h>
#ifdef HAVE_PNG_H
#include <png.h>
#else
#include <libpng/png.h>
#endif // HAVE_PNG_H
#endif // HAVE_LIBPNG && HAVE_LIBZ
} // extern "C"

/**
  \file fl_write_png.cxx

  PNG image support functions.

*/

/**
  Write a RGB(A) image to a PNG image file.

  This is a very basic and restricted function to create a PNG image file
  from an RGB image (Fl_RGB_Image).

  The image data must be aligned w/o gaps, i.e. ld() \b MUST be zero or
  equal to data_w() * data_h().

  The image file is always written with the original image size data_w()
  and data_h(), even if the image has been scaled.

  Image depth 3 (RGB) and 4 (RGBA) are supported.

  \note Behavior of grayscale images (depth 1 and 2) is currently undefined
    and may or may not work. There is no error handling except for opening
    the file. This function may be changed in the future.

  \param[in]  filename  Output filename, extension should be '.png'
  \param[in]  img       RGB image to be written

  \return     success (0) or error code

  \retval      0        success, file has been written
  \retval     -1        png or zlib library not available
  \retval     -2        file open error

  \see fl_write_png(const char *, int, int, int, const unsigned char *)
*/

int fl_write_png(const char *filename, Fl_RGB_Image *img) {
  return fl_write_png(filename,
                      img->data_w(), img->data_h(), img->d(),
                      (const unsigned char *)img->data()[0]);
}

/**
  Write raw image data to a PNG image file.

  This is a very basic and restricted function to create a PNG image file
  from raw image data, e.g. a screenshot.

  The image data must be aligned w/o gaps after each row.

  For further restrictions and return values please see
  fl_write_png(const char *filename, Fl_RGB_Image *img).

  \param[in]  filename  Output filename, extension should be '.png'
  \param[in]  w         Image data width
  \param[in]  h         Image data height
  \param[in]  d         Image depth (3 = RGB, 4 = RGBA)
  \param[in]  pixels    Image data (\p w * \p h * \p d bytes)

  \return     success (0) or error code, see ...

  \see fl_write_png(const char *filename, Fl_RGB_Image *img)
 */
int fl_write_png(const char *filename, int w, int h, int d, const unsigned char *pixels) {

#if defined(HAVE_LIBPNG) && defined(HAVE_LIBZ)

  FILE *fp;

  if ((fp = fl_fopen(filename, "wb")) == NULL) {
    return -2;
  }

  png_structp pptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  png_infop iptr = png_create_info_struct(pptr);
  png_bytep ptr = (png_bytep)pixels;

  png_init_io(pptr, fp);
  png_set_IHDR(pptr, iptr, w, h, 8,
               PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_sRGB(pptr, iptr, PNG_sRGB_INTENT_PERCEPTUAL);

  png_write_info(pptr, iptr);

  for (int i = h; i > 0; i--, ptr += w * d) {
    png_write_row(pptr, ptr);
  }

  png_write_end(pptr, iptr);
  png_destroy_write_struct(&pptr, &iptr);

  fclose(fp);
  return 0;

#else
  return -1;
#endif
}
