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

class Fl_Image_Reader {
public:
  // Create the reader.
  Fl_Image_Reader()
    : is_file_(0)
    , is_data_(0)
    , file_(0L)
    , data_(0L)
    , start_(0L)
    , end_((const unsigned char *)(-1L))
    , name_(0L)
    , error_(0) {}

  // Initialize the reader to access the file system, filename is copied
  // and stored.
  int open(const char *filename);

  // Initialize the reader for memory access, name is copied and stored
  int open(const char *imagename, const unsigned char *data, const size_t datasize);
  // Deprecated, DO NOT USE!
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
  int read_long() { return (int)read_dword(); }

  // Move the current read position to a byte offset from the beginning
  // of the file or the original start address in memory
  void seek(unsigned int n);

  // Get the current file or memory offset from the beginning
  // of the file or the original start address in memory
  long tell() const;

  // Get the current EOF or error status of the file or data block
  int error() const { return error_; }

  // return the name or filename for this reader
  const char *name() const { return name_; }

  // skip a given number of bytes
  void skip(unsigned int n) { seek((unsigned int)tell() + n); }

private:
  // open() sets this if we read from a file
  char is_file_;
  // open() sets this if we read from memory
  char is_data_;
  // a pointer to the opened file
  FILE *file_;
  // a pointer to the current byte in memory
  const unsigned char *data_;
  // a pointer to the start of the image data
  const unsigned char *start_;
  // a pointer to the end of image data if reading from memory, otherwise undefined
  // note: currently (const unsigned char *)(-1L) if end of memory is not available
  // ... which means "unlimited"
  const unsigned char *end_;
  // a copy of the name associated with this reader
  char *name_;
  // a flag to store EOF or error status
  int error_;
};

#endif // FL_IMAGE_READER_H
