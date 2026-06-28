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

#include "proj/mergeback.h"

#include <FL/fl_attr.h>

#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <set>
#include <map>
#include <sstream>

class Node;
struct Fd_Identifier_Tree;
struct Fd_Text_Tree;
struct Fd_Pointer_Tree;

int is_id(char c);

namespace fluid {

class Project;

struct string_view {
    const char *data_;
    size_t size_;

    string_view() : data_(nullptr), size_(0) {}
    string_view(const char *s) : data_(s), size_(strlen(s)) {}
    string_view(const char *s, size_t n) : data_(s), size_(n) {}
    string_view(const std::string &s) : data_(s.data()), size_(s.size()) {}

    const char *data() const { return data_; }
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    char operator[](size_t i) const { return data_[i]; }

    std::string str() const { return std::string(data_, size_); }

    string_view substr(size_t pos, size_t len = size_t(-1)) const {
        if (len == size_t(-1) || pos + len > size_) len = size_ - pos;
        return string_view(data_ + pos, len);
    }
};

namespace io {

extern std::string to_string_8x(uint32_t value);
extern std::string to_string_g(double value);

class Code_Writer
{
private:
  /// Link Code_Writer class to the project.
  Project &proj_;

  /// string stream buffer for generating C++ code file content
  std::ostringstream code_buffer;
  /// string stream buffer for generating C++ header file content
  std::ostringstream header_buffer;

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

  /// current level of source code indentation
  int indentation = 0;

  bool file_content_matches(const std::string& filename, const std::string& content);
  bool write_file_if_changed(const std::string& filename, const std::string& content);

  /// Return the current write position in the code output stream.
  int code_pos() { return (int)code_buffer.tellp(); }
  /// Return the current write position in the header output stream.
  int header_pos() { return (int)header_buffer.tellp(); }

protected:
  void crc_add(fluid::string_view block);
  int crc_puts(const std::string& text);
  int crc_putc(int c);

public:
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
  Code_Writer(const Code_Writer &) = delete;
  Code_Writer &operator=(const Code_Writer &) = delete;
  Code_Writer(Code_Writer &&) = delete;
  Code_Writer &operator=(Code_Writer &&) = delete;
  ~Code_Writer() = default;

  std::string unique_id(void* o, const std::string& type, const std::string& name, const std::string& label);

  /// Increment source code indentation level.
  void indent_more() { indentation++; }
  /// Decrement source code indentation level.
  void indent_less() { indentation--; }
  void indent_reset() { indentation = 0; }
  std::string indent() const;
  std::string indent(int set) const;
  std::string indent_plus(int offset) const;

  bool c_contains(void* ptr);

  int write_h_once(const std::string& code);
  int write_c_once(const std::string& code);
  void write_cstring(fluid::string_view text);
  void write_cdata(fluid::string_view block);
  void write_c(const std::string& code);
  void write_h(const std::string& code);
  void write_cc(const std::string& indent, const std::string& code, const std::string& comment);
  void write_hc(const std::string& indent, const std::string& code, const std::string& comment);
  void write_c_indented(const std::string& codeblock, int additional_indent, char trail_char);
  void write_public(int state); // writes pubic:/private: as needed

  Node* write_static(Node* p);
  Node* write_code(Node* p);

  int write_code(const std::string& code_arg, const std::string& header_arg, bool to_codeview=false);

  /// Return the generated source code as a string (valid after write_code() with to_codeview=true).
  std::string code_string() const { return code_buffer.str(); }
  /// Return the generated header code as a string (valid after write_code() with to_codeview=true).
  std::string header_string() const { return header_buffer.str(); }

  void tag(proj::Mergeback::Tag prev_type, proj::Mergeback::Tag next_type, unsigned short uid);

  static unsigned long block_crc(fluid::string_view block, unsigned long in_crc=0, bool *inout_line_start=nullptr);
};

} // namespace io
} // namespace fluid

#endif // FLUID_IO_CODE_WRITER_H
