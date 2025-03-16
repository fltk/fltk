//
// MergeBack header for the Fast Light Tool Kit (FLTK).
//
// Copyright 2023-2025 by Bill Spitzak and others.
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

// Matt: disabled
#if 0

#ifndef _FLUID_MERGEBACK_H
#define _FLUID_MERGEBACK_H

#include <FL/fl_attr.h>

#include <stdio.h>
#include <string>

const int FD_TAG_GENERIC = 0;
const int FD_TAG_CODE = 1;
const int FD_TAG_MENU_CALLBACK = 2;
const int FD_TAG_WIDGET_CALLBACK = 3;
const int FD_TAG_LAST = 3;

const int FD_MERGEBACK_ANALYSE = 0;
const int FD_MERGEBACK_INTERACTIVE = 1;
const int FD_MERGEBACK_APPLY = 2;
const int FD_MERGEBACK_APPLY_IF_SAFE = 3;

/** Class that implements the MergeBack functionality.
 \see merge_back(const std::string &s, int task)
 */
class Fd_Mergeback
{
protected:
  /// Pointer to the C++ code file.
  FILE *code;
  /// Current line number in the C++ code file.
  int line_no;
  /// Set if there was an error reading a tag.
  int tag_error;
  /// Number of code blocks that were different than the CRC in their tag.
  int num_changed_code;
  /// Number of generic structure blocks that were different than the CRC in their tag.
  int num_changed_structure;
  /// Number of code block that were modified, but a type node by that uid was not found.
  int num_uid_not_found;
  /// Number of modified code block where the corresponding project block also changed.
  int num_possible_override;

  void unindent(char *s);
  std::string read_and_unindent_block(long start, long end);
  void analyse_callback(unsigned long code_crc, unsigned long tag_crc, int uid);
  void analyse_code(unsigned long code_crc, unsigned long tag_crc, int uid);
  int apply_callback(long block_end, long block_start, unsigned long code_crc, int uid);
  int apply_code(long block_end, long block_start, unsigned long code_crc, int uid);

public:
  Fd_Mergeback();
  ~Fd_Mergeback();
  int merge_back(const std::string &s, const std::string &p, int task);
  int ask_user_to_merge(const std::string &s, const std::string &p);
  int analyse();
  int apply();
};

extern int merge_back(const std::string &s, const std::string &p, int task);


#endif // _FLUID_MERGEBACK_H

#endif
