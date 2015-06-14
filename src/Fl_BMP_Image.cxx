//
// "$Id$"
//
// Fl_BMP_Image routines.
//
// Copyright 1997-2010 by Easy Software Products.
// Image support by Matthias Melcher, Copyright 2000-2009.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//
// Contents:
//
//   Fl_BMP_Image::Fl_BMP_Image() - Load a BMP image file.
//

//
// Include necessary header files...
//

#include <FL/Fl_BMP_Image.H>
#include <FL/fl_utf8.h>
#include <FL/Fl.H>
#include <config.h>
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


//
// Local functions...
//

static int		read_long(FILE *fp);
static unsigned short	read_word(FILE *fp);
static unsigned int	read_dword(FILE *fp);


/**
 The constructor loads the named BMP image from the given bmp filename.

 The destructor frees all memory and server resources that are used by
 the image.

 Use Fl_Image::fail() to check if Fl_BMP_Image failed to load. fail() returns
 ERR_FILE_ACCESS if the file could not be opened or read, ERR_FORMAT if the
 BMP format could not be decoded, and ERR_NO_IMAGE if the image could not
 be loaded for another reason.
 */
Fl_BMP_Image::Fl_BMP_Image(const char *bmp) // I - File to read
  : Fl_RGB_Image(0,0,0) {
  FILE		*fp;		// File pointer
  int		info_size,	// Size of info header
		depth,		// Depth of image (bits)
		bDepth = 3,	// Depth of image (bytes)
		compression,	// Type of compression
		colors_used,	// Number of colors used
		x, y,		// Looping vars
		color,		// Color of RLE pixel
		repcount,	// Number of times to repeat
		temp,		// Temporary color
		align,		// Alignment bytes
		dataSize,	// number of bytes in image data set
		row_order,	// 1 = normal;  -1 = flipped row order
		start_y,	// Beginning Y
		end_y;		// Ending Y
  long		offbits;	// Offset to image data
  uchar		bit,		// Bit in image
		byte;		// Byte in image
  uchar		*ptr;		// Pointer into pixels
  uchar		colormap[256][3];// Colormap
  uchar		havemask;	// Single bit mask follows image data
  int		use_5_6_5;	// Use 5:6:5 for R:G:B channels in 16 bit images


  // Open the file...
  if ((fp = fl_fopen(bmp, "rb")) == NULL) {
    ld(ERR_FILE_ACCESS);
    return;
  }

  // Get the header...
  byte = (uchar)getc(fp);	// Check "BM" sync chars
  bit  = (uchar)getc(fp);
  if (byte != 'B' || bit != 'M') {
    fclose(fp);
    ld(ERR_FORMAT);
    return;
  }

  read_dword(fp);		// Skip size
  read_word(fp);		// Skip reserved stuff
  read_word(fp);
  offbits = (long)read_dword(fp);// Read offset to image data

  // Then the bitmap information...
  info_size = read_dword(fp);

//  printf("offbits = %ld, info_size = %d\n", offbits, info_size);

  havemask  = 0;
  row_order = -1;
  use_5_6_5 = 0;

  if (info_size < 40) {
    // Old Windows/OS2 BMP header...
    w(read_word(fp));
    h(read_word(fp));
    read_word(fp);
    depth = read_word(fp);
    compression = BI_RGB;
    colors_used = 0;

    repcount = info_size - 12;
  } else {
    // New BMP header...
    w(read_long(fp));
    // If the height is negative, the row order is flipped
    temp = read_long(fp);
    if (temp < 0) row_order = 1;
    h(abs(temp));
    read_word(fp);
    depth = read_word(fp);
    compression = read_dword(fp);
    dataSize = read_dword(fp);
    read_long(fp);
    read_long(fp);
    colors_used = read_dword(fp);
    read_dword(fp);

    repcount = info_size - 40;

    if (!compression && depth>=8 && w()>32/depth) {
      int Bpp = depth/8;
      int maskSize = (((w()*Bpp+3)&~3)*h()) + (((((w()+7)/8)+3)&~3)*h());
      if (maskSize==2*dataSize) {
        havemask = 1;
	h(h()/2);
	bDepth = 4;
      }
    }
  }

//  printf("w() = %d, h() = %d, depth = %d, compression = %d, colors_used = %d, repcount = %d\n",
//         w(), h(), depth, compression, colors_used, repcount);

  // Skip remaining header bytes...
  while (repcount > 0) {
    getc(fp);
    repcount --;
  }

  // Check header data...
  if (!w() || !h() || !depth) {
    fclose(fp);
    w(0); h(0); d(0); ld(ERR_FORMAT);
    return;
  }

  // Get colormap...
  if (colors_used == 0 && depth <= 8)
    colors_used = 1 << depth;

  for (repcount = 0; repcount < colors_used; repcount ++) {
    // Read BGR color...
    if (fread(colormap[repcount], 1, 3, fp)==0) { /* ignore */ }

    // Skip pad byte for new BMP files...
    if (info_size > 12) getc(fp);
  }

  // Read first dword of colormap. It tells us if 5:5:5 or 5:6:5 for 16 bit
  if (depth == 16)
    use_5_6_5 = (read_dword(fp) == 0xf800);

  // Set byte depth for RGBA images
  if (depth == 32)
    bDepth=4;

  // Setup image and buffers...
  d(bDepth);
  if (offbits) fseek(fp, offbits, SEEK_SET);

  if (((size_t)w()) * h() * d() > max_size() ) {
    Fl::warning("BMP file \"%s\" is too large!\n", bmp);
    fclose(fp);
    w(0); h(0); d(0); ld(ERR_FORMAT);
    return;
  }
  array = new uchar[w() * h() * d()];
  alloc_array = 1;

  // Read the image data...
  color = 0;
  repcount = 0;
  align = 0;
  byte  = 0;
  temp  = 0;

  if (row_order < 0) {
    start_y = h() - 1;
    end_y   = -1;
  } else {
    start_y = 0;
    end_y   = h();
  }

  for (y = start_y; y != end_y; y += row_order) {
    ptr = (uchar *)array + y * w() * d();

    switch (depth)
    {
      case 1 : // Bitmap
          for (x = w(), bit = 128; x > 0; x --) {
	    if (bit == 128) byte = (uchar)getc(fp);

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
	  for (temp = (w() + 7) / 8; temp & 3; temp ++) {
	    getc(fp);
	  }
          break;

      case 4 : // 16-color
          for (x = w(), bit = 0xf0; x > 0; x --) {
	    // Get a new repcount as needed...
	    if (repcount == 0) {
              if (compression != BI_RLE4) {
		repcount = 2;
		color = -1;
              } else {
		while (align > 0) {
	          align --;
		  getc(fp);
        	}

		if ((repcount = getc(fp)) == 0) {
		  if ((repcount = getc(fp)) == 0) {
		    // End of line...
                    x ++;
		    continue;
		  } else if (repcount == 1) {
                    // End of image...
		    break;
		  } else if (repcount == 2) {
		    // Delta...
		    repcount = getc(fp) * getc(fp) * w();
		    color = 0;
		  } else {
		    // Absolute...
		    color = -1;
		    align = ((4 - (repcount & 3)) / 2) & 1;
		  }
		} else {
	          color = getc(fp);
		}
	      }
	    }

            // Get a new color as needed...
	    repcount --;

	    // Extract the next pixel...
            if (bit == 0xf0) {
	      // Get the next color byte as needed...
              if (color < 0) temp = getc(fp);
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

	  if (!compression) {
            // Read remaining bytes to align to 32 bits...
	    for (temp = (w() + 1) / 2; temp & 3; temp ++) {
	      getc(fp);
	    }
	  }
          break;

      case 8 : // 256-color
          for (x = w(); x > 0; x --) {
	    // Get a new repcount as needed...
            if (compression != BI_RLE8) {
	      repcount = 1;
	      color = -1;
            }

	    if (repcount == 0) {
	      while (align > 0) {
	        align --;
		getc(fp);
              }

	      if ((repcount = getc(fp)) == 0) {
		if ((repcount = getc(fp)) == 0) {
		  // End of line...
                  x ++;
		  continue;
		} else if (repcount == 1) {
		  // End of image...
		  break;
		} else if (repcount == 2) {
		  // Delta...
		  repcount = getc(fp) * getc(fp) * w();
		  color = 0;
		} else {
		  // Absolute...
		  color = -1;
		  align = (2 - (repcount & 1)) & 1;
		}
	      } else {
	        color = getc(fp);
              }
            }

            // Get a new color as needed...
            if (color < 0) temp = getc(fp);
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
	    for (temp = w(); temp & 3; temp ++) {
	      getc(fp);
	    }
	  }
          break;

      case 16 : // 16-bit 5:5:5 or 5:6:5 RGB
          for (x = w(); x > 0; x --, ptr += bDepth) {
	    uchar b = getc(fp), a = getc(fp) ;
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
	  for (temp = w() * 2; temp & 3; temp ++) {
	    getc(fp);
	  }
          break;

      case 24 : // 24-bit RGB
          for (x = w(); x > 0; x --, ptr += bDepth) {
	    ptr[2] = (uchar)getc(fp);
	    ptr[1] = (uchar)getc(fp);
	    ptr[0] = (uchar)getc(fp);
	  }

          // Read remaining bytes to align to 32 bits...
	  for (temp = w() * 3; temp & 3; temp ++) {
	    getc(fp);
	  }
          break;
		  
      case 32 : // 32-bit RGBA
         for (x = w(); x > 0; x --, ptr += bDepth) {
            ptr[2] = (uchar)getc(fp);
            ptr[1] = (uchar)getc(fp);
            ptr[0] = (uchar)getc(fp);
            ptr[3] = (uchar)getc(fp);
          }
          break;
    }
  }
  
  if (havemask) {
    for (y = h() - 1; y >= 0; y --) {
      ptr = (uchar *)array + y * w() * d() + 3;
      for (x = w(), bit = 128; x > 0; x --, ptr+=bDepth) {
	if (bit == 128) byte = (uchar)getc(fp);
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
      for (temp = (w() + 7) / 8; temp & 3; temp ++)
	getc(fp);
    }
  }

  // Close the file and return...
  fclose(fp);
}


//
// 'read_word()' - Read a 16-bit unsigned integer.
//

static unsigned short	// O - 16-bit unsigned integer
read_word(FILE *fp) {	// I - File to read from
  unsigned char b0, b1;	// Bytes from file

  b0 = (uchar)getc(fp);
  b1 = (uchar)getc(fp);

  return ((b1 << 8) | b0);
}


//
// 'read_dword()' - Read a 32-bit unsigned integer.
//

static unsigned int		// O - 32-bit unsigned integer
read_dword(FILE *fp) {		// I - File to read from
  unsigned char b0, b1, b2, b3;	// Bytes from file

  b0 = (uchar)getc(fp);
  b1 = (uchar)getc(fp);
  b2 = (uchar)getc(fp);
  b3 = (uchar)getc(fp);

  return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}


//
// 'read_long()' - Read a 32-bit signed integer.
//

static int			// O - 32-bit signed integer
read_long(FILE *fp) {		// I - File to read from
  unsigned char b0, b1, b2, b3;	// Bytes from file

  b0 = (uchar)getc(fp);
  b1 = (uchar)getc(fp);
  b2 = (uchar)getc(fp);
  b3 = (uchar)getc(fp);

  return ((int)(((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}


//
// End of "$Id$".
//
