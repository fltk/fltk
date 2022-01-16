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

static int string_count;

Fl_String::Fl_String() {
  string_count++;
  init();
  // debug("created ()");
}

Fl_String::Fl_String(const char *str) {
  string_count++;
  init();
  value(str);
  // debug("created (str)");
}

Fl_String::Fl_String(const char *str, int size) {
  string_count++;
  init();
  value(str, size);
  // debug("created (str, size)");
}

void Fl_String::init() {
  size_ = 0;
  value_ = 0;
  capacity_ = 0;
}

// copy constructor
Fl_String::Fl_String(const Fl_String &in) {
  string_count++;
  init();
  value(in.value(), in.size());
  // debug("copied (c'tor)");
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
  string_count--;
  // debug("~Fl_String()");
  delete[] value_;
}

void Fl_String::alloc_buf(int size) {
  if (size < 0)
    return;
  if (size > 0 && size <= capacity_)
    return;
  if (capacity_ > 0)
    return;

  int new_size = (size + 1 + 15) & (~15); // round upwards
  char *new_value = new char[new_size];
  capacity_ = new_size - 1;

  size_ = 0;
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

void Fl_String::release() {
  delete[] value_;
  value_ = 0;
  size_ = 0;
  capacity_ = 0;
}

// =============================  DEBUG  =============================

static const char fl_string_debug = 0;

void Fl_String::debug(const char *info) const {
  if (fl_string_debug) {
    printf("Fl_String[%2d] '%-20s': %p, value = %p (%d/%d): '%s'.\n",
          string_count, info, this, value_, size_, capacity_, value_);
  }
}

void Fl_String::hexdump(const char *info) const {
  if (fl_string_debug) {
    debug(info);
    for (int i = 0; i < size_; i++) {
      if ((i & 15) == 0) {
        if (i > 0) printf("\n");
        printf("  [%04x %4d] ", i, i);
      } else if ((i & 3) == 0)
        printf(" ");
      printf(" %02x", (unsigned char)value_[i]);
    }
    printf("\n");
  }
}
