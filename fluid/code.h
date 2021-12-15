//
// Code output routines for the Fast Light Tool Kit (FLTK).
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

#ifndef _FLUID_CODE_H
#define _FLUID_CODE_H

#include <FL/fl_attr.h>

#include <stdarg.h>

extern int indentation;
extern int write_number;
extern int write_sourceview;

int is_id(char c);
const char* unique_id(void* o, const char*, const char*, const char*);
const char *indent();
const char *indent(int set);
const char *indent_plus(int offset);
int write_declare(const char *, ...) __fl_attr((__format__ (__printf__, 1, 2)));
void write_cstring(const char *,int length);
void write_cstring(const char *);
void write_cdata(const char *,int length);
void vwrite_c(const char* format, va_list args);
void write_c(const char*, ...) __fl_attr((__format__ (__printf__, 1, 2)));
void write_cc(const char *, int, const char*, const char*);
void write_h(const char*, ...) __fl_attr((__format__ (__printf__, 1, 2)));
void write_hc(const char *, int, const char*, const char*);
void write_c_indented(const char *textlines, int inIndent, char inTrailwWith);
int write_code(const char *cfile, const char *hfile);
int write_strings(const char *sfile);
void write_public(int state); // writes pubic:/private: as needed

#endif // _FLUID_CODE_H
