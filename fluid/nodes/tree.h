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

class Fl_Type;

namespace fld {

class Project;

namespace node {

class Tree {
  /// Link Tree class to the project.
  Project &proj_;

public:

  Fl_Type *first = nullptr;
  Fl_Type *last = nullptr;
  Fl_Type *current = nullptr;  // most recently picked object
  Fl_Type *current_dnd = nullptr;
  /// If this is greater zero, widgets will be allowed to lay out their children.
  int allow_layout = 0;

  //  Fl_Type *find_by_uid(unsigned short uid);
//  Fl_Type *find_in_text(int text_type, int crsr);

public:

  Tree(Project &proj);
};

} // namespace node
} // namespace fld


#endif // FLUID_NODES_TREE_H
