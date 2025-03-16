//
// Widget Node header file for the Fast Light Tool Kit (FLTK).
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

// Type for creating all subclasses of Fl_Widget
// This should have the widget pointer in it, but it is still in the
// Node base class.

#ifndef FLUID_NODES_WIDGET_NODE_H
#define FLUID_NODES_WIDGET_NODE_H

#include "nodes/Node.h"

#define NUM_EXTRA_CODE 4

class Widget_Node;
class Image_Asset;

extern void* const LOAD;
extern Widget_Node *current_widget; // one of the selected ones

extern const char* subclassname(Node* l);
extern int is_name(const char *c);
void selection_changed(Node* new_current);
Node *sort(Node *parent);

class Widget_Node : public Node
{
  typedef Node super;

  virtual Fl_Widget *widget(int,int,int,int) = 0;
  virtual Widget_Node *_make() = 0; // virtual constructor
  void setlabel(const char *) override;

  const char *extra_code_[NUM_EXTRA_CODE];
  const char *subclass_;
  const char *tooltip_;
  const char *image_name_;
  const char *inactive_name_;
  uchar hotspot_;

protected:

  /// This variable is set for visible windows in batch mode.
  /// We can't open a window in batch mode, even if we want the "visible" flags
  /// set, so we need a second place to store this information while also
  /// disabling the output of the "hide" property by the Widget Type.
  uchar override_visible_;

  void write_static(fld::io::Code_Writer& f) override;
  void write_code1(fld::io::Code_Writer& f) override;
  void write_widget_code(fld::io::Code_Writer& f);
  void write_extra_code(fld::io::Code_Writer& f);
  void write_block_close(fld::io::Code_Writer& f);
  void write_code2(fld::io::Code_Writer& f) override;
  void write_color(fld::io::Code_Writer& f, const char*, Fl_Color);
  Fl_Widget *live_widget;

public:
  Fl_Widget *o;
  int public_;
  int bind_image_;
  int compress_image_;
  int bind_deimage_;
  int compress_deimage_;
  int scale_image_w_, scale_image_h_;
  int scale_deimage_w_, scale_deimage_h_;

  Image_Asset *image;
  void setimage(Image_Asset *);
  Image_Asset *inactive;
  void setinactive(Image_Asset *);

  Widget_Node();
  Node *make(Strategy strategy) override;
  void open() override;

  const char *extra_code(int n) const {return extra_code_[n];}
  void extra_code(int n,const char *);
  const char *subclass() const {return subclass_;}
  void subclass(const char *);
  const char *tooltip() const {return tooltip_;}
  void tooltip(const char *);
  const char *image_name() const {return image_name_;}
  void image_name(const char *);
  const char *inactive_name() const {return inactive_name_;}
  void inactive_name(const char *);
  uchar hotspot() const {return hotspot_;}
  void hotspot(uchar v) {hotspot_ = v;}
  uchar resizable() const;
  void resizable(uchar v);

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
