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

#include "Project.h"
#include "app/args.h"
#include "app/history.h"
#include "tools/filename.h"

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
class Fl_Help_Dialog;

namespace fld {

/**
 Indicate the storage location for tools like layout suites and shell macros.
 \see class Fd_Shell_Command, class Fd_Layout_Suite
 */
enum class Tool_Store {
  INTERNAL,  ///< stored inside FLUID app
  USER,      ///< suite is stored in the user wide FLUID settings
  PROJECT,   ///< suite is stored within the current .fl project file
  FILE       ///< store suite in external file
};


class Project;

class Application {
  /// Currently selected project.
  Project *current_project_ = new Project();
  /// Working directory at application launch.
  fld::filename launch_path_;
  /// Path to store temporary files during app run.
  std::string tmpdir_path;
  /// True if the temporary file path was already created.
  bool tmpdir_create_called = false;
  // Generate a path to a directory for temporary data storage.
  void create_tmpdir();
  // Delete the temporary directory and all its contents.
  void delete_tmpdir();

public:
  // Create the Fluid application.
  Application();
  /// Destructor.
  ~Application() = default;
  // Launch the application.
  int run(int argc,char **argv);
  // Quit the application and clean up.
  void quit();
  /// Application wide preferences
  Fl_Preferences preferences;
  /// Project history.
  app::History history;
  /// Command line arguments
  app::Args args;
  /// Set, if Fluid runs in batch mode, and no user interface is activated.
  int batch_mode { 0 };             // fluid + any code generators (-u, -c, -cs)
  /// Quick access to the current project. Make sure it stays synched to current_project_.
  Project &proj { *current_project_ };
  // Return the working directory path at application launch.
  const fld::filename &launch_path() const;
  // Return the path to a temporary directory for this instance of Fluid.
  const std::string &get_tmpdir();
  // Return the path and filename of a temporary file for cut or duplicated data.
  const char *cutfname(int which = 0);




private: // TODO: verify stuff below

#ifdef __APPLE__
  static void apple_open_cb(const char *c);
#endif // __APPLE__



public:

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

  Fl_Window *main_window { nullptr };
  static Fl_Menu_Item main_menu[];
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
  int ipasteoffset { 0 };

  void flush_text_widgets(); // TODO: should be in a GUI class?
  char position_window(Fl_Window *w, const char *prefsName, int Visible, int X, int Y, int W=0, int H=0);
  void save_position(Fl_Window *w, const char *prefsName);
  void set_scheme(const char *new_scheme);
  void init_scheme();
  void toggle_widget_bin();

  /// FLUID-wide help dialog.
  Fl_Help_Dialog *help_dialog { nullptr };


};

} // namespace fld

extern fld::Application Fluid;


#endif // FLUID_FLUID_H

