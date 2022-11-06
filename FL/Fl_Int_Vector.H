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

#ifndef Fl_Int_Vector_H
#define Fl_Int_Vector_H

#include <FL/Fl.H>

class FL_EXPORT Fl_Int_Vector {
  int *arr_;
  unsigned int size_;
  void init() {
    arr_ = 0;
    size_ = 0;
  }
  void copy(int *newarr, unsigned int newsize);

public:
  Fl_Int_Vector() {
    init();
  }

  ~Fl_Int_Vector();

  // copy constructor
  Fl_Int_Vector(Fl_Int_Vector &o) {
    init();
    copy(o.arr_, o.size_);
  }

  // assignment operator
  Fl_Int_Vector &operator=(Fl_Int_Vector &o) {
    init();
    copy(o.arr_, o.size_);
    return *this;
  }

  int operator[](int x) const {
    return arr_[x];
  }

  int &operator[](int x) {
    return arr_[x];
  }

  unsigned int size() {
    return size_;
  }

  void size(unsigned int count);

  int pop_back() {
    int tmp = arr_[size_ - 1];
    size_--;
    return tmp;
  }

  void push_back(int val) {
    unsigned int x = size_;
    size(size_ + 1);
    arr_[x] = val;
  }

  int back() {
    return arr_[size_ - 1];
  }
};

#endif // Fl_Int_Vector_H
