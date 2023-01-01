//
// Fl_BMP_Image class for the Fast Light Tool Kit (FLTK).
//
// Copyright 2011-2022 by Bill Spitzak and others.
// Copyright 1997-2010 by Easy Software Products.
// Image support by Matthias Melcher, Copyright 2000-2009.
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

//
// Include necessary header files...
//

#include <FL/Fl_BMP_Image.H>
#include "Fl_Image_Reader.h"
#include <FL/fl_utf8.h>
#include <FL/Fl.H>
#include <stdio.h>
#include <stdlib.h>

//
// BMP definitions...
//

#ifndef BI_RGB
#  define BI_RGB       0             // No compression - straight BGR data
#  define BI_RLE8      1             // 8-bit run-length compression
#  define BI_RLE4      2             // 4-bit run-length compression
#  define BI_BITFIELDS 3             // RGB bitmap with RGB masks
#endif // !BI_RGB


/**
  This constructor loads the named BMP image from the given BMP filename.

  The destructor frees all memory and server resources that are used by
  the image.

  Use Fl_Image::fail() to check if Fl_BMP_Image failed to load. fail() returns
  ERR_FILE_ACCESS if the file could not be opened or read, ERR_FORMAT if the
  BMP format could not be decoded, and ERR_NO_IMAGE if the image could not
  be loaded for another reason.

  \param[in] filename a full path and name pointing to a BMP file.

  \see Fl_BMP_Image::Fl_BMP_Image(const char* imagename, const unsigned char *data, const long length = -1);
*/
Fl_BMP_Image::Fl_BMP_Image(const char *filename) // I - File to read
: Fl_RGB_Image(0,0,0)
{
  Fl_Image_Reader rdr;
  if (rdr.open(filename) == -1) {
    ld(ERR_FILE_ACCESS);
  } else {
    load_bmp_(rdr);
  }
}

/**
  This constructor loads a BMP image from memory.

  Construct an image from a block of memory inside the application. Fluid offers
  "binary data" chunks as a great way to add image data into the C++ source code.
  \p imagename can be NULL. If a name is given, the image is added to the list of
  shared images and will be available by that name.

  The destructor frees all memory and server resources that are used by
  the image.

  The (new and optional) third parameter \p length \b should be used so buffer
  overruns (i.e. truncated images) can be checked. See note below.

  If \p length is not used
  - it defaults to -1 (unlimited size)
  - buffer overruns will not be checked.

  \note The optional parameter \p length is available since FLTK 1.4.0.
    Not using it is deprecated and old code should be modified to use it.
    This parameter will likely become mandatory in a future FLTK version.

  Use Fl_Image::fail() to check if Fl_BMP_Image failed to load. fail() returns
  ERR_FILE_ACCESS if the image could not be read from memory, ERR_FORMAT if the
  BMP format could not be decoded, and ERR_NO_IMAGE if the image could not
  be loaded for another reason.

  \param[in] imagename  A name given to this image or NULL
  \param[in] data       Pointer to the start of the BMP image in memory.
  \param[in] length     Length of the BMP image in memory.

  \see Fl_BMP_Image::Fl_BMP_Image(const char *filename)
  \see Fl_Shared_Image
*/
Fl_BMP_Image::Fl_BMP_Image(const char *imagename, const unsigned char *data, const long length)
: Fl_RGB_Image(0,0,0)
{
  Fl_Image_Reader rdr;
  int retval = (length < 0 ? rdr.open(imagename, data) : rdr.open(imagename, data, (size_t)length));
  if (retval == -1) {
    ld(ERR_FILE_ACCESS);
  } else {
    load_bmp_(rdr);
  }
}

/*
  This macro can be used to check for end of file (EOF) or other read errors.
  In case of an error or EOF an error message is issued and the image loading
  is terminated with error code ERR_FORMAT.
*/
#define CHECK_ERROR \
  if (rdr.error()) { \
    Fl::error("[%d] Fl_BMP_Image: %s - unexpected EOF or read error at offset %ld", \
              __LINE__, rdr.name(), rdr.tell()); \
    ld(ERR_FORMAT); \
    return; \
  }

/*
  This method reads BMP image data and creates an RGB or RGBA image. The BMP
  format supports only 1 bit for alpha. To avoid code duplication, we use
  an Fl_Image_Reader that reads data from either a file or from memory.
*/
void Fl_BMP_Image::load_bmp_(Fl_Image_Reader &rdr, int ico_height, int ico_width)
{
  int   info_size,        // Size of info header
        width,            // Width of image (pixels)
        height,           // Height of image (pixels)
        depth,            // Depth of image (bits)
        bDepth = 3,       // Depth of image (bytes)
        compression,      // Type of compression
        colors_used,      // Number of colors used
        x, y,             // Looping vars
        color,            // Color of RLE pixel
        repcount,         // Number of times to repeat
        temp,             // Temporary color
        align,            // Alignment bytes
        dataSize,         // number of bytes in image data set
        row_order,        // 1 = normal;  -1 = flipped row order
        start_y,          // Beginning Y
        end_y;            // Ending Y
  long  offbits = 0;      // Offset to image data
  uchar bit,              // Bit in image
        byte;             // Byte in image
  uchar *ptr;             // Pointer into pixels
  uchar colormap[256][3]; // Colormap
  uchar havemask;         // Single bit mask follows image data
  int   use_5_6_5;        // Use 5:6:5 for R:G:B channels in 16 bit images

  // Implementation notes: Reader is already open at this point.
  // Use local variables (width, height) until image is complete
  // so we can easily use CHECK_ERROR to return with ld(ERR_FORMAT).
  // We use CHECK_ERROR only at some essential points.

  w(0); h(0); d(0); ld(0);      // make sure these are all zero

  // Get the header...
  if (ico_height < 1) {
    byte = rdr.read_byte();       // Check "BM" sync chars
    bit  = rdr.read_byte();
    if (byte != 'B' || bit != 'M') {
      ld(ERR_FORMAT);
      return;
    }

    rdr.read_dword();             // Skip size
    rdr.read_word();              // Skip reserved stuff
    rdr.read_word();
    offbits = (long)rdr.read_dword();// Read offset to image data
  }

  // Then the bitmap information...
  info_size = rdr.read_dword();
  CHECK_ERROR

  //  printf("offbits = %ld, info_size = %d\n", offbits, info_size);

  havemask  = 0;
  row_order = -1;
  use_5_6_5 = 0;

  if (info_size < 40) {
    // Old Windows/OS2 BMP header...
    width = rdr.read_word();
    height = rdr.read_word();
    rdr.read_word();
    depth = rdr.read_word();
    compression = BI_RGB;
    colors_used = 0;

    repcount = info_size - 12;
  } else {
    if (ico_height > 0 && ico_width > 0) {
      rdr.read_long();
      rdr.read_long();
      width = ico_width;
      height = ico_height;
    } else {
      // New BMP header...
      width = rdr.read_long();
      w(width);
      // If the height is negative, the row order is flipped
      temp = rdr.read_long();
      if (temp < 0) row_order = 1;
      height = abs(temp);
    }

    rdr.read_word();
    depth = rdr.read_word();
    compression = rdr.read_dword();
    dataSize = rdr.read_dword();
    rdr.read_long();
    rdr.read_long();
    colors_used = rdr.read_dword();
    rdr.read_dword();

    repcount = info_size - 40;

    if (!compression && depth >= 8 && width > 32/depth) {
      int Bpp = depth/8;
      int maskSize = (((width*Bpp+3)&~3)*height) + (((((width+7)/8)+3)&~3)*height);
      if (maskSize == 2*dataSize) {
        havemask = 1;
        height = height/2;
        bDepth = 4;
      }
    }
  }
  CHECK_ERROR

  //  printf("width =%4d, height =%4d, depth = %2d, compression = %d, colors_used = %3d, repcount = %2d, row_order = %2d\n",
  //         width, height, depth, compression, colors_used, repcount, row_order);

  // Skip remaining header bytes...
  if (repcount > 0)
    rdr.skip(repcount);
  CHECK_ERROR

  // Check header data...
  if (!width || !height || !depth) {
    ld(ERR_FORMAT);
    return;
  }

  // Get colormap...
  if (colors_used == 0 && depth <= 8)
    colors_used = 1 << depth;

  for (int i = 0; i < colors_used; i++) {
    // Read BGR color...
    colormap[i][0] = rdr.read_byte();
    colormap[i][1] = rdr.read_byte();
    colormap[i][2] = rdr.read_byte();

    // Skip pad byte for new BMP files...
    if (info_size > 12) rdr.read_byte();
  }
  CHECK_ERROR

  // Read first dword of colormap. It tells us if 5:5:5 or 5:6:5 for 16 bit
  if (depth == 16)
    use_5_6_5 = (rdr.read_dword() == 0xf800);

  // Set byte depth for RGBA images
  if (depth == 32)
    bDepth = 4;

  // Setup image and buffers...
  if (offbits) rdr.seek((unsigned int)offbits);
  CHECK_ERROR

  if (((size_t)width) * height * bDepth > max_size() ) {
    Fl::warning("BMP file \"%s\" is too large!\n", rdr.name());
    ld(ERR_FORMAT);
    return;
  }
  array = new uchar[width * height * bDepth];
  alloc_array = 1;

  // Read the image data...
  color = 0;
  repcount = 0;
  align = 0;
  byte  = 0;
  temp  = 0;

  if (row_order < 0) {
    start_y = height - 1;
    end_y   = -1;
  } else {
    start_y = 0;
    end_y   = height;
  }

  for (y = start_y; y != end_y; y += row_order) {
    ptr = (uchar *)array + y * width * bDepth;

    switch (depth)
    {
      case 1 : // Bitmap
        for (x = width, bit = 128; x > 0; x --) {
          if (bit == 128) byte = rdr.read_byte();

          if (byte & bit) {
            *ptr++ = colormap[1][2];
            *ptr++ = colormap[1][1];
            *ptr++ = colormap[1][0];
          } else {
            *ptr++ = colormap[0][2];
            *ptr++ = colormap[0][1];
            *ptr++ = colormap[0][0];
          }

          if (bit > 1)
            bit >>= 1;
          else
            bit = 128;
        }

        // Read remaining bytes to align to 32 bits...
        for (temp = (width + 7) / 8; temp & 3; temp ++) {
          rdr.read_byte();
        }
        break;

      case 4 : // 16-color
        for (x = width, bit = 0xf0; x > 0; x --) {
          // Get a new repcount as needed...
          if (repcount == 0) {
            if (compression != BI_RLE4) {
              repcount = 2;
              color = -1;
            } else {
              while (align > 0) {
                align --;
                rdr.read_byte();
              }

              if ((repcount = rdr.read_byte()) == 0) {
                if ((repcount = rdr.read_byte()) == 0) {
                  // End of line...
                  x ++;
                  continue;
                } else if (repcount == 1) {
                  // End of image...
                  break;
                } else if (repcount == 2) {
                  // Delta...
                  repcount = rdr.read_byte() * rdr.read_byte() * width;
                  color = 0;
                } else {
                  // Absolute...
                  color = -1;
                  align = ((4 - (repcount & 3)) / 2) & 1;
                }
              } else {
                color = rdr.read_byte();
              }
            }
          }

          // Get a new color as needed...
          repcount --;

          // Extract the next pixel...
          if (bit == 0xf0) {
            // Get the next color byte as needed...
            if (color < 0) temp = rdr.read_byte();
            else temp = color;

            // Copy the color value...
            *ptr++ = colormap[(temp >> 4) & 15][2];
            *ptr++ = colormap[(temp >> 4) & 15][1];
            *ptr++ = colormap[(temp >> 4) & 15][0];

            bit  = 0x0f;
          } else {
            bit  = 0xf0;

            // Copy the color value...
            *ptr++ = colormap[temp & 15][2];
            *ptr++ = colormap[temp & 15][1];
            *ptr++ = colormap[temp & 15][0];
          }

        }
        CHECK_ERROR

        if (!compression) {
          // Read remaining bytes to align to 32 bits...
          for (temp = (width + 1) / 2; temp & 3; temp ++) {
            rdr.read_byte();
          }
        }
        break;

      case 8 : // 256-color
        for (x = width; x > 0; x --) {
          // Get a new repcount as needed...
          if (compression != BI_RLE8) {
            repcount = 1;
            color = -1;
          }

          if (repcount == 0) {
            while (align > 0) {
              align --;
              rdr.read_byte();
            }
            CHECK_ERROR

            if ((repcount = rdr.read_byte()) == 0) {
              if ((repcount = rdr.read_byte()) == 0) {
                // End of line...
                x ++;
                continue;
              } else if (repcount == 1) {
                // End of image...
                break;
              } else if (repcount == 2) {
                // Delta...
                repcount = rdr.read_byte() * rdr.read_byte() * width;
                color = 0;
              } else {
                // Absolute...
                color = -1;
                align = (2 - (repcount & 1)) & 1;
              }
            } else {
              color = rdr.read_byte();
            }
          }
          CHECK_ERROR

          // Get a new color as needed...
          if (color < 0) temp = rdr.read_byte();
          else temp = color;

          repcount --;

          // Copy the color value...
          *ptr++ = colormap[temp][2];
          *ptr++ = colormap[temp][1];
          *ptr++ = colormap[temp][0];
          if (havemask) ptr++;
        }

        if (!compression) {
          // Read remaining bytes to align to 32 bits...
          for (temp = width; temp & 3; temp ++) {
            rdr.read_byte();
          }
        }
        break;

      case 16 : // 16-bit 5:5:5 or 5:6:5 RGB
        for (x = width; x > 0; x --, ptr += bDepth) {
          uchar b = rdr.read_byte(), a = rdr.read_byte() ;
          if (use_5_6_5) {
            ptr[2] = (uchar)(( b << 3 ) & 0xf8);
            ptr[1] = (uchar)(((a << 5) & 0xe0) | ((b >> 3) & 0x1c));
            ptr[0] = (uchar)(a & 0xf8);
          } else {
            ptr[2] = (uchar)((b << 3) & 0xf8);
            ptr[1] = (uchar)(((a << 6) & 0xc0) | ((b >> 2) & 0x38));
            ptr[0] = (uchar)((a<<1) & 0xf8);
          }
        }

        // Read remaining bytes to align to 32 bits...
        for (temp = width * 2; temp & 3; temp ++) {
          rdr.read_byte();
        }
        break;

      case 24 : // 24-bit RGB
        for (x = width; x > 0; x --, ptr += bDepth) {
          ptr[2] = rdr.read_byte();
          ptr[1] = rdr.read_byte();
          ptr[0] = rdr.read_byte();
        }

        // Read remaining bytes to align to 32 bits...
        for (temp = width * 3; temp & 3; temp ++) {
          rdr.read_byte();
        }
        break;

      case 32 : // 32-bit RGBA
        for (x = width; x > 0; x --, ptr += bDepth) {
          ptr[2] = rdr.read_byte();
          ptr[1] = rdr.read_byte();
          ptr[0] = rdr.read_byte();
          ptr[3] = rdr.read_byte();
        }
        break;
    }
    CHECK_ERROR
  }

  if (havemask) {
    for (y = height - 1; y >= 0; y --) {
      ptr = (uchar *)array + y * width * bDepth + 3;
      for (x = width, bit = 128; x > 0; x --, ptr += bDepth) {
        if (bit == 128) byte = rdr.read_byte();
        if (byte & bit)
          *ptr = 0;
        else
          *ptr = 255;
        if (bit > 1)
          bit >>= 1;
        else
          bit = 128;
      }
      // Read remaining bytes to align to 32 bits...
      for (temp = (width + 7) / 8; temp & 3; temp ++)
        rdr.read_byte();
    }
  }

  CHECK_ERROR

  // Success: set image attributes and return
  // File is closed when returning...

  w(width);
  h(height);
  d(bDepth);
  ld(0);

} // load_bmp_()
