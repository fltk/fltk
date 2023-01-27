//
// Widget type header file for the Fast Light Tool Kit (FLTK).
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

#ifndef _FLUID_FL_TYPE_H
#define _FLUID_FL_TYPE_H

#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

#include "code.h"

class Fl_Type;
class Fl_Group_Type;
class Fl_Window_Type;

class Fd_Project_Reader;
class Fd_Project_Writer;

typedef enum {
  kAddAsLastChild = 0,
  kAddAfterCurrent
} Strategy;

void fixvisible(Fl_Type *p);
void delete_all(int selected_only=0);
int storestring(const char *n, const char * & p, int nostrip=0);

void select_all_cb(Fl_Widget *,void *);
void select_none_cb(Fl_Widget *,void *);
void earlier_cb(Fl_Widget*,void*);
void later_cb(Fl_Widget*,void*);

class Fl_Type {

  friend class Widget_Browser;
  friend Fl_Widget *make_type_browser(int,int,int,int,const char *);
  friend class Fl_Window_Type;

  virtual void setlabel(const char *) { } // virtual part of label(char*)

protected:

  Fl_Type();

  const char *name_;
  const char *label_;
  const char *callback_;
  const char *user_data_;
  const char *user_data_type_;
  const char *comment_;

public: // things that should not be public:

  Fl_Type *parent;
  char new_selected; // browser highlight
  char selected; // copied here by selection_changed()
  char open_;   // state of triangle in browser
  char visible; // true if all parents are open
  int level;    // number of parents over this
  static Fl_Type *first, *last;
  Fl_Type *next, *prev;
  Fl_Type *prev_sibling();
  Fl_Type *next_sibling();
  Fl_Type *first_child();

  Fl_Type *factory;
  const char *callback_name(Fd_Code_Writer& f);

  int code_position, header_position;
  int code_position_end, header_position_end;

protected:
  int user_defined(const char* cbname) const;

public:

  virtual ~Fl_Type();
  virtual Fl_Type *make(Strategy strategy) = 0;

  Fl_Window_Type *window();
  Fl_Group_Type *group();

  void add(Fl_Type *parent, Strategy strategy);
  void insert(Fl_Type *n); // insert into list before n
  Fl_Type* remove();    // remove from list
  void move_before(Fl_Type*); // move before a sibling

  virtual const char *title(); // string for browser
  virtual const char *type_name() = 0; // type for code output
  virtual const char *alt_type_name() { return type_name(); } // alternate type for FLTK2 code output

  const char *name() const {return name_;}
  void name(const char *);
  const char *label() const {return label_;}
  void label(const char *);
  const char *callback() const {return callback_;}
  void callback(const char *);
  const char *user_data() const {return user_data_;}
  void user_data(const char *);
  const char *user_data_type() const {return user_data_type_;}
  void user_data_type(const char *);
  const char *comment() { return comment_; }
  void comment(const char *);

  virtual Fl_Type* click_test(int,int) { return NULL; }
  virtual void add_child(Fl_Type*, Fl_Type* beforethis) { }
  virtual void move_child(Fl_Type*, Fl_Type* beforethis) { }
  virtual void remove_child(Fl_Type*) { }

  static Fl_Type *current;  // most recently picked object
  static Fl_Type *current_dnd;

  virtual void open();  // what happens when you double-click

  // read and write data to a saved file:
  virtual void write(Fd_Project_Writer &f);
  virtual void write_properties(Fd_Project_Writer &f);
  virtual void read_property(Fd_Project_Reader &f, const char *);
  virtual int read_fdesign(const char*, const char*);
  virtual void postprocess_read() { }

  // write code, these are called in order:
  virtual void write_static(Fd_Code_Writer& f); // write static stuff to .c file
  virtual void write_code1(Fd_Code_Writer& f); // code and .h before children
  virtual void write_code2(Fd_Code_Writer& f); // code and .h after children
  void write_comment_h(Fd_Code_Writer& f, const char *ind=""); // write the commentary text into the header file
  void write_comment_c(Fd_Code_Writer& f, const char *ind=""); // write the commentary text into the source file
  void write_comment_inline_c(Fd_Code_Writer& f, const char *ind=0L); // write the commentary text

  // live mode
  virtual Fl_Widget *enter_live_mode(int top=0); // build wdgets needed for live mode
  virtual void leave_live_mode(); // free allocated resources
  virtual void copy_properties(); // copy properties from this type into a potetial live object

  // get message number for I18N
  int msgnum();

  // fake rtti:
  virtual int is_parent() const {return 0;}
  virtual int is_widget() const {return 0;}
  virtual int is_button() const {return 0;}
  virtual int is_input() const {return 0;}
  virtual int is_value_input() const {return 0;}
  virtual int is_text_display() const {return 0;}
  virtual int is_valuator() const {return 0;}
  virtual int is_spinner() const {return 0;}
  virtual int is_menu_item() const {return 0;}
  virtual int is_menu_button() const {return 0;}
  virtual int is_group() const {return 0;}
  virtual int is_flex() const {return 0;}
  virtual int is_window() const {return 0;}
  virtual int is_code() const {return 0;}
  virtual int is_code_block() const {return 0;}
  virtual int is_decl_block() const {return 0;}
  virtual int is_comment() const {return 0;}
  virtual int is_class() const {return 0;}
  virtual int is_public() const {return 1;}

  virtual int pixmapID() { return 0; }

  const char* class_name(const int need_nest) const;
  const class Fl_Class_Type* is_in_class() const;
};

#endif // _FLUID_FL_TYPE_H
