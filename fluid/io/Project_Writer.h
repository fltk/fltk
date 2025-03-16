//
// Fluid Project File Writer header for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_IO_PROJECT_WRITER_H
#define FLUID_IO_PROJECT_WRITER_H

#include <FL/fl_attr.h>

#include <stdio.h>

class Node;

namespace fld {

class Project;

namespace io {

int write_file(Project &proj, const char *, int selected_only = 0, bool to_codeview = false);

class Project_Writer
{
protected:
  /// Link Project_Writer class to the project.
  Project &proj_;

  // Project output file, always opened in "wb" mode
  FILE *fout = nullptr;
  /// If set, one space is written before text unless the format starts with a newline character
  int needspace = 0;
  /// Set if this file will be used in the codeview dialog
  bool write_codeview_ = false;

public:
  Project_Writer(Project &proj);
  ~Project_Writer();
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

} // namespace io
} // namespace fld

#endif // FLUID_IO_PROJECT_WRITER_H
