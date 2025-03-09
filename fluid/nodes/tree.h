//
// Widget type header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

#ifndef FLUID_NODES_TREE_H
#define FLUID_NODES_TREE_H

namespace fld {

class Project;

namespace node {

class Fl_Type;

class Tree {
  /// Link Tree class to the project.
  Project &proj_;

public:

  Fl_Type *first = nullptr;
  Fl_Type *last = nullptr;
//  Fl_Type *current;  // most recently picked object
//  Fl_Type *current_dnd;
//  Fl_Type *find_by_uid(unsigned short uid);
//  Fl_Type *find_in_text(int text_type, int crsr);
//  int allow_layout;

public:

  Tree(Project &proj);
};

} // namespace node
} // namespace fld


#endif // FLUID_NODES_TREE_H
