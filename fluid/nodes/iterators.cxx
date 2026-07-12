//
// Node tree iterators code for the Fast Light Tool Kit (FLTK).
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

#include "nodes/iterators.h"

#include "nodes/Node.h"
#include "nodes/Widget_Node.h"
#include "nodes/Tree.h"

using namespace fluid::node;

// ---- Node_Range ----------------------------------------------- MARK: -

Node_Range::Iterator::Iterator(Node *n, bool only_selected)
: n_(n)
, only_selected_(only_selected)
{
  if (n_ && only_selected_ && !n_->selected)
    operator++();
}

Node_Range::Iterator &Node_Range::Iterator::operator++() {
  if (only_selected_) {
    do {
      n_ = n_->next;
    } while (n_ && !n_->selected);
  } else {
    n_ = n_->next;
  }
  return *this;
}

Node_Range::Iterator Node_Range::begin() const {
  return Iterator(tree_.first, only_selected_);
}

// ---- Widget_Node_Range ------------------------------------------ MARK: -

Widget_Node_Range::Iterator::Iterator(Node *n, bool only_selected)
: n_(n)
, only_selected_(only_selected)
{
  if (n_) {
    if (only_selected_) {
      if (!n_->selected || !n_->is_widget())
        operator++();
    } else {
      if (!n_->is_widget())
        operator++();
    }
  }
}

Widget_Node* Widget_Node_Range::Iterator::operator*() const {
  return static_cast<Widget_Node*>(n_);
}

Widget_Node_Range::Iterator& Widget_Node_Range::Iterator::operator++() {
  if (only_selected_) {
    do {
      n_ = n_->next;
    } while (n_ && (!n_->selected || !n_->is_widget()));
  } else {
    do {
      n_ = n_->next;
    } while (n_ && !n_->is_widget());
  }
  return *this;
}

Widget_Node_Range::Iterator Widget_Node_Range::begin() const {
  return Iterator(tree_.first, only_selected_);
}

// ---- Child_Range ------------------------------------------------ MARK: -

Child_Range::Iterator& Child_Range::Iterator::operator++() {
  n_ = n_->next_sibling();
  return *this;
}

// ---- Descendant_Range -------------------------------------------- MARK: -

Descendant_Range::Iterator& Descendant_Range::Iterator::operator++() {
  n_ = n_->next;
  if (n_ && n_->level <= base_->level) n_ = nullptr;
  return *this;
}

Descendant_Range::Iterator Descendant_Range::begin() const {
  Node *n = base_->next;
  if (n && n->level <= base_->level) n = nullptr;
  return Iterator(base_, n);
}
