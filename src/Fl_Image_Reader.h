//
// Internal (Image) Reader class for the Fast Light Tool Kit (FLTK).
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

/*
  This internal (undocumented) class reads data chunks from a file or from
  memory in LSB-first byte order.

  This class is used in Fl_GIF_Image and Fl_BMP_Image to avoid code
  duplication and may be extended to be used in similar cases. Future
  options might be to read data in MSB-first byte order or to add more
  methods.
*/

#ifndef FL_IMAGE_READER_H
#define FL_IMAGE_READER_H

#include <stdio.h>

class Fl_Image_Reader
{
public:
  // Create the reader.
  Fl_Image_Reader() :
  pIsFile(0), pIsData(0),
  pFile(0L), pData(0L),
  pStart(0L),
  pName(0L)
  {}

  // Initialize the reader to access the file system, filename is copied
  // and stored.
  int open(const char *filename);

  // Initialize the reader for memory access, name is copied and stored
  int open(const char *imagename, const unsigned char *data);

  // Close and destroy the reader
  ~Fl_Image_Reader();

  // Read a single byte from memory or a file
  unsigned char read_byte();

  // Read a 16-bit unsigned integer, LSB-first
  unsigned short read_word();

  // Read a 32-bit unsigned integer, LSB-first
  unsigned int read_dword();

  // Read a 32-bit signed integer, LSB-first
  int read_long() {
    return (int)read_dword();
  };

  // Move the current read position to a byte offset from the beginning
  // of the file or the original start address in memory
  void seek(unsigned int n);

  // return the name or filename for this reader
  const char *name() { return pName; }

private:

  // open() sets this if we read from a file
  char pIsFile;
  // open() sets this if we read from memory
  char pIsData;
  // a pointer to the opened file
  FILE *pFile;
  // a pointer to the current byte in memory
  const unsigned char *pData;
  // a pointer to the start of the image data
  const unsigned char *pStart;
  // a copy of the name associated with this reader
  char *pName;
};

#endif // FL_IMAGE_READER_H
