//
// Node Tree code for the Fast Light Tool Kit (FLTK).
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


#include "nodes/Tree.h"

#include "Project.h"

using namespace fld;
using namespace fld::node;


Tree::Iterator::Iterator(Node *t, bool only_selected)
: type_(t)
, only_selected_(only_selected)
{
  if (t) {
    if (only_selected_) {
      if (!type_->selected) {
        operator++();
      }
    }
  }
}

Tree::Iterator &Tree::Iterator::operator++() {
  if (only_selected_) {
    do {
      type_ = type_->next;
    } while (type_ && !type_->selected);
  } else {
    type_ = type_->next;
  }
  return *this;
}

Tree::WIterator::WIterator(Node *t, bool only_selected)
: type_(t)
, only_selected_(only_selected)
{
  if (t) {
    if (only_selected_) {
      if (!type_->selected || !type_->is_widget()) {
        operator++();
      }
    } else {
      if (!type_->is_widget()) {
        operator++();
      }
    }
  }
}

Tree::WIterator& Tree::WIterator::operator++() {
  if (only_selected_) {
    do {
      type_ = type_->next;
    } while (type_ && (!type_->selected || !type_->is_widget()));
  } else {
    do {
      type_ = type_->next;
    } while (type_ && !type_->is_widget());
  }
  return *this;
}


Tree::Tree(Project &proj)
: proj_(proj)
{ (void)proj_; }


/** Find a node by its unique id.

 Every node in a type tree has an id that is unique for the current project.
 Walk the tree and return the node with this uid.

 \param[in] uid any number between 0 and 65535
 \return the node with this uid, or nullptr if not found
 */
Node *Tree::find_by_uid(unsigned short uid) {
  for (auto tp: all_nodes()) {
    if (tp->get_uid() == uid) return tp;
  }
  return nullptr;
}


/** Find a type node by using the codeview text positions.

 \param[in] text_type 0=source file, 1=header, 2=.fl project file
 \param[in] crsr cursor position in text
 \return the node we found or nullptr
 */
Node *Tree::find_in_text(int text_type, int crsr) {
  for (auto node: all_nodes()) {
    switch (text_type) {
      case 0:
        if (crsr >= node->code1_start && crsr < node->code1_end) return node;
        if (crsr >= node->code2_start && crsr < node->code2_end) return node;
        if (crsr >= node->code_static_start && crsr < node->code_static_end) return node;
        break;
      case 1:
        if (crsr >= node->header1_start && crsr < node->header1_end) return node;
        if (crsr >= node->header2_start && crsr < node->header2_end) return node;
        if (crsr >= node->header_static_start && crsr < node->header_static_end) return node;
        break;
      case 2:
        if (crsr >= node->proj1_start && crsr < node->proj1_end) return node;
        if (crsr >= node->proj2_start && crsr < node->proj2_end) return node;
        break;
    }
  }
  return nullptr;
}
