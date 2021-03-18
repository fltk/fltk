//
// "$Id$"
//
// Fl_GIF_Image routines.
//
// Copyright 1997-2019 by Bill Spitzak and others.
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
//

//
// Reference: GIF89a Specification (links valid as of Jan 05, 2019):
//
// "GRAPHICS INTERCHANGE FORMAT(sm), Version 89a" (authoritative):
// https://www.w3.org/Graphics/GIF/spec-gif89a.txt
//
// HTML version (non-authoritative):
// https://web.archive.org/web/20160304075538/http://qalle.net/gif89a.php
//

//
// Include necessary header files...
//

#include <FL/Fl.H>
#include <FL/Fl_GIF_Image.H>
#include <stdio.h>
#include <stdlib.h>
#include <FL/fl_utf8.h>
#include "flstring.h"

// Read a .gif file and convert it to a "xpm" format (actually my
// modified one with compressed colormaps).

// Extensively modified from original code for gif2ras by
// Patrick J. Naughton of Sun Microsystems.  The original
// copyright notice follows:

/* gif2ras.c - Converts from a Compuserve GIF (tm) image to a Sun Raster image.
 *
 * Copyright (c) 1988 by Patrick J. Naughton
 *
 * Author: Patrick J. Naughton
 * naughton@wind.sun.com
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 * Comments and additions should be sent to the author:
 *
 *                     Patrick J. Naughton
 *                     Sun Microsystems, Inc.
 *                     2550 Garcia Ave, MS 14-40
 *                     Mountain View, CA 94043
 *                     (415) 336-1080
 */

typedef unsigned char uchar;

#define NEXTBYTE (uchar)getc(GifFile)
#define GETSHORT(var) var = NEXTBYTE; var += NEXTBYTE << 8

/**
 The constructor loads the named GIF image.

 The destructor frees all memory and server resources that are used by
 the image.

 Use Fl_Image::fail() to check if Fl_GIF_Image failed to load. fail() returns
 ERR_FILE_ACCESS if the file could not be opened or read, ERR_FORMAT if the
 GIF format could not be decoded, and ERR_NO_IMAGE if the image could not
 be loaded for another reason.
 */
Fl_GIF_Image::Fl_GIF_Image(const char *infname) : Fl_Pixmap((char *const*)0) {
  FILE *GifFile;	// File to read
  char **new_data;	// Data array

  if ((GifFile = fl_fopen(infname, "rb")) == NULL) {
    Fl::error("Fl_GIF_Image: Unable to open %s!", infname);
    ld(ERR_FILE_ACCESS);
    return;
  }

  {char b[6];
  if (fread(b,1,6,GifFile)<6) {
    fclose(GifFile);
    ld(ERR_FILE_ACCESS);
    return; /* quit on eof */
  }
  if (b[0]!='G' || b[1]!='I' || b[2] != 'F') {
    fclose(GifFile);
    Fl::error("Fl_GIF_Image: %s is not a GIF file.\n", infname);
    ld(ERR_FORMAT);
    return;
  }
  if (b[3]!='8' || b[4]>'9' || b[5]!= 'a')
    Fl::warning("%s is version %c%c%c.",infname,b[3],b[4],b[5]);
  }

  int Width; GETSHORT(Width);
  int Height; GETSHORT(Height);

  uchar ch = NEXTBYTE;
  char HasColormap = ((ch & 0x80) != 0);
  int BitsPerPixel = (ch & 7) + 1;
  int ColorMapSize;
  if (HasColormap) {
    ColorMapSize = 2 << (ch & 7);
  } else {
    ColorMapSize = 0;
  }
  // int OriginalResolution = ((ch>>4)&7)+1;
  // int SortedTable = (ch&8)!=0;
  ch = NEXTBYTE; // Background Color index
  ch = NEXTBYTE; // Aspect ratio is N/64

  // Read in global colormap:
  uchar transparent_pixel = 0;
  char has_transparent = 0;
  uchar Red[256], Green[256], Blue[256]; /* color map */
  if (HasColormap) {
    for (int i=0; i < ColorMapSize; i++) {	
      Red[i] = NEXTBYTE;
      Green[i] = NEXTBYTE;
      Blue[i] = NEXTBYTE;
    }
  }

  int CodeSize;		/* Code size, init from GIF header, increases... */
  char Interlace;

  for (;;) {

    int i = NEXTBYTE;
    if (i<0) {
      fclose(GifFile);
      Fl::error("Fl_GIF_Image: %s - unexpected EOF",infname); 
      w(0); h(0); d(0); ld(ERR_FORMAT);
      return;
    }
    int blocklen;

    //  if (i == 0x3B) return 0;  eof code

    if (i == 0x21) {		// a "gif extension"

      ch = NEXTBYTE;
      blocklen = NEXTBYTE;

      if (ch==0xF9 && blocklen==4) { // Netscape animation extension

	char bits;
	bits = NEXTBYTE;
	getc(GifFile); getc(GifFile); // GETSHORT(delay);
	transparent_pixel = NEXTBYTE;
	if (bits & 1) has_transparent = 1;
	blocklen = NEXTBYTE;

      } else if (ch == 0xFF) { // Netscape repeat count
	;

      } else if (ch != 0xFE) { //Gif Comment
	Fl::warning("%s: unknown gif extension 0x%02x.", infname, ch);
      }
    } else if (i == 0x2c) {	// an image

      ch = NEXTBYTE; ch = NEXTBYTE; // GETSHORT(x_position);
      ch = NEXTBYTE; ch = NEXTBYTE; // GETSHORT(y_position);
      GETSHORT(Width);
      GETSHORT(Height);
      ch = NEXTBYTE;
      Interlace = ((ch & 0x40) != 0);
      if (ch & 0x80) { // image has local color table
	BitsPerPixel = (ch & 7) + 1;
	ColorMapSize = 2 << (ch & 7);
	for (i=0; i < ColorMapSize; i++) {
	  Red[i] = NEXTBYTE;
	  Green[i] = NEXTBYTE;
	  Blue[i] = NEXTBYTE;
	}
      }
      CodeSize = NEXTBYTE+1;
      break; // okay, this is the image we want
    } else {
      Fl::warning("%s: unknown gif code 0x%02x", infname, i);
      blocklen = 0;
    }

    // skip the data:
    while (blocklen>0) {while (blocklen--) {ch = NEXTBYTE;} blocklen=NEXTBYTE;}
  }

  if (BitsPerPixel >= CodeSize)
  {
    // Workaround for broken GIF files...
    BitsPerPixel = CodeSize - 1;
    ColorMapSize = 1 << BitsPerPixel;
  }

  // Fix images w/o color table. The standard allows this and lets the
  // decoder choose a default color table. The standard recommends the
  // first two color table entries should be black and white.

  if (ColorMapSize == 0) { // no global and no local color table
    Fl::warning("%s does not have a color table, using default.\n", infname);
    BitsPerPixel = CodeSize - 1;
    ColorMapSize = 1 << BitsPerPixel;
    Red[0] = Green[0] = Blue[0] = 0;	// black
    Red[1] = Green[1] = Blue[1] = 255;	// white
    for (int i = 2; i < ColorMapSize; i++) {
      Red[i] = Green[i] = Blue[i] = (uchar)(255 * i / (ColorMapSize - 1));
    }
#if (0)
    // fill color table to maximum size
    for (int i = ColorMapSize; i < 256; i++) {
      Red[i] = Green[i] = Blue[i] = 0; // black
    }
#endif
  }

  uchar *Image = new uchar[Width*Height];

  int YC = 0, Pass = 0; /* Used to de-interlace the picture */
  uchar *p = Image;
  uchar *eol = p+Width;

  int InitCodeSize = CodeSize;
  int ClearCode = (1 << (CodeSize-1));
  int EOFCode = ClearCode + 1;
  int FirstFree = ClearCode + 2;
  int FinChar = 0;
  int ReadMask = (1<<CodeSize) - 1;
  int FreeCode = FirstFree;
  int OldCode = ClearCode;

  // tables used by LZW decompresser:
  short int Prefix[4096];
  uchar Suffix[4096];

  int blocklen = NEXTBYTE;
  uchar thisbyte = NEXTBYTE; blocklen--;
  int frombit = 0;

  for (;;) {

/* Fetch the next code from the raster data stream.  The codes can be
 * any length from 3 to 12 bits, packed into 8-bit bytes, so we have to
 * maintain our location as a pointer and a bit offset.
 * In addition, gif adds totally useless and annoying block counts
 * that must be correctly skipped over. */
    int CurCode = thisbyte;
    if (frombit+CodeSize > 7) {
      if (blocklen <= 0) {
	blocklen = NEXTBYTE;
	if (blocklen <= 0) break;
      }
      thisbyte = NEXTBYTE; blocklen--;
      CurCode |= thisbyte<<8;
    }
    if (frombit+CodeSize > 15) {
      if (blocklen <= 0) {
	blocklen = NEXTBYTE;
	if (blocklen <= 0) break;
      }
      thisbyte = NEXTBYTE; blocklen--;
      CurCode |= thisbyte<<16;
    }
    CurCode = (CurCode>>frombit)&ReadMask;
    frombit = (frombit+CodeSize)%8;

    if (CurCode == ClearCode) {
      CodeSize = InitCodeSize;
      ReadMask = (1<<CodeSize) - 1;
      FreeCode = FirstFree;
      OldCode = ClearCode;
      continue;
    }

    if (CurCode == EOFCode) break;

    uchar OutCode[4097]; // temporary array for reversing codes
    uchar *tp = OutCode;
    int i;
    if (CurCode < FreeCode) i = CurCode;
    else if (CurCode == FreeCode) {*tp++ = (uchar)FinChar; i = OldCode;}
    else {Fl::error("Fl_GIF_Image: %s - LZW Barf!", infname); break;}

    while (i >= ColorMapSize) {*tp++ = Suffix[i]; i = Prefix[i];}
    *tp++ = FinChar = i;
    do {
      *p++ = *--tp;
      if (p >= eol) {
	if (!Interlace) YC++;
	else switch (Pass) {
	case 0: YC += 8; if (YC >= Height) {Pass++; YC = 4;} break;
	case 1: YC += 8; if (YC >= Height) {Pass++; YC = 2;} break;
	case 2: YC += 4; if (YC >= Height) {Pass++; YC = 1;} break;
	case 3: YC += 2; break;
	}
	if (YC>=Height) YC=0; /* cheap bug fix when excess data */
	p = Image + YC*Width;
	eol = p+Width;
      }
    } while (tp > OutCode);

    if (OldCode != ClearCode) {
      Prefix[FreeCode] = (short)OldCode;
      Suffix[FreeCode] = FinChar;
      FreeCode++;
      if (FreeCode > ReadMask) {
	if (CodeSize < 12) {
	  CodeSize++;
	  ReadMask = (1 << CodeSize) - 1;
	}
	else FreeCode--;
      }
    }
    OldCode = CurCode;
  }

  // We are done reading the file, now convert to xpm:

  // allocate line pointer arrays:
  w(Width);
  h(Height);
  d(1);
  new_data = new char*[Height+2];

  // transparent pixel must be zero, swap if it isn't:
  if (has_transparent && transparent_pixel != 0) {
    // swap transparent pixel with zero
    p = Image+Width*Height;
    while (p-- > Image) {
      if (*p==transparent_pixel) *p = 0;
      else if (!*p) *p = transparent_pixel;
    }
    uchar t;
    t                        = Red[0];
    Red[0]                   = Red[transparent_pixel];
    Red[transparent_pixel]   = t;

    t                        = Green[0];
    Green[0]                 = Green[transparent_pixel];
    Green[transparent_pixel] = t;

    t                        = Blue[0];
    Blue[0]                  = Blue[transparent_pixel];
    Blue[transparent_pixel]  = t;
  }

  // find out what colors are actually used:
  uchar used[256]; uchar remap[256];
  int i;
  for (i = 0; i < ColorMapSize; i++) used[i] = 0;
  p = Image+Width*Height;
  while (p-- > Image) used[*p] = 1;

  // remap them to start with printing characters:
  int base = has_transparent && used[0] ? ' ' : ' '+1;
  int numcolors = 0;
  for (i = 0; i < ColorMapSize; i++) if (used[i]) {
    remap[i] = (uchar)(base++);
    numcolors++;
  }

  // write the first line of xpm data (use suffix as temp array):
  int length = sprintf((char*)(Suffix),
		       "%d %d %d %d",Width,Height,-numcolors,1);
  new_data[0] = new char[length+1];
  strcpy(new_data[0], (char*)Suffix);

  // write the colormap
  new_data[1] = (char*)(p = new uchar[4*numcolors]);
  for (i = 0; i < ColorMapSize; i++) if (used[i]) {
    *p++ = remap[i];
    *p++ = Red[i];
    *p++ = Green[i];
    *p++ = Blue[i];
  }

  // remap the image data:
  p = Image+Width*Height;
  while (p-- > Image) *p = remap[*p];

  // split the image data into lines:
  for (i=0; i<Height; i++) {
    new_data[i+2] = new char[Width+1];
    memcpy(new_data[i + 2], (char*)(Image + i*Width), Width);
    new_data[i + 2][Width] = 0;
  }

  data((const char **)new_data, Height + 2);
  alloc_data = 1;

  delete[] Image;

  fclose(GifFile);
}


//
// End of "$Id$".
//
