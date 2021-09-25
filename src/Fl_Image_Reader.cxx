//
// Internal (Image) Reader class for the Fast Light Tool Kit (FLTK).
//
// Copyright 2020-2021 by Bill Spitzak and others.
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
int Fl_Image_Reader::open(const char *imagename, const unsigned char *data, const long datasize) {
  if (imagename)
    pName = fl_strdup(imagename);
  if (data) {
    pStart = pData = data;
    pIsData = 1;
    if (datasize > 0) {
      pEnd = pStart + datasize;
    }
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
  if (error()) // don't read after read error or EOF
    return 0;
  if (pIsFile) {
    int ret = getc(pFile);
    if (ret < 0) {
      if (feof(pFile))
        pError = 1;
      else if (ferror(pFile))
        pError = 2;
      else
        pError = 3; // unknown error
      return 0;
    }
    return ret;
  } else if (pIsData) {
    if (pData < pEnd)
      return *pData++;
    pError = 1; // EOF
    return 0;
  }
  pError = 3; // undefined mode
  return 0;
}

// Read a 16-bit unsigned integer, LSB-first
unsigned short Fl_Image_Reader::read_word() {
  unsigned char b0, b1;  // Bytes from file or memory
  b0 = read_byte();
  b1 = read_byte();
  if (error())
    return 0;
  return ((b1 << 8) | b0);
}

// Read a 32-bit unsigned integer, LSB-first
unsigned int Fl_Image_Reader::read_dword() {
  unsigned char b0, b1, b2, b3;  // Bytes from file or memory
  b0 = read_byte();
  b1 = read_byte();
  b2 = read_byte();
  b3 = read_byte();
  if (error())
    return 0;
  return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

// Move the current read position to a byte offset from the beginning
// of the file or the original start address in memory.
// This clears the error flag if the position is valid.
void Fl_Image_Reader::seek(unsigned int n) {
  if (pIsFile) {
    int ret = fseek(pFile, n , SEEK_SET);
    if (ret < 0)
      pError = 2; // read / position error
    else
      pError = 0;
    return;
  } else if (pIsData) {
    if (pStart + n <= pEnd)
      pData = pStart + n;
    else
      pError = 2; // read / position error
    return;
  }
  // unknown mode (not initialized ?)
  pError = 3;
}


// Get the current read position as a byte offset from the
// beginning of the file or the original start address in memory
long Fl_Image_Reader::tell() const {
  if (pIsFile) {
    return ftell(pFile);
  } else if (pIsData) {
    return long(pData - pStart);
  }
  return 0;
}
