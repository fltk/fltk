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

#include "nodes/Node.h"
#include "nodes/Widget_Image.h"

#include <string>

class Widget_Node;

constexpr int NUM_EXTRA_CODE = 4;

extern void* const LOAD;
extern Node* current_node;          // one of the selected ones
extern Widget_Node* current_widget; // one of the selected ones
extern Fl_Window* the_panel;

extern std::string subclassname(Node* l);
extern int is_name(const char* c);
extern void selection_changed(Node* new_current);
extern Node* sort(Node* parent);

/**
 Base class for all widget nodes including menu items.
 */
class Widget_Node : public Node {
  typedef Node super;

  virtual Fl_Widget* widget(int, int, int, int) = 0;
  virtual Widget_Node* _make() = 0; // virtual constructor
  void setlabel(const char*) override;

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

  void write_static(fld::io::Code_Writer& f) override;
  void write_code1(fld::io::Code_Writer& f) override;
  void write_widget_code(fld::io::Code_Writer& f);
  void write_extra_code(fld::io::Code_Writer& f);
  void write_block_close(fld::io::Code_Writer& f);
  void write_code2(fld::io::Code_Writer& f) override;
  void write_color(fld::io::Code_Writer& f, const char*, Fl_Color);

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

  Widget_Node() = default;
  ~Widget_Node() override;

  Node* make(Strategy strategy) override;
  void open() override;
  virtual void ideal_size(int& w, int& h);

  const std::string& extra_code(int n) const { return extra_code_[n]; }
  void extra_code(int n, const std::string& code);

  std::string subclass() const { return subclass_; }
  void subclass(const std::string& name);

  std::string tooltip() const { return tooltip_; }
  void tooltip(const std::string& text);

  // Note: hotspot is reused by menu items to indicate a divider
  uchar hotspot() const { return hotspot_; }
  void hotspot(uchar v) { hotspot_ = v; }

  uchar resizable() const;
  void resizable(uchar v);

  virtual int textstuff(int what, Fl_Font&, int&, Fl_Color&);

  virtual Fl_Menu_Item* subtypes();

  Type type() const override { return Type::Widget_; }
  bool is_a(Type inType) const override
  {
    return (inType == Type::Widget_) ? true : super::is_a(inType);
  }
  int is_widget() const override;
  int is_true_widget() const override { return 1; }
  int is_public() const override;

  void write_properties(fld::io::Project_Writer& f) override;
  void read_property(fld::io::Project_Reader& f, const char*) override;
  int read_fdesign(const char*, const char*) override;

  Fl_Widget* enter_live_mode(int top = 0) override;
  Fl_Widget* propagate_live_mode(Fl_Group* grp);
  void leave_live_mode() override;
  void copy_properties() override;

  void redraw();
};

#endif // FLUID_NODES_WIDGET_NODE_H
