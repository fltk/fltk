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

#include "Fl_Type.h"

#include "Fluid_Image.h"
#ifdef _WIN32
#include "ExternalCodeEditor_WIN32.h"
#else
#include "ExternalCodeEditor_UNIX.h"
#endif

#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu.H>
#include <FL/fl_draw.H>
#include <FL/fl_attr.h>

#include <stdarg.h>
#include <stdlib.h>

extern class Fl_Class_Type *current_class;

int has_toplevel_function(const char *rtype, const char *sig);

const char *c_check(const char *c, int type = 0);

// ---- Fl_Function_Type declaration

class Fl_Function_Type : public Fl_Type
{
  typedef Fl_Type super;
  const char* return_type;
  char public_, cdecl_, constructor, havewidgets;

public:
  Fl_Function_Type();
  ~Fl_Function_Type();
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write_code1(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE;
  void open() FL_OVERRIDE;
  int ismain() {return name_ == 0;}
  const char *type_name() FL_OVERRIDE {return "Function";}
  const char *title() FL_OVERRIDE {
    return name() ? name() : "main()";
  }
  int can_have_children() const FL_OVERRIDE {return 1;}
  int is_code_block() const FL_OVERRIDE {return 1;}
  int is_public() const FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Function; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Function) ? true : super::is_a(inID); }
  void write_properties(Fd_Project_Writer &f) FL_OVERRIDE;
  void read_property(Fd_Project_Reader &f, const char *) FL_OVERRIDE;
  int has_signature(const char *, const char*) const;
};

// ---- Fl_Code_Type declaration

class Fl_Code_Type : public Fl_Type
{
  typedef Fl_Type super;
  ExternalCodeEditor editor_;
  int cursor_position_;
  int code_input_scroll_row;
  int code_input_scroll_col;

public:
  Fl_Code_Type();
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write(Fd_Project_Writer &f) FL_OVERRIDE;
  void write_code1(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE { }
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

// ---- Fl_CodeBlock_Type declaration

class Fl_CodeBlock_Type : public Fl_Type
{
  typedef Fl_Type super;
  const char* after;

public:
  Fl_CodeBlock_Type();
  ~Fl_CodeBlock_Type();
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write_code1(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE;
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "codeblock";}
  int is_code_block() const FL_OVERRIDE {return 1;}
  int can_have_children() const FL_OVERRIDE {return 1;}
  int is_public() const FL_OVERRIDE { return -1; }
  ID id() const FL_OVERRIDE { return ID_CodeBlock; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_CodeBlock) ? true : super::is_a(inID); }
  void write_properties(Fd_Project_Writer &f) FL_OVERRIDE;
  void read_property(Fd_Project_Reader &f, const char *) FL_OVERRIDE;
};

// ---- Fl_Decl_Type declaration

class Fl_Decl_Type : public Fl_Type
{
  typedef Fl_Type super;

protected:
  char public_;
  char static_;

public:
  Fl_Decl_Type();
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write_code1(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE { }
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "decl";}
  void write_properties(Fd_Project_Writer &f) FL_OVERRIDE;
  void read_property(Fd_Project_Reader &f, const char *) FL_OVERRIDE;
  int is_public() const FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Decl; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Decl) ? true : super::is_a(inID); }
};

// ---- Fl_Data_Type declaration

class Fl_Data_Type : public Fl_Decl_Type
{
  typedef Fl_Decl_Type super;
  const char *filename_;
  int text_mode_;

public:
  Fl_Data_Type();
  ~Fl_Data_Type();
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write_code1(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE {}
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "data";}
  void write_properties(Fd_Project_Writer &f) FL_OVERRIDE;
  void read_property(Fd_Project_Reader &f, const char *) FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Data; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Data) ? true : super::is_a(inID); }
};

// ---- Fl_DeclBlock_Type declaration

class Fl_DeclBlock_Type : public Fl_Type
{
  typedef Fl_Type super;
  enum {
    CODE_IN_HEADER = 1,
    CODE_IN_SOURCE = 2,
    STATIC_IN_HEADER = 4,
    STATIC_IN_SOURCE = 8
  };
  const char* after; ///< code after all children of this block
  int write_map_;     ///< see enum above

public:
  Fl_DeclBlock_Type();
  ~Fl_DeclBlock_Type();
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write_static(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_static_after(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_code1(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE;
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "declblock";}
  void write_properties(Fd_Project_Writer &f) FL_OVERRIDE;
  void read_property(Fd_Project_Reader &f, const char *) FL_OVERRIDE;
  int can_have_children() const FL_OVERRIDE {return 1;}
  int is_decl_block() const FL_OVERRIDE {return 1;}
  int is_public() const FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_DeclBlock; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_DeclBlock) ? true : super::is_a(inID); }
};

// ---- Fl_Comment_Type declaration

class Fl_Comment_Type : public Fl_Type
{
  typedef Fl_Type super;
  char in_c_, in_h_, style_;

public:
  Fl_Comment_Type();
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write_code1(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE { }
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "comment";}
  void write_properties(Fd_Project_Writer &f) FL_OVERRIDE;
  void read_property(Fd_Project_Reader &f, const char *) FL_OVERRIDE;
  int is_public() const FL_OVERRIDE { return 1; }
  ID id() const FL_OVERRIDE { return ID_Comment; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Comment) ? true : super::is_a(inID); }
};

// ---- Fl_Class_Type declaration

class Fl_Class_Type : public Fl_Type
{
  typedef Fl_Type super;
  const char* subclass_of;
  char public_;
  const char* class_prefix;

public:
  Fl_Class_Type();
  ~Fl_Class_Type();
  // state variables for output:
  char write_public_state; // true when public: has been printed
  Fl_Class_Type* parent_class; // save class if nested
//
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write_code1(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE;
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "class";}
  int can_have_children() const FL_OVERRIDE {return 1;}
  int is_decl_block() const FL_OVERRIDE {return 1;}
  int is_class() const FL_OVERRIDE {return 1;}
  int is_public() const FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Class; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Class) ? true : super::is_a(inID); }
  void write_properties(Fd_Project_Writer &f) FL_OVERRIDE;
  void read_property(Fd_Project_Reader &f, const char *) FL_OVERRIDE;

  // class prefix attribute access
  void prefix(const char* p);
  const char*  prefix() const {return class_prefix;}
};

#endif // _FLUID_FL_FUNCTION_TYPE_H
