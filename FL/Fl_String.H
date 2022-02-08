//
// Basic Fl_String header for the Fast Light Tool Kit (FLTK).
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

#ifndef _FL_Fl_String_H_
#define _FL_Fl_String_H_

/** \file FL/Fl_String.H
  Basic Fl_String class for FLTK.
*/

/**
  Fl_String is the basic string class for FLTK.

  In this version Fl_String can be used to store strings, copy strings,
  and move strings. There are no string manipulation methods yet.

  Fl_String can hold the value of an Fl_Input widget including \e nul bytes
  if the constructor Fl_String(const char *str, int size) is used.

  Assignment and copy constructors \b copy the string value such that the
  source string can be freed immediately after the assignment.

  The string value() can be an empty string \c "" or \c NULL.

  If value() is not \c NULL it is guaranteed that the string is terminated by
  a trailing \c nul byte even if the string contains embedded \c nul bytes.

  The method size() returns the full string size, whether the string contains
  embedded \c nul bytes or not. The special method slen() returns 0 if value()
  is \c NULL, otherwise the same as \c strlen() would do.

  Examples:
  \code
  Fl_String np(NULL);
  printf("  np    : value = %p, size = %d, slen = %d\n", np.value(), np.size(), np.slen());
  Fl_String empty("");
  printf("  empty : value = %p, size = %d\n", empty.value(), empty.size());
  Fl_String fltk("FLTK");
  Fl_Input i(0, 0, 0, 0);
  i.value("abc\0def", 7);
  Fl_String str(i.value(), i.size());
  printf("  str   : strlen = %lu, size = %d, capacity = %d\n",
         strlen(str.value()), str.size(), str.capacity());

  Output:

  np    : value = (nil), size = 0, slen = 0
  empty : value = 0x562840befbf0, size = 0
  str   : strlen = 3, size = 7, capacity = 15
  \endcode

  \since 1.4.0

  \todo Complete documentation of class Fl_String
*/

class Fl_String {
private:
  int size_;
  char *value_;
  int capacity_;

public:
  Fl_String();
  Fl_String(const char *str);
  Fl_String(const char *str, int size);

  // copy constructor
  Fl_String(const Fl_String &in);

  // copy assignment operator
  Fl_String& operator=(const Fl_String &in);

  // assignment operator for 'const char *'
  Fl_String& operator=(const char *in);

  virtual ~Fl_String();

private:
  void init();
  void alloc_buf(int size);
  void release();

public:
  void value(const char *str);
  void value(const char *str, int slen);

  const char *value() const { return value_; }
  int size() const { return size_; }

  int slen() const;
  int capacity() const;

  // ==================================  DEBUG  ==================================

  void debug(const char *info) const;           // output string info
  void hexdump(const char *info) const;         // output info + hexdump

}; // class Fl_String

#endif // _FL_Fl_String_H_
