//
// Fl_GIF_Image routines.
//
// Copyright 1997-2020 by Bill Spitzak and others.
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
#include "Fl_Image_Reader.h"
#include <FL/fl_utf8.h>
#include "flstring.h"

#include <stdio.h>
#include <stdlib.h>

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


/**
 \brief The constructor loads the named GIF image.

 IF a GIF is animated, Fl_GIF_Image will only read and display the first frame
 of the animation.

 The destructor frees all memory and server resources that are used by
 the image.

 Use Fl_Image::fail() to check if Fl_GIF_Image failed to load. fail() returns
 ERR_FILE_ACCESS if the file could not be opened or read, ERR_FORMAT if the
 GIF format could not be decoded, and ERR_NO_IMAGE if the image could not
 be loaded for another reason.

 \param[in] filename a full path and name pointing to a valid GIF file.

 \see Fl_GIF_Image::Fl_GIF_Image(const char *imagename, const unsigned char *data)
 */
Fl_GIF_Image::Fl_GIF_Image(const char *filename) :
  Fl_Pixmap((char *const*)0)
{
  Fl_Image_Reader rdr;
  if (rdr.open(filename) == -1) {
    Fl::error("Fl_GIF_Image: Unable to open %s!", filename);
    ld(ERR_FILE_ACCESS);
  } else {
    load_gif_(rdr);
  }
}


/**
 \brief The constructor loads a GIF image from memory.

 Construct an image from a block of memory inside the application. Fluid offers
 "binary Data" chunks as a great way to add image data into the C++ source code.
 imagename can be NULL. If a name is given, the image is added to the list of
 shared images and will be available by that name.

 IF a GIF is animated, Fl_GIF_Image will only read and display the first frame
 of the animation.

 Use Fl_Image::fail() to check if Fl_GIF_Image failed to load. fail() returns
 ERR_FILE_ACCESS if the file could not be opened or read, ERR_FORMAT if the
 GIF format could not be decoded, and ERR_NO_IMAGE if the image could not
 be loaded for another reason.

 \param[in] imagename  A name given to this image or NULL
 \param[in] data       Pointer to the start of the GIF image in memory. This code will not check for buffer overruns.

 \see Fl_GIF_Image::Fl_GIF_Image(const char *filename)
 \see Fl_Shared_Image
*/
Fl_GIF_Image::Fl_GIF_Image(const char *imagename, const unsigned char *data) :
  Fl_Pixmap((char *const*)0)
{
  Fl_Image_Reader rdr;
  if (rdr.open(imagename, data)==-1) {
    ld(ERR_FILE_ACCESS);
  } else {
    load_gif_(rdr);
  }
}

/*
 This method reads GIF image data and creates an RGB or RGBA image. The GIF
 format supports only 1 bit for alpha. To avoid code duplication, we use
 an Fl_Image_Reader that reads data from either a file or from memory.
*/
void Fl_GIF_Image::load_gif_(Fl_Image_Reader &rdr)
{
  char **new_data;      // Data array

  {char b[6] = { 0 };
    for (int i=0; i<6; ++i) b[i] = rdr.read_byte();
    if (b[0]!='G' || b[1]!='I' || b[2] != 'F') {
      Fl::error("Fl_GIF_Image: %s is not a GIF file.\n", rdr.name());
      ld(ERR_FORMAT);
      return;
    }
    if (b[3]!='8' || b[4]>'9' || b[5]!= 'a')
      Fl::warning("%s is version %c%c%c.",rdr.name(),b[3],b[4],b[5]);
  }

  int Width = rdr.read_word();
  int Height = rdr.read_word();

  uchar ch = rdr.read_byte();
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
  ch = rdr.read_byte(); // Background Color index
  ch = rdr.read_byte(); // Aspect ratio is N/64

  // Read in global colormap:
  uchar transparent_pixel = 0;
  char has_transparent = 0;
  uchar Red[256], Green[256], Blue[256]; /* color map */
  if (HasColormap) {
    for (int i=0; i < ColorMapSize; i++) {
      Red[i] = rdr.read_byte();
      Green[i] = rdr.read_byte();
      Blue[i] = rdr.read_byte();
    }
  }

  int CodeSize;         /* Code size, init from GIF header, increases... */
  char Interlace;

  for (;;) {

    int i = rdr.read_byte();
    if (i<0) {
      Fl::error("Fl_GIF_Image: %s - unexpected EOF", rdr.name());
      w(0); h(0); d(0); ld(ERR_FORMAT);
      return;
    }
    int blocklen;

    //  if (i == 0x3B) return 0;  eof code

    if (i == 0x21) {            // a "gif extension"

      ch = rdr.read_byte();
      blocklen = rdr.read_byte();

      if (ch==0xF9 && blocklen==4) { // Netscape animation extension

        char bits;
        bits = rdr.read_byte();
        rdr.read_word(); // GETSHORT(delay);
        transparent_pixel = rdr.read_byte();
        if (bits & 1) has_transparent = 1;
        blocklen = rdr.read_byte();

      } else if (ch == 0xFF) { // Netscape repeat count
        ;

      } else if (ch != 0xFE) { //Gif Comment
        Fl::warning("%s: unknown gif extension 0x%02x.", rdr.name(), ch);
      }
    } else if (i == 0x2c) {     // an image

      ch = rdr.read_byte(); ch = rdr.read_byte(); // GETSHORT(x_position);
      ch = rdr.read_byte(); ch = rdr.read_byte(); // GETSHORT(y_position);
      Width = rdr.read_word();
      Height = rdr.read_word();
      ch = rdr.read_byte();
      Interlace = ((ch & 0x40) != 0);
      if (ch & 0x80) { // image has local color table
        BitsPerPixel = (ch & 7) + 1;
        ColorMapSize = 2 << (ch & 7);
        for (i=0; i < ColorMapSize; i++) {
          Red[i] = rdr.read_byte();
          Green[i] = rdr.read_byte();
          Blue[i] = rdr.read_byte();
        }
      }
      CodeSize = rdr.read_byte()+1;
      break; // okay, this is the image we want
    } else {
      Fl::warning("%s: unknown gif code 0x%02x", rdr.name(), i);
      blocklen = 0;
    }

    // skip the data:
    while (blocklen>0) {while (blocklen--) {ch = rdr.read_byte();} blocklen = rdr.read_byte();}
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
    Fl::warning("%s does not have a color table, using default.\n", rdr.name());
    BitsPerPixel = CodeSize - 1;
    ColorMapSize = 1 << BitsPerPixel;
    Red[0] = Green[0] = Blue[0] = 0;    // black
    Red[1] = Green[1] = Blue[1] = 255;  // white
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

  int blocklen = rdr.read_byte();
  uchar thisbyte = rdr.read_byte(); blocklen--;
  int frombit = 0;

  for (;;) {

    /* Fetch the next code from the raster data stream.  The codes can be
     * any length from 3 to 12 bits, packed into 8-bit bytes, so we have to
     * maintain our location as a pointer and a bit offset.
     * In addition, GIF adds totally useless and annoying block counts
     * that must be correctly skipped over. */
    int CurCode = thisbyte;
    if (frombit+CodeSize > 7) {
      if (blocklen <= 0) {
        blocklen = rdr.read_byte();
        if (blocklen <= 0) break;
      }
      thisbyte = rdr.read_byte(); blocklen--;
      CurCode |= thisbyte<<8;
    }
    if (frombit+CodeSize > 15) {
      if (blocklen <= 0) {
        blocklen = rdr.read_byte();
        if (blocklen <= 0) break;
      }
      thisbyte = rdr.read_byte(); blocklen--;
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
    else {Fl::error("Fl_GIF_Image: %s - LZW Barf!", rdr.name()); break;}

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
}
