//
// Fluid file routines for the Fast Light Tool Kit (FLTK).
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

#ifndef _FLUID_FILE_H
#define _FLUID_FILE_H

#include "Fl_Type.h"

#include <FL/fl_attr.h>

class Fl_Type;

extern int fdesign_flip;

int read_file(const char *, int merge, Strategy strategy=kAddAsLastChild);
int write_file(const char *, int selected_only = 0, bool to_codeview = false);

class Fd_Project_Reader
{
protected:
  /// Project input file
  FILE *fin;
  /// Number of most recently read line
  int lineno;
  /// Pointer to the file path and name (not copied!)
  const char *fname;
  /// Expanding buffer to store the most recently read word
  char *buffer;
  /// Exact size of the expanding buffer in bytes
  int buflen;

  void expand_buffer(int length);

  int nextchar() { for (;;) { int ret = fgetc(fin); if (ret!='\r') return ret; } }

public:
  /// Holds the file version number after reading the "version" tag
  double read_version;

public:
  Fd_Project_Reader();
  ~Fd_Project_Reader();
  int open_read(const char *s);
  int close_read();
  const char *filename_name();
  int read_quoted();
  Fl_Type *read_children(Fl_Type *p, int merge, Strategy strategy, char skip_options=0);
  int read_project(const char *, int merge, Strategy strategy=kAddAsLastChild);
  void read_error(const char *format, ...);
  const char *read_word(int wantbrace = 0);
  int read_int();
  int read_fdesign_line(const char*& name, const char*& value);
  void read_fdesign();
};

class Fd_Project_Writer
{
protected:
  // Project output file, always opened in "wb" mode
  FILE *fout;
  /// If set, one space is written before text unless the format starts with a newline character
  int needspace;
  /// Set if this file will be used in the codeview dialog
  bool write_codeview_;

public:
  Fd_Project_Writer();
  ~Fd_Project_Writer();
  int open_write(const char *s);
  int close_write();
  int write_project(const char *filename, int selected_only, bool codeview);
  void write_word(const char *);
  void write_string(const char *,...) __fl_attr((__format__ (__printf__, 2, 3)));
  void write_indent(int n);
  void write_open();
  void write_close(int n);
  FILE *file() const { return fout; }
  bool write_codeview() const { return write_codeview_; }
};

#endif // _FLUID_FILE_H
