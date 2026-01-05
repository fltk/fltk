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

// Type for creating all subclasses of Fl_Widget

#ifndef FLUID_NODES_WIDGET_NODE_H
#define FLUID_NODES_WIDGET_NODE_H

#include "nodes/Node.h"

#include <string>

#define NUM_EXTRA_CODE 4

class Widget_Node;
class Image_Asset;

extern void* const LOAD;
extern Node* current_node; // one of the selected ones
extern Widget_Node* current_widget; // one of the selected ones

extern std::string subclassname(Node* l);
extern int is_name(const char *c);
void selection_changed(Node* new_current);
Node *sort(Node *parent);

class Widget_Node : public Node
{
  typedef Node super;

  virtual Fl_Widget *widget(int,int,int,int) = 0;
  virtual Widget_Node *_make() = 0; // virtual constructor
  void setlabel(const char *) override;

  std::string extra_code_[NUM_EXTRA_CODE];
  std::string subclass_;
  std::string tooltip_;
  std::string image_name_;
  std::string inactive_name_;
  uchar hotspot_ = 0;

  bool menu_headline_ { false };

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
  Fl_Widget *live_widget;

public:
  Fl_Widget *o = nullptr;
  int public_ = 1;
  int bind_image_ = 0;
  int compress_image_ = 1;
  int bind_deimage_ = 0;
  int compress_deimage_ = 1;
  int scale_image_w_ = 0, scale_image_h_ = 0;
  int scale_deimage_w_ = 0, scale_deimage_h_ = 0;

  Image_Asset *image = nullptr;
  void setimage(Image_Asset *);
  Image_Asset *inactive = nullptr;
  void setinactive(Image_Asset *);

  Widget_Node() = default;
  Node *make(Strategy strategy) override;
  void open() override;

  std::string extra_code(int n) const { return extra_code_[n]; }
  void extra_code(int n, const std::string& code);
  std::string subclass() const { return subclass_; }
  void subclass(const std::string& name);
  std::string tooltip() const { return tooltip_; }
  void tooltip(const std::string& text);
  std::string image_name() const { return image_name_; }
  void image_name(const std::string& name);
  std::string inactive_name() const { return inactive_name_; }
  void inactive_name(const std::string& name);
  // Note: hotspot is misused by menu items to indicate a divider
  uchar hotspot() const {return hotspot_;}
  void hotspot(uchar v) {hotspot_ = v;}
  uchar resizable() const;
  void resizable(uchar v);

  bool menu_headline() const { return menu_headline_; }
  void menu_headline(bool v) { menu_headline_ = v; }

  virtual int textstuff(int what, Fl_Font &, int &, Fl_Color &);
  virtual Fl_Menu_Item *subtypes();

  Type type() const override { return Type::Widget_; }
  bool is_a(Type inType) const override { return (inType==Type::Widget_) ? true : super::is_a(inType); }
  int is_widget() const override;
  int is_true_widget() const override { return 1; }
  int is_public() const override;

  void write_properties(fld::io::Project_Writer &f) override;
  void read_property(fld::io::Project_Reader &f, const char *) override;
  int read_fdesign(const char*, const char*) override;

  Fl_Widget *enter_live_mode(int top=0) override;
  Fl_Widget *propagate_live_mode(Fl_Group* grp);
  void leave_live_mode() override;
  void copy_properties() override;

  virtual void ideal_size(int &w, int &h);

  ~Widget_Node();
  void redraw();
};

extern Fl_Window *the_panel;

#endif // FLUID_NODES_WIDGET_NODE_H
