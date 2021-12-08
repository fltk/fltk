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

struct Fl_Menu_Item;
class Fl_Type;

extern Fl_Menu_Item New_Menu[];

void fill_in_New_Menu();
Fl_Type *Fl_Type_make(const char *tn);


#endif // _FLUID_FACTORY_H
