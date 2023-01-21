//
// Scheme implementation for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022-2023 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Scheme.H>
#include <FL/fl_string_functions.h> // fl_strdup

#include <stdlib.h>         // malloc, realloc

const char **Fl_Scheme::names_ = NULL;
int Fl_Scheme::num_schemes_    = 0;
int Fl_Scheme::alloc_size_     = 0;

/**
  Return a list of all known scheme names.

  This list is only valid until a new scheme is added or one is removed.
  It is possible that scheme names are appended to the list during the
  runtime of the program but schemes can't be removed.

  Getting the list of known schemes can be useful to populate a menu of
  scheme choices to let the user select a scheme. You should process the
  names immediately and you should never store a pointer to the list or
  any individual name for later reference because the location of the
  list can be changed (reallocated) when schemes are added.

  The list of scheme names is nul-terminated.

  \note
    Currently (in FLTK 1.4.0) schemes can only be added to the list and
    not removed from the list. This may change in a later version.

  \return  List of currently known scheme names.
*/

const char **Fl_Scheme::names() {
  if (names_)
    return names_;

  alloc_size_ = 8;
  names_ = (const char **)malloc(alloc_size_ * sizeof(const char *));

  // FIXME: register "known" scheme names ...

  add_scheme_name("base");
  add_scheme_name("plastic");
  add_scheme_name("gtk+");
  add_scheme_name("gleam");
  add_scheme_name("oxy");

  return names_;
}

/**
  Add a scheme name to the list of known schemes.

  This method is public in FLTK 1.4.0 because derived classes of Fl_Scheme
  are not yet implemented. Thus, users implementing their own schemes can
  use this method to add the scheme name to the list of known schemes
  which is for instance used in Fl_Scheme::names().

  \note \b Attention!
    In a future version, when subclasses of Fl_Scheme will be implemented,
    this method will either be replaced by another \p protected method or
    it will no longer do anything (kept only for ABI reasons).

  The new scheme name must consist of valid ASCII characters as described
  below:
  - lowercase letters \p 'a' - \p 'z'
  - numbers \p '0' - \p '9'
  - any character in \p "$+_." (w/o the quotes).

  The name must not be longer than 12 ASCII characters (bytes).
  The new scheme name is added to the \b end of the \b unordered list.

  \note Call this method only once for each scheme name. If the returned
    value is \<= 0 you should check the scheme name.

  The given scheme \p name is copied and may be freed directly after the
  call to add_scheme_name().

  \param[in]  name  New scheme name

  \returns  The new number of schemes if the name was successfully added.
            This is the same as the index of the scheme + 1.
  \retval   0   Scheme \p name already exists
  \retval  -1   Invalid character(s) in \p name
  \retval  -2   The \p name is too long

  \since 1.4.0
*/
int Fl_Scheme::add_scheme_name(const char *name) {

  static const char valid_chars[] = "$+_.";

  // test if the scheme name is valid

  int nlen = static_cast<int>(strlen(name));

  if (nlen > 12)
    return (-2);

  for (int i = 0; i < nlen; i++) {
    if ((name[i] >= 'a' && name[i] <= 'z') ||
        (name[i] >= '0' && name[i] <= '9'))
      continue;
    if (!strchr(valid_chars, name[i]))
      return (-1);
  }

  // Test if the scheme name already exists.
  // We know already that it consists only of valid characters,
  // hence we can use the faster strcmp() for comparison.

  const char **s = names();
  for (int i = 0; i < num_schemes_; i++) {
    if (strcmp(name, s[i]) == 0)
      return 0;
  }

  // The scheme name is OK, we can add it. Take care that we need
  // a null pointer at the end of the list.

  num_schemes_++;

  if (num_schemes_ + 1 > alloc_size_) { // overflow, extend the list
    alloc_size_ += 8;
    names_ = (const char **)realloc(names_, alloc_size_ * sizeof(const char *));
  }

  names_[num_schemes_-1] = fl_strdup(name);
  names_[num_schemes_] = NULL;

  return num_schemes_;
}
