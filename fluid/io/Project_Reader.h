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

#ifndef FLUID_IO_PROJECT_READER_H
#define FLUID_IO_PROJECT_READER_H

#include "nodes/Fl_Type.h"

#include <FL/fl_attr.h>

#include <stdio.h>


class Fl_Type;

namespace fld {
namespace io {

extern int fdesign_flip;

int read_file(const char *, int merge, Strategy strategy=Strategy::FROM_FILE_AS_LAST_CHILD);

class Project_Reader
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
  Project_Reader();
  ~Project_Reader();
  int open_read(const char *s);
  int close_read();
  const char *filename_name();
  int read_quoted();
  Fl_Type *read_children(Fl_Type *p, int merge, Strategy strategy, char skip_options=0);
  int read_project(const char *, int merge, Strategy strategy=Strategy::FROM_FILE_AS_LAST_CHILD);
  void read_error(const char *format, ...);
  const char *read_word(int wantbrace = 0);
  int read_int();
  int read_fdesign_line(const char*& name, const char*& value);
  void read_fdesign();
};

} // namespace io
} // namespace fld

#endif // FLUID_IO_PROJECT_READER_H
