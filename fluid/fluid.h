//
// FLUID main entry for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

#ifndef _FLUID_FLUID_H
#define _FLUID_FLUID_H

#include <FL/filename.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_String.H>

#define BROWSERWIDTH 300
#define BROWSERHEIGHT 500
#define WINWIDTH 300
#define MENUHEIGHT 25
#define WINHEIGHT (BROWSERHEIGHT+MENUHEIGHT)

class Fl_Double_Window;
class Fl_Window;
class Fl_Menu_Bar;
class Fl_Type;
class Fl_Choice;
class Fl_Button;

extern int force_parent;

extern Fl_Preferences fluid_prefs;
extern Fl_Menu_Item Main_Menu[];
extern Fl_Menu_Bar *main_menubar;
extern Fl_Window *main_window;

extern int gridx;
extern int gridy;
extern int snap;
extern int show_guides;
extern int show_comments;

extern int G_use_external_editor;
extern int G_debug;
extern char G_external_editor_command[512];

extern int reading_file;

// File history info...
extern char absolute_history[10][FL_PATH_MAX];
extern char relative_history[10][FL_PATH_MAX];
extern void load_history();
extern void update_history(const char *);

extern Fl_Menu_Item *save_item;
extern Fl_Menu_Item *history_item;
extern Fl_Menu_Item *widgetbin_item;
extern Fl_Menu_Item *sourceview_item;
extern Fl_Menu_Item *overlay_item;

extern int modflag;

extern void enter_project_dir();
extern void leave_project_dir();

extern int update_file;            // fluid -u
extern int compile_file;           // fluid -c
extern int compile_strings;        // fluic -cs
extern int batch_mode;

extern int pasteoffset;

// ---- string handling

class Fd_String : public Fl_String
{
public:
  Fd_String() : Fl_String("") { }
  Fd_String(const char* s) : Fl_String(s) { }
  int empty() { return size()==0; }
  void operator=(const char* s) { value(s); }
  operator const char* () const { return value(); }
};

// ---- project settings

class Project {
public:
  Project();
  ~Project();
  void reset();

  int i18n_type;
  Fd_String i18n_include;
  Fd_String i18n_conditional;
  Fd_String i18n_function;
  Fd_String i18n_static_function;
  Fd_String i18n_file;
  Fd_String i18n_set;
  Fd_String i18n_program;
  int include_H_from_C;
  int use_FL_COMMAND;
  int utf8_in_src;
  int avoid_early_includes;
  int header_file_set;
  int code_file_set;
  Fd_String header_file_name;
  Fd_String code_file_name;
};

extern Project P;

extern Fd_String g_code_filename_arg;
extern Fd_String g_header_filename_arg;

// ---- public functions

extern void set_filename(const char *c);
extern void set_modflag(int mf, int mfc=-1);

// ---- public callback functions

extern void save_cb(Fl_Widget *, void *v);
extern void save_template_cb(Fl_Widget *, void *);
extern void revert_cb(Fl_Widget *,void *);
extern void exit_cb(Fl_Widget *,void *);

#ifdef __APPLE__
extern void apple_open_cb(const char *c);
#endif // __APPLE__

extern void open_cb(Fl_Widget *, void *v);
extern void open_history_cb(Fl_Widget *, void *v);
extern void new_cb(Fl_Widget *, void *v);
extern void new_from_template_cb(Fl_Widget *w, void *v);

extern int write_code_files();
extern void write_strings_cb(Fl_Widget *, void *);
extern void align_widget_cb(Fl_Widget *, long);
extern void widget_size_cb(Fl_Widget *, long);
extern void toggle_widgetbin_cb(Fl_Widget *, void *);

#endif // _FLUID_FLUID_H
