//
// "$Id: gif.cxx,v 1.3.2.5 2001/01/22 15:13:39 easysw Exp $"
//
// GIF support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char uchar;

#define NEXTBYTE getc(GifFile)
#define GETSHORT(var) var = NEXTBYTE; var += NEXTBYTE << 8

int gif2xpm(
    const char *infname,// filename for error messages
    FILE *GifFile,	// file to read
    char*** datap,	// return xpm data here
    int** lengthp,	// return line lengths here
    int inumber		// which image in movie (0 = first)
) {

  {char b[6];
  if (fread(b,1,6,GifFile)<6) return 0; /* quit on eof */
  if (b[0]!='G' || b[1]!='I' || b[2] != 'F') {
    fprintf(stderr,"%s is not a GIF file.\n", infname); return 0;}
  if (b[3]!='8' || b[4]>'9' || b[5]!= 'a')
    fprintf(stderr,"%s is version %c%c%c.\n",infname,b[3],b[4],b[5]);
  }

  int Width; GETSHORT(Width);
  int Height; GETSHORT(Height);

  uchar ch = NEXTBYTE;
  char HasColormap = ((ch & 0x80) != 0);
  int BitsPerPixel = (ch & 7) + 1;
  int ColorMapSize = 1 << BitsPerPixel;
  // int OriginalResolution = ((ch>>4)&7)+1;
  // int SortedTable = (ch&8)!=0;
  NEXTBYTE; // Background Color index
  NEXTBYTE; // Aspect ratio is N/64

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
  } else {
    fprintf(stderr,"%s does not have a colormap.\n", infname);
    for (int i = 0; i < ColorMapSize; i++)
      Red[i] = Green[i] = Blue[i] = (i*256+ColorMapSize-1)/ColorMapSize;
  }

  int CodeSize;		/* Code size, init from GIF header, increases... */
  char Interlace;

  for (;;) {

    int i = NEXTBYTE;
    if (i<0) {fprintf(stderr,"%s: unexpected EOF\n",infname); return 0;}
    int blocklen;

    //  if (i == 0x3B) return 0;  eof code

    if (i == 0x21) {		// a "gif extension"

      ch = NEXTBYTE;
      blocklen = NEXTBYTE;

      if (ch==0xF9 && blocklen==4) { // Netscape animation extension

	char bits;
	bits = NEXTBYTE;
	NEXTBYTE; NEXTBYTE; // GETSHORT(delay);
	transparent_pixel = NEXTBYTE;
	if (bits & 1) has_transparent = 1;
	blocklen = NEXTBYTE;

      } else if (ch == 0xFF) { // Netscape repeat count
	;

      } else if (ch == 0xFE) { //Gif Comment
#if 0
	if(blocklen>0) {
	  char *comment=new char[blocklen+1];
	  int l;
	  for(l=0;blocklen;l++,blocklen--)
	    comment[l]=NEXTBYTE;
	  comment[l]=0;
	  fprintf(stderr,"%s: Gif Comment: '%s'\n", infname, comment);
	  delete comment;
	  NEXTBYTE; //End marker
	}
#endif 
      } else {
	fprintf(stderr,"%s: unknown gif extension 0x%02x\n", infname, ch);

      }

    } else if (i == 0x2c) {	// an image

      NEXTBYTE; NEXTBYTE; // GETSHORT(x_position);
      NEXTBYTE; NEXTBYTE; // GETSHORT(y_position);
      GETSHORT(Width);
      GETSHORT(Height);
      ch = NEXTBYTE;
      Interlace = ((ch & 0x40) != 0);
      if (ch&0x80) { 
	// read local color map
	int n = 1<<((ch&7)+1); // does this replace ColorMapSize ??
	for (i=0; i < n; i++) {	
	  Red[i] = NEXTBYTE;
	  Green[i] = NEXTBYTE;
	  Blue[i] = NEXTBYTE;
	}
      }
      CodeSize = NEXTBYTE+1;

      if (!inumber--) break; // okay, this is the image we want
      blocklen = NEXTBYTE;
      
    } else {
      fprintf(stderr,"%s: unknown gif code 0x%02x\n", infname, i);
      blocklen = 0;
    }

    // skip the data:
    while (blocklen>0) {while (blocklen--) {NEXTBYTE;} blocklen=NEXTBYTE;}
  }

  uchar *Image = new uchar[Width*Height];
  if (!Image) {
    fprintf (stderr, "Insufficient memory\n");
    exit (2);
  }
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

    uchar OutCode[1025]; // temporary array for reversing codes
    uchar *tp = OutCode;
    int i;
    if (CurCode < FreeCode) i = CurCode;
    else if (CurCode == FreeCode) {*tp++ = FinChar; i = OldCode;}
    else {fprintf(stderr,"%s : LZW Barf!\n",infname); break;}

    while (i >= ColorMapSize) {*tp++ = Suffix[i]; i = Prefix[i];}
    *tp++ = FinChar = i;
    while (tp > OutCode) {
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
    }

    if (OldCode != ClearCode) {
      Prefix[FreeCode] = OldCode;
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
  char** data = new char*[Height+3];
  *datap = data;
  int* length = new int[Height+2];
  *lengthp = length;

  // transparent pixel must be zero, swap if it isn't:
  if (has_transparent && transparent_pixel != 0) {
    // swap transparent pixel with zero
    p = Image+Width*Height;
    while (p-- > Image) {
      if (*p==transparent_pixel) *p = 0;
      else if (!*p) *p = transparent_pixel;
    }
    uchar t;
    t = Red[0]; Red[0] = Red[transparent_pixel]; Red[transparent_pixel] = t;
    t =Green[0];Green[0]=Green[transparent_pixel];Green[transparent_pixel]=t;
    t =Blue[0];Blue[0] =Blue[transparent_pixel];Blue[transparent_pixel] = t;
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
    remap[i] = base++;
    numcolors++;
  }

  // write the first line of xpm data (use suffix as temp array):
  length[0] = sprintf((char*)(Suffix),
		      "%d %d %d %d",Width,Height,-numcolors,1);
  data[0] = new char[length[0]+1];
  strcpy(data[0], (char*)Suffix);

  // write the colormap
  length[1] = 4*numcolors;
  data[1] = (char*)(p = new uchar[4*numcolors]);
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
    data[i+2] = (char*)(Image + i*Width);
    length[i+2] = Width;
  }

  data[Height+2] = 0; // null to end string array
  return Height+2;
}

//
// End of "$Id: gif.cxx,v 1.3.2.5 2001/01/22 15:13:39 easysw Exp $".
//
