//
// FLTK images library core.
//
// Copyright 1997-2010 by Easy Software Products.
// Copyright 2011-2022 by Bill Spitzak and others.
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
// Contents:
//
//   fl_register_images() - Register the image formats.
//   fl_check_images()    - Check for a supported image format.
//

//
// Include necessary header files...
//

#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_GIF_Image.H>
#include <FL/Fl_Anim_GIF_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_PNM_Image.H>
#include <FL/Fl_SVG_Image.H>
#include <FL/Fl_ICO_Image.H>
#include <FL/fl_utf8.h>
#include <stdio.h>
#include <stdlib.h>
#include "flstring.h"
#if defined(HAVE_LIBZ)
#include <zlib.h>
#endif

//
// Define a simple global image registration function that registers
// the extra image formats that aren't part of the core FLTK library.
//

static Fl_Image *fl_check_images(const char *name, uchar *header, int headerlen);


/**
\brief Register the known image formats.

  This function is provided in the fltk_images library and
  registers all of the "extra" image file formats known to FLTK
  that are not part of the core FLTK library.

  You may add your own image formats with Fl_Shared_Image::add_handler().
*/
void fl_register_images() {
  Fl_Shared_Image::add_handler(fl_check_images);
  Fl_Image::register_images_done = true;
}


//
// 'fl_check_images()' - Check for a supported image format.
//
// returns 0 (NULL) if <headerlen> is less than 6 because:
//  (1) some of the comparisons would otherwise access undefined data
//  (2) there's no valid image file with less than 6 bytes
//
// Note 1: The number 6 above may be changed if necessary as long as
//   condition (2) holds.
//
// Note 2: The provided buffer <header> MUST NOT be overwritten by any
//   check function because subsequently called check functions need
//   the original image header data. <header> should be const!

Fl_Image *                                      // O - Image, if found
fl_check_images(const char *name,               // I - Filename
                uchar      *header,             // I - Header data from file
                int         headerlen) {        // I - Amount of data in header

  if (headerlen < 6) // not a valid image
    return 0;

  // GIF

  if (memcmp(header, "GIF87a", 6) == 0 ||
      memcmp(header, "GIF89a", 6) == 0) // GIF file
    return Fl_GIF_Image::animate ? new Fl_Anim_GIF_Image(name) :
                                   new Fl_GIF_Image(name);

  // BMP

  if (memcmp(header, "BM", 2) == 0)     // BMP file
    return new Fl_BMP_Image(name);

  if (memcmp(header, "\0\0\1\0", 4) == 0 && header[5] == 0)   // ICO file
    return new Fl_ICO_Image(name);

  // PNM

  if (header[0] == 'P' && header[1] >= '1' && header[1] <= '7')
                                        // Portable anymap
    return new Fl_PNM_Image(name);

  // PNG

#ifdef HAVE_LIBPNG
  if (memcmp(header, "\211PNG", 4) == 0)// PNG file
    return new Fl_PNG_Image(name);
#endif // HAVE_LIBPNG

  // JPEG

#ifdef HAVE_LIBJPEG
  if (memcmp(header, "\377\330\377", 3) == 0 && // Start-of-Image
      header[3] >= 0xc0 && header[3] <= 0xfe)   // APPn .. comment for JPEG file
    return new Fl_JPEG_Image(name);
#endif // HAVE_LIBJPEG

  // SVG or SVGZ (gzip'ed SVG)

#ifdef FLTK_USE_SVG
  uchar header2[64];      // buffer for decompression
  uchar *buf = header;    // original header data
  int count = headerlen;  // original header data size

  // Note: variables 'buf' and 'count' may be overwritten subsequently
  // if the image data is gzip'ed *and* we can decompress the data

# if defined(HAVE_LIBZ)
  if (header[0] == 0x1f && header[1] == 0x8b) { // gzip'ed data
    int fd = fl_open_ext(name, 1, 0);
    if (fd < 0) return NULL;
    gzFile gzf = gzdopen(fd, "r");
    if (gzf) {
      count = gzread(gzf, header2, (int)sizeof(header2));
      gzclose(gzf);
      buf = header2; // decompressed data
    }
  } // gzip'ed data
# endif // HAVE_LIBZ

  // Check if we have a UTF-8 BOM in the first three bytes (issue #247).
  // If yes we need at least 5 more bytes to recognize the signature.
  // Note: BOM (Byte Order Mark) in UTF-8 is not recommended but allowed.

  if (count >= 8) {
    const uchar bom[3] = { 0xef, 0xbb, 0xbf };
    if (memcmp(buf, bom, 3) == 0) {
      buf += 3;
      count -= 3;
    }
  }

  // Check svg or xml signature

  while (count && isspace(buf[0])) { buf++; count--; }
  if ((count >= 5 &&
       (memcmp(buf, "<?xml", 5) == 0 ||
        memcmp(buf, "<svg", 4) == 0  ||
        memcmp(buf, "<!--", 4) == 0))) {
    Fl_SVG_Image *image = new Fl_SVG_Image(name);
    if (image->w() && image->h())
      return image;
    delete image;
  }
#endif // FLTK_USE_SVG

  // unknown image format

  return 0;
}
