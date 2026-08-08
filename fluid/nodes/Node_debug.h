//
// Node class debug header file for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_NODES_NODE_DEBUG_H
#define FLUID_NODES_NODE_DEBUG_H

#ifndef NDEBUG

#include "nodes/Widget_Node.h"

void print_project_tree();
bool validate_project_tree();
bool validate_independent_branch(class Node *root);
bool validate_branch(class Node *root);

#endif

#endif // FLUID_NODES_NODE_DEBUG_H
