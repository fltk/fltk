//
// Basic Fl_String class for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021-2022 by Bill Spitzak and others.
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

#include <FL/Fl_String.H>

#include <stdio.h>
#include <string.h>

/** \file src/Fl_String.cxx
  Basic Fl_String class for FLTK.
*/

Fl_String::Fl_String() {
  init();
}

Fl_String::Fl_String(const char *str) {
  init();
  value(str);
}

Fl_String::Fl_String(const char *str, int size) {
  init();
  value(str, size);
}

void Fl_String::init() {
  size_ = 0;
  value_ = 0;
  capacity_ = 0;
}

// copy constructor
Fl_String::Fl_String(const Fl_String &in) {
  init();
  value(in.value(), in.size());
}

// copy assignment operator
Fl_String& Fl_String::operator=(const Fl_String &in) {
  if (this == &in)
    return *this;
  value(in.value(), in.size());
  // debug("copy assigned");
  return *this;
}

// assignment operator for 'const char *'
Fl_String& Fl_String::operator=(const char *in) {
  value(in);
  // debug("*STRING* assigned");
  return *this;
}

Fl_String::~Fl_String() {
  delete[] value_;
}

/** Grow the buffer size to at least size+1 bytes.
 By default, this call destroys the contents of the current buffer.
 \param size in bytes
 \param preserve_text copy existing text into the new buffer
 */
void Fl_String::alloc_buf(int size, bool preserve_text) {
  if (size < 0)
    return;
  if (size > 0 && size <= capacity_)
    return;

  int new_size = (size + 1 + 15) & (~15); // round upwards
  char *new_value = new char[new_size];
  capacity_ = new_size - 1;

  if (preserve_text) {
    size_ = (int)strlen(value_);
    // the new buffer always has a higher capacity than the old one
    // make sure we copy the trailing NUL.
    memcpy(new_value, value_, size_+1);
  } else {
    size_ = 0;
  }
  delete[] value_;
  value_ = new_value;
}

void Fl_String::value(const char *str) {
  value(str, str ? (int)strlen(str) : 0);
}

int Fl_String::slen() const {
  if (!value_) return 0;
  return (int)strlen(value_);
}

void Fl_String::value(const char *str, int len) {
  if (str) {
    alloc_buf(len);
    size_ = len;
    memcpy(value_, str, size_);
    value_[size_] = '\0';
  } else {            // str == NULL
    size_ = 0;        // ignore len !
    delete[] value_;  // free buffer
    value_ = NULL;    // set null pointer (!)
    capacity_ = 0;    // reset capacity
  }
}

int Fl_String::capacity() const {
  return capacity_; // > 0 ? capacity_ - 1 : capacity_;
}

/** Set the minumum capacity to num_bytes plus one for a terminating NUL.
 The cintents of the string buffer will be copied if needed.
 \param num_bytes minimum size of buffer
 */
void Fl_String::capacity(int num_bytes) {
  alloc_buf(num_bytes, true);
}


void Fl_String::release() {
  delete[] value_;
  value_ = 0;
  size_ = 0;
  capacity_ = 0;
}

// =============================  DEBUG  =============================

/**
  Write some details about the string to stdout.

  Nothing at all is written if \p info is NULL, otherwise the short info
  string and details are written to stdout.

  The \p info string should not be longer than 20 characters to align the
  debug output of several strings.

  \param[in]  info  short info string or NULL
*/
void Fl_String::debug(const char *info) const {
  if (info) {
    printf("Fl_String '%-20s': %p, value = %p (%d/%d): '%s'\n",
           info, this, value_, size_, capacity_, value_ ? value_ : "<NULL>");
  }
}

/**
  Write some details about the string to stdout, followed by a hex dump of
  the string.

  The first part is the same as written by Fl_String::debug(). The following
  part is a hexadecimal dump of all bytes of the string. Embedded \p nul bytes
  are possible and will be dumped as well.

  \param[in]  info  short info string or NULL

  \see Fl_String::debug(const char *info) const
*/
void Fl_String::hexdump(const char *info) const {
  debug(info);
  if (size_ == 0)
    return;
  for (int i = 0; i < size_; i++) {
    if ((i & 15) == 0) {
      if (i > 0)
        printf("\n");
      printf("  [%04x %4d] ", i, i);  // position
    } else if ((i & 3) == 0) {        // separator after 4 bytes
      printf(" ");
    }
    printf(" %02x", (unsigned char)value_[i]);
  }
  printf("\n");
}
