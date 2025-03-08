//
// FLUID main entry for the Fast Light Tool Kit (FLTK).
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

#include "app/undo.h"
#include "app/templates.h"
#include "nodes/Fl_Type.h"
#include "nodes/Fl_Group_Type.h"
#include "nodes/Fl_Window_Type.h"
#include "nodes/factory.h"
#include "panels/codeview_panel.h"
#include "app/shell_command.h"

#include <FL/Fl_Menu_Bar.H>

// In Fd_Snap_Action.h
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
 if (m->mvalue()->value()) Fl_Type::allow_layout = 1; else Fl_Type::allow_layout = 0; 
}
static void menu_file_revert_cb(Fl_Widget *, void *) { Fluid.revert_project(); }

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
Fl_Menu_Item Main_Menu[] = {
{"&File",0,0,0,FL_SUBMENU},
  {"&New", FL_COMMAND+'n', menu_file_new_cb},
  {"&Open...", FL_COMMAND+'o', menu_file_open_cb},
  {"&Insert...", FL_COMMAND+'i', menu_file_insert_cb, 0, FL_MENU_DIVIDER},
  {"&Save", FL_COMMAND+'s', menu_file_save_cb, 0},
  {"Save &As...", FL_COMMAND+FL_SHIFT+'s', menu_file_save_cb, (void*)1},
  {"Sa&ve A Copy...", 0, menu_file_save_cb, (void*)2},
  {"&Revert...", 0, menu_file_revert_cb, 0, FL_MENU_DIVIDER},
  {"New &From Template...", FL_COMMAND+'N', menu_file_new_from_template_cb, 0},
  {"Save As &Template...", 0, save_template_cb, 0, FL_MENU_DIVIDER},
  {"&Print...", FL_COMMAND+'p', menu_file_print_cb},
  {"Write &Code", FL_COMMAND+FL_SHIFT+'c', write_cb, 0},
// Matt: disabled {"MergeBack Code", FL_COMMAND+FL_SHIFT+'m', mergeback_cb, 0},
  {"&Write Strings", FL_COMMAND+FL_SHIFT+'w', write_strings_cb, 0, FL_MENU_DIVIDER},
  {Fluid.project_history_relpath[0], FL_COMMAND+'1', menu_file_open_history_cb, Fluid.project_history_abspath[0]},
  {Fluid.project_history_relpath[1], FL_COMMAND+'2', menu_file_open_history_cb, Fluid.project_history_abspath[1]},
  {Fluid.project_history_relpath[2], FL_COMMAND+'3', menu_file_open_history_cb, Fluid.project_history_abspath[2]},
  {Fluid.project_history_relpath[3], FL_COMMAND+'4', menu_file_open_history_cb, Fluid.project_history_abspath[3]},
  {Fluid.project_history_relpath[4], FL_COMMAND+'5', menu_file_open_history_cb, Fluid.project_history_abspath[4]},
  {Fluid.project_history_relpath[5], FL_COMMAND+'6', menu_file_open_history_cb, Fluid.project_history_abspath[5]},
  {Fluid.project_history_relpath[6], FL_COMMAND+'7', menu_file_open_history_cb, Fluid.project_history_abspath[6]},
  {Fluid.project_history_relpath[7], FL_COMMAND+'8', menu_file_open_history_cb, Fluid.project_history_abspath[7]},
  {Fluid.project_history_relpath[8], FL_COMMAND+'9', menu_file_open_history_cb, Fluid.project_history_abspath[8]},
  {Fluid.project_history_relpath[9], 0, menu_file_open_history_cb, Fluid.project_history_abspath[9], FL_MENU_DIVIDER},
  {"&Quit", FL_COMMAND+'q', exit_cb},
  {0},
{"&Edit",0,0,0,FL_SUBMENU},
  {"&Undo", FL_COMMAND+'z', undo_cb},
  {"&Redo", FL_COMMAND+FL_SHIFT+'z', redo_cb, 0, FL_MENU_DIVIDER},
  {"C&ut", FL_COMMAND+'x', cut_cb},
  {"&Copy", FL_COMMAND+'c', copy_cb},
  {"&Paste", FL_COMMAND+'v', paste_cb},
  {"Dup&licate", FL_COMMAND+'u', duplicate_cb},
  {"&Delete", FL_Delete, delete_cb, 0, FL_MENU_DIVIDER},
  {"Select &All", FL_COMMAND+'a', select_all_cb},
  {"Select &None", FL_COMMAND+FL_SHIFT+'a', select_none_cb, 0, FL_MENU_DIVIDER},
  {"Pr&operties...", FL_F+1, openwidget_cb},
  {"&Sort",0,sort_cb},
  {"&Earlier", FL_F+2, earlier_cb},
  {"&Later", FL_F+3, later_cb},
  {"&Group", FL_F+7, group_cb},
  {"Ung&roup", FL_F+8, ungroup_cb,0, FL_MENU_DIVIDER},
  {"Hide O&verlays",FL_COMMAND+FL_SHIFT+'o',toggle_overlays},
  {"Hide Guides",FL_COMMAND+FL_SHIFT+'g',toggle_guides},
  {"Hide Restricted",FL_COMMAND+FL_SHIFT+'r',toggle_restricted},
  {"Show Widget &Bin...",FL_ALT+'b',toggle_widgetbin_cb},
  {"Show Code View",FL_ALT+'c', (Fl_Callback*)toggle_codeview_cb, 0, FL_MENU_DIVIDER},
  {"Settings...",FL_ALT+'p',show_settings_cb},
  {0},
{"&New", 0, 0, (void *)New_Menu, FL_SUBMENU_POINTER},
{"&Layout",0,0,0,FL_SUBMENU},
  {"&Align",0,0,0,FL_SUBMENU},
    {"&Left",0,(Fl_Callback *)align_widget_cb,(void*)10},
    {"&Center",0,(Fl_Callback *)align_widget_cb,(void*)11},
    {"&Right",0,(Fl_Callback *)align_widget_cb,(void*)12},
    {"&Top",0,(Fl_Callback *)align_widget_cb,(void*)13},
    {"&Middle",0,(Fl_Callback *)align_widget_cb,(void*)14},
    {"&Bottom",0,(Fl_Callback *)align_widget_cb,(void*)15},
    {0},
  {"&Space Evenly",0,0,0,FL_SUBMENU},
    {"&Across",0,(Fl_Callback *)align_widget_cb,(void*)20},
    {"&Down",0,(Fl_Callback *)align_widget_cb,(void*)21},
    {0},
  {"&Make Same Size",0,0,0,FL_SUBMENU},
    {"&Width",0,(Fl_Callback *)align_widget_cb,(void*)30},
    {"&Height",0,(Fl_Callback *)align_widget_cb,(void*)31},
    {"&Both",0,(Fl_Callback *)align_widget_cb,(void*)32},
    {0},
  {"&Center In Group",0,0,0,FL_SUBMENU},
    {"&Horizontal",0,(Fl_Callback *)align_widget_cb,(void*)40},
    {"&Vertical",0,(Fl_Callback *)align_widget_cb,(void*)41},
    {0},
  {"Synchronized Resize", 0, (Fl_Callback*)menu_layout_sync_resize_cb, NULL, FL_MENU_TOGGLE|FL_MENU_DIVIDER },
  {"&Grid and Size Settings...",FL_COMMAND+'g',show_grid_cb, NULL, FL_MENU_DIVIDER},
  {"Presets", 0, layout_suite_marker, (void*)main_layout_submenu_, FL_SUBMENU_POINTER },
  {"Application", 0, select_layout_preset_cb, (void*)0, FL_MENU_RADIO|FL_MENU_VALUE },
  {"Dialog",      0, select_layout_preset_cb, (void*)1, FL_MENU_RADIO },
  {"Toolbox",     0, select_layout_preset_cb, (void*)2, FL_MENU_RADIO },
  {0},
{"&Shell", 0, Fd_Shell_Command_List::menu_marker, (void*)Fd_Shell_Command_List::default_menu, FL_SUBMENU_POINTER},
{"&Help",0,0,0,FL_SUBMENU},
  {"&Rapid development with FLUID...",0,help_cb},
  {"&FLTK Programmers Manual...",0,manual_cb, 0, FL_MENU_DIVIDER},
  {"&About FLUID...",0,about_cb},
  {0},
{0}};

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

