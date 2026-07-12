//
// Node tree iterators header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2026 by Bill Spitzak and others.
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

#ifndef FLUID_NODES_ITERATORS_H
#define FLUID_NODES_ITERATORS_H

class Node;
class Widget_Node;

namespace fluid {
namespace node {

class Tree;

/**
 Range over every Node in a project tree, in list order
 (`for (auto *n : tree.all_nodes())`).
 Also used, with only_selected true, for `Tree::all_selected_nodes()`.
 */
class Node_Range {
  Tree &tree_;
  bool only_selected_;
public:
  class Iterator {
    Node *n_;
    bool only_selected_;
  public:
    explicit Iterator(Node *n, bool only_selected);
    Node* operator*() const { return n_; }
    Iterator& operator++();
    bool operator!=(const Iterator& other) const { return n_ != other.n_; }
  };
  Node_Range(Tree &tree, bool only_selected) : tree_(tree), only_selected_(only_selected) { }
  Iterator begin() const;
  Iterator end() const { return Iterator(nullptr, only_selected_); }
};

/**
 Range over every Widget_Node in a project tree, in list order, skipping
 non-widget nodes (`for (auto *w : tree.all_widgets())`).
 Also used, with only_selected true, for `Tree::all_selected_widgets()`.
 */
class Widget_Node_Range {
  Tree &tree_;
  bool only_selected_;
public:
  class Iterator {
    Node *n_;
    bool only_selected_;
  public:
    explicit Iterator(Node *n, bool only_selected);
    Widget_Node* operator*() const;
    Iterator& operator++();
    bool operator!=(const Iterator& other) const { return n_ != other.n_; }
  };
  Widget_Node_Range(Tree &tree, bool only_selected) : tree_(tree), only_selected_(only_selected) { }
  Iterator begin() const;
  Iterator end() const { return Iterator(nullptr, only_selected_); }
};

} // namespace node
} // namespace fluid

/**
 Range over the direct children of a Node (`for (auto *c : n->children())`).
 */
class Child_Range {
  Node *first_;
public:
  class Iterator {
    Node *n_;
  public:
    explicit Iterator(Node *n) : n_(n) { }
    Node* operator*() const { return n_; }
    Iterator& operator++();
    bool operator!=(const Iterator& other) const { return n_ != other.n_; }
  };
  explicit Child_Range(Node *first) : first_(first) { }
  Iterator begin() const { return Iterator(first_); }
  Iterator end() const { return Iterator(nullptr); }
};

/**
 Range over all descendants of a Node, depth-first
 (`for (auto *d : n->descendants())`).
 */
class Descendant_Range {
  Node *base_;
public:
  class Iterator {
    Node *base_;
    Node *n_;
  public:
    Iterator(Node *base, Node *n) : base_(base), n_(n) { }
    Node* operator*() const { return n_; }
    Iterator& operator++();
    bool operator!=(const Iterator& other) const { return n_ != other.n_; }
  };
  explicit Descendant_Range(Node *base) : base_(base) { }
  Iterator begin() const;
  Iterator end() const { return Iterator(base_, nullptr); }
};

#endif // FLUID_NODES_ITERATORS_H
