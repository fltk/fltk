//
// Basic Fl_String header for the Fast Light Tool Kit (FLTK).
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

#ifndef _FL_Fl_String_H_
#define _FL_Fl_String_H_

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

/** \file src/Fl_String.H
  Basic Fl_String class for FLTK.
*/

#include <FL/Fl_Export.H>

#include <string>

#if 1

// Since fltk 1.5.0, Fl_String is no longer needed and now maps to std::string.
// Fl_String is not part of the public API. All occurrences of Fl_String in the
// core library and in Fluid should be replaced with std::string. When done,
// src/Fl_String.H and src/Fl_String.cxx can be deleted.
using Fl_String = std::string;

#else

// See: https://en.cppreference.com/w/cpp/string/basic_string/basic_string

/**
  Fl_String is the basic string class for FLTK.

  In this version Fl_String can be used to store strings, copy strings,
  move strings, and do basic string manipulation. Fl_String implements a
  subset of std::string with a couple of extensions. std::string should be
  a drop-in if we ever decide to allow templates and the std library.

  Fl_String always maintains a trailing \e nul byte, but can also contain
  \e nul bytes inside the string if the constructor
  Fl_String(const char *str, int size) is used.

  Assignment and copy constructors \b copy the string value such that the
  source string can be freed immediately after the assignment.

  c_str() and data() can be an empty string \c "", but never be \c NULL.

  The method size() returns the full string size, whether the string contains
  embedded \c nul bytes or not. The special method Fl_String::strlen() returns
  the length of the string up to the first \e nul.

  All methods of Fl_String work on a byte level. They are not UTF-8 aware,
  but may hold and manipulate UTF-8 strings if done with care.

  \since 1.4.0
*/
class FL_EXPORT Fl_String {

private:
  /*
   FLTK does no small string optimization.
   If the string is empty and capacity is not set, buffer_ will be NULL.
   */
  char *buffer_;
  int size_;
  int capacity_;

  void init_();
  void grow_(int n);
  void shrink_(int n);
  Fl_String &replace_(int at, int n_del, const char *src, int n_ins);

protected:
  static const char NUL;

public:
  static const int npos;

  // ---- Assignment
  Fl_String();
  Fl_String(const Fl_String &str);
  Fl_String(const char *cstr);
  Fl_String(const char *str, int size);
  ~Fl_String();
  Fl_String& operator=(const Fl_String &str);
  Fl_String& operator=(const char *cstr);
  Fl_String &assign(const Fl_String &str);
  Fl_String &assign(const char *cstr);
  Fl_String &assign(const char *str, int size);

  // ---- Element Access
  char at(int pos) const;
  char operator[](int n) const;
  char &operator[](int n);
  const char *data() const;
  char *data();
  const char *c_str() const;

  // ---- Capacity
  bool empty() const;
  int size() const;
  void reserve(int n);
  int capacity() const;
  void shrink_to_fit();

  // --- Operations
  void clear();
  Fl_String &insert(int at, const char *src, int n_ins=npos);
  Fl_String &insert(int at, const Fl_String &src);
  Fl_String &erase(int at, int n_del);
  void push_back(char c);
  void pop_back();
  Fl_String &append(const char *src, int n_ins=npos);
  Fl_String &append(const Fl_String &src);
  Fl_String &append(char c);
  Fl_String &operator+=(const char *src);
  Fl_String &operator+=(const Fl_String &src);
  Fl_String &operator+=(char c);
  int find(const Fl_String &needle, int start_pos=0) const;
  Fl_String &replace(int at, int n_del, const char *src, int n_ins=npos);
  Fl_String &replace(int at, int n_del, const Fl_String &src);
  Fl_String substr(int pos=0, int n=npos) const;
  void resize(int n);

  // --- Non Standard
  int strlen() const;
  void debug(const char *info = 0) const;
  void hexdump(const char *info = 0) const;
}; // class Fl_String

// ---- Non-member functions
FL_EXPORT Fl_String operator+(const Fl_String& lhs, const Fl_String& rhs);
FL_EXPORT Fl_String operator+(const Fl_String& lhs, const char* rhs);
FL_EXPORT bool operator==(const Fl_String & lhs, const Fl_String & rhs);
FL_EXPORT bool operator!=(const Fl_String & lhs, const Fl_String & rhs);

#endif

/**
\}
\endcond
*/

#endif // _FL_Fl_String_H_
