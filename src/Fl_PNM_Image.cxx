//
// "$Id$"
//
// Fl_PNM_Image routines.
//
// Copyright 1997-2009 by Easy Software Products.
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
//   Fl_PNM_Image::Fl_PNM_Image() - Load a PNM image...
//

//
// Include necessary header files...
//

#include <FL/Fl.H>
#include <FL/Fl_PNM_Image.H>
#include <stdio.h>
#include <stdlib.h>
#include <FL/fl_utf8.h>
#include "flstring.h"


//
// 'Fl_PNM_Image::Fl_PNM_Image()' - Load a PNM image...
//

/** 
  The constructor loads the named PNM image. 
  <P>The inherited destructor free all memory and server resources that are used by the image.
*/

Fl_PNM_Image::Fl_PNM_Image(const char *name)	// I - File to read
  : Fl_RGB_Image(0,0,0) {
  FILE		*fp;		// File pointer
  int		x, y;		// Looping vars
  char		line[1024],	// Input line
		*lineptr;	// Pointer in line
  uchar		*ptr,		// Pointer to pixel values
		byte,		// Byte from file
		bit;		// Bit in pixel
  int		format,		// Format of PNM file
		val,		// Pixel value
		maxval;		// Maximum pixel value


  if ((fp = fl_fopen(name, "rb")) == NULL) return;

  //
  // Read the file header in the format:
  //
  //   Pformat
  //   # comment1
  //   # comment2
  //   ...
  //   # commentN
  //   width
  //   height
  //   max sample
  //

  lineptr = fgets(line, sizeof(line), fp);
  if (!lineptr) {
    fclose(fp);
    Fl::error("Early end-of-file in PNM file \"%s\"!", name);
    return;
  }

  lineptr ++;

  format = atoi(lineptr);
  while (isdigit(*lineptr)) lineptr ++;

  if (format == 7) lineptr = (char *)"";

  while (lineptr != NULL && w() == 0) {
    if (*lineptr == '\0' || *lineptr == '#') {
      lineptr = fgets(line, sizeof(line), fp);
    } else if (isdigit(*lineptr)) {
      w(strtol(lineptr, &lineptr, 10));
    } else lineptr ++;
  }

  while (lineptr != NULL && h() == 0) {
    if (*lineptr == '\0' || *lineptr == '#') {
      lineptr = fgets(line, sizeof(line), fp);
    } else if (isdigit(*lineptr)) {
      h(strtol(lineptr, &lineptr, 10));
    } else lineptr ++;
  }

  if (format != 1 && format != 4) {
    maxval = 0;

    while (lineptr != NULL && maxval == 0) {
      if (*lineptr == '\0' || *lineptr == '#') {
	lineptr = fgets(line, sizeof(line), fp);
      } else if (isdigit(*lineptr)) {
	maxval = strtol(lineptr, &lineptr, 10);
      } else lineptr ++;
    }
  } else maxval = 1;

  // Allocate memory...
  if (format == 1 || format == 2 || format == 4 || format == 5) d(1);
  else d(3);

//  printf("%s = %dx%dx%d\n", name, w(), h(), d());

  array       = new uchar[w() * h() * d()];
  alloc_array = 1;

  // Read the image file...
  for (y = 0; y < h(); y ++) {
    ptr = (uchar *)array + y * w() * d();

    switch (format) {
      case 1 :
      case 2 :
          for (x = w(); x > 0; x --)
            if (fscanf(fp, "%d", &val) == 1) *ptr++ = (uchar)(255 * val / maxval);
          break;

      case 3 :
          for (x = w(); x > 0; x --) {
            if (fscanf(fp, "%d", &val) == 1) *ptr++ = (uchar)(255 * val / maxval);
            if (fscanf(fp, "%d", &val) == 1) *ptr++ = (uchar)(255 * val / maxval);
            if (fscanf(fp, "%d", &val) == 1) *ptr++ = (uchar)(255 * val / maxval);
          }
          break;

      case 4 :
          for (x = w(), byte = (uchar)getc(fp), bit = 128; x > 0; x --) {
	    if (byte & bit) *ptr++ = 255;
	    else *ptr++ = 0;

            if (bit > 1) bit >>= 1;
            else {
              bit  = 128;
              byte = (uchar)getc(fp);
            }
          }
          break;
          
      case 5 :
      case 6 :
        if (maxval < 256) {
          fread(ptr, w(), d(), fp);
        } else {
          for (x = d() * w(); x > 0; x --) {
            val = (uchar)getc(fp);
            val = (val<<8)|(uchar)getc(fp);
            *ptr++ = (255*val)/maxval;
          }
        }
        break;
        
      case 7 : /* XV 3:3:2 thumbnail format */
        for (x = w(); x > 0; x --) {
          byte = (uchar)getc(fp);
          
          *ptr++ = (uchar)(255 * ((byte >> 5) & 7) / 7);
          *ptr++ = (uchar)(255 * ((byte >> 2) & 7) / 7);
          *ptr++ = (uchar)(255 * (byte & 3) / 3);
        }
        break;
    }
  }

  fclose(fp);
}


//
// End of "$Id$".
//
