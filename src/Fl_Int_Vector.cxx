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

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

#include "Fl_Int_Vector.H"
#include <stdlib.h>
#include <string.h>

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

  Setting size to zero clears the array and frees any memory it used.

  Shrinking truncates the array and frees memory of truncated elements.
  Enlarging creates new elements that are zero in value.
*/
void Fl_Int_Vector::size(unsigned int count) {
  if (count == 0) {             // zero? special case frees memory
    if (arr_)
      free(arr_);
    arr_ = 0;
    size_ = 0;
    return;
  }
  if (count > size_) {          // array enlarged? realloc + init new vals to 0
    arr_ = (int *)realloc(arr_, count * sizeof(int));
    while ( size_ < count ) {
      arr_[size_++] = 0;
    }
    return;                     // leaves with size_ == count
  }
  // count <= size_? just truncate
  size_ = count;
}

/**
\}
\endcond
*/
