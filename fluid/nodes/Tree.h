//
// Node Tree header file for the Fast Light Tool Kit (FLTK).
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

#include "nodes/iterators.h"

class Node;

namespace fluid {

class Project;

namespace node {

class Tree {

  /// Link Tree class to the project.
  Project &proj_;

public:

  Node *first = nullptr;
  Node *last = nullptr;
  Node *current = nullptr;  // most recently picked object
  Node *current_dnd = nullptr;
  /// If this is greater zero, widgets will be allowed to lay out their children.
  int allow_layout = 0;

public:

  Tree(Project &proj);

  bool empty() { return first == nullptr; }

  // Iterators: `for (auto &n: tree.all_nodes()) { n.print(); }
  Node_Range all_nodes() { return Node_Range(*this, false); }
  Widget_Node_Range all_widgets() { return Widget_Node_Range(*this, false); }
  Node_Range all_selected_nodes() { return Node_Range(*this, true); }
  Widget_Node_Range all_selected_widgets() { return Widget_Node_Range(*this, true); }

  Node *find_by_uid(unsigned short uid);
  Node *find_in_text(int text_type, int crsr);
};

} // namespace node
} // namespace fluid


#endif // FLUID_NODES_TREE_H
