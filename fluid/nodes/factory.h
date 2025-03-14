//
// Node Factory header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

#ifndef FLUID_NODES_FACTORY_H
#define FLUID_NODES_FACTORY_H

#include "nodes/Node.h"

struct Fl_Menu_Item;

extern Fl_Menu_Item New_Menu[];

void fill_in_New_Menu();
Node *typename_to_prototype(const char *inName);

Node *add_new_widget_from_file(const char *inName, Strategy strategy);
Node *add_new_widget_from_user(Node *inPrototype, Strategy strategy, bool and_open=true);
Node *add_new_widget_from_user(const char *inName, Strategy strategy, bool and_open=true);


#endif // FLUID_NODES_FACTORY_H
