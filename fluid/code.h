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
#include "../src/Fl_String.H"

#include <stdarg.h>
#include <stdio.h>

class Fl_Type;
struct Fd_Identifier_Tree;
struct Fd_Text_Tree;
struct Fd_Pointer_Tree;

int is_id(char c);
int write_strings(const Fl_String &filename);

class Fd_Code_Writer
{
protected:
  /// file pointer for the C++ code file
  FILE *code_file;
  /// file pointer for the C++ header file
  FILE *header_file;

  /// tree of unique but human-readable identifiers
  Fd_Identifier_Tree* id_root;
  /// searchable text tree for text that is only written once to the header file
  Fd_Text_Tree *text_in_header;
  /// searchable text tree for text that is only written once to the code file
  Fd_Text_Tree *text_in_code;
  /// searchable tree for pointers that are only written once to the code file
  Fd_Pointer_Tree *ptr_in_code;

  /// crc32 for blocks of text written to the code file
  unsigned long block_crc_;
  /// if set, we are at the start of a line and can ignore leading spaces in crc
  bool block_line_start_;
  /// expanding buffer for vsnprintf
  char *block_buffer_;
  /// size of expanding buffer for vsnprintf
  int block_buffer_size_;

  void crc_add(const void *data, int n=-1);
  int crc_printf(const char *format, ...);
  int crc_vprintf(const char *format, va_list args);
  int crc_puts(const char *text);
  int crc_putc(int c);

public:
  /// current level of source code indentation
  int indentation;
  /// set if we write abbreviated file for the source code previewer
  /// (disables binary data blocks, for example)
  bool write_codeview;
  /// silly thing to prevent declaring unused variables:
  /// When this symbol is on, all attempts to write code don't write
  /// anything, but set a variable if it looks like the variable "o" is used:
  int varused_test;
  /// set to 1 if varused_test found that a variable is actually used
  int varused;

public:
  Fd_Code_Writer();
  ~Fd_Code_Writer();
  const char* unique_id(void* o, const char*, const char*, const char*);
  /// Increment source code indentation level.
  void indent_more() { indentation++; }
  /// Decrement source code indentation level.
  void indent_less() { indentation--; }
  const char *indent();
  const char *indent(int set);
  const char *indent_plus(int offset);
  int write_h_once(const char *, ...) __fl_attr((__format__ (__printf__, 2, 3)));
  int write_c_once(const char *, ...) __fl_attr((__format__ (__printf__, 2, 3)));
  bool c_contains(void* ptr);
  void write_cstring(const char *,int length);
  void write_cstring(const char *);
  void write_cdata(const char *,int length);
  void vwrite_c(const char* format, va_list args);
  void write_c(const char*, ...) __fl_attr((__format__ (__printf__, 2, 3)));
  void write_cc(const char *, int, const char*, const char*);
  void write_h(const char*, ...) __fl_attr((__format__ (__printf__, 2, 3)));
  void write_hc(const char *, int, const char*, const char*);
  void write_c_indented(const char *textlines, int inIndent, char inTrailwWith);
  Fl_Type* write_static(Fl_Type* p);
  Fl_Type* write_code(Fl_Type* p);
  int write_code(const char *cfile, const char *hfile, bool to_codeview=false);
  void write_public(int state); // writes pubic:/private: as needed

  void tag(int type, unsigned short uid);

  static unsigned long block_crc(const void *data, int n=-1, unsigned long in_crc=0, bool *inout_line_start=NULL);
};

#endif // _FLUID_CODE_H
