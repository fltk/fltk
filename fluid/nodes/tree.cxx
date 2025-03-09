//
// Widget type code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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


#include "nodes/tree.h"

#include "Project.h"

using namespace fld;
using namespace fld::node;


Tree::Tree(Project &proj)
: proj_(proj)
{ }


///** Find a node by its unique id.
//
// Every node in a type tree has an id that is unique for the current project.
// Walk the tree and return the node with this uid.
//
// \param[in] uid any number between 0 and 65535
// \return the node with this uid, or NULL if not found
// */
//Fl_Type *Fl_Type::find_by_uid(unsigned short uid) {
//  for (Fl_Type *tp = Fl_Type::first; tp; tp = tp->next) {
//    if (tp->uid_ == uid) return tp;
//  }
//  return NULL;
//}
//
///** Find a type node by using the codeview text positions.
//
// \param[in] text_type 0=source file, 1=header, 2=.fl project file
// \param[in] crsr cursor position in text
// \return the node we found or NULL
// */
//Fl_Type *Fl_Type::find_in_text(int text_type, int crsr) {
//  for (Fl_Type *node = first; node; node = node->next) {
//    switch (text_type) {
//      case 0:
//        if (crsr >= node->code1_start && crsr < node->code1_end) return node;
//        if (crsr >= node->code2_start && crsr < node->code2_end) return node;
//        if (crsr >= node->code_static_start && crsr < node->code_static_end) return node;
//        break;
//      case 1:
//        if (crsr >= node->header1_start && crsr < node->header1_end) return node;
//        if (crsr >= node->header2_start && crsr < node->header2_end) return node;
//        if (crsr >= node->header_static_start && crsr < node->header_static_end) return node;
//        break;
//      case 2:
//        if (crsr >= node->proj1_start && crsr < node->proj1_end) return node;
//        if (crsr >= node->proj2_start && crsr < node->proj2_end) return node;
//        break;
//    }
//  }
//  return 0;
//}

