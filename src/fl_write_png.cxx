//
// Fl_PNG_Image support functions for the Fast Light Tool Kit (FLTK).
//
// Copyright 2005-2021 by Bill Spitzak and others.
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
#include <FL/fl_string_functions.h>
#include <FL/fl_utf8.h> // fl_fopen()
#include <stdio.h>
#include <time.h> // hack to restore "configure --enable-x11" on macOS ≥ 11

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
  Write an RGB(A) image to a PNG image file.

  This is a very basic and restricted function to create a PNG image file
  from an RGB image (Fl_RGB_Image).

  The image data must be aligned w/o gaps, i.e. ld() \b MUST be zero or
  equal to data_w() * data_h().

  The image file is always written with the original image size data_w()
  and data_h(), even if the image has been scaled.

  Image depth 1 (gray), 2 (gray + alpha channel), 3 (RGB) and 4 (RGBA)
  are supported.

  \note Currently there is no error handling except for errors when opening
    the file. This may be changed in the future.

  \param[in]  filename  Output filename, extension should be '.png'
  \param[in]  img       RGB image to be written

  \return     success (0) or error code: negative values are errors

  \retval      0        success, file has been written
  \retval     -1        png or zlib library not available
  \retval     -2        file open error

  \see fl_write_png(const char *, int, int, int, const unsigned char *)
*/

int fl_write_png(const char *filename, Fl_RGB_Image *img) {
  return fl_write_png(filename,
                      img->data()[0],
                      img->data_w(),
                      img->data_h(),
                      img->d(),
                      img->ld());
}

/**
  Write raw image data to a PNG image file.

  \see fl_write_png(const char *filename, const char *pixels, int w, int h, int d, int ld)
*/
int fl_write_png(const char *filename, const unsigned char *pixels, int w, int h, int d, int ld) {
  return fl_write_png(filename, (const char *)pixels, w, h, d, ld);
}

/**
  Write raw image data to a PNG image file.

  This is a very basic and restricted function to create a PNG image file
  from raw image data, e.g. a screenshot.

  The image data must be aligned w/o gaps after each row (ld = 0 or ld = w * d)
  or \p ld must be the total length of each row, i.e. w * d + gapsize.
  If ld == 0 then ld = w * d is assumed.

  The total data size must be (w * d + gapsize) * h = ld' * h
  where ld' = w * d if ld == 0.

  For further restrictions and return values please see
  fl_write_png(const char *filename, Fl_RGB_Image *img).

  \param[in]  filename  Output filename, extension should be '.png'
  \param[in]  pixels    Image data
  \param[in]  w         Image data width
  \param[in]  h         Image data height
  \param[in]  d         Image depth: 1 = GRAY, 2 = GRAY + alpha, 3 = RGB, 4 = RGBA
  \param[in]  ld        Line delta: default (0) = w * d

  \return     success (0) or error code, see ...

  \see fl_write_png(const char *filename, Fl_RGB_Image *img)
*/
int fl_write_png(const char *filename, const char *pixels, int w, int h, int d, int ld) {

#if defined(HAVE_LIBPNG) && defined(HAVE_LIBZ)

  FILE *fp;
  int color_type;

  if ((fp = fl_fopen(filename, "wb")) == NULL) {
    return -2;
  }

  switch (d) {
    case 1:  color_type = PNG_COLOR_TYPE_GRAY;        break;
    case 2:  color_type = PNG_COLOR_TYPE_GRAY_ALPHA;  break;
    case 3:  color_type = PNG_COLOR_TYPE_RGB;         break;
    case 4:  color_type = PNG_COLOR_TYPE_RGB_ALPHA;   break;
    default: color_type = PNG_COLOR_TYPE_RGB;
  }

  if (ld == 0)
    ld = w * d;

  png_structp pptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  png_infop iptr = png_create_info_struct(pptr);
  png_bytep ptr = (png_bytep)pixels;

  png_init_io(pptr, fp);
  png_set_IHDR(pptr, iptr, w, h, 8,
               color_type,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_set_sRGB(pptr, iptr, PNG_sRGB_INTENT_PERCEPTUAL);

  png_write_info(pptr, iptr);

  for (int i = 0; i < h; i++, ptr += ld) {
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
