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

#include "nodes/Fl_Widget_Type.h"

class Fl_Type;

namespace fld {

class Project;

namespace node {

class Tree {

  // A class that can iterate over the entire scene graph.
  class Iterator {
    Fl_Type *type_ = nullptr;
  public:
    explicit Iterator(Fl_Type *t) : type_(t) {}
    Fl_Type& operator*() { return *type_; }
    Iterator& operator++() { type_ = type_->next; return *this; }
    bool operator!=(const Iterator& other) const { return type_ != other.type_; }
  };

  // A container for a node iterator
  class Container {
    Tree &tree_;
  public:
    Container(Tree &tree) : tree_ { tree } { }
    Iterator begin() { return Iterator(tree_.first); }
    Iterator end() { return Iterator(nullptr); }
  };

  // A class that iterate over the scene graph, but returns only nodes of type widget.
  class WIterator {
    Fl_Type *type_ = nullptr;
  public:
    explicit WIterator(Fl_Type *t) : type_(t) {}
    Fl_Widget_Type& operator*() { return *static_cast<Fl_Widget_Type*>(type_); }
    WIterator& operator++() { do { type_ = type_->next; } while (type_ && !type_->is_widget()); return *this; }
    bool operator!=(const WIterator& other) const { return type_ != other.type_; }
  };

  // A container for a widget node iterator
  class WContainer {
    Tree &tree_;
  public:
    WContainer(Tree &tree) : tree_ { tree } { }
    WIterator begin() { return WIterator(tree_.first); }
    WIterator end() { return WIterator(nullptr); }
  };

  /// Link Tree class to the project.
  Project &proj_;

public:

  Fl_Type *first = nullptr;
  Fl_Type *last = nullptr;
  Fl_Type *current = nullptr;  // most recently picked object
  Fl_Type *current_dnd = nullptr;
  /// If this is greater zero, widgets will be allowed to lay out their children.
  int allow_layout = 0;

public:

  Tree(Project &proj);

  // Iterators: `for (auto &n: tree.all_nodes()) { n.print(); }
  Container all_nodes() { return Container(*this); }
  WContainer all_widgets() { return WContainer(*this); }

//  Fl_Type *find_by_uid(unsigned short uid);
//  Fl_Type *find_in_text(int text_type, int crsr);
};

} // namespace node
} // namespace fld


#endif // FLUID_NODES_TREE_H
