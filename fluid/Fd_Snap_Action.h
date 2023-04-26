//
// Snap action header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2023 by Bill Spitzak and others.
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

#ifndef _FLUID_FD_SNAP_ACTION_H
#define _FLUID_FD_SNAP_ACTION_H

#include "Fl_Window_Type.h"

struct Fl_Menu_Item;

extern Fl_Menu_Item main_layout_submenu_[];

enum {
  FD_STORE_INTERNAL,
  FD_STORE_USER,
  FD_STORE_PROJECT,
  FD_STORE_FILE
};

class Fd_Layout_Preset {
public:
  int left_window_margin;
  int right_window_margin;
  int top_window_margin;
  int bottom_window_margin;
  int window_grid_x;
  int window_grid_y;

  int left_group_margin;
  int right_group_margin;
  int top_group_margin;
  int bottom_group_margin;
  int group_grid_x;
  int group_grid_y;

  int top_tabs_margin;
  int bottom_tabs_margin;

  int widget_min_w;
  int widget_inc_w;
  int widget_gap_x;
  int widget_min_h;
  int widget_inc_h;
  int widget_gap_y;

  int labelfont;
  int labelsize;
  int textfont;
  int textsize;

  void write(Fl_Preferences &prefs);
  void read(Fl_Preferences &prefs);
  void write(Fd_Project_Writer*);
  void read(Fd_Project_Reader*);
};

extern Fd_Layout_Preset *layout;

class Fd_Layout_Suite {
public:
  char *name_;
  char *menu_label;
  Fd_Layout_Preset *layout[3]; // application, dialog, toolbox;
  int storage_;
  void write(Fl_Preferences &prefs);
  void read(Fl_Preferences &prefs);
  void write(Fd_Project_Writer*);
  void read(Fd_Project_Reader*);
  void update_label();
  void storage(int s) { storage_ = s; update_label(); }
  void name(const char *n);
  void init();
  ~Fd_Layout_Suite();
public:

};

class Fd_Layout_List {
public:
  Fl_Menu_Item *main_menu_;
  Fl_Menu_Item *choice_menu_;
  Fd_Layout_Suite *list_;
  int list_size_;
  int list_capacity_;
  bool list_is_static_;
  int current_suite_;
  int current_preset_;
  char *filename_;
public:
  Fd_Layout_List();
  ~Fd_Layout_List();
  void update_dialogs();
  void update_menu_labels();
  int current_suite() const { return current_suite_; }
  void current_suite(int ix);
  void current_suite(Fl_String);
  int current_preset() const { return current_preset_; }
  void current_preset(int ix);
  Fd_Layout_Suite &operator[](int ix) { return list_[ix]; }
  int add(const char *name);
  void rename(const char *name);
  void capacity(int);

  int load(const char *filename);
  int save(const char *filename);
  void write(Fl_Preferences &prefs, int storage);
  void read(Fl_Preferences &prefs, int storage);
  void write(Fd_Project_Writer*);
  void read(Fd_Project_Reader*);
  int add(Fd_Layout_Suite*);
  void remove(int index);
  void remove_all(int storage);
  Fd_Layout_Preset *at(int);
  int size();
};

extern Fd_Layout_List g_layout_list;

/**
 \brief Structure holding all the data to perform interactive alignment operations.
 */
typedef struct Fd_Snap_Data {
  int dx, dy;           ///< distance of the mouse from its initial PUSH event
  int bx, by, br, bt;   ///< bounding box of the original push event or current bounding box when drawing
  int drag;             ///< drag event mask
  int x_dist, y_dist;   ///< current closest snapping distance in x and y
  int dx_out, dy_out;   ///< current closest snapping point as a delta
  Fl_Widget_Type *wgt;  ///< first selected widget
  Fl_Window_Type *win;  ///< window that handles the drag action
  int ex_out, ey_out;   ///< chosen snap position
} Fd_Snap_Data;

/**
 \brief Find points of interest when moving the bounding box of all selected widgets.
 */
class Fd_Snap_Action {
protected:
  int check_x_(Fd_Snap_Data &d, int x_ref, int x_snap);
  int check_y_(Fd_Snap_Data &d, int y_ref, int y_snap);
  void check_x_y_(Fd_Snap_Data &d, int x_ref, int x_snap, int y_ref, int y_snap);
  void clr() { ex = dx = 0x7fff;  }
public:
  int ex, ey, dx, dy, type, mask;
  Fd_Snap_Action() : ex(0x7fff), ey(0x7fff), dx(128), dy(128), type(0), mask(0) { }
  virtual void check(Fd_Snap_Data &d) = 0;
  virtual void draw(Fd_Snap_Data &d) { }
  virtual bool matches(Fd_Snap_Data &d);
public:
  static int eex, eey;
  static Fd_Snap_Action *list[];
  static void check_all(Fd_Snap_Data &d);
  static void draw_all(Fd_Snap_Data &d);
  static void get_resize_stepsize(int &x_step, int &y_step);
  static void get_move_stepsize(int &x_step, int &y_step);
};

#endif // _FLUID_FD_SNAP_ACTION_H
