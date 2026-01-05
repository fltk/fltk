//
// C function Node header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2026 by Bill Spitzak and others.
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

#ifndef FLUID_NODES_FUNCTION_NODE_H
#define FLUID_NODES_FUNCTION_NODE_H

#include "nodes/Node.h"

#include "app/Image_Asset.h"
#ifdef _WIN32
#include "tools/ExternalCodeEditor_WIN32.h"
#else
#include "tools/ExternalCodeEditor_UNIX.h"
#endif

#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu.H>
#include <FL/fl_draw.H>
#include <FL/fl_attr.h>

#include <stdarg.h>
#include <stdlib.h>

#include <string>

extern class Class_Node *current_class;

int has_toplevel_function(const char *rtype, const char *sig);

const char *c_check(const char *c, int type = 0);

// ---- Function_Node declaration

class Function_Node : public Node
{
public:
  typedef Node super;
  static Function_Node prototype;

private:
  std::string return_type_;
  char public_ = 0;
  char declare_c_ = 0;
  char constructor = 0;
  char havewidgets = 0;

public:
  Function_Node() = default;
  ~Function_Node() override = default;

  Node *make(Strategy strategy) override;
  void write_code1(fld::io::Code_Writer& f) override;
  void write_code2(fld::io::Code_Writer& f) override;
  void open() override;
  int ismain() {return name_ == nullptr;}
  const char *type_name() override {return "Function";}
  const char *title() override { return name() ? name() : "main()"; }
  int can_have_children() const override {return 1;}
  int is_code_block() const override {return 1;}
  int is_public() const override;
  Type type() const override { return Type::Function; }
  bool is_a(Type inType) const override { return (inType==Type::Function) ? true : super::is_a(inType); }
  void write_properties(fld::io::Project_Writer &f) override;
  void read_property(fld::io::Project_Reader &f, const char *) override;
  int has_signature(const char *, const char*) const;
  std::string return_type() const { return return_type_; }
  void return_type(const std::string& t) { storestring(t, return_type_); }
  char visibility() { return public_; }
  void visibility(char v) { public_ = v; }
  char declare_c() { return declare_c_; }
  void declare_c(char v) { declare_c_ = v; }
};

// ---- Code_Node declaration

class Code_Node : public Node
{
public:
  typedef Node super;
  static Code_Node prototype;

private:
  int cursor_position_ = 0;
  int code_input_scroll_row_ = 0;
  int code_input_scroll_col_ = 0;
  ExternalCodeEditor editor_;

public:
  Code_Node() = default;
  ~Code_Node() override = default;

  Node *make(Strategy strategy) override;
  void write(fld::io::Project_Writer &f) override;
  void write_code1(fld::io::Code_Writer& f) override;
  void write_code2(fld::io::Code_Writer& f) override { }
  void open() override;
  const char *type_name() override {return "code";}
  int is_code_block() const override {return 0;}
  Type type() const override { return Type::Code; }
  bool is_a(Type inType) const override { return (inType==Type::Code) ? true : super::is_a(inType); }
  int is_public() const override { return -1; }
  int is_editing();
  int reap_editor();
  int handle_editor_changes();
  int cursor_position() { return cursor_position_; }
  int code_input_scroll_row() { return code_input_scroll_row_; }
  int code_input_scroll_col() { return code_input_scroll_col_; }
  void save_editor_state(int pos, int row, int col) {
    cursor_position_ = pos; code_input_scroll_row_ = row; code_input_scroll_col_ = col;
  }
};

// ---- CodeBlock_Node declaration

class CodeBlock_Node : public Node
{
public:
  typedef Node super;
  static CodeBlock_Node prototype;
  
private:
  std::string end_code_;

public:
  CodeBlock_Node() = default;
  ~CodeBlock_Node() override = default;

  Node *make(Strategy strategy) override;
  void write_code1(fld::io::Code_Writer& f) override;
  void write_code2(fld::io::Code_Writer& f) override;
  void open() override;
  const char *type_name() override {return "codeblock";}
  int is_code_block() const override {return 1;}
  int can_have_children() const override {return 1;}
  int is_public() const override { return -1; }
  Type type() const override { return Type::CodeBlock; }
  bool is_a(Type inType) const override { return (inType==Type::CodeBlock) ? true : super::is_a(inType); }
  void write_properties(fld::io::Project_Writer &f) override;
  void read_property(fld::io::Project_Reader &f, const char *) override;
  std::string end_code() { return end_code_; }
  void end_code(const std::string& c) { storestring(c, end_code_); }
};

// ---- Decl_Node declaration

class Decl_Node : public Node
{
public:
  typedef Node super;
  static Decl_Node prototype;

protected:
  char public_ = 0; // public = 0, private = 1, protected = 2
  char static_ = 1;

public:
  Decl_Node() = default;
  ~Decl_Node() override = default;

  Node *make(Strategy strategy) override;
  void write_code1(fld::io::Code_Writer& f) override;
  void write_code2(fld::io::Code_Writer& f) override { }
  void open() override;
  const char *type_name() override {return "decl";}
  void write_properties(fld::io::Project_Writer &f) override;
  void read_property(fld::io::Project_Reader &f, const char *) override;
  int is_public() const override;
  Type type() const override { return Type::Decl; }
  bool is_a(Type inType) const override { return (inType==Type::Decl) ? true : super::is_a(inType); }
  char visibility() { return public_; }
  void visibility(char v) { public_ = v; }
  char output_file() { return (public_&1)|((static_&1)<<1); }
  void output_file(char f) { public_ = (f&1); static_ = ((f>>1)&1); }
};

// ---- Data_Node declaration

class Data_Node : public Decl_Node
{
public:
  typedef Decl_Node super;
  static Data_Node prototype;

private:
  std::string filename_;
  int output_format_ = 0;

public:
  Data_Node() = default;
  ~Data_Node() override = default;

  Node *make(Strategy strategy) override;
  void write_code1(fld::io::Code_Writer& f) override;
  void write_code2(fld::io::Code_Writer& f) override {}
  void open() override;
  const char *type_name() override {return "data";}
  void write_properties(fld::io::Project_Writer &f) override;
  void read_property(fld::io::Project_Reader &f, const char *) override;
  Type type() const override { return Type::Data; }
  bool is_a(Type inType) const override { return (inType==Type::Data) ? true : super::is_a(inType); }
  void filename(const std::string& fn) { storestring(fn, filename_); }
  std::string filename() { return filename_; }
  int output_format() { return output_format_; }
  void output_format(int fmt) { output_format_ = fmt; }
};

// ---- DeclBlock_Node declaration

class DeclBlock_Node : public Node
{
public:
  typedef Node super;
  static DeclBlock_Node prototype;
  enum {
    CODE_IN_HEADER = 1,
    CODE_IN_SOURCE = 2,
    STATIC_IN_HEADER = 4,
    STATIC_IN_SOURCE = 8
  };

private:
  std::string end_code_;            ///< code after all children of this block
  int write_map_ = CODE_IN_SOURCE;  ///< see enum above

public:
  DeclBlock_Node() = default;
  ~DeclBlock_Node() override = default;

  Node *make(Strategy strategy) override;
  void write_static(fld::io::Code_Writer& f) override;
  void write_static_after(fld::io::Code_Writer& f) override;
  void write_code1(fld::io::Code_Writer& f) override;
  void write_code2(fld::io::Code_Writer& f) override;
  void open() override;
  const char *type_name() override {return "declblock";}
  void write_properties(fld::io::Project_Writer &f) override;
  void read_property(fld::io::Project_Reader &f, const char *) override;
  int can_have_children() const override {return 1;}
  int is_decl_block() const override {return 1;}
  int is_public() const override;
  Type type() const override { return Type::DeclBlock; }
  bool is_a(Type inType) const override { return (inType==Type::DeclBlock) ? true : super::is_a(inType); }
  std::string end_code() { return end_code_; }
  void end_code(const std::string& p) { storestring(p, end_code_); }
  int write_map() { return write_map_; }
  void write_map(int v) { write_map_ = v; }
};

// ---- Comment_Node declaration

class Comment_Node : public Node
{
public:
  typedef Node super;
  static Comment_Node prototype;

private:
  char in_c_ = 1;
  char in_h_ = 1;
  char style_ = 0;

public:
  Comment_Node() = default;
  ~Comment_Node() override = default;

  Node *make(Strategy strategy) override;
  void write_code1(fld::io::Code_Writer& f) override;
  void write_code2(fld::io::Code_Writer& f) override { }
  void open() override;
  const char *type_name() override {return "comment";}
  void write_properties(fld::io::Project_Writer &f) override;
  void read_property(fld::io::Project_Reader &f, const char *) override;
  int is_public() const override { return 1; }
  Type type() const override { return Type::Comment; }
  bool is_a(Type inType) const override { return (inType==Type::Comment) ? true : super::is_a(inType); }
  bool in_h() { return in_h_; }
  void in_h(bool v) { in_h_ = v; }
  bool in_c() { return in_c_; }
  void in_c(bool v) { in_c_ = v; }
};

// ---- Class_Node declaration

class Class_Node : public Node
{
public:
  typedef Node super;
  static Class_Node prototype;

private:
  std::string base_class_;
  std::string prefix_;
  char public_ = 1;

public:
  Class_Node() = default;
  ~Class_Node() override = default;

  // State variables used when writing code to file
  char write_public_state; // true when public: has been printed
  Class_Node* parent_class; // save class if nested

  Node *make(Strategy strategy) override;
  void write_code1(fld::io::Code_Writer& f) override;
  void write_code2(fld::io::Code_Writer& f) override;
  void open() override;
  const char *type_name() override {return "class";}
  int can_have_children() const override {return 1;}
  int is_decl_block() const override {return 1;}
  int is_class() const override {return 1;}
  int is_public() const override;
  Type type() const override { return Type::Class; }
  bool is_a(Type inType) const override { return (inType==Type::Class) ? true : super::is_a(inType); }
  void write_properties(fld::io::Project_Writer &f) override;
  void read_property(fld::io::Project_Reader &f, const char *) override;

  /** Get base class access and name. */
  std::string base_class() { return base_class_; }
  /** Set base class access and name, i.e. `public Fl_Widget`. */
  void base_class(const std::string& name) { storestring(name, base_class_); }

  char visibility() { return public_; }
  void visibility(char v) { public_ = v; }

  /** Get the text between `class` and the class name */
  std::string prefix() const { return prefix_; }
  /** Set the text between `class` and the class name */
  void prefix(const std::string& p) { prefix_ = p; }
};

#endif // FLUID_NODES_FUNCTION_NODE_H
