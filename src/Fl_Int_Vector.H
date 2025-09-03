//
// An STL-ish vector without templates for the Fast Light Tool Kit (FLTK).
//
// Copyright 2002 by Greg Ercolano.
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

#ifndef Fl_Int_Vector_H
#define Fl_Int_Vector_H

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

#include <FL/Fl_Export.H>

/** \file src/Fl_Int_Vector.H
  An STL-ish vector implemented without templates.
*/

/**
  An STL-ish vector without templates.

  Handles dynamic memory management of an integer array, and allows
  array elements to be accessed with zero based indexing: v[0], v[1]..

  Common use:
  \code
  #include <stdio.h>
  #include "Fl_Int_Vector.H"
  int main() {
      Fl_Int_Vector v;

      // Create an array of values 11,22,33:
      v.push_back(11);   // add first element
      v.push_back(22);   // add second element
      v.push_back(33);   // add third element

      // Assignment by subscript
      v[1] = 222;        // changes 2nd element from 22 to 222

      // Loop through printing the values
      for ( unsigned int i=0; i<v.size(); i++ )
          printf("%d ", v[i]);  // access the elements
      printf("\n");

      // Clear the array
      v.size(0);
  }
  \endcode

  \todo
      - Note: this class is only for internal use and deprecated by design.
        It will be replaced with std::vector<int> in the next version after 1.4.x.
      - Add other std::vector methods like erase(), etc.
      - Make memory blocking size flexible, and add related methods like capacity(), reserve(), shrink_to_fit(), etc.
      - Add non-std methods that are nevertheless needed, e.g. insert(index,val), delete(index), delete(start, end), swap(a_idx,b_idx)
*/
class FL_EXPORT Fl_Int_Vector {
  int *arr_;
  unsigned int size_;

  /**
    Initialize internals.
    Private: For use internally by the class's ctors only.
  */
  void init() {
    arr_ = 0;
    size_ = 0;
  }
  void copy(int *newarr, unsigned int newsize);

public:
  /** Create an empty vector of integers. */
  Fl_Int_Vector() {
    init();
  }

  ~Fl_Int_Vector();

  /** Copy constructor. */
  Fl_Int_Vector(Fl_Int_Vector &o) {
    init();
    copy(o.arr_, o.size_);
  }

  /**
    Assignment operator. Similar to the copy constructor,
    creates a separate copy of the source array, freeing any
    previous contents in the current integer array.
  */
  Fl_Int_Vector &operator=(Fl_Int_Vector &o) {
    init();
    copy(o.arr_, o.size_);
    return *this;
  }

  /**
    Access the specified integer element at index position \p x.
    \warning No range checking is done on \p x, which must be less than size().
  */
  int operator[](int x) const {
    return arr_[x];
  }

  /**
    Access the specified integer element at index position \p x as a reference.

    This allows assignment by index through the returned reference, e.g. arr[1] = 222;
    where arr[1] ends up being a reference to ptr[1], and then 222 is assigned to that ref.

    \warning No range checking is done on \p x, which must be less than size().
  */
  int &operator[](int x) {
    return arr_[x];
  }

  /** Return the number of integer elements in the array. */
  unsigned int size() const {
    return size_;
  }

  void size(unsigned int count);

  /**
    Removes the last element the last element and returns its value.

    \warning You must not call pop_back() if the array is empty, i.e. if (size() == 0).
    \todo Internals should maybe assert(size_ != 0)
  */
  int pop_back() {
    int tmp = arr_[size_ - 1];
    size_--;
    return tmp;
  }

  /** Appends \p val to the array, enlarging the array by one. */
  void push_back(int val) {
    unsigned int x = size_;
    size(size_ + 1);
    arr_[x] = val;
  }

  /**
    Return the last element in the array.
    \warning You must not call back() if the array is empty, i.e. if (size() == 0).
    \todo Internals should maybe assert(size_ != 0)
  */
  int back() const {
    return arr_[size_ - 1];
  }

  /**
    Checks if array has no elements.
    Same as a test for (size() == 0).
  */
  bool empty() const {
    return (size_ == 0) ? true : false;
  }
};

/**
\}
\endcond
*/

#endif // Fl_Int_Vector_H
