//
// Fluid C++ Code Writer header for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_IO_CODE_WRITER_H
#define FLUID_IO_CODE_WRITER_H

#include <FL/fl_attr.h>

#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <set>
#include <map>

class Node;
struct Fd_Identifier_Tree;
struct Fd_Text_Tree;
struct Fd_Pointer_Tree;

int is_id(char c);

namespace fld {

class Project;

namespace io {

class Code_Writer
{
private:
  /// Link Code_Writer class to the project.
  Project &proj_;

  /// file pointer for the C++ code file
  FILE *code_file = nullptr;
  /// file pointer for the C++ header file
  FILE *header_file = nullptr;

  /// tree of unique but human-readable identifiers
  std::map<std::string, void*> unique_id_list { };
  /// searchable text tree for text that is only written once to the header file
  std::set<std::string> text_in_header { };
  /// searchable text tree for text that is only written once to the code file
  std::set<std::string> text_in_code { };
  /// searchable tree for pointers that are only written once to the code file
  std::set<void*> ptr_in_code { };

  /// crc32 for blocks of text written to the code file
  unsigned long block_crc_ = 0;
  /// if set, we are at the start of a line and can ignore leading spaces in crc
  bool block_line_start_ = true;
  /// expanding buffer for vsnprintf
  char *block_buffer_ = nullptr;
  /// size of expanding buffer for vsnprintf
  int block_buffer_size_ = 0;

  void crc_add(const void *data, int n=-1);
  int crc_printf(const char *format, ...);
  int crc_vprintf(const char *format, va_list args);
  int crc_puts(const char *text);
  int crc_putc(int c);

public:
  /// current level of source code indentation
  int indentation = 0;
  /// set if we write abbreviated file for the source code previewer
  /// (disables binary data blocks, for example)
  bool write_codeview = false;
  /// silly thing to prevent declaring unused variables:
  /// When this symbol is on, all attempts to write code don't write
  /// anything, but set a variable if it looks like the variable "o" is used:
  int varused_test = 0;
  /// set to 1 if varused_test found that a variable is actually used
  int varused = 0;

public:
  Code_Writer(Project &proj);
  ~Code_Writer();
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
  Node* write_static(Node* p);
  Node* write_code(Node* p);
  int write_code(const char *cfile, const char *hfile, bool to_codeview=false);
  void write_public(int state); // writes pubic:/private: as needed

  void tag(int type, unsigned short uid);

  static unsigned long block_crc(const void *data, int n=-1, unsigned long in_crc=0, bool *inout_line_start=nullptr);
};

} // namespace io
} // namespace fld

#endif // FLUID_IO_CODE_WRITER_H
