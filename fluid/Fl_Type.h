//
// Widget type header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

/**
 Declare where a new type is placed in the hierarchy.

 Note that a type can also be the start of a hierarchy of types. In that case,

 \see Fl_Type *Fl_..._Type::make(Strategy strategy) calls `add()`
 Add single Type:
    Fl_Type *add_new_widget_from_user(Fl_Type *inPrototype, Strategy strategy, bool and_open)
    Fl_Type *add_new_widget_from_user(const char *inName, Strategy strategy, bool and_open)
    Fl_Type *add_new_widget_from_file(const char *inName, Strategy strategy)
 Add a hierarchy of Types
    void Fl_Type::add(Fl_Type *p, Strategy strategy)
    int read_file(const char *filename, int merge, Strategy strategy)
    Fl_Type *Fd_Project_Reader::read_children(Fl_Type *p, int merge, Strategy strategy, char skip_options)
    int Fd_Project_Reader::read_project(const char *filename, int merge, Strategy strategy)
 */
typedef enum {
  kAddAsFirstChild = 0,
  kAddAsLastChild,
  kAddAfterCurrent
} Strategy;

enum ID {
  // administrative
  ID_Base_, ID_Widget_, ID_Menu_Manager_, ID_Menu_, ID_Browser_, ID_Valuator_,
  // non-widget
  ID_Function, ID_Code, ID_CodeBlock,
  ID_Decl, ID_DeclBlock, ID_Class,
  ID_Widget_Class, ID_Comment, ID_Data,
  // groups
  ID_Window, ID_Group, ID_Pack,
  ID_Flex, ID_Tabs, ID_Scroll,
  ID_Tile, ID_Wizard, ID_Grid,
  // buttons
  ID_Button, ID_Return_Button, ID_Light_Button,
  ID_Check_Button, ID_Repeat_Button, ID_Round_Button,
  // valuators
  ID_Slider, ID_Scrollbar, ID_Value_Slider,
  ID_Adjuster, ID_Counter, ID_Spinner,
  ID_Dial, ID_Roller, ID_Value_Input, ID_Value_Output,
  // text
  ID_Input, ID_Output, ID_Text_Editor,
  ID_Text_Display, ID_File_Input, ID_Terminal,
  // menus
  ID_Menu_Bar, ID_Menu_Button, ID_Choice,
  ID_Input_Choice, ID_Submenu, ID_Menu_Item,
  ID_Checkbox_Menu_Item, ID_Radio_Menu_Item,
  // browsers
  ID_Browser, ID_Check_Browser, ID_File_Browser,
  ID_Tree, ID_Help_View, ID_Table,
  // misc
  ID_Box, ID_Clock, ID_Progress,
  ID_Max_
};

void update_visibility_flag(Fl_Type *p);
void delete_all(int selected_only=0);
int storestring(const char *n, const char * & p, int nostrip=0);

void select_all_cb(Fl_Widget *,void *);
void select_none_cb(Fl_Widget *,void *);
void earlier_cb(Fl_Widget*,void*);
void later_cb(Fl_Widget*,void*);

#ifndef NDEBUG
void print_project_tree();
bool validate_project_tree();
bool validate_independent_branch(class Fl_Type *root);
bool validate_branch(class Fl_Type *root);
#endif

/**
 \brief This is the base class for all elements in the project tree.

 All widgets and other types in the project are derived from Fl_Types. They
 are organized in a doubly linked list. Every Type also has depth information
 to create a pseudo tree structure. To make walking up the tree faster, Type
 also holds a pointer to the `parent` Type.

 Types can be identified using the builtin ID system that works like RTTI. The
 method `id()` returns the exact type, and the method `is_a(ID)` returns true
 if this is the exact type or derived from the type, and a dynamic cast will
 work reliably.

 \todo it would be nice if we can handle multiple independent trees. To do that
 we must remove static members like `first` and `last`.

 \todo add virtual methods to handle events, draw widgets, and draw overlays.
 It may also make sense to have a virtual method that returns a boolean if
 a specific type can be added as a child.

 \todo it may make sense to have a readable iterator class instead of relying
 on pointer manipulation. Or use std in future releases.
 */
class Fl_Type {
  /** Copy the label text to Widgets and Windows, does nothing in Type. */
  virtual void setlabel(const char *) { } // virtual part of label(char*)

protected:

  Fl_Type();

  /** Name of a widget, or code some non-widget Types. */
  const char *name_;
  /** Label text of a widget. */
  const char *label_;
  /** If it is just a word, it's the name of the callback function. Otherwise
   it is the full callback C++ code. Can be NULL. */
  const char *callback_;
  /** Widget user data field as C++ text. */
  const char *user_data_;
  /** Widget user data type as C++ text, usually `void*` or `long`. */
  const char *user_data_type_;
  /** Optional comment for every node in the graph. Visible in browser and
   panels, and will also be copied to the source code. */
  const char *comment_;
  /** a unique ID within the project */
  unsigned short uid_;

public: // things that should not be public:

  /** Quick link to the parent Type instead of walking up the linked list. */
  Fl_Type *parent;
  /** This type is rendered "selected" in the tree browser. */
  char new_selected; // browser highlight
  /** Backup storage for selection if an error occurred during some operation
   (see `haderror`). It seems that this is often confused with new_selected
   which seems to hold the true and visible selection state. */
  char selected; // copied here by selection_changed()
  char folded_;  // if set, children are not shown in browser
  char visible; // true if all parents are open
  int level;    // number of parents over this
  static Fl_Type *first, *last;
  Fl_Type *next, *prev;
  Fl_Type *prev_sibling();
  Fl_Type *next_sibling();
  Fl_Type *first_child();

  Fl_Type *factory;
  const char *callback_name(Fd_Code_Writer& f);

  // text positions of this type in code, header, and project file (see codeview)
  int code_static_start, code_static_end;
  int code1_start, code1_end;
  int code2_start, code2_end;
  int header1_start, header1_end;
  int header2_start, header2_end;
  int header_static_start, header_static_end;
  int proj1_start, proj1_end;
  int proj2_start, proj2_end;

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

  /** Give widgets a change to arrange their children after all children were add.
   If adding individual children, this is called immediately, but if children
   are read via a project file, we wait until all children are read and then
   lay out the group.
   */
  virtual void layout_widget() { }

  static Fl_Type *current;  // most recently picked object
  static Fl_Type *current_dnd;

  virtual void open();  // what happens when you double-click

  // read and write data to a saved file:
  virtual void write(Fd_Project_Writer &f);
  virtual void write_properties(Fd_Project_Writer &f);
  virtual void read_property(Fd_Project_Reader &f, const char *);
  virtual void write_parent_properties(Fd_Project_Writer &f, Fl_Type *child, bool encapsulate);
  virtual void read_parent_property(Fd_Project_Reader &f, Fl_Type *child, const char *property);
  virtual int read_fdesign(const char*, const char*);
  virtual void postprocess_read() { }

  // write code, these are called in order:
  virtual void write_static(Fd_Code_Writer& f); // write static stuff to .c file
  virtual void write_static_after(Fd_Code_Writer& f); // write static stuff after children
  virtual void write_code1(Fd_Code_Writer& f); // code and .h before children
  virtual void write_code2(Fd_Code_Writer& f); // code and .h after children
  void write_comment_h(Fd_Code_Writer& f, const char *ind=""); // write the commentary text into the header file
  void write_comment_c(Fd_Code_Writer& f, const char *ind=""); // write the commentary text into the source file
  void write_comment_inline_c(Fd_Code_Writer& f, const char *ind=0L); // write the commentary text

  // live mode
  virtual Fl_Widget *enter_live_mode(int top=0); // build widgets needed for live mode
  virtual void leave_live_mode(); // free allocated resources
  virtual void copy_properties(); // copy properties from this type into a potential live object
  virtual void copy_properties_for_children() { } // copy remaining properties after children were added

  // get message number for I18N
  int msgnum();

  /** Return 1 if the Type can have children. */
  virtual int can_have_children() const {return 0;}
  /** Return 1 if the type is a widget or menu item. */
  virtual int is_widget() const {return 0;}
  /** Return 1 if the type is a widget but not a menu item. */
  virtual int is_true_widget() const {return 0;}
  /** Return 1 if a type behaves like a button (Fl_Button and Fl_Menu_Item and derived, but not Fl_Submenu_Type. */
  virtual int is_button() const {return 0;}
  /** Return 1 if this is a Fl_Widget_Class_Type, Fl_CodeBlock_Type, or Fl_Function_Type */
  virtual int is_code_block() const {return 0;}
  /** Return 1 if this is a Fl_Widget_Class_Type, Fl_Class_Type, or Fl_DeclBlock_Type */
  virtual int is_decl_block() const {return 0;}
  /** Return 1 if this is a Fl_Class_Type or Fl_Widget_Class_Type. */
  virtual int is_class() const {return 0;}
  /** Return 1 if the type browser shall draw a padlock over the icon. */
  virtual int is_public() const {return 1;}
  /** Return the type ID for this Type. */
  virtual ID id() const { return ID_Base_; }
  /** Check if this Type is of the give type ID or derived from that type ID. */
  virtual bool is_a(ID inID) const { return (inID==ID_Base_); }

  const char* class_name(const int need_nest) const;
  bool is_in_class() const;

  int has_function(const char*, const char*) const;

  unsigned short set_uid(unsigned short suggested_uid=0);
  unsigned short get_uid() { return uid_; }
  static Fl_Type *find_by_uid(unsigned short uid);

  static Fl_Type *find_in_text(int text_type, int crsr);

  /// If this is greater zero, widgets will be allowed to lay out their children.
  static int allow_layout;
};

#endif // _FLUID_FL_TYPE_H
