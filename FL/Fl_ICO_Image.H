//
// "$Id$"
//
// ICO image header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2020 by Bill Spitzak and others.
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

// https://en.wikipedia.org/wiki/ICO_(file_format)
// http://www.daubnet.com/en/file-format-ico

#ifndef Fl_ICO_Image_H
#define Fl_ICO_Image_H
#  include "Fl_BMP_Image.H"

typedef struct {
  int bWidth;         // Image width
  int bHeight;        // Image height
  int bColorCount;    // Number of colors (0 if >=8bpp)
  int bReserved;      // Reserved
  int wPlanes;        // Color Planes
  int wBitCount;      // Bits per pixel
  int dwBytesInRes;   // Resource size in bytes
  int dwImageOffset;  // Offset to the image
} fl_ICONDIRENTRY;


class FL_EXPORT Fl_ICO_Image : public Fl_BMP_Image {

protected:

  int idcount_;
  fl_ICONDIRENTRY *icondirentry_;


public:

  // id: load image resource by ID (0, 1, 2, ...)
  // id == -1: load any icon (try to get the "best")
  // id <= -2: load ICONDIRENTRY but no image
  Fl_ICO_Image(const char *filename, const unsigned char *data = 0, int id = -1);

  int idcount() { return idcount_; }
  fl_ICONDIRENTRY * const icondirentry() { return icondirentry_; }


protected:

  void load_ico_(class Fl_Image_Reader &rdr, int id);

};

#endif

