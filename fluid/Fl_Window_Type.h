//
// Window type header file for the Fast Light Tool Kit (FLTK).
//
// Type for creating all subclasses of Fl_Widget
// This should have the widget pointer in it, but it is still in the
// Fl_Type base class.
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

#ifndef _FLUID_FL_WINDOW_TYPE_H
#define _FLUID_FL_WINDOW_TYPE_H

#include "Fl_Group_Type.h"

class Fl_Widget_Class_Type;

extern Fl_Menu_Item window_type_menu[];
extern Fl_Widget_Class_Type *current_widget_class;

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

class Fl_Window_Type : public Fl_Group_Type
{
  typedef Fl_Group_Type super;
protected:

  Fl_Menu_Item* subtypes() FL_OVERRIDE {return window_type_menu;}

  friend class Overlay_Window;
  int mx,my;            // mouse position during dragging
  int x1,y1;            // initial position of selection box
  int bx,by,br,bt;      // bounding box of selection before snapping
  int sx,sy,sr,st;      // bounding box of selection after snapping to guides
  int dx,dy;
  int drag;             // which parts of bbox are being moved
  int numselected;      // number of children selected
  void draw_out_of_bounds(Fl_Widget_Type *group, int x, int y, int w, int h);
  void draw_out_of_bounds();
  void draw_overlaps();
  void draw_overlay();
  void newdx();
  void newposition(Fl_Widget_Type *,int &x,int &y,int &w,int &h);
  int handle(int);
  void setlabel(const char *) FL_OVERRIDE;
  void write_code1(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE;
  Fl_Widget_Type *_make() FL_OVERRIDE {return 0;} // we don't call this
  Fl_Widget *widget(int,int,int,int) FL_OVERRIDE {return 0;}
  int recalc;           // set by fix_overlay()
  void moveallchildren(int key=0);
  ID id() const FL_OVERRIDE { return ID_Window; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Window) ? true : super::is_a(inID); }
  void open_();

public:

  Fl_Window_Type() :
    mx(0), my(0),
    x1(0), y1(0),
    bx(0), by(0), br(0), bt(0),
    sx(0), sy(0), sr(0), st(0),
    dx(0), dy(0),
    drag(0),
    numselected(0),
    recalc(0),
    modal(0), non_modal(0),
    xclass(NULL),
    sr_min_w(0), sr_min_h(0), sr_max_w(0), sr_max_h(0)
  { }
  uchar modal, non_modal;
  const char *xclass; // junk string, used for shortcut

  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "Fl_Window";}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::Window";}

  void open() FL_OVERRIDE;
  void ideal_size(int &w, int &h) FL_OVERRIDE;

  void fix_overlay();                   // Update the bounding box, etc
  uchar *read_image(int &ww, int &hh);  // Read an image of the window

  void write_properties(Fd_Project_Writer &f) FL_OVERRIDE;
  void read_property(Fd_Project_Reader &f, const char *) FL_OVERRIDE;
  int read_fdesign(const char*, const char*) FL_OVERRIDE;

  void add_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  void move_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  void remove_child(Fl_Type*) FL_OVERRIDE;

  int can_have_children() const FL_OVERRIDE {return 1;}

  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  void leave_live_mode() FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;

  int sr_min_w, sr_min_h, sr_max_w, sr_max_h;

  static int popupx, popupy;
};

class Fl_Widget_Class_Type : private Fl_Window_Type
{
  typedef Fl_Window_Type super;
protected:
  Fl_Menu_Item* subtypes() FL_OVERRIDE {return 0;}

public:
  Fl_Widget_Class_Type() {
    write_public_state = 0;
    wc_relative = 0;
  }
  // state variables for output:
  char write_public_state; // true when public: has been printed
  char wc_relative; // if 1, reposition all children, if 2, reposition and resize

  void write_properties(Fd_Project_Writer &f) FL_OVERRIDE;
  void read_property(Fd_Project_Reader &f, const char *) FL_OVERRIDE;

  void write_code1(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE;
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "widget_class";}
  ID id() const FL_OVERRIDE { return ID_Widget_Class; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Widget_Class) ? true : super::is_a(inID); }
  int can_have_children() const FL_OVERRIDE {return 1;}
  int is_code_block() const FL_OVERRIDE {return 1;}
  int is_decl_block() const FL_OVERRIDE {return 1;}
  int is_class() const FL_OVERRIDE {return 1;}
};

#endif // _FLUID_FL_WINDOW_TYPE_H
