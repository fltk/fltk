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
#include <FL/fl_string_functions.h>
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
  name_ = fl_strdup(filename);
  if ((file_ = fl_fopen(filename, "rb")) == NULL) {
    return -1;
  }
  is_file_ = 1;
  return 0;
}

// Initialize the reader for memory access, name is copied and stored.
int Fl_Image_Reader::open(const char *imagename, const unsigned char *data, const size_t datasize) {
  if (imagename)
    name_ = fl_strdup(imagename);
  if (data) {
    start_ = data_ = data;
    is_data_ = 1;
    end_ = start_ + datasize;
    return 0;
  }
  return -1;
}


// Initialize the reader for memory access, name is copied and stored.
// Deprecated, DO NOT USE! Buffer overruns will not be checked!
// Please use instead:
// Fl_Image_Reader::open(const char *imagename, const unsigned char *data, const size_t datasize)

int Fl_Image_Reader::open(const char *imagename, const unsigned char *data) {
  if (imagename)
    name_ = fl_strdup(imagename);
  if (data) {
    start_ = data_ = data;
    is_data_ = 1;
    return 0;
  }
  return -1;
}

// Close and destroy the reader
Fl_Image_Reader::~Fl_Image_Reader() {
  if (is_file_ && file_) {
    fclose(file_);
  }
  if (name_)
    free(name_);
}

// Read a single byte from memory or a file
uchar Fl_Image_Reader::read_byte() {
  if (error()) // don't read after read error or EOF
    return 0;
  if (is_file_) {
    int ret = getc(file_);
    if (ret < 0) {
      if (feof(file_))
        error_ = 1;
      else if (ferror(file_))
        error_ = 2;
      else
        error_ = 3; // unknown error
      return 0;
    }
    return ret;
  } else if (is_data_) {
    if (data_ < end_)
      return *data_++;
    error_ = 1; // EOF
    return 0;
  }
  error_ = 3; // undefined mode
  return 0;
}

// Read a 16-bit unsigned integer, LSB-first
unsigned short Fl_Image_Reader::read_word() {
  unsigned char b0, b1; // Bytes from file or memory
  b0 = read_byte();
  b1 = read_byte();
  if (error())
    return 0;
  return ((b1 << 8) | b0);
}

// Read a 32-bit unsigned integer, LSB-first
unsigned int Fl_Image_Reader::read_dword() {
  unsigned char b0, b1, b2, b3; // Bytes from file or memory
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
// This method clears the error flag if the position is valid.
// If reading from memory and (start_ + n) overflows, then the result is undefined.

void Fl_Image_Reader::seek(unsigned int n) {
  error_ = 0;
  if (is_file_) {
    int ret = fseek(file_, n, SEEK_SET);
    if (ret < 0)
      error_ = 2; // read / position error
    return;
  } else if (is_data_) {
    if (start_ + n <= end_)
      data_ = start_ + n;
    else
      error_ = 2; // read / position error
    return;
  }
  // unknown mode (not initialized ?)
  error_ = 3;
}

// Get the current read position as a byte offset from the
// beginning of the file or the original start address in memory.
// This method does neither affect the error flag nor is it affected
// by the current error status. If reading from a file, this may
// return -1 or any error code from ftell().

long Fl_Image_Reader::tell() const {
  if (is_file_) {
    return ftell(file_);
  } else if (is_data_) {
    return long(data_ - start_);
  }
  return 0;
}
