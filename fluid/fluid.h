//
// FLUID main entry for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

#include "fluid_filename.h"
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Menu_Item.H>
#include "../src/Fl_String.H"

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

extern int reading_file;

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

extern Fl_String g_code_filename_arg;
extern Fl_String g_header_filename_arg;
extern Fl_String g_launch_path;

extern Fl_String g_autodoc_path;

// ---- project class declaration

/**
 Enumeration of available internationalization types.
 */
typedef enum {
  FD_I18N_NONE = 0, ///< No i18n, all strings are litearals
  FD_I18N_GNU,      ///< GNU gettext internationalization
  FD_I18N_POSIX     ///< Posix catgets internationalization
} Fd_I18n_Type;

/**
 Data and settings for a FLUID project file.
 */
class Fluid_Project {
public:
  Fluid_Project();
  ~Fluid_Project();
  void reset();
  void update_settings_dialog();

  Fl_String projectfile_path() const;
  Fl_String projectfile_name() const;
  Fl_String codefile_path() const;
  Fl_String codefile_name() const;
  Fl_String headerfile_path() const;
  Fl_String headerfile_name() const;
  Fl_String stringsfile_path() const;
  Fl_String stringsfile_name() const;
  Fl_String basename() const;

  /// One of the available internationalization types.
  Fd_I18n_Type i18n_type;
  /// Include file for GNU i18n, writes an #include statement into the source
  /// file. This is usually `<libintl.h>` or `"gettext.h"` for GNU gettext.
  Fl_String i18n_gnu_include;
  // Optional name of a macro for conditional i18n compilation.
  Fl_String i18n_gnu_conditional;
  /// For the gettext/intl.h options, this is the function that translates text
  /// at runtime. This is usually "gettext" or "_".
  Fl_String i18n_gnu_function;
  /// For the gettext/intl.h options, this is the function that marks the translation
  /// of text at initialisation time. This is usually "gettext_noop" or "N_".
  Fl_String i18n_gnu_static_function;

  /// Include file for Posix i18n, write a #include statement into the source
  /// file. This is usually `<nl_types.h>` for Posix catgets.
  Fl_String i18n_pos_include;
  // Optional name of a macro for conditional i18n compilation.
  Fl_String i18n_pos_conditional;
  /// Name of the nl_catd database
  Fl_String i18n_pos_file;
  /// Message set ID for the catalog.
  Fl_String i18n_pos_set;

  /// If set, generate code to include the header file form the c++ file
  int include_H_from_C;
  /// If set, handle keyboard shortcut Ctrl on macOS using Cmd instead
  int use_FL_COMMAND;
  /// Clear if UTF-8 characters in statics texts are written as escape sequences
  int utf8_in_src;
  /// If set, <FL/Fl.H> will not be included from the header code before anything else
  int avoid_early_includes;
  /// If set, command line overrides header file name in .fl file.
  int header_file_set;
  ///  If set, command line overrides source code file name in .fl file.
  int code_file_set;
  int write_mergeback_data;
  /// Hold the default extension for header files, or the entire filename if set via command line.
  Fl_String header_file_name;
  /// Hold the default extension for source code  files, or the entire filename if set via command line.
  Fl_String code_file_name;
};

extern Fluid_Project g_project;

// ---- public functions

extern bool new_project(bool user_must_confirm = true);
extern void enter_project_dir();
extern void leave_project_dir();
extern void set_filename(const char *c);
extern void set_modflag(int mf, int mfc=-1);

extern const Fl_String &get_tmpdir();

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

#endif // _FLUID_FLUID_H
