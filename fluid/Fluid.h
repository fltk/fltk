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

constexpr int BROWSERWIDTH = 300;
constexpr int BROWSERHEIGHT = 500;
constexpr int WINWIDTH = 300;
constexpr int MENUHEIGHT = 25;
constexpr int WINHEIGHT = (BROWSERHEIGHT+MENUHEIGHT);

// ---- types

class Fl_Double_Window;
class Fl_Window;
class Fl_Menu_Bar;
class Fl_Type;
class Fl_Choice;
class Fl_Button;
class Fl_Check_Button;


namespace fld {

/**
 Indicate the storage location for tools like layout suites and shell macros.
 \see class Fd_Shell_Command, class Fd_Layout_Suite
 */
enum class ToolStore {
  INTERNAL,  ///< stored inside FLUID app
  USER,      ///< suite is stored in the user wide FLUID settings
  PROJECT,   ///< suite is stored within the current .fl project file
  FILE       ///< store suite in external file
};


class Project;

class Application {
  Project current_project_ { };
  /// path to store temporary files during app run
  std::string tmpdir_path;
  /// true if the temporary file path was already created
  bool tmpdir_create_called = false;

#ifdef __APPLE__
  static void apple_open_cb(const char *c);
#endif // __APPLE__

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
  void quit();

  std::string open_project_filechooser(const std::string &title);
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
  bool confirm_project_clear();

  void create_tmpdir();
  void delete_tmpdir();
  const std::string &get_tmpdir();
  char* cutfname(int which = 0);

  Fl_Window *main_window { nullptr };
  Fl_Menu_Item *Main_Menu { nullptr };
  Fl_Menu_Bar *main_menubar { nullptr };
  Fl_Menu_Item *save_item { nullptr };
  Fl_Menu_Item *history_item { nullptr };
  Fl_Menu_Item *widgetbin_item { nullptr };
  Fl_Menu_Item *codeview_item { nullptr };
  Fl_Menu_Item *overlay_item { nullptr };
  Fl_Button *overlay_button { nullptr };
  Fl_Menu_Item *guides_item { nullptr };
  Fl_Menu_Item *restricted_item { nullptr };
  void make_main_window();
  /// Offset in pixels when adding widgets from an .fl file.
  int pasteoffset { 0 };

  void flush_text_widgets(); // TODO: should be in a GUI class?
  char position_window(Fl_Window *w, const char *prefsName, int Visible, int X, int Y, int W=0, int H=0);
  void save_position(Fl_Window *w, const char *prefsName);
  void set_scheme(const char *new_scheme);
  void init_scheme();
  void toggle_widget_bin();

};

} // namespce fld

extern fld::Application Fluid;


#endif // FLUID_FLUID_H

