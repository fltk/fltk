//
// Fl_JPEG_Image support functions for the Fast Light Tool Kit (FLTK).
//
// Copyright 2005-2025 by Bill Spitzak and others.
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
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/fl_utf8.h>               // fl_fopen()
#include <stdio.h>
#include <stdlib.h>                   // malloc, free

extern "C" {
#ifdef HAVE_LIBJPEG
#  include <jpeglib.h>
#endif // HAVE_LIBJPEG
} // extern "C"

/**
  \file fl_write_jpeg.cxx

  JPEG image support functions.

*/

/**
  Write an RGB image to a JPEG image file.

  Create a JPEG image file from an RGB image (Fl_RGB_Image).

  The image file is always written with the original image size data_w()
  and data_h(), even if the image has been scaled.

  Image depth 1 (gray), 2 (gray + alpha), 3 (RGB), and 4 (RGBA) are supported.
  For images with alpha channel (depth 2 or 4), the alpha component is ignored
  and only the color data is written since JPEG does not support transparency.

  \note Error handling is limited to basic error detection: library availability
    and file opening errors.

  \param[in]  filename  Output filename, extension should be '.jpg' or '.jpeg'
  \param[in]  img       RGB image to be written

  \return     success (0) or error code: negative values are errors

  \retval      0        success, file has been written
  \retval     -1        jpeg library not available
  \retval     -2        file open error
  \retval     -3        invalid image depth (must be 1, 2, 3, or 4)
  \retval     -4        memory allocation error

  \see fl_write_jpeg(const char *, const char *, int, int, int, int)
*/

int fl_write_jpeg(const char *filename, Fl_RGB_Image *img) {
  return fl_write_jpeg(filename,
                       img->data()[0],
                       img->data_w(),
                       img->data_h(),
                       img->d(),
                       img->ld());
}

/**
  Write raw image data to a JPEG image file.

  \see fl_write_jpeg(const char *filename, const char *pixels, int w, int h, int d, int ld)
*/
int fl_write_jpeg(const char *filename, const unsigned char *pixels, int w, int h, int d, int ld) {
  return fl_write_jpeg(filename, (const char *)pixels, w, h, d, ld);
}

/**
  Write raw image data to a JPEG image file.

  For further restrictions and return values please see
  fl_write_jpeg(const char *filename, Fl_RGB_Image *img).

  \param[in]  filename  Output filename, extension should be '.jpg' or '.jpeg'
  \param[in]  pixels    Image data
  \param[in]  w         Image data width
  \param[in]  h         Image data height
  \param[in]  d         Image depth: 1 = GRAY, 2 = GRAY+alpha, 3 = RGB, 4 = RGBA
  \param[in]  ld        Line delta: default (0) = w * d

  \return     success (0) or error code: negative values are errors

  \see fl_write_jpeg(const char *filename, Fl_RGB_Image *img)
*/
int fl_write_jpeg(const char *filename, const char *pixels, int w, int h, int d, int ld) {

#ifdef HAVE_LIBJPEG

  FILE *fp;
  J_COLOR_SPACE color_space;
  int out_d;              // output depth (without alpha)
  unsigned char *row_buf = NULL;  // buffer for stripping alpha channel

  // Validate depth: must be 1, 2, 3, or 4
  if (d < 1 || d > 4) {
    return -3;
  }

  if ((fp = fl_fopen(filename, "wb")) == NULL) {
    return -2;
  }

  // Determine output depth and color space
  // Strip alpha channel: depth 2 -> 1 (gray), depth 4 -> 3 (RGB)
  switch (d) {
    case 1:
    case 2:
      color_space = JCS_GRAYSCALE;
      out_d = 1;
      break;
    default:  // 3 or 4
      color_space = JCS_RGB;
      out_d = 3;
      break;
  }

  if (ld == 0)
    ld = w * d;

  // Allocate buffer for stripping alpha if needed
  int strip_alpha = (d == 2 || d == 4);
  if (strip_alpha) {
    row_buf = (unsigned char *)malloc(w * out_d);
    if (row_buf == NULL) {
      fclose(fp);
      return -4;
    }
  }

  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, fp);

  cinfo.image_width = w;
  cinfo.image_height = h;
  cinfo.input_components = out_d;
  cinfo.in_color_space = color_space;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 95, TRUE);  // Quality 95 is a good balance

  jpeg_start_compress(&cinfo, TRUE);

  JSAMPROW row_pointer;
  const unsigned char *ptr = (const unsigned char *)pixels;

  while (cinfo.next_scanline < cinfo.image_height) {
    if (strip_alpha) {
      // Strip alpha channel: copy only color components
      const unsigned char *src = ptr;
      unsigned char *dst = row_buf;
      for (int x = 0; x < w; x++) {
        for (int c = 0; c < out_d; c++) {
          *dst++ = *src++;
        }
        src++;  // skip alpha byte
      }
      row_pointer = (JSAMPROW)row_buf;
    } else {
      row_pointer = (JSAMPROW)ptr;
    }
    jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    ptr += ld;
  }

  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  if (row_buf)
    free(row_buf);

  fclose(fp);
  return 0;

#else
  return -1;
#endif
}
