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
#include "nodes/Node.h"
#include "nodes/Widget_Node.h"
#include "widgets/Node_Browser.h"

using namespace fluid;
using namespace fluid::node;


Tree::Tree(Project &proj)
: proj_(proj)
{ (void)proj_; }


/**
 Delete all nodes in the tree.
 Refreshes the widget browser and resets the selection.
 */
void Tree::delete_all_nodes() {
  if (widget_browser) {
    widget_browser->new_list();
  }
  for (Node *f = first; f;) {
    f->delete_children();
    Node *g = f->next;
    delete f;
    f = g;
  }
  if (widget_browser) {
    widget_browser->hposition(0);
    widget_browser->vposition(0);
  }
  selection_changed(nullptr);
  if (widget_browser) {
    widget_browser->rebuild();
  }
}

/**
  Delete all selected nodes in the tree.
  Refreshes the widget browser and resets the selection.
 */
void Tree::delete_selected_nodes() {
  if (widget_browser) {
    widget_browser->save_scroll_position();
    widget_browser->new_list();
  }
  for (Node *f = first; f;) {
    if (f->selected) {
      f->delete_children();
      Node *g = f->next;
      delete f;
      f = g;
    } else {
      f = f->next;
    }
  }
  selection_changed(nullptr);
  if (widget_browser) {
    widget_browser->restore_scroll_position();
    widget_browser->rebuild();
  }
}

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


/** Find a node by using the codeview text positions.

 \param[in] text_type 0=source file, 1=header, 2=.fl project file
 \param[in] crsr cursor position in text
 \return the node we found or nullptr
 */
Node *Tree::find_in_text(int text_type, int crsr) {
  for (auto node: all_nodes()) {
    switch (text_type) {
      case 0:
        if (crsr >= node->setup_node.c.start && crsr < node->setup_node.c.end) return node;
        if (crsr >= node->finalize_node.c.start && crsr < node->finalize_node.c.end) return node;
        if (crsr >= node->static_data.c.start && crsr < node->static_data.c.end) return node;
        break;
      case 1:
        if (crsr >= node->setup_node.h.start && crsr < node->setup_node.h.end) return node;
        if (crsr >= node->finalize_node.h.start && crsr < node->finalize_node.h.end) return node;
        if (crsr >= node->static_data.h.start && crsr < node->static_data.h.end) return node;
        break;
      case 2:
        if (crsr >= node->proj1.start && crsr < node->proj1.end) return node;
        if (crsr >= node->proj2.start && crsr < node->proj2.end) return node;
        break;
    }
  }
  return nullptr;
}
