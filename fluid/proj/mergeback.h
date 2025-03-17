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

#ifndef FLUID_PROJ_MERGEBACK_H
#define FLUID_PROJ_MERGEBACK_H

#include <FL/fl_attr.h>

#include <stdint.h>
#include <stdio.h>
#include <string>

namespace fld {

class Project;

namespace proj {


/** Class that implements the MergeBack functionality.
 \see merge_back(const std::string &s, int task)
 */
class Mergeback
{
  public:
  enum class Tag {
    GENERIC = 0, CODE, MENU_CALLBACK, WIDGET_CALLBACK, UNUSED_
  };
  enum class Task {
    ANALYSE = 0, INTERACTIVE, APPLY, APPLY_IF_SAFE = 3
  };
protected:
  /// Apply mergeback for this project.
  Project &proj_;
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

  static uint32_t decode_trichar32(const char *text);
  static void print_trichar32(FILE *out, uint32_t value);

  static const char *find_mergeback_tag(const char *line);
  static bool read_tag(const char *tag, Tag *prev_type, uint16_t *uid, uint32_t *crc);

public:
  Mergeback(Project &proj);
  ~Mergeback();
  int merge_back(const std::string &s, const std::string &p, Task task);
  int ask_user_to_merge(const std::string &s, const std::string &p);
  int analyse();
  int apply();
  static void print_tag(FILE *out, Tag prev_type, Tag next_type, uint16_t uid, uint32_t crc);
};

extern int merge_back(const std::string &s, const std::string &p, int task);

} // namespace proj
} // namespace fld

#endif // FLUID_PROJ_MERGEBACK_H

