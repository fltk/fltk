//
// Fluid Application header for the Fast Light Tool Kit (FLTK).
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
#include "app/Snap_Action.h"
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
class Node;
class Fl_Choice;
class Fl_Button;
class Fl_Check_Button;
class Fl_Help_Dialog;

namespace fld {
namespace app {
class Layout_List;
}

/**
 Indicate the storage location for tools like layout suites and shell macros.
 \see class Fd_Shell_Command, class Layout_Suite
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
  std::string launch_path_;
  /// Path to store temporary files during app run.
  std::string tmpdir_path;
  /// True if the temporary file path was already created.
  bool tmpdir_create_called = false;
  // Generate a path to a directory for temporary data storage.
  void create_tmpdir();
  // Delete the temporary directory and all its contents.
  void delete_tmpdir();

public: // Member Variables
  /// Application wide preferences
  Fl_Preferences preferences;
  /// Project history.
  app::History history;
  /// Command line arguments
  app::Args args;
  /// List of available layouts 
  app::Layout_List layout_list;
  /// Set, if Fluid runs in batch mode, and no user interface is activated.
  int batch_mode { 0 };             // fluid + any code generators (-u, -c, -cs)

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
  /// Use external editor for editing Code_Node, saved in app preferences.
  int use_external_editor { 0 };
  /// Debugging help for external Code_Node editor.
  int debug_external_editor { 0 };
  /// Run this command to load an Code_Node into an external editor, save in app preferences.
  // TODO: make this into a std::string
  char external_editor_command[512] { };

  // TODO: make this into a class: app::GUI
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
  /// Offset in pixels when adding widgets from an .fl file.
  int pasteoffset { 0 };
  int ipasteoffset { 0 };
  /// FLUID-wide help dialog.
  Fl_Help_Dialog *help_dialog { nullptr };

public: // Methods
  // Create the Fluid application.
  Application();
  /// Destructor.
  ~Application() = default;
  // Launch the application.
  int run(int argc,char **argv);
  // Quit the application and clean up.
  void quit();
  /// Quick access to the current project. Make sure it stays synched to current_project_.
  Project &proj { *current_project_ };
  // Return the working directory path at application launch.
  const std::string &launch_path() const;
  // Return the path to a temporary directory for this instance of Fluid.
  const std::string &get_tmpdir();
  // Return the path and filename of a temporary file for cut or duplicated data.
  const char *cutfname(int which = 0);

  // Clear the current project and create a new, empty one.
  bool new_project(bool user_must_confirm = true);
  // Open a file chooser and load an exiting project file.
  bool open_project_file(const std::string &filename_arg);
  // Load a project from the give file name and path.
  bool merge_project_file(const std::string &filename_arg);
  // Save the current design to the file given by \c filename.
  void save_project_file(void *arg);
  // Reload the file set by \c filename, replacing the current design.
  void revert_project();
  // Open the template browser and load a new file from templates.
  bool new_project_from_template();
  // Open the dialog to allow the user to print the current window.
  void print_snapshots();
  // Generate the C++ source and header filenames and write those files.
  int write_code_files(bool dont_show_completion_dialog=false);

  // User chose to cut the currently selected widgets.
  void cut_selected();
  // User chose to copy the currently selected widgets.
  void copy_selected();
  // User chose to paste the widgets from the cut buffer.
  void paste_from_clipboard();
  // Duplicate the selected widgets.
  void duplicate_selected();
  // User chose to delete the currently selected widgets.
  void delete_selected();
  // Show the editor for the \c current Node.
  void edit_selected();
  // User wants to sort selected widgets by y coordinate.
  void sort_selected();
  // Show or hide the widget bin.
  void toggle_widget_bin();
  // Open a dialog to show the HTML help page form the FLTK documentation folder.
  void show_help(const char *name);
  // Open the "About" dialog.
  void about();

  // Build the main app window and create a few other dialogs.
  void make_main_window();
  // Open a native file chooser to allow choosing a project file for reading.
  std::string open_project_filechooser(const std::string &title);
  // Give the user the opportunity to save a project before clearing it.
  bool confirm_project_clear();
  // Ensure that text widgets in the widget panel propagates apply current changes.
  void flush_text_widgets();
  // Position the given window window based on entries in the app preferences.
  char position_window(Fl_Window *w, const char *prefsName, int Visible, int X, int Y, int W=0, int H=0);
  // Save the position and visibility state of a window to the app preferences.
  void save_position(Fl_Window *w, const char *prefsName);
  // Change the app's and hence preview the design's scheme.
  void set_scheme(const char *new_scheme);
  // Read Fluid's scheme preferences and set the app's scheme.
  void init_scheme();

#ifdef __APPLE__
  static void apple_open_cb(const char *c);
#endif // __APPLE__
};

} // namespace fld

extern fld::Application Fluid;


#endif // FLUID_FLUID_H

