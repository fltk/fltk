//
// Widget type header file for the Fast Light Tool Kit (FLTK).
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

#ifndef _FLUID_FACTORY_H
#define _FLUID_FACTORY_H

#include "Fl_Type.h"

struct Fl_Menu_Item;

extern Fl_Menu_Item New_Menu[];

void fill_in_New_Menu();
Fl_Type *typename_to_prototype(const char *inName);

Fl_Type *add_new_widget_from_file(const char *inName, Strategy strategy);
Fl_Type *add_new_widget_from_user(Fl_Type *inPrototype, Strategy strategy);
Fl_Type *add_new_widget_from_user(const char *inName, Strategy strategy);


#endif // _FLUID_FACTORY_H
