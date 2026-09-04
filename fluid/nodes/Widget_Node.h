//
// Widget Node header file for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_NODES_WIDGET_NODE_H
#define FLUID_NODES_WIDGET_NODE_H

// ---- Fluid includes

#include "nodes/Node.h"
#include "nodes/Widget_Image.h"

// ---- C++ includes

#include <string>

// ---- Forward Declarations

class Widget_Node;

constexpr int NUM_EXTRA_CODE = 4;

extern void* const LOAD;
extern Node* current_node;          // one of the selected ones
extern Widget_Node* current_widget; // one of the selected ones
extern Fl_Window* the_panel;

extern std::string subclassname(Node* l);
extern bool is_function_name(const std::string& name);
extern bool is_lambda(const std::string& name);
extern void selection_changed(Node* new_current);
extern Node* sort(Node* parent);

/**
 Base class for all widget nodes including menu items.
 */
class Widget_Node : public Node
{
  typedef Node super;

private: // Member Variables

  /// Additional code blocks that can be inserted in the generated code
  std::string extra_code_[NUM_EXTRA_CODE];

  /// User can call the ctor for a class that is derived from the node class
  std::string subclass_;

  /// Keep a copy the tooltip here, also always updates the widget's tooltip
  std::string tooltip_;

  /// Set's a widget's hotspot, or adds a divider to a menu item
  uchar hotspot_ = 0;

protected:

  /// This variable is set for visible windows in batch mode.
  /// We can't open a window in batch mode, even if we want the "visible" flags
  /// set, so we need a second place to store this information while also
  /// disabling the output of the "hide" property by the Widget Type.
  uchar override_visible_ = 0;

  /// Pointer to widget representing this node in live mode.
  Fl_Widget* live_widget = nullptr;

public:

  /// Pointer to widget for interactive editing.
  Fl_Widget* o = nullptr;

  /// Widget access mode, 0=private, 1=public, 2=protected
  int public_ = 1;


  // ---- Image stuff

  /// Active (normal state) image data and display options.
  Widget_Image active_image;

  /// Inactive (deactivated state) image data and display options.
  Widget_Image inactive_image;

private: // Methods

  // Override this to generate a widget, representing this node
  virtual Fl_Widget* widget(int X, int Y, int W, int H) = 0;

  // Override this, so the factory can build a node based on its prototype
  virtual Widget_Node* _make() = 0; // virtual constructor

  // Override to call `label()` for the right subclass of the widget in `o`.
  void setlabel(const std::string&) override;

protected:


  // ---- Code Writer functions for generating source code for this widget

  // Write the static initializer code for the widget
  void write_static(fluid::io::Code_Writer& f) override;

  // Write the code that creates the widget before the children are created
  void write_code1(fluid::io::Code_Writer& f) override;

  // Write the code after the children of the widget are created
  void write_code2(fluid::io::Code_Writer& f) override;

  // Write the code to set properties, called from write_code1
  void write_widget_code(fluid::io::Code_Writer& f);

  // Write the code to end instantiation of this widget, called by write_code2
  void write_block_close(fluid::io::Code_Writer& f);

  // Helper to write code for a color property using symbolic names
  void write_color(fluid::io::Code_Writer& f, const char*, Fl_Color);


  // ---- Construction and Destruction

  // Never called directly, called by constructor of derived class
  Widget_Node() = default;

public:

  // Return all resources
  ~Widget_Node() override;

  // Delete all copy and move operators
  Widget_Node(const Widget_Node&) = delete;
  Widget_Node& operator=(const Widget_Node&) = delete;
  Widget_Node(Widget_Node&&) = delete;
  Widget_Node& operator=(Widget_Node&&) = delete;


  // ---- Node Lifetime Management

  // Make a new Node and add it to the tree
  Node* make(Strategy strategy) override;

  // Help the user create the required hierarchy for this widget
  bool node_creation_assistant(Strategy& strategy, Node*& anchor) override;

  // Open the Node editor panel
  void open() override;

  // Find the ideal size for this widget
  virtual void ideal_size(int& w, int& h);

  /// Return a list of subtypes for this widget class as a pulldown menu
  virtual Fl_Menu_Item* subtypes() { return nullptr; }

  // Redraw after cahnges via widget panel, rebuilds menu item arrays if needed
  void redraw();

  // Return true for Widget_Node and derived classes
  int is_widget() const override { return true; }

  // Return true for Widget_Node and derived classes that are not menu items
  int is_true_widget() const override { return 1; }

  // Return true for Widget_Node and derived classes that are buttons
  int is_public() const override { return public_; }


  // ---- Handle user created code blocks

  /// Return one of the extra code blocks
  const std::string& extra_code(int n) const { return extra_code_[n]; }

  // Set one of the extra code blocks cropping leading and trailing whitespace
  void extra_code(int n, const std::string& code);

  // Add one or more lines of code to one of the extra code blocks
  void extra_code_append(int n, const std::string& code);


  // ---- Getter and setter for various properties

  // Subclass can be an arbitraty string
  std::string subclass() const { return subclass_; }
  void subclass(const std::string& name);

  // Tooltip text, can contain newlines
  std::string tooltip() const { return tooltip_; }
  void tooltip(const std::string& text);

  // Set to 1 if this widget is the hotspot for a dialog box, i.e. this
  // widget will be used to position a dialog box under the mouse pointer.
  // Note: hotspot is reused by menu items to indicate a divider
  uchar hotspot() const { return hotspot_; }
  void hotspot(uchar v) { hotspot_ = v; }

  // Set to 1 if this widget is the resizable widget for a window or group
  uchar resizable() const;
  void resizable(uchar v);

  // Read or write text attributes in widgets that support text, like Fl_Input, Fl_Text_Display, etc.
  virtual int textstuff(int what, Fl_Font&, int&, Fl_Color&);


  // ---- Read and write project files

  // Write all properties of this node, calls super class to write more properties
  void write_properties(fluid::io::Project_Writer& f) override;

  // Read a property of this node, calls super class if property is not recognized
  void read_property(fluid::io::Project_Reader& f, const char*) override;

  // Back compatibility to Forms FDesign project files
  int read_fdesign(const char*, const char*) override;


  // ---- Live mode support

  // Create a live mode widget and all its children
  Fl_Widget* propagate_live_mode(Fl_Group* grp);

  // Create a live widget for this node
  Fl_Widget* enter_live_mode() override;

  // Copy the properties from the edit widget to the live widget
  void copy_properties() override;
};

#endif // FLUID_NODES_WIDGET_NODE_H
