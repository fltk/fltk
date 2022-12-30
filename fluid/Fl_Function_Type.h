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

extern Fl_Class_Type *current_class;

int has_toplevel_function(const char *rtype, const char *sig);

const char *c_check(const char *c, int type = 0);

// ---- Fl_Function_Type declaration

class Fl_Function_Type : public Fl_Type {
  const char* return_type;
  char public_, cdecl_, constructor, havewidgets;

public:
  Fl_Function_Type();
  ~Fl_Function_Type();
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write_code1() FL_OVERRIDE;
  void write_code2() FL_OVERRIDE;
  void open() FL_OVERRIDE;
  int ismain() {return name_ == 0;}
  const char *type_name() FL_OVERRIDE {return "Function";}
  const char *title() FL_OVERRIDE {
    return name() ? name() : "main()";
  }
  int is_parent() const FL_OVERRIDE {return 1;}
  int is_code_block() const FL_OVERRIDE {return 1;}
  int is_public() const FL_OVERRIDE;
  int pixmapID() FL_OVERRIDE { return 7; }
  void write_properties() FL_OVERRIDE;
  void read_property(const char *) FL_OVERRIDE;
  int has_signature(const char *, const char*) const;
};

// ---- Fl_Code_Type declaration

class Fl_Code_Type : public Fl_Type {
  ExternalCodeEditor editor_;
  int cursor_position_;
  int code_input_scroll_row;
  int code_input_scroll_col;

public:
  Fl_Code_Type();
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write() FL_OVERRIDE;
  void write_code1() FL_OVERRIDE;
  void write_code2() FL_OVERRIDE { }
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "code";}
  int is_code_block() const FL_OVERRIDE {return 0;}
  int is_code() const FL_OVERRIDE {return 1;}
  int pixmapID() FL_OVERRIDE { return 8; }
  int is_public() const FL_OVERRIDE { return -1; }
  int is_editing();
  int reap_editor();
  int handle_editor_changes();
};

// ---- Fl_CodeBlock_Type declaration

class Fl_CodeBlock_Type : public Fl_Type {
  const char* after;

public:
  Fl_CodeBlock_Type();
  ~Fl_CodeBlock_Type();
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write_code1() FL_OVERRIDE;
  void write_code2() FL_OVERRIDE;
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "codeblock";}
  int is_code_block() const FL_OVERRIDE {return 1;}
  int is_parent() const FL_OVERRIDE {return 1;}
  int is_public() const FL_OVERRIDE { return -1; }
  int pixmapID() FL_OVERRIDE { return 9; }
  void write_properties() FL_OVERRIDE;
  void read_property(const char *) FL_OVERRIDE;
};

// ---- Fl_Decl_Type declaration

class Fl_Decl_Type : public Fl_Type {

protected:
  char public_;
  char static_;

public:
  Fl_Decl_Type();
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write_code1() FL_OVERRIDE;
  void write_code2() FL_OVERRIDE { }
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "decl";}
  void write_properties() FL_OVERRIDE;
  void read_property(const char *) FL_OVERRIDE;
  int is_public() const FL_OVERRIDE;
  int pixmapID() FL_OVERRIDE { return 10; }
};

// ---- Fl_Data_Type declaration

class Fl_Data_Type : public Fl_Decl_Type {
  const char *filename_;
  int text_mode_;

public:
  Fl_Data_Type();
  ~Fl_Data_Type();
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write_code1() FL_OVERRIDE;
  void write_code2() FL_OVERRIDE {}
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "data";}
  void write_properties() FL_OVERRIDE;
  void read_property(const char *) FL_OVERRIDE;
  int pixmapID() FL_OVERRIDE { return 49; }
};

// ---- Fl_DeclBlock_Type declaration

class Fl_DeclBlock_Type : public Fl_Type {
  const char* after;
  char public_;

public:
  Fl_DeclBlock_Type();
  ~Fl_DeclBlock_Type();
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write_code1() FL_OVERRIDE;
  void write_code2() FL_OVERRIDE;
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "declblock";}
  void write_properties() FL_OVERRIDE;
  void read_property(const char *) FL_OVERRIDE;
  int is_parent() const FL_OVERRIDE {return 1;}
  int is_decl_block() const FL_OVERRIDE {return 1;}
  int is_public() const FL_OVERRIDE;
  int pixmapID() FL_OVERRIDE { return 11; }
};

// ---- Fl_Comment_Type declaration

class Fl_Comment_Type : public Fl_Type {
  char in_c_, in_h_, style_;
  char title_buf[64];

public:
  Fl_Comment_Type();
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write_code1() FL_OVERRIDE;
  void write_code2() FL_OVERRIDE { }
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "comment";}
  const char *title() FL_OVERRIDE; // string for browser
  void write_properties() FL_OVERRIDE;
  void read_property(const char *) FL_OVERRIDE;
  int is_public() const FL_OVERRIDE { return 1; }
  int is_comment() const FL_OVERRIDE { return 1; }
  int pixmapID() FL_OVERRIDE { return 46; }
};

// ---- Fl_Class_Type declaration

class Fl_Class_Type : public Fl_Type {
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
  void write_code1() FL_OVERRIDE;
  void write_code2() FL_OVERRIDE;
  void open() FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "class";}
  int is_parent() const FL_OVERRIDE {return 1;}
  int is_decl_block() const FL_OVERRIDE {return 1;}
  int is_class() const FL_OVERRIDE {return 1;}
  int is_public() const FL_OVERRIDE;
  int pixmapID() FL_OVERRIDE { return 12; }
  void write_properties() FL_OVERRIDE;
  void read_property(const char *) FL_OVERRIDE;

  // class prefix attribute access
  void prefix(const char* p);
  const char*  prefix() const {return class_prefix;}
  int has_function(const char*, const char*) const;
};

#endif // _FLUID_FL_FUNCTION_TYPE_H
