//
// Node class debug code for the Fast Light Tool Kit (FLTK).
//
// Copyright 2026 by Bill Spitzak and others.
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

#include "nodes/Node_debug.h"

#ifndef NDEBUG

#include "nodes/Node.h"

#include "Fluid.h"
#include "Project.h"

#include <stdlib.h>
#include <stdio.h>

#endif


#ifndef NDEBUG
/**
 Print the current project tree to stderr.
 */
void print_project_tree() {
  fprintf(stderr, "---- %s --->\n", Fluid.proj.projectfile_name().c_str());
  for (Node *t = Fluid.proj.tree.first; t; t = t->next) {
    for (int i = t->level; i > 0; i--)
      fprintf(stderr, ". ");
    fprintf(stderr, "%s\n", subclassname(t).c_str());
  }
}
#endif

#ifndef NDEBUG
/**
 Check the validity of the project tree.

 Write problems with the project tree to stderr.

 \return true if the project tree is valid
 */
bool validate_project_tree() {
  // Validate `first` and `last`
  if (Fluid.proj.tree.first == nullptr) {
    if (Fluid.proj.tree.last == nullptr) {
      return true;
    } else {
      fprintf(stderr, "ERROR: `first` is nullptr, but `last` is not!\n");
      return false;
    }
  }
  if (Fluid.proj.tree.last == nullptr) {
    fprintf(stderr, "ERROR: `last` is nullptr, but `first` is not!\n");
    return false;
  }
  // Validate the branch linkage, parent links, etc.
  return validate_branch(Fluid.proj.tree.first);
}
#endif

#ifndef NDEBUG
/**
 Check the validity of a Type branch that is not connected to the project.

 Write problems with the branch to stderr.

 \param[in] root the first node in a branch
 \return true if the branch is correctly separated and valid
 */
bool validate_independent_branch(class Node *root) {
  // Make sure that `first` and `last` do not point at any node in this branch
  if (Fluid.proj.tree.first) {
    for (Node *t = root; t; t = t->next) {
      if (Fluid.proj.tree.first == t) {
        fprintf(stderr, "ERROR: Branch is not independent, `first` is pointing to branch member!\n");
        return false;
      }
    }
  }
  if (Fluid.proj.tree.last) {
    for (Node *t = root; t; t = t->next) {
      if (Fluid.proj.tree.last == t) {
        fprintf(stderr, "ERROR: Branch is not independent, `last` is pointing to branch member!\n");
        return false;
      }
    }
  }
  // Validate the branch linkage, parent links, etc.
  return validate_branch(root);
}
#endif

#ifndef NDEBUG
/**
 Check the validity of a Type branch.

 Write problems with the branch to stderr.

 \param[in] root the first node in a branch
 \return true if the branch is valid
 */
bool validate_branch(class Node *root) {
  // Only check real branches
  if (!root) {
    fprintf(stderr, "WARNING: Branch is empty!\n");
    return false;
  }
  // Check relation between this and next node
  for (Node *t = root; t; t = t->next) {
    if (t->level < root->level) {
      fprintf(stderr, "ERROR: Node in tree is above root level!\n");
      return false;
    }
    if (t->next) {
      // Make sure that all `next` types have the `prev` member link back
      if (t->next->prev != t) {
        fprintf(stderr, "ERROR: Doubly linked list broken!\n");
        return false;
      }
      if (t->next->level > t->level) {
        // Validate `level` changes
        if (t->next->level - t->level > 1) {
          fprintf(stderr, "ERROR: Child level increment greater than one!\n");
          return false;
        }
        // Ensure that this node can actually have children
        if (!t->can_have_children()) {
          fprintf(stderr, "ERROR: This parent must not have children!\n");
          return false;
        }
      }
    }
    // Validate the `parent` entry
    for (Node *p = t->prev; ; p = p->prev) {
      if (p == nullptr) {
        if (t->parent != nullptr) {
          fprintf(stderr, "ERROR: `parent` pointer should be nullptr!\n");
          return false;
        }
        break;
      }
      if (p->level < t->level) {
        if (t->parent != p) {
          fprintf(stderr, "ERROR: `parent` points to wrong parent!\n");
          return false;
        }
        break;
      }
    }
  }
  return true;
}
#endif
