//
// An STL-ish vector without templates for the Fast Light Tool Kit (FLTK).
//
// Copyright 2002 by Greg Ercolano.
// Copyright 2022 by Bill Spitzak and others.
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

#include <FL/Fl_Int_Vector.H>
#include <stdlib.h>

/**
  Make a copy of another array.
  Private: For use internally by the class's copy ctors only.
*/
void Fl_Int_Vector::copy(int *newarr, unsigned int newsize) {
  size(newsize);
  memcpy(arr_, newarr, newsize * sizeof(int));
}

/** Destructor - frees the internal array and destroys the class. */
Fl_Int_Vector::~Fl_Int_Vector() {
  if (arr_)
    free(arr_);
}

/**
  Set the size of the array to \p count.

  A size of zero empties the array completely and frees all memory.

  \warning
    - Only advised use currently is to shrink the array size, i.e. (count < size()).
    - Currently enlarging the array leaves the new values uninitialized.
    - When assignment via indexes is supported, i.e. v[x] = 123, array enlargement should zero new values
  \todo Check if count > size, and if so init new values to 0.
*/
void Fl_Int_Vector::size(unsigned int count) {
  if (count <= 0) {
    if (arr_)
      free(arr_);
    arr_ = 0;
    size_ = 0;
    return;
  }
  if (count > size_) {
    arr_ = (int *)realloc(arr_, count * sizeof(int));
    size_ = count;
  }
}
