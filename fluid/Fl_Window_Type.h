//
// Widget type header file for the Fast Light Tool Kit (FLTK).
//
// Type for creating all subclasses of Fl_Widget
// This should have the widget pointer in it, but it is still in the
// Fl_Type base class.
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

#include "Fl_Widget_Type.h"

class Fl_Widget_Class_Type;

extern Fl_Menu_Item window_type_menu[];
extern Fl_Widget_Class_Type *current_widget_class;
void toggle_overlays(Fl_Widget *,void *);
void show_project_cb(Fl_Widget *, void *);
void show_grid_cb(Fl_Widget *, void *);
void show_settings_cb(Fl_Widget *, void *);
void show_global_settings_cb(Fl_Widget *, void *);

class Fl_Window_Type : public Fl_Widget_Type {
protected:

  Fl_Menu_Item* subtypes() {return window_type_menu;}

  friend class Overlay_Window;
  int mx,my;            // mouse position during dragging
  int x1,y1;            // initial position of selection box
  int bx,by,br,bt;      // bounding box of selection before snapping
  int sx,sy,sr,st;      // bounding box of selection after snapping to guides
  int dx,dy;
  int drag;             // which parts of bbox are being moved
  int numselected;      // number of children selected
  enum {LEFT=1,RIGHT=2,BOTTOM=4,TOP=8,DRAG=16,BOX=32};
  void draw_overlay();
  void newdx();
  void newposition(Fl_Widget_Type *,int &x,int &y,int &w,int &h);
  int handle(int);
  virtual void setlabel(const char *);
  void write_code1();
  void write_code2();
  Fl_Widget_Type *_make() {return 0;} // we don't call this
  Fl_Widget *widget(int,int,int,int) {return 0;}
  int recalc;           // set by fix_overlay()
  void moveallchildren();
  int pixmapID() { return 1; }

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
    sr_min_w(0), sr_min_h(0), sr_max_w(0), sr_max_h(0)
  { }
  uchar modal, non_modal;

  Fl_Type *make(Strategy strategy);
  virtual const char *type_name() {return "Fl_Window";}
  virtual const char *alt_type_name() {return "fltk::Window";}

  void open();

  void fix_overlay();                   // Update the bounding box, etc
  uchar *read_image(int &ww, int &hh);  // Read an image of the window

  virtual void write_properties();
  virtual void read_property(const char *);
  virtual int read_fdesign(const char*, const char*);

  void add_child(Fl_Type*, Fl_Type*);
  void move_child(Fl_Type*, Fl_Type*);
  void remove_child(Fl_Type*);

  int is_parent() const {return 1;}
  int is_group() const {return 1;}
  int is_window() const {return 1;}

  Fl_Widget *enter_live_mode(int top=0);
  void leave_live_mode();
  void copy_properties();

  int sr_min_w, sr_min_h, sr_max_w, sr_max_h;

  static int popupx, popupy;
};

class Fl_Widget_Class_Type : private Fl_Window_Type {
protected:
  Fl_Menu_Item* subtypes() {return 0;}

public:
  Fl_Widget_Class_Type() {
    write_public_state = 0;
    wc_relative = 0;
  }
  // state variables for output:
  char write_public_state; // true when public: has been printed
  char wc_relative; // if 1, reposition all children, if 2, reposition and resize

  virtual void write_properties();
  virtual void read_property(const char *);

  void write_code1();
  void write_code2();
  Fl_Type *make(Strategy strategy);
  virtual const char *type_name() {return "widget_class";}
  int pixmapID() { return 48; }
  int is_parent() const {return 1;}
  int is_code_block() const {return 1;}
  int is_decl_block() const {return 1;}
  int is_class() const {return 1;}
};

#endif // _FLUID_FL_WINDOW_TYPE_H
