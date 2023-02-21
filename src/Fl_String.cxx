//
// Basic Fl_String class for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021-2023 by Bill Spitzak and others.
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
#include <stdlib.h>
#include <string.h>
#include <limits.h>


/** \file src/Fl_String.cxx
 Basic Fl_String class for FLTK.
 */


/*
 If value_ is NULL, c_str() and buffer() will point here.
 */
const char Fl_String::NUL = 0;

/**
 Indicate a maximum value or error.
 This value is generally used as end of string indicator or as the error
 indicator by the functions that return a string index.
 */
const int Fl_String::npos = INT_MAX;

/**
 Initialise the class instance.
 */
void Fl_String::init_() {
  buffer_ = NULL;
  size_ = 0;
  capacity_ = 0;
}

/**
 Grow the buffer to a capacity of at least n bytes.
 */
void Fl_String::grow_(int n) {
  if (n <= capacity_)
    return;
  int alloc_size_ = n + 1; // trailling NUL
  // round n up so we can grow in chunks
  if (alloc_size_ <= 24) { // allocate at least 24 bytes
    alloc_size_ = 24;
  } else if (alloc_size_ < 1024) {
    alloc_size_ = (alloc_size_+128) & ~127; // allocate in 128 byte chunks
  } else {
    alloc_size_ = (alloc_size_+2048) & ~2047; // allocate in 2k chunks
  }
  // allocate now
  char *new_buffer = (char*)::malloc(alloc_size_);
  if (buffer_) {
    memcpy(new_buffer, buffer_, size_);
    ::free(buffer_);
  }
  new_buffer[size_] = 0; // trailing NUL
  buffer_ = new_buffer;
  capacity_ = alloc_size_-1;  // trailing NUL
}

/**
 Shrink the buffer to n bytes, or size, if size > n.
 */
void Fl_String::shrink_(int n) {
  if (n < size_)
    n = size_;
  if (n == capacity_)
    return;
  if (n == 0) {
    if (buffer_)
      ::free(buffer_);
    buffer_ = NULL;
  } else {
    buffer_ = (char*)::realloc(buffer_, n+1); // NUL
    buffer_[size_] = 0; // trailing NUL
  }
  capacity_ = n;
}

// ---- Assignment ----------------------------------------------------- MARK: -

/**
 Allocate an empty string.
 */
Fl_String::Fl_String() {
  init_();
}

/**
 Copy constructor.
 */
Fl_String::Fl_String(const Fl_String &str) {
  init_();
  assign(str);
}

/**
 Constructor from a C-style string.
 */
Fl_String::Fl_String(const char *cstr) {
  init_();
  assign(cstr);
}

/**
 Constructor from a buffer of size bytes.
 */
Fl_String::Fl_String(const char *str, int size) {
  init_();
  assign(str, size);
}

/**
 Destructor.
 */
Fl_String::~Fl_String() {
  if (buffer_)
    ::free(buffer_);
}

/**
 Copy assignment operator
 */
Fl_String &Fl_String::operator=(const Fl_String &str) {
  return assign(str);
}

/**
 Assign a C-style string.
 */
Fl_String &Fl_String::operator=(const char *cstr) {
  return assign(cstr);
}

/**
 Copy another string.
 */
Fl_String &Fl_String::assign(const Fl_String &str) {
  if (&str == this) return *this;
  return assign(str.data(), str.size());
}

/**
 Assign a C-style string.
 */
Fl_String &Fl_String::assign(const char *cstr) {
  if (cstr && *cstr) {
    int len = (int)::strlen(cstr);
    return assign(cstr, len);
  } else {
    resize(0);
  }
  return *this;
}

/**
 Assign a buffer of size bytes.
 */
Fl_String &Fl_String::assign(const char *str, int size) {
  if (size > 0) {
    grow_(size);
    memcpy(buffer_, str, size);
    buffer_[size] = 0;
    size_ = size;
  } else {
    resize(0);
  }
  return *this;
}

// ---- Element Access ------------------------------------------------- MARK: -

/**
 Returns a reference to the character at specified location.
 */
char Fl_String::operator[](int n) const {
  if (buffer_)
    return buffer_[n];
  else
    return 0;
}

/**
 Returns a reference to the character at specified location.
 */
char &Fl_String::operator[](int n) {
  if (!buffer_)
    reserve(1);
  return buffer_[n];
}

/**
 Return a pointer to the NUL terminated buffer.
 */
const char *Fl_String::data() const {
  if (buffer_)
    return buffer_;
  else
    return &NUL;
}

/**
 Return a pointer to the writable NUL terminated buffer.
 */
char *Fl_String::data() {
  if (!buffer_)
    reserve(1);
  return buffer_;
}

/**
 Return a pointer to the NUL terminated buffer.
 \note same as Fl_String::data()
 */
const char *Fl_String::c_str() const {
  return data();
}

// ---- Capacity ------------------------------------------------------- MARK: -

/**
 Checks if the string is empty.
 */
bool Fl_String::empty() const {
  return (size_ == 0);
}

/**
 Returns the number of bytes in the string.
 */
int Fl_String::size() const {
  return size_;
}

/**
 Reserve n bytes for storage.
 If n is less or equal than size, the capacity is set to size.
 */
void Fl_String::reserve(int n) {
  grow_(n);
}

/**
 Return the number of chars that are allocated for storage.
 */
int Fl_String::capacity() const {
  return capacity_;
}

/**
 Shrink the capacity to fit the current size.
 */
void Fl_String::shrink_to_fit() {
  shrink_(size_);
}

// ---- Operations ----------------------------------------------------- MARK: -

/**
 Set an ampty string.
 */
void Fl_String::clear() {
  resize(0);
}

/**
 Resizes the string to contain n characters.
 If the current size is less than n, the space is filled with NUL.
 */
void Fl_String::resize(int n) {
  if (n == size_)
    return;
  if (n < size_) {
    size_ = n;
    if (buffer_) buffer_[size_] = 0;
  } else {
    grow_(n);
    if (buffer_) ::memset(buffer_+size_, 0, n-size_+1);
  }
  size_ = n;
}

// --- Non Standard ---------------------------------------------------- MARK: -

/**
 Returns the number bytes until the first NUL byte.
 */
int Fl_String::strlen() const {
  if (!buffer_) return 0;
  return (int)::strlen(buffer_);
}

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
    printf("Fl_String '%-20s': %p, value = %p (%d/%d):\n%s\n",
           info, this, buffer_, size_, capacity_, buffer_ ? buffer_ : "<NULL>");
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
    printf(" %02x", (unsigned char)buffer_[i]);
  }
  printf("\n");
}
