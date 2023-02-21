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
*/

class Fl_String {

private:
  /*
   FLTK does no small string optimisation.
   If the string is empty and cpacity is not set, value_ will be NULL.
   */
  char *buffer_;
  int size_;
  int capacity_;

  void init_();
  void grow_(int n);
  void shrink_(int n);

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
  void resize(int n);

  // --- Non Standard
  int strlen() const;
  void debug(const char *info = 0) const;
  void hexdump(const char *info = 0) const;
}; // class Fl_String


/*
 Reminder: these parts of the std::string interface should be implemented:

 https://en.cppreference.com/w/cpp/string/basic_string/basic_string

 ✔︎ Fl_String::npos = maximum size value
 ✔︎ Fl_String() = constructs an empty string
 ✔︎ Fl_String(const Fl_String other) = copy
   Fl_String(const Fl_String other, int pos) = copy, starting at pos (clips to other)
   Fl_String(const Fl_String other, int pos, int n) = copy n bytes, starting at pos (clips to other)
 ✔︎ Fl_String(const char*, int n) = copy n characters, can contain NUL
 ✔︎ Fl_String(const char*) = copy up to NUL
 ? Fl_String(int cap) = create an empty string with that capacity (not C++ std)

 ✔︎ operator=(const Fl_String) = copy
 ✔︎ operator=(const char *) = copy

   char at(int pos) const = get character at pos, out of range returns 0
 ✔︎ char [](int pos) const = get character at, no bounds check
 ✔︎ char &[](int pos) = get reference to character at, no bounds check
 ✔︎ char *data() = return a pointer to the buffer that is always NUL terminated even if empty()
 ✔︎ const char *c_str() = return a pointer to the buffer that is always NUL terminated even if empty()

 ✔︎ bool empty() = return true if the string has no characters
 ✔︎ size_type size() const = number of bytes in buffer including in-sequnce NUL, but not trailing NUL
 ? size_type strlen() const = strlen(c_str()) (not C++ std)
 ✔︎ void reserve(int n) = make buffer at least n bytes plus possible trailing NUL
 ✔︎ size_t capacity() = room in buffer plus possible trailing NUL
 ✔︎ void shrink_to_fit() = make buffer as big as content

 ✔︎ void clear()
   String insert(...) - many variants
   String erase(int pos, int cnt)
   void push_back(char)
   char pop_back() = returns char, not C++ std
   append(...) - many variants like insert
   +=
   replace(...) - many variants
   Fl_String substr(int pos, int n)
 ✔︎ resize(int n)

   operator+
   operator==
 */

#endif // _FL_Fl_String_H_
