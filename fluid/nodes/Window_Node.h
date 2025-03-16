//
// Window type header file for the Fast Light Tool Kit (FLTK).
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

//
// Type for creating all subclasses of Fl_Widget
// This should have the widget pointer in it, but it is still in the
// Node base class.

#ifndef FLUID_NODES_WINDOW_NODE_H
#define FLUID_NODES_WINDOW_NODE_H

#include "nodes/Group_Node.h"

class Widget_Class_Node;

extern Fl_Menu_Item window_type_menu[];
extern Widget_Class_Node *current_widget_class;

void toggle_overlays(Fl_Widget *,void *);
void toggle_guides(Fl_Widget *,void *);
void toggle_restricted(Fl_Widget *,void *);
void show_project_cb(Fl_Widget *, void *);
void show_grid_cb(Fl_Widget *, void *);
void show_settings_cb(Fl_Widget *, void *);

enum {
  FD_LEFT   = 1,  // user drags the left side of the selection box
  FD_RIGHT  = 2,
  FD_BOTTOM = 4,
  FD_TOP    = 8,
  FD_DRAG   = 16, // user drags the entire selection
  FD_BOX    = 32  // user creates a new selection box
};

class Window_Node : public Group_Node
{
public:
  typedef Group_Node super;
  static Window_Node prototype;
protected:

  Fl_Menu_Item* subtypes() override {return window_type_menu;}

  friend class Overlay_Window;
  int mx,my;            // mouse position during dragging
  int x1,y1;            // initial position of selection box
  int bx,by,br,bt;      // bounding box of selection before snapping
  int sx,sy,sr,st;      // bounding box of selection after snapping to guides
  int dx,dy;
  int drag;             // which parts of bbox are being moved
  int numselected;      // number of children selected
  void draw_out_of_bounds(Widget_Node *group, int x, int y, int w, int h);
  void draw_out_of_bounds();
  void draw_overlaps();
  void draw_overlay();
  void newdx();
  void newposition(Widget_Node *,int &x,int &y,int &w,int &h);
  int handle(int);
  void setlabel(const char *) override;
  void write_code1(fld::io::Code_Writer& f) override;
  void write_code2(fld::io::Code_Writer& f) override;
  Widget_Node *_make() override {return nullptr;} // we don't call this
  Fl_Widget *widget(int,int,int,int) override {return nullptr;}
  int recalc;           // set by fix_overlay()
  void moveallchildren(int key=0);
  Type type() const override { return Type::Window; }
  bool is_a(Type inType) const override { return (inType==Type::Window) ? true : super::is_a(inType); }
  void open_();

public:

  Window_Node() :
    mx(0), my(0),
    x1(0), y1(0),
    bx(0), by(0), br(0), bt(0),
    sx(0), sy(0), sr(0), st(0),
    dx(0), dy(0),
    drag(0),
    numselected(0),
    recalc(0),
    modal(0), non_modal(0),
    xclass(nullptr),
    sr_min_w(0), sr_min_h(0), sr_max_w(0), sr_max_h(0)
  { }
  uchar modal, non_modal;
  const char *xclass; // junk string, used for shortcut

  Node *make(Strategy strategy) override;
  const char *type_name() override {return "Fl_Window";}
  const char *alt_type_name() override {return "fltk::Window";}

  void open() override;
  void ideal_size(int &w, int &h) override;

  void fix_overlay();                   // Update the bounding box, etc
  uchar *read_image(int &ww, int &hh);  // Read an image of the window

  void write_properties(fld::io::Project_Writer &f) override;
  void read_property(fld::io::Project_Reader &f, const char *) override;
  int read_fdesign(const char*, const char*) override;

  void add_child(Node*, Node*) override;
  void move_child(Node*, Node*) override;
  void remove_child(Node*) override;

  int can_have_children() const override {return 1;}

  Fl_Widget *enter_live_mode(int top=0) override;
  void leave_live_mode() override;
  void copy_properties() override;

  int sr_min_w, sr_min_h, sr_max_w, sr_max_h;

  static int popupx, popupy;
};

class Widget_Class_Node : private Window_Node
{
public:
  typedef Window_Node super;
  static Widget_Class_Node prototype;

protected:
  Fl_Menu_Item* subtypes() override {return nullptr;}

public:
  Widget_Class_Node() {
    write_public_state = 0;
    wc_relative = 0;
  }
  // state variables for output:
  char write_public_state; // true when public: has been printed
  char wc_relative; // if 1, reposition all children, if 2, reposition and resize

  void write_properties(fld::io::Project_Writer &f) override;
  void read_property(fld::io::Project_Reader &f, const char *) override;

  void write_code1(fld::io::Code_Writer& f) override;
  void write_code2(fld::io::Code_Writer& f) override;
  Node *make(Strategy strategy) override;
  const char *type_name() override {return "widget_class";}
  Type type() const override { return Type::Widget_Class; }
  bool is_a(Type inType) const override { return (inType==Type::Widget_Class) ? true : super::is_a(inType); }
  int can_have_children() const override {return 1;}
  int is_code_block() const override {return 1;}
  int is_decl_block() const override {return 1;}
  int is_class() const override {return 1;}
};

#endif // FLUID_NODES_WINDOW_NODE_H
