//
// Fl_ICO_Image class for the Fast Light Tool Kit (FLTK).
//
// Copyright 2020 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     https://www.fltk.org/str.php
//


//
// Include necessary header files...
//

#include <FL/Fl.H>
#include "config.h"
#include "Fl_Image_Reader.h"
#include <FL/Fl_ICO_Image.H>
#if defined(HAVE_LIBPNG) && defined(HAVE_LIBZ)
#  include <FL/Fl_PNG_Image.H>
#endif

Fl_ICO_Image::Fl_ICO_Image(const char *filename)
: Fl_BMP_Image(0)
{
  Fl_Image_Reader rdr;
  if (rdr.open(filename) == -1) {
    ld(ERR_FILE_ACCESS);
  } else {
    load_ico_(rdr);
  }
}

Fl_ICO_Image::Fl_ICO_Image(const char *imagename, const unsigned char *data)
: Fl_BMP_Image(0,0)
{
  Fl_Image_Reader rdr;
  if (rdr.open(imagename, data) == -1) {
    ld(ERR_FILE_ACCESS);
  } else {
    load_ico_(rdr);
  }
}

/*
 This method attempts to load the biggest image resource available
 inside a .ICO file (Windows Icon format).
 */
void Fl_ICO_Image::load_ico_(Fl_Image_Reader &rdr)
{
  w(0); h(0); d(0); ld(0);
  alloc_array = 0;
  array = 0;

  // Check file header (ICONDIR, 6 bytes)
  unsigned short idReserved = rdr.read_word();
  unsigned short idType = rdr.read_word();
  unsigned short idCount = rdr.read_word();

  if (idReserved != 0 || idType != 1) {
    Fl::error("Fl_ICO_Image: %s is not an ICO file.\n", rdr.name());
    ld(ERR_FORMAT);
    return;
  }

  if (idCount == 0) {
    Fl::error("Fl_ICO_Image: %s - no image resources found\n", rdr.name());
    ld(ERR_FORMAT);
    return;
  }

  // Check directory entries

  uint highestRes = 0;
  uint offset = 0;
  uint numBytes = 0;
  uint bitcount = 0;

  for (uint i = 0; i < idCount; ++i) {
    // ICONDIRENTRY, 16 bytes
    uint bWidth = rdr.read_byte();
    uint bHeight = rdr.read_byte();
    rdr.read_byte();  // bColorCount
    rdr.read_byte();  // bReserved
    rdr.read_word();  // wPlanes
    uint wBitCount = rdr.read_word();  // wBitCount
    uint dwBytesInRes = rdr.read_dword();
    uint dwImageOffset = rdr.read_dword();

    if (bWidth == 0) bWidth = 256;
    if (bHeight == 0) bHeight = 256;
    uint res = bWidth * bHeight;

    // pick icon with highest resolution + highest bitcount
    if (res > highestRes || (res == highestRes && wBitCount > bitcount)) {
      w(bWidth);
      h(bHeight);
      offset = dwImageOffset;
      numBytes = dwBytesInRes;
      highestRes = res;
      bitcount = wBitCount;
    }
  }

  if (offset==0 || numBytes==0 || highestRes==0 || w()==0 || h()==0) {
    w(0); h(0); d(0);
    ld(ERR_FORMAT);
    return;
  }

  rdr.seek(offset);

  // Check for a PNG image resource
  uchar b[8];
  for (int i=0; i<8; ++i) b[i] = rdr.read_byte();

  if (b[0]==0x89 && b[1]=='P' && b[2]=='N' && b[3]=='G' &&
      b[4]=='\r' && b[5]=='\n' && b[6]==0x1A && b[7]=='\n')
  {
#if defined(HAVE_LIBPNG) && defined(HAVE_LIBZ)
    Fl_PNG_Image *png = new Fl_PNG_Image(rdr.name(), offset);

    int loaded = png ? png->fail() : ERR_FILE_ACCESS;
    if (loaded < 0) {
      w(0); h(0); d(0);
      ld(loaded);
      if (png) delete png;
      return;
    }

    w(png->w());
    h(png->h());
    d(png->d());

    // take over pointer of Fl_PNG_Image's array
    array = png->array;
    alloc_array = 1;
    png->array = NULL;
    png->alloc_array = 0;

    delete png;
    return;
#else
    Fl::error("Fl_ICO_Image: %s - cannot decode PNG resource (no libpng support)!\n", rdr.name());
    w(0); h(0); d(0);
    ld(ERR_FORMAT);
    return;
#endif
  }

  // Bitmap resource

  d(4);

  if (((size_t)w()) * h() * d() > max_size()) {
    Fl::warning("ICO file \"%s\" is too large!\n", rdr.name());
    w(0); h(0); d(0);
    ld(ERR_FORMAT);
    return;
  }

  rdr.seek(offset);

  desired_h(h());
  load_bmp_(rdr, 1);
}


