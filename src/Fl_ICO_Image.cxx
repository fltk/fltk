//
// Fl_ICO_Image class for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022 by Bill Spitzak and others.
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

/**
 Loads the named icon image from the given .ico filename or from memory
 \param filename Name of a .ico file, or of the in-memory image
 \param id When id is -1 (default), the highest-resolution icon is loaded;
 when id ≥ 0, load the icon with this ID;
 when id = -2, load all IconDirEntry structures but no image.
 \param data NULL, or in-memory icon data
 \param datasize  Size  in bytes of the \p data byte array (used when \p data is not NULL)
*/
Fl_ICO_Image::Fl_ICO_Image(const char *filename, int id, const unsigned char *data, const size_t datasize)
: Fl_BMP_Image(0,0),
 idcount_(0),
 icondirentry_(0)
{
  Fl_Image_Reader rdr;
  int r;

  w(0); h(0); d(0); ld(0);
  alloc_array = 0;
  array = 0;

  if (data) {
    r = rdr.open(filename, data, datasize);
  } else {
    r = rdr.open(filename);
  }

  if (r == -1) {
    ld(ERR_FILE_ACCESS);
  } else {
    load_ico_(rdr, id);
  }
}

/** Destructor */
Fl_ICO_Image::~Fl_ICO_Image() {
  delete[] icondirentry_;
}


/*
 This method attempts to load the biggest image resource available
 inside a .ICO file (Windows Icon format).
 */
void Fl_ICO_Image::load_ico_(Fl_Image_Reader &rdr, int id)
{
  int pickedID = -1;

  // Check file header (ICONDIR, 6 bytes)

  if (rdr.read_word() != 0 || rdr.read_word() != 1) {
    Fl::error("Fl_ICO_Image: %s is not an ICO file.\n", rdr.name());
    ld(ERR_FORMAT);
    return;
  }

  idcount_ = rdr.read_word();

  if (idcount() == 0) {
    Fl::error("Fl_ICO_Image: %s - no image resources found\n", rdr.name());
    ld(ERR_FORMAT);
    return;
  }


  // read entries (IconDirEntry, 16 bytes each)

  icondirentry_ = new IconDirEntry[idcount()];

  for (int i = 0; i < idcount(); ++i) {
    icondirentry_[i].bWidth = (int)rdr.read_byte();
    icondirentry_[i].bHeight = (int)rdr.read_byte();
    icondirentry_[i].bColorCount = (int)rdr.read_byte();
    icondirentry_[i].bReserved = (int)rdr.read_byte();
    icondirentry_[i].wPlanes = (int)rdr.read_word();
    icondirentry_[i].wBitCount = (int)rdr.read_word();
    icondirentry_[i].dwBytesInRes = (int)rdr.read_dword();
    icondirentry_[i].dwImageOffset = (int)rdr.read_dword();

    if (icondirentry_[i].bWidth == 0) icondirentry_[i].bWidth = 256;
    if (icondirentry_[i].bHeight == 0) icondirentry_[i].bHeight = 256;
  }

  if (id <= -2) return;

  if (!icondirentry_ || idcount() < 1 || id >= idcount()) {
    ld(ERR_FORMAT);
    return;
  }

  if (id == -1) {
    // pick icon with highest resolution + highest bitcount
    int highestRes = 0, bitcount = 0;
    for (int i = 0; i < idcount(); ++i) {
      int res = icondirentry_[i].bWidth * icondirentry_[i].bHeight;
      if (res > highestRes || (res == highestRes && icondirentry_[i].wBitCount > bitcount)) {
        highestRes = res;
        bitcount = icondirentry_[i].wBitCount;
        pickedID = i;
      }
    }
  } else {
    pickedID = id;
  }

  if (pickedID < 0 ||
      icondirentry_[pickedID].bWidth <= 0 ||
      icondirentry_[pickedID].bHeight <= 0 ||
      icondirentry_[pickedID].dwImageOffset <= 0||
      icondirentry_[pickedID].dwBytesInRes <= 0)
  {
    ld(ERR_FORMAT);
    return;
  }

  rdr.seek(icondirentry_[pickedID].dwImageOffset);


  // Check for a PNG image resource
  uchar b[8];
  for (int i=0; i<8; ++i) b[i] = rdr.read_byte();

  if (b[0]==0x89 && b[1]=='P' && b[2]=='N' && b[3]=='G' &&
      b[4]=='\r' && b[5]=='\n' && b[6]==0x1A && b[7]=='\n')
  {
#if defined(HAVE_LIBPNG) && defined(HAVE_LIBZ)
    Fl_PNG_Image *png = new Fl_PNG_Image(rdr.name(), icondirentry_[pickedID].dwImageOffset);

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

  w(icondirentry_[pickedID].bWidth);
  h(icondirentry_[pickedID].bHeight);
  d(4);

  if (((size_t)w()) * h() * d() > max_size()) {
    Fl::warning("ICO file \"%s\" is too large!\n", rdr.name());
    w(0); h(0); d(0);
    ld(ERR_FORMAT);
    return;
  }

  rdr.seek(icondirentry_[pickedID].dwImageOffset);
  load_bmp_(rdr, h(), w());
}
