//
// Node base class header file for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_NODES_NODE_H
#define FLUID_NODES_NODE_H

#include "io/Code_Writer.h"

#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

class Node;
class Group_Node;
class Window_Node;

namespace fld {
namespace io {

class Project_Reader;
class Project_Writer;

} // namespace io
} // namespace fld

/**
 Declare where a new type is placed and how to create it.

 Placement can be as the first or last child of the anchor, or right after the
 anchor. In most cases, the anchor is the last selected type node.

 If the source is FROM_USER, widgets may be created with default titles and
 labels. Type created FROM_FILE will start with no label, so the label is set
 correctly later.

 \see Node *Fl_..._Type::make(Strategy strategy) calls `add()`
 Add single Type:
    Node *add_new_widget_from_user(Node *inPrototype, Strategy strategy, bool and_open)
    Node *add_new_widget_from_user(const char *inName, Strategy strategy, bool and_open)
    Node *add_new_widget_from_file(const char *inName, Strategy strategy)
 Add a hierarchy of Types
    void Node::add(Node *p, Strategy strategy)
    int read_file(const char *filename, int merge, Strategy strategy)
    Node *fld::io::Project_Reader::read_children(Node *p, int merge, Strategy strategy, char skip_options)
    int fld::io::Project_Reader::read_project(const char *filename, int merge, Strategy strategy)
 */
typedef struct Strategy {
  enum Flags {
    AS_FIRST_CHILD = 0x0000,
    AS_LAST_CHILD  = 0x0001,
    AFTER_CURRENT  = 0x0002,
    PLACEMENT_MASK = 0x000f,
    FROM_USER      = 0x0000,
    FROM_FILE      = 0x0010,
    SOURCE_MASK    = 0x00f0,
    FROM_FILE_AS_FIRST_CHILD = 0x0010,
    FROM_FILE_AS_LAST_CHILD  = 0x0011,
    FROM_FILE_AFTER_CURRENT  = 0x0012,
  };
  Flags flags;
  Strategy(Flags f) { flags = f; }
  void placement(Flags f) { flags = (Flags)((flags & ~PLACEMENT_MASK) | (f & PLACEMENT_MASK)); }
  Flags placement() { return (Flags)(flags & PLACEMENT_MASK); }
  void source(Flags f) { flags = (Flags)((flags & ~SOURCE_MASK) | (f & SOURCE_MASK)); }
  Flags source() { return (Flags)(flags & SOURCE_MASK); }
} Strategy;

enum class Type {
  // administrative
  Base_, Widget_, Menu_Manager_, Menu_, Browser_, Valuator_,
  // non-widget
  Function, Code, CodeBlock,
  Decl, DeclBlock, Class,
  Widget_Class, Comment, Data,
  // groups
  Window, Group, Pack,
  Flex, Tabs, Scroll,
  Tile, Wizard, Grid,
  // buttons
  Button, Return_Button, Light_Button,
  Check_Button, Repeat_Button, Round_Button,
  // valuators
  Slider, Scrollbar, Value_Slider,
  Adjuster, Counter, Spinner,
  Dial, Roller, Value_Input, Value_Output,
  // text
  Input, Output, Text_Editor,
  Text_Display, File_Input, Terminal,
  // menus
  Menu_Bar, Menu_Button, Choice,
  Input_Choice, Submenu, Menu_Item,
  Checkbox_Menu_Item, Radio_Menu_Item,
  // browsers
  Browser, Check_Browser, File_Browser,
  Tree, Help_View, Table,
  // misc
  Box, Clock, Progress,
  Max_
};

void update_visibility_flag(Node *p);
void delete_all(int selected_only=0);
int storestring(const char *n, const char * & p, int nostrip=0);

void select_all_cb(Fl_Widget *,void *);
void select_none_cb(Fl_Widget *,void *);
void earlier_cb(Fl_Widget*,void*);
void later_cb(Fl_Widget*,void*);

#ifndef NDEBUG
void print_project_tree();
bool validate_project_tree();
bool validate_independent_branch(class Node *root);
bool validate_branch(class Node *root);
#endif

/**
 \brief This is the base class for all elements in the project tree.

 All widgets and other types in the project are derived from Fl_Types. They
 are organized in a doubly linked list. Every Type also has depth information
 to create a pseudo tree structure. To make walking up the tree faster, Type
 also holds a pointer to the `parent` Type.

 Types can be identified using the builtin Type system that works like RTTI. The
 method `type()` returns the exact type, and the method `is_a(Type)` returns true
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
class Node {
  /** Copy the label text to Widgets and Windows, does nothing in Type. */
  virtual void setlabel(const char *) { } // virtual part of label(char*)

protected:

  Node();

  /** Name of a widget, or code some non-widget Types. */
  const char *name_;
  /** Label text of a widget. */
  const char *label_;
  /** If it is just a word, it's the name of the callback function. Otherwise
   it is the full callback C++ code. Can be nullptr. */
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
  // TODO: reference back to the tree
  /** Quick link to the parent Type instead of walking up the linked list. */
  Node *parent;
  /** This type is rendered "selected" in the tree browser. */
  char new_selected; // browser highlight
  /** Backup storage for selection if an error occurred during some operation
   (see `haderror`). It seems that this is often confused with new_selected
   which seems to hold the true and visible selection state. */
  char selected; // copied here by selection_changed()
  char folded_;  // if set, children are not shown in browser
  char visible; // true if all parents are open
  int level;    // number of parents over this
  Node *next, *prev;
  Node *prev_sibling();
  Node *next_sibling();
  Node *first_child();

  Node *factory;
  const char *callback_name(fld::io::Code_Writer& f);

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

  virtual ~Node();
  virtual Node *make(Strategy strategy) = 0;

  Window_Node *window();
  Group_Node *group();

  void add(Node *parent, Strategy strategy);
  void insert(Node *n); // insert into list before n
  Node* remove();    // remove from list
  void move_before(Node*); // move before a sibling

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

  virtual Node* click_test(int,int) { return nullptr; }

  virtual void add_child(Node*, Node* beforethis) { }
  virtual void move_child(Node*, Node* beforethis) { }
  virtual void remove_child(Node*) { }

  /** Give widgets a chance to arrange their children after all children were added.
   If adding individual children, this is called immediately, but if children
   are read via a project file, we wait until all children are read and then
   lay out the group.
   */
  virtual void layout_widget() { }

  virtual void open();  // what happens when you double-click

  // read and write data to a saved file:
  virtual void write(fld::io::Project_Writer &f);
  virtual void write_properties(fld::io::Project_Writer &f);
  virtual void read_property(fld::io::Project_Reader &f, const char *);
  virtual void write_parent_properties(fld::io::Project_Writer &f, Node *child, bool encapsulate);
  virtual void read_parent_property(fld::io::Project_Reader &f, Node *child, const char *property);
  virtual int read_fdesign(const char*, const char*);
  virtual void postprocess_read() { }

  // write code, these are called in order:
  virtual void write_static(fld::io::Code_Writer& f); // write static stuff to .c file
  virtual void write_static_after(fld::io::Code_Writer& f); // write static stuff after children
  virtual void write_code1(fld::io::Code_Writer& f); // code and .h before children
  virtual void write_code2(fld::io::Code_Writer& f); // code and .h after children
  void write_comment_h(fld::io::Code_Writer& f, const char *ind=""); // write the commentary text into the header file
  void write_comment_c(fld::io::Code_Writer& f, const char *ind=""); // write the commentary text into the source file
  void write_comment_inline_c(fld::io::Code_Writer& f, const char *ind=nullptr); // write the commentary text

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
  /** Return 1 if a type behaves like a button (Fl_Button and Fl_Menu_Item and derived, but not Submenu_Node. */
  virtual int is_button() const {return 0;}
  /** Return 1 if this is a Widget_Class_Node, CodeBlock_Node, or Function_Node */
  virtual int is_code_block() const {return 0;}
  /** Return 1 if this is a Widget_Class_Node, Class_Node, or DeclBlock_Node */
  virtual int is_decl_block() const {return 0;}
  /** Return 1 if this is a Class_Node or Widget_Class_Node. */
  virtual int is_class() const {return 0;}
  /** Return 1 if the type browser shall draw a padlock over the icon. */
  virtual int is_public() const {return 1;}
  /** Return the type Type for this Type. */
  virtual Type type() const { return Type::Base_; }
  /** Check if this Type is of the give type Type or derived from that type Type. */
  virtual bool is_a(Type inType) const { return (inType==Type::Base_); }

  const char* class_name(const int need_nest) const;
  bool is_in_class() const;

  int has_function(const char*, const char*) const;

  unsigned short set_uid(unsigned short suggested_uid=0);
  unsigned short get_uid() { return uid_; }
};

#endif // FLUID_NODES_NODE_H
