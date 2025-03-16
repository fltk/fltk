//
// C function type header file for the Fast Light Tool Kit (FLTK).
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

#ifndef _FLUID_FL_FUNCTION_TYPE_H
#define _FLUID_FL_FUNCTION_TYPE_H

#include "nodes/Node.h"

#include "app/Fluid_Image.h"
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
  typedef Node super;
  const char* return_type;
  char public_, cdecl_, constructor, havewidgets;

public:
  Function_Node();
  ~Function_Node();
  Node *make(Strategy strategy) FL_OVERRIDE;
  void write_code1(fld::io::Code_Writer& f) FL_OVERRIDE;
  void write_code2(fld::io::Code_Writer& f) FL_OVERRIDE;
  void open() FL_OVERRIDE;
  int ismain() {return name_ == nullptr;}
  const char *type_name() FL_OVERRIDE {return "Function";}
  const char *title() FL_OVERRIDE {
    return name() ? name() : "main()";
  }
  int can_have_children() const FL_OVERRIDE {return 1;}
  int is_code_block() const FL_OVERRIDE {return 1;}
  int is_public() const FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Function; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Function) ? true : super::is_a(inID); }
  void write_properties(fld::io::Project_Writer &f) FL_OVERRIDE;
  void read_property(fld::io::Project_Reader &f, const char *) FL_OVERRIDE;
  int has_signature(const char *, const char*) const;
};

// ---- Code_Node declaration

class Code_Node : public Node
{
  typedef Node super;
  ExternalCodeEditor editor_;
  int cursor_position_;
  int code_input_scroll_row;
  int code_input_scroll_col;

public:
  Code_Node();
  Node *make(Strategy strategy) FL_OVERRIDE;
  void write(fld::io::Project_Writer &f) FL_OVERRIDE;
  void write_code1(fld::io::Code_Writer& f) FL_OVERRIDE;
  void write_code2(fld::io::Code_Writer& f) FL_OVERRIDE { }
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "code";}
  int is_code_block() const FL_OVERRIDE {return 0;}
  ID id() const FL_OVERRIDE { return ID_Code; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Code) ? true : super::is_a(inID); }
  int is_public() const FL_OVERRIDE { return -1; }
  int is_editing();
  int reap_editor();
  int handle_editor_changes();
};

// ---- CodeBlock_Node declaration

class CodeBlock_Node : public Node
{
  typedef Node super;
  const char* after;

public:
  CodeBlock_Node();
  ~CodeBlock_Node();
  Node *make(Strategy strategy) FL_OVERRIDE;
  void write_code1(fld::io::Code_Writer& f) FL_OVERRIDE;
  void write_code2(fld::io::Code_Writer& f) FL_OVERRIDE;
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "codeblock";}
  int is_code_block() const FL_OVERRIDE {return 1;}
  int can_have_children() const FL_OVERRIDE {return 1;}
  int is_public() const FL_OVERRIDE { return -1; }
  ID id() const FL_OVERRIDE { return ID_CodeBlock; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_CodeBlock) ? true : super::is_a(inID); }
  void write_properties(fld::io::Project_Writer &f) FL_OVERRIDE;
  void read_property(fld::io::Project_Reader &f, const char *) FL_OVERRIDE;
};

// ---- Decl_Node declaration

class Decl_Node : public Node
{
  typedef Node super;

protected:
  char public_;
  char static_;

public:
  Decl_Node();
  Node *make(Strategy strategy) FL_OVERRIDE;
  void write_code1(fld::io::Code_Writer& f) FL_OVERRIDE;
  void write_code2(fld::io::Code_Writer& f) FL_OVERRIDE { }
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "decl";}
  void write_properties(fld::io::Project_Writer &f) FL_OVERRIDE;
  void read_property(fld::io::Project_Reader &f, const char *) FL_OVERRIDE;
  int is_public() const FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Decl; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Decl) ? true : super::is_a(inID); }
};

// ---- Data_Node declaration

class Data_Node : public Decl_Node
{
  typedef Decl_Node super;
  const char *filename_;
  int text_mode_;

public:
  Data_Node();
  ~Data_Node();
  Node *make(Strategy strategy) FL_OVERRIDE;
  void write_code1(fld::io::Code_Writer& f) FL_OVERRIDE;
  void write_code2(fld::io::Code_Writer& f) FL_OVERRIDE {}
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "data";}
  void write_properties(fld::io::Project_Writer &f) FL_OVERRIDE;
  void read_property(fld::io::Project_Reader &f, const char *) FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Data; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Data) ? true : super::is_a(inID); }
};

// ---- DeclBlock_Node declaration

class DeclBlock_Node : public Node
{
  typedef Node super;
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
  Node *make(Strategy strategy) FL_OVERRIDE;
  void write_static(fld::io::Code_Writer& f) FL_OVERRIDE;
  void write_static_after(fld::io::Code_Writer& f) FL_OVERRIDE;
  void write_code1(fld::io::Code_Writer& f) FL_OVERRIDE;
  void write_code2(fld::io::Code_Writer& f) FL_OVERRIDE;
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "declblock";}
  void write_properties(fld::io::Project_Writer &f) FL_OVERRIDE;
  void read_property(fld::io::Project_Reader &f, const char *) FL_OVERRIDE;
  int can_have_children() const FL_OVERRIDE {return 1;}
  int is_decl_block() const FL_OVERRIDE {return 1;}
  int is_public() const FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_DeclBlock; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_DeclBlock) ? true : super::is_a(inID); }
};

// ---- Comment_Node declaration

class Comment_Node : public Node
{
  typedef Node super;
  char in_c_, in_h_, style_;

public:
  Comment_Node();
  Node *make(Strategy strategy) FL_OVERRIDE;
  void write_code1(fld::io::Code_Writer& f) FL_OVERRIDE;
  void write_code2(fld::io::Code_Writer& f) FL_OVERRIDE { }
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "comment";}
  void write_properties(fld::io::Project_Writer &f) FL_OVERRIDE;
  void read_property(fld::io::Project_Reader &f, const char *) FL_OVERRIDE;
  int is_public() const FL_OVERRIDE { return 1; }
  ID id() const FL_OVERRIDE { return ID_Comment; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Comment) ? true : super::is_a(inID); }
};

// ---- Class_Node declaration

class Class_Node : public Node
{
  typedef Node super;
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
  Node *make(Strategy strategy) FL_OVERRIDE;
  void write_code1(fld::io::Code_Writer& f) FL_OVERRIDE;
  void write_code2(fld::io::Code_Writer& f) FL_OVERRIDE;
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "class";}
  int can_have_children() const FL_OVERRIDE {return 1;}
  int is_decl_block() const FL_OVERRIDE {return 1;}
  int is_class() const FL_OVERRIDE {return 1;}
  int is_public() const FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Class; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Class) ? true : super::is_a(inID); }
  void write_properties(fld::io::Project_Writer &f) FL_OVERRIDE;
  void read_property(fld::io::Project_Reader &f, const char *) FL_OVERRIDE;

  // class prefix attribute access
  void prefix(const char* p);
  const char*  prefix() const {return class_prefix;}
};

#endif // _FLUID_FL_FUNCTION_TYPE_H
