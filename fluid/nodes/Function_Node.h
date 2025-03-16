//
// C function Node header file for the Fast Light Tool Kit (FLTK).
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
  const char* return_type;
  char public_, cdecl_, constructor, havewidgets;
public:
  Function_Node();
  ~Function_Node();
  Node *make(Strategy strategy) override;
  void write_code1(fld::io::Code_Writer& f) override;
  void write_code2(fld::io::Code_Writer& f) override;
  void open() override;
  int ismain() {return name_ == nullptr;}
  const char *type_name() override {return "Function";}
  const char *title() override {
    return name() ? name() : "main()";
  }
  int can_have_children() const override {return 1;}
  int is_code_block() const override {return 1;}
  int is_public() const override;
  Type type() const override { return Type::Function; }
  bool is_a(Type inType) const override { return (inType==Type::Function) ? true : super::is_a(inType); }
  void write_properties(fld::io::Project_Writer &f) override;
  void read_property(fld::io::Project_Reader &f, const char *) override;
  int has_signature(const char *, const char*) const;
};

// ---- Code_Node declaration

class Code_Node : public Node
{
public:
  typedef Node super;
  static Code_Node prototype;
private:
  ExternalCodeEditor editor_;
  int cursor_position_;
  int code_input_scroll_row;
  int code_input_scroll_col;
public:
  Code_Node();
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
};

// ---- CodeBlock_Node declaration

class CodeBlock_Node : public Node
{
public:
  typedef Node super;
  static CodeBlock_Node prototype;
private:
  const char* after;
public:
  CodeBlock_Node();
  ~CodeBlock_Node();
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
};

// ---- Decl_Node declaration

class Decl_Node : public Node
{
public:
  typedef Node super;
  static Decl_Node prototype;
protected:
  char public_;
  char static_;

public:
  Decl_Node();
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
};

// ---- Data_Node declaration

class Data_Node : public Decl_Node
{
public:
  typedef Decl_Node super;
  static Data_Node prototype;
private:
  const char *filename_;
  int text_mode_;

public:
  Data_Node();
  ~Data_Node();
  Node *make(Strategy strategy) override;
  void write_code1(fld::io::Code_Writer& f) override;
  void write_code2(fld::io::Code_Writer& f) override {}
  void open() override;
  const char *type_name() override {return "data";}
  void write_properties(fld::io::Project_Writer &f) override;
  void read_property(fld::io::Project_Reader &f, const char *) override;
  Type type() const override { return Type::Data; }
  bool is_a(Type inType) const override { return (inType==Type::Data) ? true : super::is_a(inType); }
};

// ---- DeclBlock_Node declaration

class DeclBlock_Node : public Node
{
public:
  typedef Node super;
  static DeclBlock_Node prototype;
private:
  enum {
    CODE_IN_HEADER = 1,
    CODE_IN_SOURCE = 2,
    STATIC_IN_HEADER = 4,
    STATIC_IN_SOURCE = 8
  };
  const char* after; ///< code after all children of this block
  int write_map_;     ///< see enum above

public:
  DeclBlock_Node();
  ~DeclBlock_Node();
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
};

// ---- Comment_Node declaration

class Comment_Node : public Node
{
public:
  typedef Node super;
  static Comment_Node prototype;
private:
  char in_c_, in_h_, style_;

public:
  Comment_Node();
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
};

// ---- Class_Node declaration

class Class_Node : public Node
{
public:
  typedef Node super;
  static Class_Node prototype;
private:
  const char* subclass_of;
  char public_;
  const char* class_prefix;
public:
  Class_Node();
  ~Class_Node();
  // state variables for output:
  char write_public_state; // true when public: has been printed
  Class_Node* parent_class; // save class if nested
//
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

  // class prefix attribute access
  void prefix(const char* p);
  const char*  prefix() const {return class_prefix;}
};

#endif // FLUID_NODES_FUNCTION_NODE_H
