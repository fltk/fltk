//
// Snap action code file for the Fast Light Tool Kit (FLTK).
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

#include "Fd_Snap_Action.h"

#include "Fl_Group_Type.h"
#include "settings_panel.h"
#include "shell_command.h"  // get and set Fl_String preferences
#include "file.h"

#include <FL/fl_draw.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_string_functions.h>
#include <math.h>
#include <string.h>
#include <assert.h>

// TODO: warning if the user wants to change builtin layouts
// TODO: move panel to global settings panel (move load & save to main pulldown, or to toolbox?)
// INFO: how about a small tool box for quick preset selection and disabling of individual snaps?

void select_layout_suite_cb(Fl_Widget *, void *user_data);

int Fd_Snap_Action::eex = 0;
int Fd_Snap_Action::eey = 0;

static Fd_Layout_Preset fltk_app = {
  15, 15, 15, 15, 0, 0, // window:    l, r, t, b, gx, gy
  10, 10, 10, 10, 0, 0, // group:     l, r, t, b, gx, gy
  25, 25,               // tabs:      t, b
  20, 10, 4,            // widget_x:  min, inc, gap
  20,  4, 8,            // widget_y:  min, inc, gap
  0, 14, -1, 14          // labelfont/size, textfont/size
};
static Fd_Layout_Preset fltk_dlg = {
  10, 10, 10, 10, 0, 0, // window:    l, r, t, b, gx, gy
  10, 10, 10, 10, 0, 0, // group:     l, r, t, b, gx, gy
  20, 20,               // tabs:      t, b
  20, 10, 5,            // widget_x:  min, inc, gap
  20,  5, 5,            // widget_y:  min, inc, gap
  0, 11, -1, 11          // labelfont/size, textfont/size
};
static Fd_Layout_Preset fltk_tool = {
  10, 10, 10, 10, 0, 0, // window:    l, r, t, b, gx, gy
  10, 10, 10, 10, 0, 0, // group:     l, r, t, b, gx, gy
  18, 18,               // tabs:      t, b
  16,  8, 2,            // widget_x:  min, inc, gap
  16,  4, 2,            // widget_y:  min, inc, gap
  0, 10, -1, 10          // labelfont/size, textfont/size
};

static Fd_Layout_Preset grid_app = {
  12, 12, 12, 12, 12, 12, // window:    l, r, t, b, gx, gy
  12, 12, 12, 12, 12, 12, // group:     l, r, t, b, gx, gy
  24, 24,                 // tabs:      t, b
  12, 6, 6,               // widget_x:  min, inc, gap
  12, 6, 6,               // widget_y:  min, inc, gap
  0, 14, -1, 14            // labelfont/size, textfont/size
};

static Fd_Layout_Preset grid_dlg = {
  10, 10, 10, 10, 10, 10, // window:    l, r, t, b, gx, gy
  10, 10, 10, 10, 10, 10, // group:     l, r, t, b, gx, gy
  20, 20,                 // tabs:      t, b
  10, 5, 5,               // widget_x:  min, inc, gap
  10, 5, 5,               // widget_y:  min, inc, gap
  0, 12, -1, 12            // labelfont/size, textfont/size
};

static Fd_Layout_Preset grid_tool = {
  8, 8, 8, 8, 8, 8, // window:    l, r, t, b, gx, gy
  8, 8, 8, 8, 8, 8, // group:     l, r, t, b, gx, gy
  16, 16,           // tabs:      t, b
  8, 4, 4,          // widget_x:  min, inc, gap
  8, 4, 4,          // widget_y:  min, inc, gap
  0, 10, -1, 10      // labelfont/size, textfont/size
};

static Fd_Layout_Suite static_suite_list[] = {
  { (char*)"FLTK", (char*)"@fd_beaker FLTK", { &fltk_app, &fltk_dlg, &fltk_tool }, FD_STORE_INTERNAL },
  { (char*)"Grid", (char*)"@fd_beaker Grid", { &grid_app, &grid_dlg, &grid_tool }, FD_STORE_INTERNAL }
};

Fl_Menu_Item main_layout_submenu_[] = {
  { static_suite_list[0].menu_label, 0, select_layout_suite_cb, (void*)0, FL_MENU_RADIO|FL_MENU_VALUE },
  { static_suite_list[1].menu_label, 0, select_layout_suite_cb, (void*)1, FL_MENU_RADIO },
  { NULL }
};

static Fl_Menu_Item static_choice_menu[] = {
  { static_suite_list[0].menu_label },
  { static_suite_list[1].menu_label },
  { NULL }
};

Fd_Layout_Preset *layout = &fltk_app;
Fd_Layout_List g_layout_list;

// ---- Callbacks ------------------------------------------------------ MARK: -

void layout_suite_marker(Fl_Widget *, void *) {
  // intentionally left empty
}

void select_layout_suite_cb(Fl_Widget *, void *user_data) {
  int index = (int)(fl_intptr_t)user_data;
  assert(index >= 0);
  assert(index < g_layout_list.list_size_);
  g_layout_list.current_suite(index);
  g_layout_list.update_dialogs();
}

void select_layout_preset_cb(Fl_Widget *, void *user_data) {
  int index = (int)(fl_intptr_t)user_data;
  assert(index >= 0);
  assert(index < 3);
  g_layout_list.current_preset(index);
  g_layout_list.update_dialogs();
}

void edit_layout_preset_cb(Fl_Button *w, long user_data) {
  int index = (int)w->argument();
  assert(index >= 0);
  assert(index < 3);
  if (user_data == (long)(fl_intptr_t)LOAD) {
    w->value(g_layout_list.current_preset() == index);
  } else {
    g_layout_list.current_preset(index);
    g_layout_list.update_dialogs();
  }
}

// ---- Fd_Layout_Suite ------------------------------------------------ MARK: -

/**
 Write presets to a Preferences database.
 */
void Fd_Layout_Preset::write(Fl_Preferences &prefs) {
  assert(this);
  Fl_Preferences p_win(prefs, "Window");
  p_win.set("left_margin", left_window_margin);
  p_win.set("right_margin", right_window_margin);
  p_win.set("top_margin", top_window_margin);
  p_win.set("bottom_margin", bottom_window_margin);
  p_win.set("grid_x", window_grid_x);
  p_win.set("grid_y", window_grid_y);

  Fl_Preferences p_grp(prefs, "Group");
  p_grp.set("left_margin", left_group_margin);
  p_grp.set("right_margin", right_group_margin);
  p_grp.set("top_margin", top_group_margin);
  p_grp.set("bottom_margin", bottom_group_margin);
  p_grp.set("grid_x", group_grid_x);
  p_grp.set("grid_y", group_grid_y);

  Fl_Preferences p_tbs(prefs, "Tabs");
  p_tbs.set("top_margin", top_tabs_margin);
  p_tbs.set("bottom_margin", bottom_tabs_margin);

  Fl_Preferences p_wgt(prefs, "Widget");
  p_wgt.set("min_w", widget_min_w);
  p_wgt.set("inc_w", widget_inc_w);
  p_wgt.set("gap_x", widget_gap_x);
  p_wgt.set("min_h", widget_min_h);
  p_wgt.set("inc_h", widget_inc_h);
  p_wgt.set("gap_y", widget_gap_y);

  Fl_Preferences p_lyt(prefs, "Layout");
  p_lyt.set("labelfont", labelfont);
  p_lyt.set("labelsize", labelsize);
  p_lyt.set("textfont", textfont);
  p_lyt.set("textsize", textsize);
}

/**
 Read presets from a Preferences database.
 */
void Fd_Layout_Preset::read(Fl_Preferences &prefs) {
  assert(this);
  Fl_Preferences p_win(prefs, "Window");
  p_win.get("left_margin", left_window_margin, 15);
  p_win.get("right_margin", right_window_margin, 15);
  p_win.get("top_margin", top_window_margin, 15);
  p_win.get("bottom_margin", bottom_window_margin, 15);
  p_win.get("grid_x", window_grid_x, 0);
  p_win.get("grid_y", window_grid_y, 0);

  Fl_Preferences p_grp(prefs, "Group");
  p_grp.get("left_margin", left_group_margin, 10);
  p_grp.get("right_margin", right_group_margin, 10);
  p_grp.get("top_margin", top_group_margin, 10);
  p_grp.get("bottom_margin", bottom_group_margin, 10);
  p_grp.get("grid_x", group_grid_x, 0);
  p_grp.get("grid_y", group_grid_y, 0);

  Fl_Preferences p_tbs(prefs, "Tabs");
  p_tbs.get("top_margin", top_tabs_margin, 25);
  p_tbs.get("bottom_margin", bottom_tabs_margin, 25);

  Fl_Preferences p_wgt(prefs, "Widget");
  p_wgt.get("min_w", widget_min_w, 20);
  p_wgt.get("inc_w", widget_inc_w, 10);
  p_wgt.get("gap_x", widget_gap_x, 4);
  p_wgt.get("min_h", widget_min_h, 20);
  p_wgt.get("inc_h", widget_inc_h, 4);
  p_wgt.get("gap_y", widget_gap_y, 8);

  Fl_Preferences p_lyt(prefs, "Layout");
  p_lyt.get("labelfont", labelfont, 0);
  p_lyt.get("labelsize", labelsize, 14);
  p_lyt.get("textfont", textfont, 0);
  p_lyt.get("textsize", textsize, 14);
}

/**
 Write presets to an .fl project file.
 */
void Fd_Layout_Preset::write(Fd_Project_Writer *out) {
  out->write_string("    preset { 1\n"); // preset format version
  out->write_string("      %d %d %d %d %d %d\n",
                    left_window_margin, right_window_margin,
                    top_window_margin, bottom_window_margin,
                    window_grid_x, window_grid_y);
  out->write_string("      %d %d %d %d %d %d\n",
                    left_group_margin, right_group_margin,
                    top_group_margin, bottom_group_margin,
                    group_grid_x, group_grid_y);
  out->write_string("      %d %d\n", top_tabs_margin, bottom_tabs_margin);
  out->write_string("      %d %d %d %d %d %d\n",
                    widget_min_w, widget_inc_w, widget_gap_x,
                    widget_min_h, widget_inc_h, widget_gap_y);
  out->write_string("      %d %d %d %d\n",
                    labelfont, labelsize, textfont, textsize);
  out->write_string("    }\n"); // preset format version
}

/**
 Read presets from an .fl project file.
 */
void Fd_Layout_Preset::read(Fd_Project_Reader *in) {
  const char *key;
  key = in->read_word(1);
  if (key && !strcmp(key, "{")) {
    for (;;) {
      key = in->read_word();
      if (!key) return;
      if (key[0] == '}') break;
      int ver = atoi(key);
      if (ver == 0) {
        continue;
      } else if (ver == 1) {
        left_window_margin = in->read_int();
        right_window_margin = in->read_int();
        top_window_margin = in->read_int();
        bottom_window_margin = in->read_int();
        window_grid_x = in->read_int();
        window_grid_y = in->read_int();

        left_group_margin = in->read_int();
        right_group_margin = in->read_int();
        top_group_margin = in->read_int();
        bottom_group_margin = in->read_int();
        group_grid_x = in->read_int();
        group_grid_y = in->read_int();

        top_tabs_margin = in->read_int();
        bottom_tabs_margin = in->read_int();

        widget_min_w = in->read_int();
        widget_inc_w = in->read_int();
        widget_gap_x = in->read_int();
        widget_min_h = in->read_int();
        widget_inc_h = in->read_int();
        widget_gap_y = in->read_int();

        labelfont = in->read_int();
        labelsize = in->read_int();
        textfont = in->read_int();
        textsize = in->read_int();
      } else { // skip unknown chunks
        for (;;) {
          key = in->read_word(1);
          if (key && (key[0] == '}'))
            return;
        }
      }
    }
  } else {
    // format error
  }
}

/**
 Return the preferred text size, but make sure it's not 0.
 */
int Fd_Layout_Preset::textsize_not_null() {
  // try the user selected text size
  if (textsize > 0) return textsize;
  // if the user did not set one, try the label size
  if (labelsize > 0) return labelsize;
  // if that doesn;t work, fall back to the default value
  return 14;
}


// ---- Fd_Layout_Suite ------------------------------------------------ MARK: -

/**
 Write a presets suite to a Preferences database.
 */
void Fd_Layout_Suite::write(Fl_Preferences &prefs) {
  assert(this);
  assert(name_);
  prefs.set("name", name_);
  for (int i = 0; i < 3; ++i) {
    Fl_Preferences prefs_preset(prefs, Fl_Preferences::Name(i));
    assert(layout[i]);
    layout[i]->write(prefs_preset);
  }
}

/**
 Read a presets suite from a Preferences database.
 */
void Fd_Layout_Suite::read(Fl_Preferences &prefs) {
  assert(this);
  for (int i = 0; i < 3; ++i) {
    Fl_Preferences prefs_preset(prefs, Fl_Preferences::Name(i));
    assert(layout[i]);
    layout[i]->read(prefs_preset);
  }
}

/**
 Write a presets suite to an .fl project file.
 */
void Fd_Layout_Suite::write(Fd_Project_Writer *out) {
  out->write_string("  suite {\n");
  out->write_string("    name "); out->write_word(name_); out->write_string("\n");
  for (int i = 0; i < 3; ++i) {
    layout[i]->write(out);
  }
  out->write_string("  }\n");
}

/**
 Read a presets suite from an .fl project file.
 */
void Fd_Layout_Suite::read(Fd_Project_Reader *in) {
  const char *key;
  key = in->read_word(1);
  if (key && !strcmp(key, "{")) {
    int ix = 0;
    for (;;) {
      key = in->read_word();
      if (!key) return;
      if (!strcmp(key, "name")) {
        name(in->read_word());
      } else if (!strcmp(key, "preset")) {
        if (ix >= 3) return; // file format error
        layout[ix++]->read(in);
      } else if (!strcmp(key, "}")) {
        break;
      } else {
        in->read_word(); // unknown key, ignore, hopefully a key-value pair
      }
    }
  } else {
    // file format error
  }
}

/**
 \brief Update the menu_label to show a symbol representing the storage location.
 Also updates the FLUID user interface.
 */
void Fd_Layout_Suite::update_label() {
  Fl_String sym;
  switch (storage_) {
    case FD_STORE_INTERNAL: sym.assign("@fd_beaker  "); break;
    case FD_STORE_USER: sym.assign("@fd_user  "); break;
    case FD_STORE_PROJECT: sym.assign("@fd_project  "); break;
    case FD_STORE_FILE: sym.assign("@fd_file  "); break;
  }
  sym.append(name_);
  if (menu_label)
    ::free(menu_label);
  menu_label = fl_strdup(sym.c_str());
  g_layout_list.update_menu_labels();
}

/**
 \brief Update the Suite name and the Suite menu_label.
 Also updates the FLUID user interface.
 */
void Fd_Layout_Suite::name(const char *n) {
  if (name_)
    ::free(name_);
  if (n)
    name_ = fl_strdup(n);
  else
    name_ = NULL;
  update_label();
}

/**
 Initialize the class for first use.
 */
void Fd_Layout_Suite::init() {
  name_ = NULL;
  menu_label = NULL;
  layout[0] = layout[1] = layout[2] = NULL;
  storage_ = FD_STORE_INTERNAL;
}

/**
 Free all allocated resources.
 */
Fd_Layout_Suite::~Fd_Layout_Suite() {
  if (storage_ == FD_STORE_INTERNAL) return;
  if (name_) ::free(name_);
  for (int i = 0; i < 3; ++i) {
    delete layout[i];
  }
}

// ---- Fd_Layout_List ------------------------------------------------- MARK: -

/**
 Draw a little FLUID beaker symbol.
 */
static void fd_beaker(Fl_Color c) {
  fl_color(221);
  fl_begin_polygon();
  fl_vertex(-0.6,  0.2);
  fl_vertex(-0.9,  0.8);
  fl_vertex(-0.8,  0.9);
  fl_vertex( 0.8,  0.9);
  fl_vertex( 0.9,  0.8);
  fl_vertex( 0.6,  0.2);
  fl_end_polygon();
  fl_color(c);
  fl_begin_line();
  fl_vertex(-0.3, -0.9);
  fl_vertex(-0.2, -0.8);
  fl_vertex(-0.2, -0.2);
  fl_vertex(-0.9,  0.8);
  fl_vertex(-0.8,  0.9);
  fl_vertex( 0.8,  0.9);
  fl_vertex( 0.9,  0.8);
  fl_vertex( 0.2, -0.2);
  fl_vertex( 0.2, -0.8);
  fl_vertex( 0.3, -0.9);
  fl_end_line();
}

/**
 Draw a user silhouette symbol
 */
static void fd_user(Fl_Color c) {
  fl_color(245);
  fl_begin_complex_polygon();
  fl_arc( 0.1,  0.9, 0.8,  0.0,  80.0);
  fl_arc( 0.0, -0.5, 0.4, -65.0, 245.0);
  fl_arc(-0.1,  0.9, 0.8, 100.0, 180.0);
  fl_end_complex_polygon();
  fl_color(c);
  fl_begin_line();
  fl_arc( 0.1,  0.9, 0.8,  0.0,  80.0);
  fl_arc( 0.0, -0.5, 0.4, -65.0, 245.0);
  fl_arc(-0.1,  0.9, 0.8, 100.0, 180.0);
  fl_end_line();
}

/**
 Draw a document symbol.
 */
static void fd_project(Fl_Color c) {
  Fl_Color fc = FL_LIGHT2;
  fl_color(fc);
  fl_begin_complex_polygon();
  fl_vertex(-0.7, -1.0);
  fl_vertex(0.1, -1.0);
  fl_vertex(0.1, -0.4);
  fl_vertex(0.7, -0.4);
  fl_vertex(0.7, 1.0);
  fl_vertex(-0.7, 1.0);
  fl_end_complex_polygon();

  fl_color(fl_lighter(fc));
  fl_begin_polygon();
  fl_vertex(0.1, -1.0);
  fl_vertex(0.1, -0.4);
  fl_vertex(0.7, -0.4);
  fl_end_polygon();

  fl_color(fl_darker(c));
  fl_begin_loop();
  fl_vertex(-0.7, -1.0);
  fl_vertex(0.1, -1.0);
  fl_vertex(0.1, -0.4);
  fl_vertex(0.7, -0.4);
  fl_vertex(0.7, 1.0);
  fl_vertex(-0.7, 1.0);
  fl_end_loop();

  fl_begin_line();
  fl_vertex(0.1, -1.0);
  fl_vertex(0.7, -0.4);
  fl_end_line();
}

/**
 Draw a 3 1/2" floppy symbol.
 */
void fd_file(Fl_Color c) {
  Fl_Color fl = FL_LIGHT2;
  Fl_Color fc = FL_DARK3;
  fl_color(fc);
  fl_begin_polygon(); // case
  fl_vertex(-0.9, -1.0);
  fl_vertex(0.9, -1.0);
  fl_vertex(1.0, -0.9);
  fl_vertex(1.0, 0.9);
  fl_vertex(0.9, 1.0);
  fl_vertex(-0.9, 1.0);
  fl_vertex(-1.0, 0.9);
  fl_vertex(-1.0, -0.9);
  fl_end_polygon();

  fl_color(fl_lighter(fl));
  fl_begin_polygon();
  fl_vertex(-0.7, -1.0); // slider
  fl_vertex(0.7, -1.0);
  fl_vertex(0.7, -0.4);
  fl_vertex(-0.7, -0.4);
  fl_end_polygon();

  fl_begin_polygon(); // label
  fl_vertex(-0.7, 0.0);
  fl_vertex(0.7, 0.0);
  fl_vertex(0.7, 1.0);
  fl_vertex(-0.7, 1.0);
  fl_end_polygon();

  fl_color(fc);
  fl_begin_polygon();
  fl_vertex(-0.5, -0.9); // slot
  fl_vertex(-0.3, -0.9);
  fl_vertex(-0.3, -0.5);
  fl_vertex(-0.5, -0.5);
  fl_end_polygon();

  fl_color(fl_darker(c));
  fl_begin_loop();
  fl_vertex(-0.9, -1.0);
  fl_vertex(0.9, -1.0);
  fl_vertex(1.0, -0.9);
  fl_vertex(1.0, 0.9);
  fl_vertex(0.9, 1.0);
  fl_vertex(-0.9, 1.0);
  fl_vertex(-1.0, 0.9);
  fl_vertex(-1.0, -0.9);
  fl_end_loop();
}

/**
 Instantiate the class that holds a list of all layouts and manages the UI.
 */
Fd_Layout_List::Fd_Layout_List()
: main_menu_(main_layout_submenu_),
  choice_menu_(static_choice_menu),
  list_(static_suite_list),
  list_size_(2),
  list_capacity_(2),
  list_is_static_(true),
  current_suite_(0),
  current_preset_(0)
{
  fl_add_symbol("fd_beaker", fd_beaker, 1);
  fl_add_symbol("fd_user", fd_user, 1);
  fl_add_symbol("fd_project", fd_project, 1);
  fl_add_symbol("fd_file", fd_file, 1);
}

/**
 Release allocated resources.
 */
Fd_Layout_List::~Fd_Layout_List() {
  assert(this);
  if (!list_is_static_) {
    ::free(main_menu_);
    ::free(choice_menu_);
    for (int i = 0; i < list_size_; i++) {
      Fd_Layout_Suite &suite = list_[i];
      if (suite.storage_ != FD_STORE_INTERNAL)
        suite.~Fd_Layout_Suite();
    }
    ::free(list_);
  }
}

/**
 Update the Setting dialog and menus to reflect the current Layout selection state.
 */
void Fd_Layout_List::update_dialogs() {
  static Fl_Menu_Item *preset_menu = NULL;
  if (!preset_menu) {
    preset_menu = (Fl_Menu_Item*)main_menubar->find_item(select_layout_preset_cb);
    assert(preset_menu);
  }
  assert(this);
  assert(current_suite_ >= 0 );
  assert(current_suite_ < list_size_);
  assert(current_preset_ >= 0 );
  assert(current_preset_ < 3);
  layout = list_[current_suite_].layout[current_preset_];
  assert(layout);
  if (w_settings_layout_tab) {
    w_settings_layout_tab->do_callback(w_settings_layout_tab, LOAD);
    layout_choice->redraw();
  }
  preset_menu[current_preset_].setonly(preset_menu);
  main_menu_[current_suite_].setonly(main_menu_);
}

/**
 Refresh the label pointers for both pulldown menus.
 */
void Fd_Layout_List::update_menu_labels() {
  for (int i=0; i<list_size_; i++) {
    main_menu_[i].label(list_[i].menu_label);
    choice_menu_[i].label(list_[i].menu_label);
  }
}

/**
 Load all user layouts from the FLUID user preferences.
 */
int Fd_Layout_List::load(const Fl_String &filename) {
  remove_all(FD_STORE_FILE);
  Fl_Preferences prefs(filename.c_str(), "layout.fluid.fltk.org", NULL, Fl_Preferences::C_LOCALE);
  read(prefs, FD_STORE_FILE);
  return 0;
}

/**
 Save all user layouts to the FLUID user preferences.
 */
int Fd_Layout_List::save(const Fl_String &filename) {
  assert(this);
  Fl_Preferences prefs(filename.c_str(), "layout.fluid.fltk.org", NULL, (Fl_Preferences::Root)(Fl_Preferences::C_LOCALE|Fl_Preferences::CLEAR));
  prefs.clear();
  write(prefs, FD_STORE_FILE);
  return 0;
}

/**
 Write Suite and Layout selection and selected layout data to Preferences database.
 */
void Fd_Layout_List::write(Fl_Preferences &prefs, Fd_Tool_Store storage) {
  Fl_Preferences prefs_list(prefs, "Layouts");
  prefs_list.clear();
  prefs_list.set("current_suite", list_[current_suite()].name_);
  prefs_list.set("current_preset", current_preset());
  int n = 0;
  for (int i = 0; i < list_size_; ++i) {
    Fd_Layout_Suite &suite = list_[i];
    if (suite.storage_ == storage) {
      Fl_Preferences prefs_suite(prefs_list, Fl_Preferences::Name(n++));
      suite.write(prefs_suite);
    }
  }
}

/**
 Read Suite and Layout selection and selected layout data to Preferences database.
 */
void Fd_Layout_List::read(Fl_Preferences &prefs, Fd_Tool_Store storage) {
  Fl_Preferences prefs_list(prefs, "Layouts");
  Fl_String cs;
  int cp = 0;
  preferences_get(prefs_list, "current_suite", cs, "");
  prefs_list.get("current_preset", cp, 0);
  for (int i = 0; i < prefs_list.groups(); ++i) {
    Fl_Preferences prefs_suite(prefs_list, Fl_Preferences::Name(i));
    char *new_name = NULL;
    prefs_suite.get("name", new_name, NULL);
    if (new_name) {
      int n = add(new_name);
      list_[n].read(prefs_suite);
      list_[n].storage(storage);
      ::free(new_name);
    }
  }
  current_suite(cs);
  current_preset(cp);
  update_dialogs();
}

/**
 Write Suite and Layout selection and project layout data to an .fl project file.
 */
void Fd_Layout_List::write(Fd_Project_Writer *out) {
  // Don't write the Snap field if no custom layout was used
  if ((current_suite()==0) && (current_preset()==0)) {
    int nSuite = 0;
    for (int i=0; i<list_size_; i++) {
      if (list_[i].storage_ == FD_STORE_PROJECT) nSuite++;
    }
    if (nSuite == 0) return;
  }
  out->write_string("\nsnap {\n  ver 1\n");
  out->write_string("  current_suite "); out->write_word(list_[current_suite()].name_); out->write_string("\n");
  out->write_string("  current_preset %d\n", current_preset());
  for (int i=0; i<list_size_; i++) {
    Fd_Layout_Suite &suite = list_[i];
    if (suite.storage_ == FD_STORE_PROJECT)
      suite.write(out);
  }
  out->write_string("}");
}

/**
 Read Suite and Layout selection and project layout data from an .fl project file.
 */
void Fd_Layout_List::read(Fd_Project_Reader *in) {
  const char *key;
  key = in->read_word(1);
  if (key && !strcmp(key, "{")) {
    Fl_String cs;
    int cp = 0;
    for (;;) {
      key = in->read_word();
      if (!key) return;
      if (!strcmp(key, "ver")) {
        in->read_int();
      } else if (!strcmp(key, "current_suite")) {
        cs = in->read_word();
      } else if (!strcmp(key, "current_preset")) {
        cp = in->read_int();
      } else if (!strcmp(key, "suite")) {
        int n = add(in->filename_name());
        list_[n].read(in);
        list_[n].storage(FD_STORE_PROJECT);
      } else if (!strcmp(key, "}")) {
        break;
      } else {
        in->read_word(); // unknown key, ignore, hopefully a key-value pair
      }
    }
    current_suite(cs);
    current_preset(cp);
    update_dialogs();
  } else {
    // old style "snap" is followed by an integer. Ignore.
  }
}

/**
 Set the current Suite.
 \param[in] ix index into list of suites
 */
void Fd_Layout_List::current_suite(int ix) {
  assert(ix >= 0);
  assert(ix < list_size_);
  current_suite_ = ix;
  layout = list_[current_suite_].layout[current_preset_];
}

/**
 Set the current Suite.
 \param[in] arg_name name of the selected suite
 \return if no name is given or the name is not found, keep the current suite selected
 */
void Fd_Layout_List::current_suite(Fl_String arg_name) {
  if (arg_name.empty()) return;
  for (int i = 0; i < list_size_; ++i) {
    Fd_Layout_Suite &suite = list_[i];
    if (suite.name_ && (strcmp(suite.name_, arg_name.c_str()) == 0)) {
      current_suite(i);
      break;
    }
  }
}

/**
 Select a Preset within the current Suite.
 \param[in] ix 0 = application, 1 = dialog, 2 = toolbox
 */
void Fd_Layout_List::current_preset(int ix) {
  assert(ix >= 0);
  assert(ix < 3);
  current_preset_ = ix;
  layout = list_[current_suite_].layout[current_preset_];
}

/**
 Allocate enough space for n entries in the list.
 */
void Fd_Layout_List::capacity(int n) {
  static Fl_Menu_Item *suite_menu = NULL;
  if (!suite_menu)
    suite_menu = (Fl_Menu_Item*)main_menubar->find_item(layout_suite_marker);

  int old_n = list_size_;
  int i;

  Fd_Layout_Suite *new_list = (Fd_Layout_Suite*)::calloc(n, sizeof(Fd_Layout_Suite));
  for (i = 0; i < old_n; i++)
    new_list[i] = list_[i];
  if (!list_is_static_) ::free(list_);
  list_ = new_list;

  Fl_Menu_Item *new_main_menu = (Fl_Menu_Item*)::calloc(n+1, sizeof(Fl_Menu_Item));
  for (i = 0; i < old_n; i++)
    new_main_menu[i] = main_menu_[i];
  if (!list_is_static_) ::free(main_menu_);
  main_menu_ = new_main_menu;
  suite_menu->user_data(main_menu_);

  Fl_Menu_Item *new_choice_menu = (Fl_Menu_Item*)::calloc(n+1, sizeof(Fl_Menu_Item));
  for (i = 0; i < old_n; i++)
    new_choice_menu[i] = choice_menu_[i];
  if (!list_is_static_) ::free(choice_menu_);
  choice_menu_ = new_choice_menu;
  if (layout_choice) layout_choice->menu(choice_menu_);

  list_capacity_ = n;
  list_is_static_ = false;
}

/**
 \brief Clone the currently selected suite and append it to the list.
 Selects the new layout and updates the UI.
 */
int Fd_Layout_List::add(const char *name) {
  if (list_size_ == list_capacity_) {
    capacity(list_capacity_ * 2);
  }
  int n = list_size_;
  Fd_Layout_Suite &old_suite = list_[current_suite_];
  Fd_Layout_Suite &new_suite = list_[n];
  new_suite.init();
  new_suite.name(name);
  for (int i=0; i<3; ++i) {
    new_suite.layout[i] = new Fd_Layout_Preset;
    ::memcpy(new_suite.layout[i], old_suite.layout[i], sizeof(Fd_Layout_Preset));
  }
  Fd_Tool_Store new_storage = old_suite.storage_;
  if (new_storage == FD_STORE_INTERNAL)
    new_storage = FD_STORE_USER;
  new_suite.storage(new_storage);
  main_menu_[n].label(new_suite.menu_label);
  main_menu_[n].callback(main_menu_[0].callback());
  main_menu_[n].argument(n);
  main_menu_[n].flags = main_menu_[0].flags;
  choice_menu_[n].label(new_suite.menu_label);
  list_size_++;
  current_suite(n);
  return n;
}

/**
 Rename the current Suite.
 */
void Fd_Layout_List::rename(const char *name) {
  int n = current_suite();
  list_[n].name(name);
  main_menu_[n].label(list_[n].menu_label);
  choice_menu_[n].label(list_[n].menu_label);
}

/**
 Remove the given suite.
 \param[in] ix index into list of suites
 */
void Fd_Layout_List::remove(int ix) {
  int tail = list_size_-ix-1;
  if (tail) {
    for (int i = ix; i < list_size_-1; i++)
      list_[i] = list_[i+1];
  }
  ::memmove(main_menu_+ix, main_menu_+ix+1, (tail+1) * sizeof(Fl_Menu_Item));
  ::memmove(choice_menu_+ix, choice_menu_+ix+1, (tail+1) * sizeof(Fl_Menu_Item));
  list_size_--;
  if (current_suite() >= list_size_)
    current_suite(list_size_ - 1);
}

/**
 Remove all Suites that use the given storage attribute.
 \param[in] storage storage attribute, see FD_STORE_INTERNAL, etc.
 */
void Fd_Layout_List::remove_all(Fd_Tool_Store storage) {
  for (int i=list_size_-1; i>=0; --i) {
    if (list_[i].storage_ == storage)
      remove(i);
  }
}

// ---- Helper --------------------------------------------------------- MARK: -

static void draw_h_arrow(int, int, int);
static void draw_v_arrow(int x, int y1, int y2);
static void draw_left_brace(const Fl_Widget *w);
static void draw_right_brace(const Fl_Widget *w);
static void draw_top_brace(const Fl_Widget *w);
static void draw_bottom_brace(const Fl_Widget *w);
static void draw_grid(int x, int y, int dx, int dy);
void draw_width(int x, int y, int r, Fl_Align a);
void draw_height(int x, int y, int b, Fl_Align a);

static int nearest(int x, int left, int grid, int right=0x7fff) {
  int grid_x = ((x-left+grid/2)/grid)*grid+left;
  if (grid_x < left+grid/2) return left; // left+grid/2;
  if (grid_x > right-grid/2) return right; // right-grid/2;
  return grid_x;
}

static bool in_window(Fd_Snap_Data &d) {
  return (d.wgt && d.wgt->parent == d.win);
}

static bool in_group(Fd_Snap_Data &d) {
  return (d.wgt && d.wgt->parent && d.wgt->parent->is_a(ID_Group) && d.wgt->parent != d.win);
}

static bool in_tabs(Fd_Snap_Data &d) {
  return (d.wgt && d.wgt->parent && d.wgt->parent->is_a(ID_Tabs));
}

static Fl_Group *parent(Fd_Snap_Data &d) {
  return (d.wgt->o->parent());
}

// ---- Fd_Snap_Action ------------------------------------------------- MARK: -

/** \class Fd_Snap_Action

 When a user drags one or more widgets, snap actions can be defined that provide
 hints if a preferred widget position or size is nearby. The user's motion is
 then directed towards the nearest preferred position, and the widget selection
 snaps into place.

 FLUID provides a list of various snap actions. Every snap action uses the data
 from the motion event and combines it with the sizes and positions of all other
 widgets in the layout.

 Common snap actions include gaps and margins, but also alignments and
 simple grid positions.
 */

/**
 \brief Check if a snap action has reached a preferred x position.
 \param[inout] d current event data
 \param[in] x_ref position of moving point
 \param[in] x_snap position of target point
 \return 1 if the points are not within range and won;t be considered
 \return 0 if the point is as close as another in a previous action
 \return -1 if this point is closer than any previous check, and this is the
    new distance to beat.
 */
int Fd_Snap_Action::check_x_(Fd_Snap_Data &d, int x_ref, int x_snap) {
  int dd = x_ref + d.dx - x_snap;
  int d2 = abs(dd);
  if (d2 > d.x_dist) return 1;
  dx = d.dx_out = d.dx - dd;
  ex = d.ex_out = x_snap;
  if (d2 == d.x_dist) return 0;
  d.x_dist = d2;
  return -1;
}

/**
 \brief Check if a snap action has reached a preferred y position.
 \see Fd_Snap_Action::check_x_(Fd_Snap_Data &d, int x_ref, int x_snap)
 */
int Fd_Snap_Action::check_y_(Fd_Snap_Data &d, int y_ref, int y_snap) {
  int dd = y_ref + d.dy - y_snap;
  int d2 = abs(dd);
  if (d2 > d.y_dist) return 1;
  dy = d.dy_out = d.dy - dd;
  ey = d.ey_out = y_snap;
  if (d2 == d.y_dist) return 0;
  d.y_dist = d2;
  return -1;
}

/**
 \brief Check if a snap action has reached a preferred x and y position.
 \see Fd_Snap_Action::check_x_(Fd_Snap_Data &d, int x_ref, int x_snap)
 */
void Fd_Snap_Action::check_x_y_(Fd_Snap_Data &d, int x_ref, int x_snap, int y_ref, int y_snap) {
  int ddx = x_ref + d.dx - x_snap;
  int d2x = abs(ddx);
  int ddy = y_ref + d.dy - y_snap;
  int d2y = abs(ddy);
  if ((d2x <= d.x_dist) && (d2y <= d.y_dist)) {
    dx = d.dx_out = d.dx - ddx;
    ex = d.ex_out = x_snap;
    d.x_dist = d2x;
    dy = d.dy_out = d.dy - ddy;
    ey = d.ey_out = y_snap;
    d.y_dist = d2y;
  }
}

/**
 \brief Check if a snap action was applied to the current event.
 This method is used to determine if a visual indicator for this snap action
 should be drawn.
 \param[inout] d current event data
 */
bool Fd_Snap_Action::matches(Fd_Snap_Data &d) {
  switch (type) {
    case 1: return (d.drag & mask) && (eex == ex) && (d.dx == dx);
    case 2: return (d.drag & mask) && (eey == ey) && (d.dy == dy);
    case 3: return (d.drag & mask) && (eex == ex) && (d.dx == dx) && (eey == ey) && (d.dy == dy);
  }
  return false;
}

/**
 \brief Run through all possible snap actions and store the winning coordinates in eex and eey.
 \param[inout] d current event data
 */
void Fd_Snap_Action::check_all(Fd_Snap_Data &data) {
  for (int i=0; list[i]; i++) {
    if (list[i]->mask & data.drag)
      list[i]->check(data);
  }
  eex = data.ex_out;
  eey = data.ey_out;
}

/**
 \brief Draw a visual indicator for all snap actions that were applied during the last check.
 Only one snap coordinate can win. FLUID chooses the one that is closest to
 the current user event. If two or more snap actions suggest the same
 coordinate, all of them will be drawn.
 \param[inout] d current event data
 */
void Fd_Snap_Action::draw_all(Fd_Snap_Data &data) {
  for (int i=0; list[i]; i++) {
    if (list[i]->matches(data))
      list[i]->draw(data);
  }
}

/** Return a sensible step size for resizing a widget. */
void Fd_Snap_Action::get_resize_stepsize(int &x_step, int &y_step) {
  if ((layout->widget_inc_w > 1) && (layout->widget_inc_h > 1)) {
    x_step = layout->widget_inc_w;
    y_step = layout->widget_inc_h;
  } else if ((layout->group_grid_x > 1) && (layout->group_grid_y > 1)) {
    x_step = layout->group_grid_x;
    y_step = layout->group_grid_y;
  } else {
    x_step = layout->window_grid_x;
    y_step = layout->window_grid_y;
  }
}

/** Return a sensible step size for moving a widget. */
void Fd_Snap_Action::get_move_stepsize(int &x_step, int &y_step) {
  if ((layout->group_grid_x > 1) && (layout->group_grid_y > 1)) {
    x_step = layout->group_grid_x;
    y_step = layout->group_grid_y;
  } else if ((layout->window_grid_x > 1) && (layout->window_grid_y > 1)) {
    x_step = layout->window_grid_x;
    y_step = layout->window_grid_y;
  } else {
    x_step = layout->widget_gap_x;
    y_step = layout->widget_gap_y;
  }
}

/** Fix the given size to the same or next bigger snap position. */
void Fd_Snap_Action::better_size(int &w, int &h) {
  int x_min = 1, y_min = 1, x_inc = 1, y_inc = 1;
  get_resize_stepsize(x_inc, y_inc);
  if (x_inc < 1) x_inc = 1;
  if (y_inc < 1) y_inc = 1;
  if ((layout->widget_min_w > 1) && (layout->widget_min_h > 1)) {
    x_min = layout->widget_min_w;
    y_min = layout->widget_min_h;
  } else if ((layout->group_grid_x > 1) && (layout->group_grid_y > 1)) {
    x_min = layout->group_grid_x;
    y_min = layout->group_grid_y;
  } else {
    x_min = x_inc;
    y_min = y_inc;
  }
  int ww = fd_max(w - x_min, 0); w = (w - ww + x_inc - 1) / x_inc; w = w * x_inc; w = w + ww;
  int hh = fd_max(h - y_min, 0); h = (h - hh + y_inc - 1) / y_inc; h = h * y_inc; h = h + hh;
}


// ---- snapping prototypes -------------------------------------------- MARK: -

/**
 Base class for all actions that drag the left side or the entire widget.
 */
class Fd_Snap_Left : public Fd_Snap_Action {
public:
  Fd_Snap_Left() { type = 1; mask = FD_LEFT|FD_DRAG; }
};

/**
 Base class for all actions that drag the right side or the entire widget.
 */
class Fd_Snap_Right : public Fd_Snap_Action {
public:
  Fd_Snap_Right() { type = 1; mask = FD_RIGHT|FD_DRAG; }
};

/**
 Base class for all actions that drag the top side or the entire widget.
 */
class Fd_Snap_Top : public Fd_Snap_Action {
public:
  Fd_Snap_Top() { type = 2; mask = FD_TOP|FD_DRAG; }
};

/**
 Base class for all actions that drag the bottom side or the entire widget.
 */
class Fd_Snap_Bottom : public Fd_Snap_Action {
public:
  Fd_Snap_Bottom() { type = 2; mask = FD_BOTTOM|FD_DRAG; }
};

// ---- window snapping ------------------------------------------------ MARK: -

/**
 Check if the widget hits the left window edge.
 */
class Fd_Snap_Left_Window_Edge : public Fd_Snap_Left {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE { clr(); check_x_(d, d.bx, 0); }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE { draw_left_brace(d.win->o); };
};
Fd_Snap_Left_Window_Edge snap_left_window_edge;

/**
 Check if the widget hits the right window edge.
 */
class Fd_Snap_Right_Window_Edge : public Fd_Snap_Right {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE { clr(); check_x_(d, d.br, d.win->o->w()); }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE { draw_right_brace(d.win->o); };
};
Fd_Snap_Right_Window_Edge snap_right_window_edge;

/**
 Check if the widget hits the top window edge.
 */
class Fd_Snap_Top_Window_Edge : public Fd_Snap_Top {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE { clr(); check_y_(d, d.by, 0); }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE { draw_top_brace(d.win->o); };
};
Fd_Snap_Top_Window_Edge snap_top_window_edge;

/**
 Check if the widget hits the bottom window edge.
 */
class Fd_Snap_Bottom_Window_Edge : public Fd_Snap_Bottom {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE { clr(); check_y_(d, d.bt, d.win->o->h()); }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE { draw_bottom_brace(d.win->o); };
};
Fd_Snap_Bottom_Window_Edge snap_bottom_window_edge;

/**
 Check if the widget hits the left window edge plus a user defined margin.
 */
class Fd_Snap_Left_Window_Margin : public Fd_Snap_Left {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (in_window(d)) check_x_(d, d.bx, layout->left_window_margin);
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_h_arrow(d.bx, (d.by+d.bt)/2, 0);
  };
};
Fd_Snap_Left_Window_Margin snap_left_window_margin;

class Fd_Snap_Right_Window_Margin : public Fd_Snap_Right {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (in_window(d)) check_x_(d, d.br, d.win->o->w()-layout->right_window_margin);
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_h_arrow(d.br, (d.by+d.bt)/2, d.win->o->w()-1);
  };
};
Fd_Snap_Right_Window_Margin snap_right_window_margin;

class Fd_Snap_Top_Window_Margin : public Fd_Snap_Top {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (in_window(d)) check_y_(d, d.by, layout->top_window_margin);
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_v_arrow((d.bx+d.br)/2, d.by, 0);
  };
};
Fd_Snap_Top_Window_Margin snap_top_window_margin;

class Fd_Snap_Bottom_Window_Margin : public Fd_Snap_Bottom {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (in_window(d)) check_y_(d, d.bt, d.win->o->h()-layout->bottom_window_margin);
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_v_arrow((d.bx+d.br)/2, d.bt, d.win->o->h()-1);
  };
};
Fd_Snap_Bottom_Window_Margin snap_bottom_window_margin;

// ---- group snapping ------------------------------------------------- MARK: -

/**
 Check if the widget hits the left group edge.
 */
class Fd_Snap_Left_Group_Edge : public Fd_Snap_Left {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (in_group(d)) check_x_(d, d.bx, parent(d)->x());
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_left_brace(parent(d));
  };
};
Fd_Snap_Left_Group_Edge snap_left_group_edge;

class Fd_Snap_Right_Group_Edge : public Fd_Snap_Right {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (in_group(d)) check_x_(d, d.br, parent(d)->x() + parent(d)->w());
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_right_brace(parent(d));
  };
};
Fd_Snap_Right_Group_Edge snap_right_group_edge;

class Fd_Snap_Top_Group_Edge : public Fd_Snap_Top {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (in_group(d)) check_y_(d, d.by, parent(d)->y());
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_top_brace(parent(d));
  };
};
Fd_Snap_Top_Group_Edge snap_top_group_edge;

class Fd_Snap_Bottom_Group_Edge : public Fd_Snap_Bottom {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (in_group(d)) check_y_(d, d.bt, parent(d)->y() + parent(d)->h());
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_bottom_brace(parent(d));
  };
};
Fd_Snap_Bottom_Group_Edge snap_bottom_group_edge;


/**
 Check if the widget hits the left group edge plus a user defined margin.
 */
class Fd_Snap_Left_Group_Margin : public Fd_Snap_Left {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (in_group(d)) check_x_(d, d.bx, parent(d)->x() + layout->left_group_margin);
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_left_brace(parent(d));
    draw_h_arrow(d.bx, (d.by+d.bt)/2, parent(d)->x());
  };
};
Fd_Snap_Left_Group_Margin snap_left_group_margin;

class Fd_Snap_Right_Group_Margin : public Fd_Snap_Right {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (in_group(d)) check_x_(d, d.br, parent(d)->x()+parent(d)->w()-layout->right_group_margin);
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_right_brace(parent(d));
    draw_h_arrow(d.br, (d.by+d.bt)/2, parent(d)->x()+parent(d)->w()-1);
  };
};
Fd_Snap_Right_Group_Margin snap_right_group_margin;

class Fd_Snap_Top_Group_Margin : public Fd_Snap_Top {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (in_group(d) && !in_tabs(d)) check_y_(d, d.by, parent(d)->y()+layout->top_group_margin);
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_top_brace(parent(d));
    draw_v_arrow((d.bx+d.br)/2, d.by, parent(d)->y());
  };
};
Fd_Snap_Top_Group_Margin snap_top_group_margin;

class Fd_Snap_Bottom_Group_Margin : public Fd_Snap_Bottom {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (in_group(d) && !in_tabs(d)) check_y_(d, d.bt, parent(d)->y()+parent(d)->h()-layout->bottom_group_margin);
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_bottom_brace(parent(d));
    draw_v_arrow((d.bx+d.br)/2, d.bt, parent(d)->y()+parent(d)->h()-1);
  };
};
Fd_Snap_Bottom_Group_Margin snap_bottom_group_margin;

// ----- tabs snapping ------------------------------------------------- MARK: -

/**
 Check if the widget top hits the Fl_Tabs group top edge plus a user defined margin.
 */
class Fd_Snap_Top_Tabs_Margin : public Fd_Snap_Top_Group_Margin {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (in_tabs(d)) check_y_(d, d.by, parent(d)->y()+layout->top_tabs_margin);
  }
};
Fd_Snap_Top_Tabs_Margin snap_top_tabs_margin;

class Fd_Snap_Bottom_Tabs_Margin : public Fd_Snap_Bottom_Group_Margin {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (in_tabs(d)) check_y_(d, d.bt, parent(d)->y()+parent(d)->h()-layout->bottom_tabs_margin);
  }
};
Fd_Snap_Bottom_Tabs_Margin snap_bottom_tabs_margin;

// ----- grid snapping ------------------------------------------------- MARK: -

/**
 Base class for grid based snapping.
 */
class Fd_Snap_Grid : public Fd_Snap_Action {
protected:
  int nearest_x, nearest_y;
public:
  Fd_Snap_Grid() { type = 3; mask = FD_LEFT|FD_TOP|FD_DRAG; }
  void check_grid(Fd_Snap_Data &d, int left, int grid_x, int right, int top, int grid_y, int bottom) {
    if ((grid_x <= 1) || (grid_y <= 1)) return;
    int suggested_x = d.bx + d.dx;
    nearest_x = nearest(suggested_x, left, grid_x, right);
    int suggested_y = d.by + d.dy;
    nearest_y = nearest(suggested_y, top, grid_y, bottom);
    if (d.drag == FD_LEFT)
      check_x_(d, d.bx, nearest_x);
    else if (d.drag == FD_TOP)
      check_y_(d, d.by, nearest_y);
    else
      check_x_y_(d, d.bx, nearest_x, d.by, nearest_y);
  }
  bool matches(Fd_Snap_Data &d) FL_OVERRIDE {
    if (d.drag == FD_LEFT) return (eex == ex);
    if (d.drag == FD_TOP) return (eey == ey) && (d.dx == dx);
    return (d.drag & mask) && (eex == ex) && (d.dx == dx) && (eey == ey) && (d.dy == dy);
  }
};

/**
 Check if the widget hits window grid coordinates.
 */
class Fd_Snap_Window_Grid : public Fd_Snap_Grid {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (in_window(d)) check_grid(d, layout->left_window_margin, layout->window_grid_x, d.win->o->w()-layout->right_window_margin,
                                 layout->top_window_margin, layout->window_grid_y, d.win->o->h()-layout->bottom_window_margin);
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_grid(nearest_x, nearest_y, layout->window_grid_x, layout->window_grid_y);
  };
};
Fd_Snap_Window_Grid snap_window_grid;

/**
 Check if the widget hits group grid coordinates.
 */
class Fd_Snap_Group_Grid : public Fd_Snap_Grid {
public:
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    if (in_group(d)) {
      clr();
      Fl_Widget *g = parent(d);
      check_grid(d, g->x()+layout->left_group_margin, layout->group_grid_x, g->x()+g->w()-layout->right_group_margin,
                 g->y()+layout->top_group_margin, layout->group_grid_y, g->y()+g->h()-layout->bottom_group_margin);
    }
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_grid(nearest_x, nearest_y, layout->group_grid_x, layout->group_grid_y);
  };
};
Fd_Snap_Group_Grid snap_group_grid;

// ----- sibling snapping ---------------------------------------------- MARK: -

/**
 Base class the check distance to other widgets in the same group.
 */
class Fd_Snap_Sibling : public Fd_Snap_Action {
protected:
  Fl_Widget *best_match;
public:
  Fd_Snap_Sibling() : best_match(NULL) { }
  virtual int sibling_check(Fd_Snap_Data &d, Fl_Widget *s) = 0;
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    best_match = NULL;
    if (!d.wgt) return;
    if (!d.wgt->parent->is_a(ID_Group)) return;
    int dsib_min = 1024;
    Fl_Group_Type *gt = (Fl_Group_Type*)d.wgt->parent;
    Fl_Group *g = (Fl_Group*)gt->o;
    Fl_Widget *w = d.wgt->o;
    for (int i=0; i<g->children(); i++) {
      Fl_Widget *c = g->child(i);
      if (c == w) continue;
      int sret = sibling_check(d, c);
      if (sret < 1) {
        int dsib;
        if (type==1)
          dsib = abs( ((d.by+d.bt)/2+d.dy) - (c->y()+c->h()/2) );
        else
          dsib = abs( ((d.bx+d.br)/2+d.dx) - (c->x()+c->w()/2) );
        if (sret == -1 || (dsib < dsib_min)) {
          dsib_min = dsib;
          best_match = c;
        }
      }
    }
  }
};

/**
 Check if widgets have the same x coordinate, so they can be vertically aligned.
 */
class Fd_Snap_Siblings_Left_Same : public Fd_Snap_Sibling {
public:
  Fd_Snap_Siblings_Left_Same() { type = 1; mask = FD_LEFT|FD_DRAG; }
  int sibling_check(Fd_Snap_Data &d, Fl_Widget *s) FL_OVERRIDE {
    return check_x_(d, d.bx, s->x());
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    if (best_match) draw_left_brace(best_match);
  };
};
Fd_Snap_Siblings_Left_Same snap_siblings_left_same;

/**
 Check if widgets touch left to right, or have a user selected gap left to right.
 */
class Fd_Snap_Siblings_Left : public Fd_Snap_Sibling {
public:
  Fd_Snap_Siblings_Left() { type = 1; mask = FD_LEFT|FD_DRAG; }
  int sibling_check(Fd_Snap_Data &d, Fl_Widget *s) FL_OVERRIDE {
    return fd_min(check_x_(d, d.bx, s->x()+s->w()),
                  check_x_(d, d.bx, s->x()+s->w()+layout->widget_gap_x) );
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    if (best_match) draw_right_brace(best_match);
  };
};
Fd_Snap_Siblings_Left snap_siblings_left;

class Fd_Snap_Siblings_Right_Same : public Fd_Snap_Sibling {
public:
  Fd_Snap_Siblings_Right_Same() { type = 1; mask = FD_RIGHT|FD_DRAG; }
  int sibling_check(Fd_Snap_Data &d, Fl_Widget *s) FL_OVERRIDE {
    return check_x_(d, d.br, s->x()+s->w());
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    if (best_match) draw_right_brace(best_match);
  };
};
Fd_Snap_Siblings_Right_Same snap_siblings_right_same;

class Fd_Snap_Siblings_Right : public Fd_Snap_Sibling {
public:
  Fd_Snap_Siblings_Right() { type = 1; mask = FD_RIGHT|FD_DRAG; }
  int sibling_check(Fd_Snap_Data &d, Fl_Widget *s) FL_OVERRIDE {
    return fd_min(check_x_(d, d.br, s->x()),
                  check_x_(d, d.br, s->x()-layout->widget_gap_x));
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    if (best_match) draw_left_brace(best_match);
  };
};
Fd_Snap_Siblings_Right snap_siblings_right;

class Fd_Snap_Siblings_Top_Same : public Fd_Snap_Sibling {
public:
  Fd_Snap_Siblings_Top_Same() { type = 2; mask = FD_TOP|FD_DRAG; }
  int sibling_check(Fd_Snap_Data &d, Fl_Widget *s) FL_OVERRIDE {
    return check_y_(d, d.by, s->y());
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    if (best_match) draw_top_brace(best_match);
  };
};
Fd_Snap_Siblings_Top_Same snap_siblings_top_same;

class Fd_Snap_Siblings_Top : public Fd_Snap_Sibling {
public:
  Fd_Snap_Siblings_Top() { type = 2; mask = FD_TOP|FD_DRAG; }
  int sibling_check(Fd_Snap_Data &d, Fl_Widget *s) FL_OVERRIDE {
    return fd_min(check_y_(d, d.by, s->y()+s->h()),
                  check_y_(d, d.by, s->y()+s->h()+layout->widget_gap_y));
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    if (best_match) draw_bottom_brace(best_match);
  };
};
Fd_Snap_Siblings_Top snap_siblings_top;

class Fd_Snap_Siblings_Bottom_Same : public Fd_Snap_Sibling {
public:
  Fd_Snap_Siblings_Bottom_Same() { type = 2; mask = FD_BOTTOM|FD_DRAG; }
  int sibling_check(Fd_Snap_Data &d, Fl_Widget *s) FL_OVERRIDE {
    return check_y_(d, d.bt, s->y()+s->h());
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    if (best_match) draw_bottom_brace(best_match);
  };
};
Fd_Snap_Siblings_Bottom_Same snap_siblings_bottom_same;

class Fd_Snap_Siblings_Bottom : public Fd_Snap_Sibling {
public:
  Fd_Snap_Siblings_Bottom() { type = 2; mask = FD_BOTTOM|FD_DRAG; }
  int sibling_check(Fd_Snap_Data &d, Fl_Widget *s) FL_OVERRIDE {
    return fd_min(check_y_(d, d.bt, s->y()),
                  check_y_(d, d.bt, s->y()-layout->widget_gap_y));
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    if (best_match) draw_top_brace(best_match);
  };
};
Fd_Snap_Siblings_Bottom snap_siblings_bottom;


// ------ widget snapping ---------------------------------------------- MARK: -

/**
 Snap horizontal resizing to min_w or min_w and a multiple of inc_w.
 */
class Fd_Snap_Widget_Ideal_Width : public Fd_Snap_Action {
public:
  Fd_Snap_Widget_Ideal_Width() { type = 1; mask = FD_LEFT|FD_RIGHT; }
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (!d.wgt) return;
    int iw = 15, ih = 15;
    d.wgt->ideal_size(iw, ih);
    if (d.drag == FD_RIGHT) {
      check_x_(d, d.br, d.bx+iw);
      iw = layout->widget_min_w;
      if (iw > 0) iw = nearest(d.br-d.bx+d.dx, layout->widget_min_w, layout->widget_inc_w);
      check_x_(d, d.br, d.bx+iw);
    } else {
      check_x_(d, d.bx, d.br-iw);
      iw = layout->widget_min_w;
      if (iw > 0) iw = nearest(d.br-d.bx-d.dx, layout->widget_min_w, layout->widget_inc_w);
      check_x_(d, d.bx, d.br-iw);
    }
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_width(d.bx, d.bt+7, d.br, 0);
  };
};
Fd_Snap_Widget_Ideal_Width snap_widget_ideal_width;

class Fd_Snap_Widget_Ideal_Height : public Fd_Snap_Action {
public:
  Fd_Snap_Widget_Ideal_Height() { type = 2; mask = FD_TOP|FD_BOTTOM; }
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    clr();
    if (!d.wgt) return;
    int iw, ih;
    d.wgt->ideal_size(iw, ih);
    if (d.drag == FD_BOTTOM) {
      check_y_(d, d.bt, d.by+ih);
      ih = layout->widget_min_h;
      if (ih > 0) ih = nearest(d.bt-d.by+d.dy, layout->widget_min_h, layout->widget_inc_h);
      check_y_(d, d.bt, d.by+ih);
    } else {
      check_y_(d, d.by, d.bt-ih);
      ih = layout->widget_min_h;
      if (ih > 0) ih = nearest(d.bt-d.by-d.dy, layout->widget_min_h, layout->widget_inc_h);
      check_y_(d, d.by, d.bt-ih);
    }
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_height(d.br+7, d.by, d.bt, 0);
  };
};
Fd_Snap_Widget_Ideal_Height snap_widget_ideal_height;

// ---- snap actions list ---------------------------------------------- MARK: -

/**
 /brief The list of all snap actions available to FLUID.
 New snap actions can be appended to the list. If multiple snap actions
 with different coordinates, but the same snap distance are found, the last
 action in the list wins. All snap actions with the same distance and same
 winning coordinates are drawn in the overlay plane.
 */
Fd_Snap_Action *Fd_Snap_Action::list[] = {
  &snap_left_window_edge,
  &snap_right_window_edge,
  &snap_top_window_edge,
  &snap_bottom_window_edge,

  &snap_left_window_margin,
  &snap_right_window_margin,
  &snap_top_window_margin,
  &snap_bottom_window_margin,

  &snap_window_grid,
  &snap_group_grid,

  &snap_left_group_edge,
  &snap_right_group_edge,
  &snap_top_group_edge,
  &snap_bottom_group_edge,

  &snap_left_group_margin,
  &snap_right_group_margin,
  &snap_top_group_margin,
  &snap_bottom_group_margin,

  &snap_top_tabs_margin,
  &snap_bottom_tabs_margin,

  &snap_siblings_left_same, &snap_siblings_left,
  &snap_siblings_right_same, &snap_siblings_right,
  &snap_siblings_top_same, &snap_siblings_top,
  &snap_siblings_bottom_same, &snap_siblings_bottom,

  &snap_widget_ideal_width,
  &snap_widget_ideal_height,

  NULL
};

// ---- draw alignment marks ------------------------------------------- MARK: -

static void draw_v_arrow(int x, int y1, int y2) {
  int dy = (y1>y2) ? -1 : 1 ;
  fl_yxline(x, y1, y2);
  fl_xyline(x-4, y2, x+4);
  fl_line(x-2, y2-dy*5, x, y2-dy);
  fl_line(x+2, y2-dy*5, x, y2-dy);
}

static void draw_h_arrow(int x1, int y, int x2) {
  int dx = (x1>x2) ? -1 : 1 ;
  fl_xyline(x1, y, x2);
  fl_yxline(x2, y-4, y+4);
  fl_line(x2-dx*5, y-2, x2-dx, y);
  fl_line(x2-dx*5, y+2, x2-dx, y);
}

static void draw_top_brace(const Fl_Widget *w) {
  int x = w->as_window() ? 0 : w->x();
  int y = w->as_window() ? 0 : w->y();
  fl_yxline(x, y-2, y+6);
  fl_yxline(x+w->w()-1, y-2, y+6);
  fl_xyline(x-2, y, x+w->w()+1);
}

static void draw_left_brace(const Fl_Widget *w)  {
  int x = w->as_window() ? 0 : w->x();
  int y = w->as_window() ? 0 : w->y();
  fl_xyline(x-2, y, x+6);
  fl_xyline(x-2, y+w->h()-1, x+6);
  fl_yxline(x, y-2, y+w->h()+1);
}

static void draw_right_brace(const Fl_Widget *w) {
  int x = w->as_window() ? w->w() - 1 : w->x() + w->w() - 1;
  int y = w->as_window() ? 0 : w->y();
  fl_xyline(x-6, y, x+2);
  fl_xyline(x-6, y+w->h()-1, x+2);
  fl_yxline(x, y-2, y+w->h()+1);
}

static void draw_bottom_brace(const Fl_Widget *w) {
  int x = w->as_window() ? 0 : w->x();
  int y = w->as_window() ? w->h() - 1 : w->y() + w->h() - 1;
  fl_yxline(x, y-6, y+2);
  fl_yxline(x+w->w()-1, y-6, y+2);
  fl_xyline(x-2, y, x+w->w()+1);
}

void draw_height(int x, int y, int b, Fl_Align a) {
  char buf[16];
  int h = b - y;
  sprintf(buf, "%d", h);
  fl_font(FL_HELVETICA, 9);
  int lw = (int)fl_width(buf);
  int lx;

  b --;
  if (h < 30) {
    // Move height to the side...
    if (a == FL_ALIGN_LEFT) lx = x - lw - 2;
    else lx = x + 2;
    fl_yxline(x, y, b);
  } else {
    // Put height inside the arrows...
    if (a == FL_ALIGN_LEFT) lx = x - lw + 2;
    else lx = x - lw / 2;
    fl_yxline(x, y, y + (h - 11) / 2);
    fl_yxline(x, y + (h + 11) / 2, b);
  }

  // Draw the height...
  fl_draw(buf, lx, y + (h + 7) / 2);

  // Draw the arrowheads...
  fl_line(x-2, y+5, x, y+1, x+2, y+5);
  fl_line(x-2, b-5, x, b-1, x+2, b-5);

  // Draw the end lines...
  fl_xyline(x - 4, y, x + 4);
  fl_xyline(x - 4, b, x + 4);
}

void draw_width(int x, int y, int r, Fl_Align a) {
  char buf[16];
  int w = r-x;
  sprintf(buf, "%d", w);
  fl_font(FL_HELVETICA, 9);
  int lw = (int)fl_width(buf);
  int ly = y + 4;

  r--;

  if (lw > (w - 20)) {
    // Move width above/below the arrows...
    if (a == FL_ALIGN_TOP) ly -= 10;
    else ly += 10;

    fl_xyline(x, y, r);
  } else {
    // Put width inside the arrows...
    fl_xyline(x, y, x + (w - lw - 2) / 2);
    fl_xyline(x + (w + lw + 2) / 2, y, r);
  }

  // Draw the width...
  fl_draw(buf, x + (w - lw) / 2, ly-2);

  // Draw the arrowheads...
  fl_line(x+5, y-2, x+1, y, x+5, y+2);
  fl_line(r-5, y-2, r-1, y, r-5, y+2);

  // Draw the end lines...
  fl_yxline(x, y - 4, y + 4);
  fl_yxline(r, y - 4, y + 4);
}

static void draw_grid(int x, int y, int dx, int dy) {
  int dx2 = 1, dy2 = 1;
  const int n = 2;
  for (int i=-n; i<=n; i++) {
    for (int j=-n; j<=n; j++) {
      if (abs(i)+abs(j) < 4) {
        int xx = x + i*dx , yy = y + j*dy;
        fl_xyline(xx-dx2, yy, xx+dx2);
        fl_yxline(xx, yy-dy2, yy+dy2);
      }
    }
  }
}
