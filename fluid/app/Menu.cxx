//
// Application Main Menu code for the Fast Light Tool Kit (FLTK).
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

#include "app/Menu.h"

#include "Fluid.h"

#include "proj/undo.h"
#include "app/templates.h"
#include "nodes/Node.h"
#include "nodes/Group_Node.h"
#include "nodes/Window_Node.h"
#include "nodes/factory.h"
#include "panels/codeview_panel.h"
#include "app/shell_command.h"

#include <FL/Fl_Menu_Bar.H>

// In Snap_Action.h
extern void layout_suite_marker(Fl_Widget *, void *user_data);
extern void select_layout_preset_cb(Fl_Widget *, void *user_data);
extern Fl_Menu_Item main_layout_submenu_[];

using namespace fld;


void write_cb(Fl_Widget *, void *) {
   Fluid.write_code_files();
}
void openwidget_cb(Fl_Widget *, void *) { Fluid.edit_selected(); }
void copy_cb(Fl_Widget*, void*) { Fluid.copy_selected(); }
void cut_cb(Fl_Widget *, void *) { Fluid.cut_selected(); }
void delete_cb(Fl_Widget *, void *) { Fluid.delete_selected(); }
void paste_cb(Fl_Widget*, void*) { Fluid.paste_from_clipboard(); }
void duplicate_cb(Fl_Widget*, void*) { Fluid.duplicate_selected(); }
static void sort_cb(Fl_Widget *,void *) { Fluid.sort_selected(); }
void about_cb(Fl_Widget *, void *) { Fluid.about(); }
void help_cb(Fl_Widget *, void *) {
  Fluid.show_help("fluid.html");
}
static void save_template_cb(Fl_Widget *, void *) { fld::app::save_template(); }

void manual_cb(Fl_Widget *, void *) {
  Fluid.show_help("index.html");
}

static void menu_file_new_cb(Fl_Widget *, void *) { Fluid.new_project(); }
static void menu_file_new_from_template_cb(Fl_Widget *, void *) { Fluid.new_project_from_template(); }
static void menu_file_open_cb(Fl_Widget *, void *) { Fluid.open_project_file(""); }
static void menu_file_insert_cb(Fl_Widget *, void *) { Fluid.merge_project_file(""); }
void menu_file_save_cb(Fl_Widget *, void *arg) { Fluid.save_project_file(arg); }
static void menu_file_print_cb(Fl_Widget *, void *arg) { Fluid.print_snapshots(); }
void menu_file_open_history_cb(Fl_Widget *, void *v) { Fluid.open_project_file(std::string((const char*)v)); }
static void menu_layout_sync_resize_cb(Fl_Menu_ *m, void*) {
 if (m->mvalue()->value()) Fluid.proj.tree.allow_layout = 1; else Fluid.proj.tree.allow_layout = 0; 
}
static void menu_file_revert_cb(Fl_Widget *, void *) { Fluid.revert_project(); }
void exit_cb(Fl_Widget *,void *) { Fluid.quit(); }
static void write_strings_cb(Fl_Widget *, void *) { Fluid.proj.write_strings(); }
void toggle_widgetbin_cb(Fl_Widget *, void *) { Fluid.toggle_widget_bin(); }
/**
 This is the main Fluid menu.

 Design history is manipulated right inside this menu structure.
 Some menu items change or deactivate correctly, but most items just trigger
 various callbacks.

 \c New_Menu creates new widgets and is explained in detail in another location.

 \see New_Menu
 \todo This menu needs some major modernization. Menus are too long and their
    sorting is not always obvious.
 \todo Shortcuts are all over the place (Alt, Ctrl, Command, Shift-Ctrl,
    function keys), and there should be a help page listing all shortcuts.
 */
Fl_Menu_Item Application::main_menu[] = {
  {"&File",0,nullptr,nullptr,FL_SUBMENU},
  {"&New", FL_COMMAND+'n', menu_file_new_cb},
  {"&Open...", FL_COMMAND+'o', menu_file_open_cb},
  {"&Insert...", FL_COMMAND+'i', menu_file_insert_cb, nullptr, FL_MENU_DIVIDER},
  {"&Save", FL_COMMAND+'s', menu_file_save_cb, nullptr},
  {"Save &As...", FL_COMMAND+FL_SHIFT+'s', menu_file_save_cb, (void*)1},
  {"Sa&ve A Copy...", 0, menu_file_save_cb, (void*)2},
  {"&Revert...", 0, menu_file_revert_cb, nullptr, FL_MENU_DIVIDER},
  {"New &From Template...", FL_COMMAND+'N', menu_file_new_from_template_cb, nullptr},
  {"Save As &Template...", 0, save_template_cb, nullptr, FL_MENU_DIVIDER},
  {"&Print...", FL_COMMAND+'p', menu_file_print_cb},
  {"Write &Code", FL_COMMAND+FL_SHIFT+'c', write_cb, nullptr},
// Matt: disabled {"MergeBack Code", FL_COMMAND+FL_SHIFT+'m', mergeback_cb, 0},
  {"&Write Strings", FL_COMMAND+FL_SHIFT+'w', write_strings_cb, nullptr, FL_MENU_DIVIDER},
  {Fluid.history.relpath[0], FL_COMMAND+'1', menu_file_open_history_cb, Fluid.history.abspath[0]},
  {Fluid.history.relpath[1], FL_COMMAND+'2', menu_file_open_history_cb, Fluid.history.abspath[1]},
  {Fluid.history.relpath[2], FL_COMMAND+'3', menu_file_open_history_cb, Fluid.history.abspath[2]},
  {Fluid.history.relpath[3], FL_COMMAND+'4', menu_file_open_history_cb, Fluid.history.abspath[3]},
  {Fluid.history.relpath[4], FL_COMMAND+'5', menu_file_open_history_cb, Fluid.history.abspath[4]},
  {Fluid.history.relpath[5], FL_COMMAND+'6', menu_file_open_history_cb, Fluid.history.abspath[5]},
  {Fluid.history.relpath[6], FL_COMMAND+'7', menu_file_open_history_cb, Fluid.history.abspath[6]},
  {Fluid.history.relpath[7], FL_COMMAND+'8', menu_file_open_history_cb, Fluid.history.abspath[7]},
  {Fluid.history.relpath[8], FL_COMMAND+'9', menu_file_open_history_cb, Fluid.history.abspath[8]},
  {Fluid.history.relpath[9], 0, menu_file_open_history_cb, Fluid.history.abspath[9], FL_MENU_DIVIDER},
  {"&Quit", FL_COMMAND+'q', exit_cb},
  {nullptr},
  {"&Edit",0,nullptr,nullptr,FL_SUBMENU},
  {"&Undo", FL_COMMAND+'z', fld::proj::Undo::undo_cb},
  {"&Redo", FL_COMMAND+FL_SHIFT+'z', fld::proj::Undo::redo_cb, nullptr, FL_MENU_DIVIDER},
  {"C&ut", FL_COMMAND+'x', cut_cb},
  {"&Copy", FL_COMMAND+'c', copy_cb},
  {"&Paste", FL_COMMAND+'v', paste_cb},
  {"Dup&licate", FL_COMMAND+'u', duplicate_cb},
  {"&Delete", FL_Delete, delete_cb, nullptr, FL_MENU_DIVIDER},
  {"Select &All", FL_COMMAND+'a', select_all_cb},
  {"Select &None", FL_COMMAND+FL_SHIFT+'a', select_none_cb, nullptr, FL_MENU_DIVIDER},
  {"Pr&operties...", FL_F+1, openwidget_cb},
  {"&Sort",0,sort_cb},
  {"&Earlier", FL_F+2, earlier_cb},
  {"&Later", FL_F+3, later_cb},
  {"&Group", FL_F+7, group_cb},
  {"Ung&roup", FL_F+8, ungroup_cb,nullptr, FL_MENU_DIVIDER},
  {"Hide O&verlays",FL_COMMAND+FL_SHIFT+'o',toggle_overlays},
  {"Hide Guides",FL_COMMAND+FL_SHIFT+'g',toggle_guides},
  {"Hide Restricted",FL_COMMAND+FL_SHIFT+'r',toggle_restricted},
  {"Show Widget &Bin...",FL_ALT+'b',toggle_widgetbin_cb},
  {"Show Code View",FL_ALT+'c', (Fl_Callback*)toggle_codeview_cb, nullptr, FL_MENU_DIVIDER},
  {"Settings...",FL_ALT+'p',show_settings_cb},
  {nullptr},
  {"&New", 0, nullptr, (void *)New_Menu, FL_SUBMENU_POINTER},
  {"&Layout",0,nullptr,nullptr,FL_SUBMENU},
  {"&Align",0,nullptr,nullptr,FL_SUBMENU},
    {"&Left",0,(Fl_Callback *)align_widget_cb,(void*)10},
    {"&Center",0,(Fl_Callback *)align_widget_cb,(void*)11},
    {"&Right",0,(Fl_Callback *)align_widget_cb,(void*)12},
    {"&Top",0,(Fl_Callback *)align_widget_cb,(void*)13},
    {"&Middle",0,(Fl_Callback *)align_widget_cb,(void*)14},
    {"&Bottom",0,(Fl_Callback *)align_widget_cb,(void*)15},
  {nullptr},
  {"&Space Evenly",0,nullptr,nullptr,FL_SUBMENU},
    {"&Across",0,(Fl_Callback *)align_widget_cb,(void*)20},
    {"&Down",0,(Fl_Callback *)align_widget_cb,(void*)21},
  {nullptr},
  {"&Make Same Size",0,nullptr,nullptr,FL_SUBMENU},
    {"&Width",0,(Fl_Callback *)align_widget_cb,(void*)30},
    {"&Height",0,(Fl_Callback *)align_widget_cb,(void*)31},
    {"&Both",0,(Fl_Callback *)align_widget_cb,(void*)32},
  {nullptr},
  {"&Center In Group",0,nullptr,nullptr,FL_SUBMENU},
    {"&Horizontal",0,(Fl_Callback *)align_widget_cb,(void*)40},
    {"&Vertical",0,(Fl_Callback *)align_widget_cb,(void*)41},
  {nullptr},
  {"Synchronized Resize", 0, (Fl_Callback*)menu_layout_sync_resize_cb, nullptr, FL_MENU_TOGGLE|FL_MENU_DIVIDER },
  {"&Grid and Size Settings...",FL_COMMAND+'g',show_grid_cb, nullptr, FL_MENU_DIVIDER},
  {"Presets", 0, layout_suite_marker, (void*)main_layout_submenu_, FL_SUBMENU_POINTER },
  {"Application", 0, select_layout_preset_cb, (void*)nullptr, FL_MENU_RADIO|FL_MENU_VALUE },
  {"Dialog",      0, select_layout_preset_cb, (void*)1, FL_MENU_RADIO },
  {"Toolbox",     0, select_layout_preset_cb, (void*)2, FL_MENU_RADIO },
  {nullptr},
{"&Shell", 0, Fd_Shell_Command_List::menu_marker, (void*)Fd_Shell_Command_List::default_menu, FL_SUBMENU_POINTER},
  {"&Help",0,nullptr,nullptr,FL_SUBMENU},
  {"&Rapid development with FLUID...",0,help_cb},
  {"&FLTK Programmers Manual...",0,manual_cb, nullptr, FL_MENU_DIVIDER},
  {"&About FLUID...",0,about_cb},
  {nullptr},
{nullptr}};

/**
Show or hide the code preview window.
*/
void toggle_codeview_cb(Fl_Double_Window *, void *) {
 codeview_toggle_visibility();
}

/**
Show or hide the code preview window, button callback.
*/
void toggle_codeview_b_cb(Fl_Button*, void *) {
 codeview_toggle_visibility();
}

