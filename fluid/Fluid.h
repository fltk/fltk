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

#ifndef FLUID_FLUID_H
#define FLUID_FLUID_H

#include "tools/filename.h"
#include "Project.h"

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
// TODO: make this into a class
typedef enum {
  FD_STORE_INTERNAL,  ///< stored inside FLUID app
  FD_STORE_USER,      ///< suite is stored in the user wide FLUID settings
  FD_STORE_PROJECT,   ///< suite is stored within the current .fl project file
  FD_STORE_FILE       ///< store suite in external file
} Fd_Tool_Store;

namespace fld {

class Project;

class Application {
  Project current_project_ { };
  /// path to store temporary files during app run
  std::string tmpdir_path;
  /// true if the temporary file path was already created
  bool tmpdir_create_called = false;

public:
  /// Create the main application class
  Application();
  ~Application() = default;
  /// Quick access to the current project
  Project &proj { current_project_ };
  /// Application wide preferences
  Fl_Preferences preferences;

  // TODO: make this into a class: app::Settings
  /// Show guides in the design window when positioning widgets, saved in app preferences.
  int show_guides { 1 };
  /// Show areas of restricted use in overlay plane.
  /// Restricted areas are widget that overlap each other, widgets that are outside
  /// of their parent's bounds (except children of Scroll groups), and areas
  /// within an Fl_Tile that are not covered by children.
  int show_restricted { 1 };
  /// Show a ghosted outline for groups that have very little contrast.
  /// This makes groups with NO_BOX or FLAT_BOX better editable.
  int show_ghosted_outline { 1 };
  /// Show widget comments in the browser, saved in app preferences.
  int show_comments { 1 };

  // TODO: make this into a class: app::External_Editor
  /// Use external editor for editing Fl_Code_Type, saved in app preferences.
  int use_external_editor { 0 };
  /// Debugging help for external Fl_Code_Type editor.
  int debug_external_editor { 0 };
  /// Run this command to load an Fl_Code_Type into an external editor, save in app preferences.
  // TODO: make this into a std::string
  char external_editor_command[512] { };

  // File history info...
  // TODO: make this into a class: app::Project_History
  /// Stores the absolute filename of the last 10 design files, saved in app preferences.
  char project_history_abspath[10][FL_PATH_MAX] { };
  /// This list of filenames is computed from \c Fluid.project_history_abspath and displayed in the main menu.
  char project_history_relpath[10][FL_PATH_MAX] { };
  void load_project_history();
  void update_project_history(const char *);

  // TODO: make this into a class for command line args: app::Args 
  /// Set, if Fluid was started with the command line argument -u
  int update_file { 0 };            // fluid -u
  /// Set, if Fluid was started with the command line argument -c
  int compile_file { 0 };           // fluid -c
  /// Set, if Fluid was started with the command line argument -cs
  int compile_strings { 0 };        // fluic -cs
  /// Set, if Fluid runs in batch mode, and no user interface is activated.
  int batch_mode { 0 };
  /// command line arguments that overrides the generate code file extension or name
  std::string code_filename_arg;
  /// command line arguments that overrides the generate header file extension or name
  std::string header_filename_arg;
  /// if set, generate images for automatic documentation in this directory
  std::string autodoc_path;
  static int arg_cb(int argc, char** argv, int& i);
  int arg(int argc, char** argv, int& i);

  /// current directory path at application launch
  std::string launch_path;

  int run(int argc,char **argv);

  bool new_project(bool user_must_confirm = true);
  bool open_project_file(const std::string &filename_arg);
  bool merge_project_file(const std::string &filename_arg);
  void save_project_file(void *arg);
  void revert_project();
  bool new_project_from_template();
  void print_snapshots();
  int write_code_files(bool dont_show_completion_dialog=false);
  void cut_selected();
  void copy_selected();
  void paste_from_clipboard();
  void duplicate_selected();
  void delete_selected();
  void edit_selected();
  void sort_selected();
  void show_help(const char *name);
  void about();

  void create_tmpdir();
  void delete_tmpdir();
  const std::string &get_tmpdir();


};

} // namespce fld

extern fld::Application Fluid;

// ---- global variables

extern Fl_Menu_Item Main_Menu[];
extern Fl_Menu_Bar *main_menubar;
extern Fl_Window *main_window;
extern Fl_Menu_Item *save_item;
extern Fl_Menu_Item *history_item;
extern Fl_Menu_Item *widgetbin_item;
extern Fl_Menu_Item *codeview_item;
extern Fl_Menu_Item *overlay_item;
extern Fl_Button *overlay_button;
extern Fl_Menu_Item *guides_item;
extern Fl_Menu_Item *restricted_item;
extern Fl_Check_Button *guides_button;

extern int modflag; // TODO: move to Project class
extern int pasteoffset;

// ---- public functions

extern int fluid_main(int argc,char **argv);

extern void set_filename(const char *c);
extern void set_modflag(int mf, int mfc=-1);

// ---- public callback functions

extern void exit_cb(Fl_Widget *,void *);

extern void write_strings_cb(Fl_Widget *, void *);
extern void align_widget_cb(Fl_Widget *, long);
extern void toggle_widgetbin_cb(Fl_Widget *, void *);

extern char position_window(Fl_Window *w, const char *prefsName, int Visible, int X, int Y, int W=0, int H=0);

inline int fd_min(int a, int b) { return (a < b ? a : b); }
inline int fd_max(int a, int b) { return (a > b ? a : b); }
inline int fd_min(int a, int b, int c) { return fd_min(a, fd_min(b, c)); }

#endif // FLUID_FLUID_H

