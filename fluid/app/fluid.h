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

#ifndef FLUID_APP_FLUID_H
#define FLUID_APP_FLUID_H

#include "tools/filename.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/filename.H>

#include <string>

#define BROWSERWIDTH 300
#define BROWSERHEIGHT 500
#define WINWIDTH 300
#define MENUHEIGHT 25
#define WINHEIGHT (BROWSERHEIGHT+MENUHEIGHT)

// ---- types

class Fl_Double_Window;
class Fl_Window;
class Fl_Menu_Bar;
class Fl_Type;
class Fl_Choice;
class Fl_Button;
class Fl_Check_Button;

/**
 Indicate the storage location for tools like layout suites and shell macros.
 \see class Fd_Shell_Command, class Fd_Layout_Suite
 */
typedef enum {
  FD_STORE_INTERNAL,  ///< stored inside FLUID app
  FD_STORE_USER,      ///< suite is stored in the user wide FLUID settings
  FD_STORE_PROJECT,   ///< suite is stored within the current .fl project file
  FD_STORE_FILE       ///< store suite in external file
} Fd_Tool_Store;

// ---- global variables

extern Fl_Preferences fluid_prefs;
extern Fl_Menu_Item Main_Menu[];
extern Fl_Menu_Bar *main_menubar;
extern Fl_Window *main_window;

extern int show_guides;
extern int show_restricted;
extern int show_ghosted_outline;
extern int show_comments;

extern int G_use_external_editor;
extern int G_debug;
extern char G_external_editor_command[512];

// File history info...
extern char absolute_history[10][FL_PATH_MAX];
extern char relative_history[10][FL_PATH_MAX];
extern void load_history();
extern void update_history(const char *);

extern Fl_Menu_Item *save_item;
extern Fl_Menu_Item *history_item;
extern Fl_Menu_Item *widgetbin_item;
extern Fl_Menu_Item *codeview_item;
extern Fl_Menu_Item *overlay_item;
extern Fl_Button *overlay_button;
extern Fl_Menu_Item *guides_item;
extern Fl_Menu_Item *restricted_item;
extern Fl_Check_Button *guides_button;

extern int modflag;

extern int update_file;            // fluid -u
extern int compile_file;           // fluid -c
extern int compile_strings;        // fluic -cs
extern int batch_mode;

extern int pasteoffset;

extern std::string g_code_filename_arg;
extern std::string g_header_filename_arg;
extern std::string g_launch_path;

extern std::string g_autodoc_path;

// ---- public functions

extern int fluid_main(int argc,char **argv);

extern bool new_project(bool user_must_confirm = true);
extern void enter_project_dir();
extern void leave_project_dir();
extern void set_filename(const char *c);
extern void set_modflag(int mf, int mfc=-1);

extern const std::string &get_tmpdir();
extern std::string end_with_slash(const std::string &str);

// ---- public callback functions

extern void save_cb(Fl_Widget *, void *v);
extern void save_template_cb(Fl_Widget *, void *);
extern void revert_cb(Fl_Widget *,void *);
extern void exit_cb(Fl_Widget *,void *);

extern int write_code_files(bool dont_show_completion_dialog=false);
extern void write_strings_cb(Fl_Widget *, void *);
extern void align_widget_cb(Fl_Widget *, long);
extern void toggle_widgetbin_cb(Fl_Widget *, void *);

extern char position_window(Fl_Window *w, const char *prefsName, int Visible, int X, int Y, int W=0, int H=0);

inline int fd_min(int a, int b) { return (a < b ? a : b); }
inline int fd_max(int a, int b) { return (a > b ? a : b); }
inline int fd_min(int a, int b, int c) { return fd_min(a, fd_min(b, c)); }

#endif // FLUID_APP_FLUID_H

