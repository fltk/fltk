//
// Fluid file routines for the Fast Light Tool Kit (FLTK).
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

#ifndef _FLUID_FILE_H
#define _FLUID_FILE_H

#include "Fl_Type.h"

#include <FL/fl_attr.h>

extern double read_version;
extern int fdesign_flip;
extern int fdesign_magic;

void write_word(const char *);
void write_string(const char *,...) __fl_attr((__format__ (__printf__, 1, 2)));
void write_indent(int n);
void write_open(int);
void write_close(int n);

void read_error(const char *format, ...);
const char *read_word(int wantbrace = 0);

int write_file(const char *, int selected_only = 0);

int read_file(const char *, int merge, Strategy strategy=kAddAsLastChild);
void read_fdesign();

#endif // _FLUID_FILE_H
