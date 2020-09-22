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
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//


//
// Include necessary header files...
//

#include "Fl_Image_Reader.h"

#include <FL/fl_utf8.h>
#include <FL/fl_string.h>
#include <stdlib.h>
#include <string.h>

/*
  This internal (undocumented) class reads data chunks from a file or from
  memory in LSB-first byte order.

  This class is used in Fl_GIF_Image and Fl_BMP_Image to avoid code
  duplication and may be extended to be used in similar cases. Future
  options might be to read data in MSB-first byte order or to add more
  methods.
*/

// Initialize the reader to access the file system, filename is copied
// and stored.
int Fl_Image_Reader::open(const char *filename) {
  if (!filename)
    return -1;
  pName = fl_strdup(filename);
  if ( (pFile = fl_fopen(filename, "rb")) == NULL ) {
    return -1;
  }
  pIsFile = 1;
  return 0;
}

// Initialize the reader for memory access, name is copied and stored
int Fl_Image_Reader::open(const char *imagename, const unsigned char *data) {
  if (imagename)
    pName = fl_strdup(imagename);
  if (data) {
    pStart = pData = data;
    pIsData = 1;
    return 0;
  } else {
    return -1;
  }
}

// Close and destroy the reader
Fl_Image_Reader::~Fl_Image_Reader() {
  if (pIsFile && pFile) {
    fclose(pFile);
  }
  if (pName)
    ::free(pName);
}

// Read a single byte from memory or a file
uchar Fl_Image_Reader::read_byte() {
  if (pIsFile) {
    return getc(pFile);
  } else if (pIsData) {
    return *pData++;
  } else {
    return 0;
  }
}

// Read a 16-bit unsigned integer, LSB-first
unsigned short Fl_Image_Reader::read_word() {
  unsigned char b0, b1;  // Bytes from file
  if (pIsFile) {
    b0 = (uchar)getc(pFile);
    b1 = (uchar)getc(pFile);
    return ((b1 << 8) | b0);
  } else if (pIsData) {
    b0 = *pData++;
    b1 = *pData++;
    return ((b1 << 8) | b0);
  } else {
    return 0;
  }
}

// Read a 32-bit unsigned integer, LSB-first
unsigned int Fl_Image_Reader::read_dword() {
  unsigned char b0, b1, b2, b3;  // Bytes from file
  if (pIsFile) {
    b0 = (uchar)getc(pFile);
    b1 = (uchar)getc(pFile);
    b2 = (uchar)getc(pFile);
    b3 = (uchar)getc(pFile);
    return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
  } else if (pIsData) {
    b0 = *pData++;
    b1 = *pData++;
    b2 = *pData++;
    b3 = *pData++;
    return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
  } else {
    return 0;
  }
}

// Read a 32-bit signed integer, LSB-first
// int Fl_Image_Reader::read_long() -- implementation in header file

// Move the current read position to a byte offset from the beginning
// of the file or the original start address in memory
void Fl_Image_Reader::seek(unsigned int n) {
  if (pIsFile) {
    fseek(pFile, n , SEEK_SET);
  } else if (pIsData) {
    pData = pStart + n;
  }
}
