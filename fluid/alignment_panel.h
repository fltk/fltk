//
// Setting and shell dialogs for the Fast Light Tool Kit (FLTK).
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

// generated by Fast Light User Interface Designer (fluid) version 1.0400

#ifndef alignment_panel_h
#define alignment_panel_h
#include <FL/Fl.H>
#include "fluid.h"
#include "widget_browser.h"
#include "Fd_Snap_Action.h"
#include "shell_command.h"
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/filename.H>
#include <FL/fl_string_functions.h>
#include <FL/Fl_Scheme_Choice.H>
/**
 // initialize the scheme from preferences
*/
void init_scheme(void);
extern struct Fl_Menu_Item *dbmanager_item;
extern void i18n_cb(Fl_Choice *,void *);
extern void scheme_cb(Fl_Scheme_Choice *, void *);
extern int w_settings_shell_list_selected;
#include <FL/Fl_Double_Window.H>
extern Fl_Double_Window *settings_window;
#include <FL/Fl_Tabs.H>
extern Fl_Tabs *w_settings_tabs;
#include <FL/Fl_Group.H>
extern void scheme_cb(Fl_Scheme_Choice*, void*);
extern Fl_Scheme_Choice *scheme_choice;
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
extern Fl_Check_Button *tooltips_button;
extern Fl_Check_Button *completion_button;
extern Fl_Check_Button *openlast_button;
extern Fl_Check_Button *prevpos_button;
extern Fl_Check_Button *show_comments_button;
#include <FL/Fl_Spinner.H>
extern Fl_Spinner *recent_spinner;
extern Fl_Check_Button *use_external_editor_button;
#include <FL/Fl_Input.H>
extern Fl_Input *editor_command_input;
extern void toggle_guides_cb(Fl_Check_Button*, void*);
extern Fl_Check_Button *guides_button;
extern void toggle_restricted_cb(Fl_Check_Button*, void*);
extern Fl_Check_Button *restricted_button;
extern Fl_Group *w_settings_project_tab;
extern Fl_Input *header_file_input;
extern Fl_Input *code_file_input;
extern Fl_Check_Button *include_H_from_C_button;
extern Fl_Check_Button *use_FL_COMMAND_button;
extern Fl_Check_Button *utf8_in_src_button;
extern Fl_Check_Button *avoid_early_includes_button;
extern Fl_Group *w_settings_layout_tab;
#include <FL/Fl_Choice.H>
extern Fl_Choice *layout_choice;
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Button.H>
extern Fl_Menu_Button *w_layout_menu;
#include <FL/Fl_Native_File_Chooser.H>
extern void propagate_load(Fl_Group*, void*);
extern void edit_layout_preset_cb(Fl_Button*, long);
extern Fl_Button *preset_choice[3];
#include <FL/Fl_Value_Input.H>
extern Fl_Menu_Item fontmenu_w_default[];
extern Fl_Group *w_settings_shell_tab;
#include <FL/Fl_Browser.H>
extern Fl_Browser *w_settings_shell_list;
extern Fl_Group *w_settings_shell_toolbox;
extern Fl_Button *w_settings_shell_dup;
extern Fl_Button *w_settings_shell_remove;
extern Fl_Menu_Button *w_settings_shell_menu;
extern Fl_Button *w_settings_shell_play;
extern Fl_Group *w_settings_shell_cmd;
#include <FL/Fl_Shortcut_Button.H>
#include <FL/Fl_Text_Editor.H>
extern Fl_Text_Editor *w_settings_shell_command;
extern Fl_Menu_Button *w_settings_shell_text_macros;
extern Fl_Group *w_settings_i18n_tab;
extern void i18n_type_cb(Fl_Choice*, void*);
extern Fl_Choice *i18n_type_chooser;
extern Fl_Group *i18n_gnu_group;
extern Fl_Input *i18n_gnu_include_input;
extern Fl_Input *i18n_gnu_conditional_input;
extern Fl_Input *i18n_gnu_function_input;
extern Fl_Input *i18n_gnu_static_function_input;
extern Fl_Group *i18n_posix_group;
extern Fl_Input *i18n_pos_include_input;
extern Fl_Input *i18n_pos_conditional_input;
extern Fl_Input *i18n_pos_file_input;
#include <FL/Fl_Int_Input.H>
extern Fl_Int_Input *i18n_pos_set_input;
Fl_Double_Window* make_settings_window();
extern Fl_Menu_Item menu_layout_choice[];
extern Fl_Menu_Item menu_w_layout_menu[];
#define w_layout_menu_rename (menu_w_layout_menu+0)
extern Fl_Menu_Item *w_layout_menu_storage[4];
#define w_layout_menu_load (menu_w_layout_menu+5)
#define w_layout_menu_save (menu_w_layout_menu+6)
#define w_layout_menu_delete (menu_w_layout_menu+7)
extern Fl_Menu_Item menu_w_settings_shell_menu[];
extern Fl_Menu_Item menu_Store[];
extern Fl_Menu_Item menu_Condition[];
extern Fl_Menu_Item menu_w_settings_shell_text_macros[];
extern Fl_Menu_Item menu_i18n_type_chooser[];
extern Fl_Double_Window *shell_run_window;
#include <FL/Fl_Simple_Terminal.H>
extern Fl_Simple_Terminal *shell_run_terminal;
#include <FL/Fl_Return_Button.H>
extern Fl_Return_Button *shell_run_button;
Fl_Double_Window* make_shell_window();
#endif
