//
// Snap action header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2023-2025 by Bill Spitzak and others.
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

#include <string>

class Window_Node;
class Widget_Node;
struct Fl_Menu_Item;
class Fl_Preferences;

extern Fl_Menu_Item main_layout_submenu_[];

namespace fld {
enum class Tool_Store; // fld::
namespace io {
class Project_Reader; // fld::io::
class Project_Writer; // fld::io::
} // namespace io
} // namespace fld

namespace fld {
namespace app {

/**
 \brief Collection of layout settings.

 Presets contain default fonts and font sizes for labels and text. They
 can be used to guide widget positions using margins, grids, and gap sizes.
 There are three Presets available in one Suite, marked "application",
 "dialog", and "toolbox".
 */
class Layout_Preset {
public:
  int left_window_margin;   ///< gap between the window border and the widget
  int right_window_margin;
  int top_window_margin;
  int bottom_window_margin;
  int window_grid_x;        ///< a regular grid across the window with its origin in the top left window corner
  int window_grid_y;

  int left_group_margin;    ///< gap between the border of a widget and its parent group
  int right_group_margin;
  int top_group_margin;
  int bottom_group_margin;
  int group_grid_x;         ///< a regular grid across the group with its origin in the top left group corner
  int group_grid_y;

  int top_tabs_margin;      ///< preferred top edge tab size inside Fl_Tabs
  int bottom_tabs_margin;   ///< preferred bottom edge tab size inside Fl_Tabs

  int widget_min_w;         ///< minimum widget width
  int widget_inc_w;         ///< widget width increments starting from widget_min_w
  int widget_gap_x;         ///< preferred horizontal gap between widgets
  int widget_min_h;
  int widget_inc_h;
  int widget_gap_y;

  int labelfont;            ///< preferred font for labels
  int labelsize;            ///< preferred size for labels
  int textfont;             ///< preferred font for text elements
  int textsize;             ///< preferred size for text elements

  void write(Fl_Preferences &prefs);
  void read(Fl_Preferences &prefs);
  void write(fld::io::Project_Writer*);
  void read(fld::io::Project_Reader*);

  int textsize_not_null();
};

extern Layout_Preset *default_layout_preset;

/**
 \brief A collection of layout presets.

 A suite of layout presets is designed to cover various use cases when
 designing UI layouts for applications.
 There are three Presets available in one Suite, marked "application",
 "dialog", and "toolbox".
 */
class Layout_Suite {
public:
  char *name_;                  ///< name of the suite
  char *menu_label;             ///< label text used in pulldown menu
  Layout_Preset *layout[3];  ///< presets for application, dialog, and toolbox windows
  fld::Tool_Store storage_;       ///< storage location (see fld::Tool_Store::INTERNAL, etc.)
  void write(Fl_Preferences &prefs);
  void read(Fl_Preferences &prefs);
  void write(fld::io::Project_Writer*);
  void read(fld::io::Project_Reader*);
  void update_label();
  void storage(fld::Tool_Store s) { storage_ = s; update_label(); }
  void name(const char *n);
  void init();
  ~Layout_Suite();
public:

};


/**
 \brief Manage all layout suites that are available to the user.

 FLUID has two built-in suites. More suites can be cloned or added and stored
 as a user preference, as part of an .fl project file, or in a separate file
 for import/export and sharing.
 */
class Layout_List {
public:
  Fl_Menu_Item *main_menu_;
  Fl_Menu_Item *choice_menu_;
  Layout_Suite *list_;
  int list_size_;
  int list_capacity_;
  bool list_is_static_;
  int current_suite_;
  int current_preset_;
  std::string filename_;
public:
  Layout_List();
  ~Layout_List();
  void update_dialogs();
  void update_menu_labels();
  int current_suite() const { return current_suite_; }
  void current_suite(int ix);
  void current_suite(std::string);
  int current_preset() const { return current_preset_; }
  void current_preset(int ix);
  Layout_Suite &operator[](int ix) { return list_[ix]; }
  int add(const char *name);
  void rename(const char *name);
  void capacity(int);

  int load(const std::string &filename);
  int save(const std::string &filename);
  void write(Fl_Preferences &prefs, fld::Tool_Store storage);
  void read(Fl_Preferences &prefs, fld::Tool_Store storage);
  void write(fld::io::Project_Writer*);
  void read(fld::io::Project_Reader*);
  int add(Layout_Suite*);
  void remove(int index);
  void remove_all(fld::Tool_Store storage);
  Layout_Preset *at(int);
  int size();
};

/**
 \brief Structure holding all the data to perform interactive alignment operations.
 */
typedef struct Snap_Data {
  int dx, dy;           ///< distance of the mouse from its initial PUSH event
  int bx, by, br, bt;   ///< bounding box of the original push event or current bounding box when drawing
  int drag;             ///< drag event mask
  int x_dist, y_dist;   ///< current closest snapping distance in x and y
  int dx_out, dy_out;   ///< current closest snapping point as a delta
  Widget_Node *wgt;  ///< first selected widget
  Window_Node *win;  ///< window that handles the drag action
  int ex_out, ey_out;   ///< chosen snap position
} Snap_Data;

/**
 \brief Find points of interest when moving the bounding box of all selected widgets.
 */
class Snap_Action {
protected:
  int check_x_(Snap_Data &d, int x_ref, int x_snap);
  int check_y_(Snap_Data &d, int y_ref, int y_snap);
  void check_x_y_(Snap_Data &d, int x_ref, int x_snap, int y_ref, int y_snap);
  void clr() { ex = dx = 0x7fff;  }
public:
  int ex, ey, dx, dy, type, mask;
  Snap_Action() : ex(0x7fff), ey(0x7fff), dx(128), dy(128), type(0), mask(0) { }
  virtual ~Snap_Action() { }
  virtual void check(Snap_Data &d) = 0;
  virtual void draw(Snap_Data &d) { }
  virtual bool matches(Snap_Data &d);
public:
  static int eex, eey;
  static Snap_Action *list[];
  static void check_all(Snap_Data &d);
  static void draw_all(Snap_Data &d);
  static void get_resize_stepsize(int &x_step, int &y_step);
  static void get_move_stepsize(int &x_step, int &y_step);
  static void better_size(int &w, int &h);
};

} // namespace app
} // namespace fld


#endif // _FLUID_FD_SNAP_ACTION_H
