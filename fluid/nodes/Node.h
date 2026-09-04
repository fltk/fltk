//
// Node base class header file for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_NODES_NODE_H
#define FLUID_NODES_NODE_H

// ---- Fluid includes

#include "io/Code_Writer.h"
#include "nodes/iterators.h"

// ---- FLTK includes

#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

// ---- C++ includes

#include <string>

// ---- Forward Declarations

class Node;
class Group_Node;
class Window_Node;

namespace fluid {
namespace io {

class Project_Reader;
class Project_Writer;

} // namespace io
} // namespace fluid


/**
 Declare where a new type is placed and how to create it.

 Placement can be as the first or last child of the anchor, or right after the
 anchor. In most cases, the anchor is the last selected node.

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
    Node *fluid::io::Project_Reader::read_children(Node *p, int merge, Strategy strategy, char skip_options)
    int fluid::io::Project_Reader::read_project(const char *filename, int merge, Strategy strategy)
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


/** A half-open [start, end) character range into a generated text buffer, used by codeview. */
struct TextSpan {
  int start = -1, end = -1;
};


/** Hold two text spans. */
struct TextSpan2 {
  TextSpan h, c;
};


int storestring(const char *n, const char * & p, int nostrip=0);
int storestring(const std::string& n, std::string& p, int nostrip=0);

void select_all_cb(Fl_Widget *,void *);
void select_none_cb(Fl_Widget *,void *);
void earlier_cb(Fl_Widget*,void*);
void later_cb(Fl_Widget*,void*);


/**
 \brief Base class for all entries in Fluid's project tree.

 Node is the common base of all objects that appear in a Fluid project,
 including widgets, groups, functions, declarations, and comments.

 The tree is stored as a doubly linked list in depth-first order. Structure is
 represented by \c level (depth), \c parent, and sibling/child navigation helpers.
 This keeps traversal and insertion efficient while preserving hierarchy.

 Node also defines the polymorphic interface used by Fluid for:
 - project file read/write,
 - C/C++ code generation,
 - type classification and editor behavior,
 - optional live-mode support.

 Use \c dynamic_cast to test whether a node is of a given type or derived type.
 For exact type checks, compare \c typeid(*node) with \c typeid(SomeNode).
 */
class Node
{
  // ---- Node Properties
protected:
  /// Name of a widget, or code some non-widget Types
  const char* name_ { nullptr };

  /// Label text of a widget
  const char* label_ { nullptr };

  /// Callback function name, lambda, or function code
  const char* callback_ { nullptr };

  /// Widget user data field as C++ text.
  std::string user_data_ { };

  /// Widget user data type as C++ text, usually `void*` or `long`.
  std::string user_data_type_ { };

  /// Optional comment, visible in browser and in the source code
  const char* comment_ { nullptr };


  // ---- Properties that should probably not be public
public:
  /// Mark the node as one of the selected ones
  char selected { 0 };

  /// Backup marker, so we can undo a selection if an error occurs
  char backup_selected { 0 };

  /// If set, children are not shown in browser
  char folded_ { 0 };

  // True if none of the parents are folded_
  char visible { 0 };

  // Pointing back to the prototype that generated this node
  Node* factory { nullptr };

  // Text positions of this node in code, header, and project file (see codeview)
  TextSpan2 static_data;
  TextSpan2 setup_node;
  TextSpan2 finalize_node;
  TextSpan proj1, proj2;


  // ---- Construction and Destruction
protected:
  // Never called directly, called by constructor of derived class
  Node() = default;
public:
  // Return all resources, free links to this node
  virtual ~Node();

  // Delete all copy and move operators
  Node(const Node&) = delete;
  Node& operator=(const Node&) = delete;
  Node(Node&&) = delete;
  Node& operator=(Node&&) = delete;


  // ---- Node tree management, doubly linked list with depth level
public:
  /// Double linked list of all nodes in the project tree, in depth-first order.
  Node* next { nullptr };
  Node* prev { nullptr };

  /// Quick link to the parent Type instead of walking up the linked list
  Node* parent { nullptr };

  /// Depth within the tree, 0 for top-level nodes, 1 for children of top-level nodes, etc.
  int level { 0 };

  // ------ Node topology access

  // Previous entry of the same depth or nullptr
  Node* prev_sibling();

  // Next entry of the same depth or nullptr
  Node* next_sibling();
  const Node* next_sibling() const { return const_cast<Node*>(this)->next_sibling(); }

  // First child node or nullptr
  Node* first_child();
  const Node* first_child() const { return const_cast<Node*>(this)->first_child(); }

  /// Range over the direct children of this node (`for (auto *c : n->children())`)
  Child_Range children() { return Child_Range(first_child()); }

  /// Const range over the direct children of this node (`for (const auto *c : n->children())`).
  Const_Child_Range children() const { return Const_Child_Range(first_child()); }

  /// Range over all descendants of this node, depth-first (`for (auto *d : n->descendants())`).
  Descendant_Range descendants() { return Descendant_Range(this); }

  // ------ Node topology modification

  /// Add node or branch to this node
  void add(Node* parent, Strategy strategy);

  /// Insert node or branch before n
  void insert(Node* n);

  // Delete all children of this node
  void delete_children();

  // Remove node from the list
  Node* remove();

  // move before a sibling
  void move_before(Node*);

  // ------ Callbacks for changes in Node topology

  /// Handle adding a child
  virtual void add_child(Node*, Node* beforethis) { (void)beforethis; }

  /// Handle moving a child
  virtual void move_child(Node*, Node* beforethis) { (void)beforethis; }

  /// Handle removing a child
  virtual void remove_child(Node*) { }


  // ---- Node Lifetime and Interaction

  // Make a new Node and add it to the tree
  virtual Node* make(Strategy strategy) = 0;

  /// Give widgets a chance to arrange their children after all children were added.
  virtual void layout_widget() { }

  // What happens when you double-click
  virtual void open();

  // Update flags
  void update_visibility_flag();

  // Handle mouse clicks on widget nodes
  virtual Node* click_test(int,int) { return nullptr; }

  // Open a multiple choice dialog to help the user place the node correctly in the tree.
  virtual bool node_creation_assistant(Strategy& strategy, Node*& anchor) { return false; }

  // ---- Getter and setter for various properties
public:
  // Name of the node, used for code generation and as a unique identifier in the project.
  const char* name() const { return name_; }
  void name(const char*);

  // Label text of the node, used for widgets and windows.
  const char* label() const { return label_; }
  void label(const char*);

  // Copy the label text to Widgets and Windows, does nothing in base Node.
  virtual void setlabel(const char *) { } // virtual part of label(char*)

  // Callback name, callback code, or lambda function for the node, used for widgets and windows.
  const char* callback() const { return callback_; }
  void callback(const char*);
  std::string callback_name(fluid::io::Code_Writer& f);

  // User data associated with the node.
  std::string user_data() const { return user_data_; }
  void user_data(const std::string&);

  // User data type associated with the node.
  std::string user_data_type() const { return user_data_type_; }
  std::string user_data_type_or_voidp() const { return user_data_type_.empty() ? "void*" : user_data_type_; }
  void user_data_type(const std::string&);

  // Optional comment for the node, used for documentation and code generation.
  const char* comment() { return comment_; }
  void comment(const char*);

  // Find the window node that contains this node, or nullptr if not in a window.
  Window_Node* window();

  // Find the group node that contains this node, or nullptr if not in a group.
  Group_Node* group();

  // get message number for I18N
  int msgnum();

  // Check if this node has a function with the given return type and signature, using regex matching.
  bool has_function(const std::string& return_type_regex, const std::string& function_sig_regex) const;

  // The node name, or something else human readable if there is no name
  virtual const char* title(); // string for browser

  // FLTK 1 name of the underlying type
  virtual const char* type_name() = 0; // type for code output

  // fltk 2 name for back compatibility
  virtual const char* alt_type_name() { return type_name(); }


  // ---- Code Writer functions for generating source code for this widget
public:
  // Write the static initializer code for the node
  virtual void write_static(fluid::io::Code_Writer& f);

  // Write static stuff after children
  virtual void write_static_after(fluid::io::Code_Writer& f);

  // Write the code that creates the node before the children are created
  virtual void write_code1(fluid::io::Code_Writer& f); // code and .h before children

  // Write the code after the children of the node are created
  virtual void write_code2(fluid::io::Code_Writer& f); // code and .h after children

  // Write the commentary text into the header file
  void write_comment_h(fluid::io::Code_Writer& f, const char *ind="");

  // Write the commentary text into the source file
  void write_comment_c(fluid::io::Code_Writer& f, const char *ind="");

  // Write the commentary text
  void write_comment_inline_c(fluid::io::Code_Writer& f, const char *ind=nullptr);


  // ---- Read and write project files
public:
  // Write this node and all its children to the project file
  virtual void write(fluid::io::Project_Writer& f);

  // Write the properties of this node to the project file
  virtual void write_properties(fluid::io::Project_Writer& f);

  // Write properties that the parent stores for this node
  virtual void write_parent_properties(fluid::io::Project_Writer& f, Node *child, bool encapsulate);

  // Read one property of this node, or call the parent if not recognized
  virtual void read_property(fluid::io::Project_Reader& f, const char *);

  // Read properties that the parent stores for this node
  virtual void read_parent_property(fluid::io::Project_Reader& f, Node *child, const char *property);

  // Fixup nodes after all children are read
  virtual void postprocess_read() { }

  // Back compatibility to Forms FDesign project files
  virtual int read_fdesign(const char*, const char*);


  // ---- Type classification methods
public:
  /// Return 1 if the Type can have children.
  virtual int can_have_children() const { return 0; }

  /// Return 1 if the type is a widget or menu item.
  virtual int is_widget() const { return 0; }

  /// Return 1 if the type is a widget but not a menu item.
  virtual int is_true_widget() const { return 0; }

  /// Return 1 if a type behaves like a button (Fl_Button and Fl_Menu_Item and derived, but not Submenu_Node.
  virtual int is_button() const { return 0; }

  /// Return 1 if this is a Widget_Class_Node, CodeBlock_Node, or Function_Node
  virtual int is_code_block() const { return 0; }

  /// Return 1 if this is a Widget_Class_Node, Class_Node, or DeclBlock_Node
  virtual int is_decl_block() const { return 0; }

  /// Return 1 if this is a Class_Node or Widget_Class_Node
  virtual int is_class() const { return 0; }

  /// Return 1 if the type browser shall draw a padlock over the icon.
  virtual int is_public() const { return 1; }


  // ---- Handle enclosing class
public:
  // Return the name of the first enclosing class
  std::string class_name() const;

  // Return the name of all enclosing classes, separated by '::'
  std::string full_class_name() const;

  // Check if node is inside a class or widget class
  bool is_in_class() const;

  // Return the enclosing class node or widget class node, or nullptr
  Node* find_parent_class_node() const;


  // ---- Unique ID management
private:
  /// a unique ID within the project
  unsigned short uid_ { 0 };
public:
  // Set the given UID, or find a new one if it already appears in the tree
  unsigned short set_uid(unsigned short suggested_uid=0);

  // Ensure that the current UID is unique within the project, optionally replace it, and return it.
  unsigned short ensure_unique_uid() { return set_uid(uid_); }

  // Get the unique ID for this Node
  unsigned short get_uid() { return uid_; }


  // ---- Live mode support

  // Build widgets needed for live mode
  virtual Fl_Widget *enter_live_mode();

  // Copy the properties from the edit widget to the live widget
  virtual void copy_properties();

  // Copy remaining properties after children were added
  virtual void copy_properties_for_children() { }
};

#endif // FLUID_NODES_NODE_H
