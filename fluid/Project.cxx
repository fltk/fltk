//
// Fluid Project code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2026 by Bill Spitzak and others.
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

#include <errno.h>      // strerror(errno)
#include "Project.h"

#include "message.h"
#include "io/file_chooser.h"
#include "io/Project_Reader.h"
#include "io/Project_Writer.h"
#include "io/String_Writer.h"
#include "nodes/Node.h"
#include "nodes/Widget_Node.h"
#include "panels/settings_panel.h"
#include "panels/codeview_panel.h"
#include "widgets/Node_Browser.h"

using namespace fluid;

// ---- project settings

/**
 Initialize a new project.
 */
Project::Project() {
}

/**
 Clear all project resources.
 Not implemented.
 */
Project::~Project() {
}

/**
 Reset all project setting to create a new empty project.
 */
void Project::reset() {
  // Remove all nodes in the project tree
  tree.delete_all_nodes();

  // reset the setting for the external shell command
  if (g_shell_config) {
    g_shell_config->clear(fluid::Tool_Store::PROJECT);
    g_shell_config->rebuild_shell_menu();
    g_shell_config->update_settings_dialog();
  }

  // Reset Layout List
  Fluid.layout_list.remove_all(fluid::Tool_Store::PROJECT);
  Fluid.layout_list.current_suite(0);
  Fluid.layout_list.current_preset(0);
  Fluid.layout_list.update_dialogs();

  // Reset I18N Tab
  i18n.reset();

  include_H_from_C = 1;
  use_FL_COMMAND = 0;
  utf8_in_src = 0;
  avoid_early_includes = 0;
  header_file_set = 0;
  code_file_set = 0;
  header_file_name = ".h";
  code_file_name = ".cxx";
  include_guard = "";
  write_mergeback_data = 0;
}

/**
 Tell the project and i18n tab of the settings dialog to refresh themselves.
 */
void Project::update_settings_dialog() {
  if (settings_window) {
    w_settings_project_tab->do_callback(w_settings_project_tab, LOAD);
    w_settings_i18n_tab->do_callback(w_settings_i18n_tab, LOAD);
  }
}

/**
 Get the absolute path of the project file, for example `/Users/matt/dev/`.
 \return the path ending in '/'
 */
std::string Project::projectfile_path() const {
  if (proj_filename.empty()) return std::string{};
  return end_with_slash(fl_filename_absolute_str(fl_filename_path_str(proj_filename), Fluid.launch_path()));
}

/**
 Get the project file name including extension, for example `test.fl`.
 \return the file name without path
 */
std::string Project::projectfile_name() const {
  if (proj_filename.empty()) return std::string{};
  return fl_filename_name_str(proj_filename);
}

/**
 Get the absolute path of the generated C++ code file, for example `/Users/matt/dev/src/`.
 \return the path ending in '/'
 */
std::string Project::codefile_path() const {
  std::string path = fl_filename_path_str(code_file_name);
  if (Fluid.batch_mode)
    return end_with_slash(fl_filename_absolute_str(path, Fluid.launch_path()));
  else
    return end_with_slash(fl_filename_absolute_str(path, projectfile_path()));
}

/**
 Get the generated C++ code file name including extension, for example `test.cxx`.
 \return the file name without path
 */
std::string Project::codefile_name() const {
  std::string name = fl_filename_name_str(code_file_name);
  if (name.empty()) {
    if (proj_filename.empty()) return std::string{};
    return fl_filename_setext_str(fl_filename_name_str(proj_filename), ".cxx");
  } else if (name[0] == '.') {
    if (proj_filename.empty()) return std::string{};
    return fl_filename_setext_str(fl_filename_name_str(proj_filename), code_file_name);
  } else {
    return name;
  }
}

/**
 Get the absolute path of the generated C++ header file, for example `/Users/matt/dev/src/`.
 \return the path ending in '/'
 */
std::string Project::headerfile_path() const {
  std::string path = fl_filename_path_str(header_file_name);
  if (Fluid.batch_mode)
    return end_with_slash(fl_filename_absolute_str(path, Fluid.launch_path()));
  else
    return end_with_slash(fl_filename_absolute_str(path, projectfile_path()));
}

/**
 Get the generated C++ header file name including extension, for example `test.cxx`.
 \return the file name without path
 */
std::string Project::headerfile_name() const {
  std::string name = fl_filename_name_str(header_file_name);
  if (name.empty()) {
    if (proj_filename.empty()) return std::string{};
    return fl_filename_setext_str(fl_filename_name_str(proj_filename), ".h");
  } else if (name[0] == '.') {
    if (proj_filename.empty()) return std::string{};
    return fl_filename_setext_str(fl_filename_name_str(proj_filename), header_file_name);
  } else {
    return name;
  }
}

/**
 Get the absolute path of the generated i18n strings file, for example `/Users/matt/dev/`.
 Although it may be more useful to put the text file into the same directory
 with the source and header file, historically, the text is always saved with
 the project file in interactive mode, and in the FLUID launch directory in
 batch mode.
 \return the path ending in '/'
 */
std::string Project::stringsfile_path() const {
  if (Fluid.batch_mode)
    return Fluid.launch_path();
  else
    return projectfile_path();
}

/**
 Get the generated i18n text file name including extension, for example `test.po`.
 \return the file name without path
 */
std::string Project::stringsfile_name() const {
  if (proj_filename.empty()) return std::string{};
  switch (i18n.type) {
    default: return fl_filename_setext_str(fl_filename_name_str(proj_filename), ".txt");
    case fluid::I18n_Type::GNU: return fl_filename_setext_str(fl_filename_name_str(proj_filename), ".po");
    case fluid::I18n_Type::POSIX: return fl_filename_setext_str(fl_filename_name_str(proj_filename), ".msg");
  }
}

/**
 Get the name of the project file without the filename extension.
 \return the file name without path or extension
 */
std::string Project::basename() const {
  if (proj_filename.empty()) return std::string{};
  return fl_filename_setext_str(fl_filename_name_str(proj_filename), "");
}


/**
 Change the current working directory to the .fl project directory.

 Every call to enter_project_dir() must have a corresponding leave_project_dir()
 call. Enter and leave calls can be nested.

 The first call to enter_project_dir() remembers the original directory, usually
 the launch directory of the application. Nested calls will increment a nesting
 counter. When the nesting counter is back to 0, leave_project_dir() will return
 to the original directory.

 The global variable 'filename' must be set to the current project file with
 absolute or relative path information.

 \see leave_project_dir(), pwd, in_project_dir
 */
void Project::enter_project_dir() {
  if (in_project_dir<0) {
    fprintf(stderr, "** Fluid internal error: enter_project_dir() calls unmatched\n");
    return;
  }
  in_project_dir++;
  // check if we are already in the project dir and do nothing if so
  if (in_project_dir>1) return;
  // check if there is an active project, and do nothing if there is none
  if (proj_filename.empty()) {
    fprintf(stderr, "** Fluid internal error: enter_project_dir() no filename set\n");
    return;
  }
  // store the current working directory for later
  app_work_dir = fl_getcwd_str();
  // set the current directory to the path of our .fl file
  std::string project_path = fl_filename_path_str(fl_filename_absolute_str(proj_filename));
  if (fl_chdir(project_path.c_str()) == -1) {
    fprintf(stderr, "** Fluid internal error: enter_project_dir() can't chdir to %s: %s\n",
            project_path.c_str(), strerror(errno));
    return;
  }
  //fprintf(stderr, "chdir from %s to %s\n", app_work_dir.c_str(), fl_getcwd().c_str());
}

/**
 Change the current working directory to the previous directory.
 \see enter_project_dir(), pwd, in_project_dir
 */
void Project::leave_project_dir() {
  if (in_project_dir == 0) {
    fprintf(stderr, "** Fluid internal error: leave_project_dir() calls unmatched\n");
    return;
  }
  in_project_dir--;
  // still nested, stay in the project directory
  if (in_project_dir > 0) return;
  // no longer nested, return to the original, usually the application working directory
  if (fl_chdir(app_work_dir.c_str()) < 0) {
    fprintf(stderr, "** Fluid internal error: leave_project_dir() can't chdir back to %s : %s\n",
            app_work_dir.c_str(), strerror(errno));
  }
}

/**
 Alias for set_filename("").
 Instead, change proj_filename into a std::string and add clear_filename().
 */
void Project::set_filename(std::nullptr_t) {
  set_filename(std::string());
}

/**
 Set the filename of the current .fl design.
 \param[in] c the new absolute filename and path
 */
void Project::set_filename(const std::string &c) {
  proj_filename = c;
  if (!proj_filename.empty() && !Fluid.batch_mode)
    Fluid.history.update(proj_filename);
  set_modflag(modflag);
}

/**
 Save the current design to the file given by \c proj_filename.

 If no filename is set, or \c option requests one, open a filechooser first.

 \param[in] option SaveOption::ASK_FOR_FILENAME always prompts for a filename
    before saving; SaveOption::SAVE_COPY prompts for a filename and writes the
    file without changing the project's filename or clearing the modified
    flag ("save a copy...")
 */
void Project::save(SaveOption option) {
  Fluid.flush_text_widgets();

  std::string c = proj_filename;
  std::string new_filename;
  if (option == SaveOption::ASK_FOR_FILENAME || option == SaveOption::SAVE_COPY || c.empty()) {
    new_filename = fluid::io::filechooser(
      fluid::io::FileChooserType::SAVE_FILE,
      fluid::io::FileChooserPath::ABSOLUTE_PATH,
      "Save Project File As",
      "Can't create project file:\n%s.",
      c.c_str(),
      Fluid.history.latest_project_path(),
      "Fluid Project Files\t*.fl"
    );
    if (new_filename.empty()) return;
    c = new_filename;

#if 0 // filechooser is already doing this check, so we don't need to do it again here
    if (!fl_access(c, 0)) {
      std::string basename = fl_filename_name_str(c);
      if (fluid_choice("The file \"%s\" already exists.\n"
                    "Do you want to replace it?", "Cancel",
                    "Replace", nullptr, basename.c_str()) == 0) return;
    }
#endif
    if (option != SaveOption::SAVE_COPY) set_filename(c);
  }
  if (!fluid::io::write_file(*this, c.c_str())) {
    fluid::alert("Fluid ERROR", "Can't save project file '" + c + "':\n" + strerror(errno));
    return;
  }

  if (option != SaveOption::SAVE_COPY) {
    set_modflag(0, 1);
    undo.save_ = undo.current_;
  }
}

/**
 Reload the file using the current filename, replacing the current project.
 If the project is marked modified, a dialog will ask for confirmation.
 */
void Project::revert() {
  if (modflag) {
    if (!fluid_choice("This user interface has been changed. Really revert?",
                   "Cancel", "Revert", nullptr)) return;
  }
  undo.suspend();
  if (!fluid::io::read_file(*this, proj_filename.c_str(), 0)) {
    undo.resume();
    widget_browser->rebuild();
    update_settings_dialog();
    fluid::alert("Fluid ERROR", "Can't load project file '" + proj_filename + "':\n" + strerror(errno));
    return;
  }
  widget_browser->rebuild();
  undo.resume();
  set_modflag(0, 0);
  undo.clear();
  update_settings_dialog();
}

/**
 Write the strings that are used in i18n.
 */
void Project::write_strings() {
  Fluid.flush_text_widgets();
  if (proj_filename.empty()) {
    save();
    if (proj_filename.empty()) return;
  }
  std::string filename = stringsfile_path() + stringsfile_name();
  int x = fluid::io::write_strings(*this, filename);
  if (x) {
    fluid_message("Can't write %s: %s", filename.c_str(), strerror(errno));
  } else if (completion_button->value() && !Fluid.batch_mode) {
    fluid_message("Wrote %s", stringsfile_name().c_str());
  }
}


/**
 Set the "modified" flag and update the title of the main window.

 The first argument sets the modification state of the current design against
 the corresponding .fl design file. Any change to the widget tree will mark
 the design 'modified'. Saving the design will mark it clean.

 The second argument is optional and set the modification state of the current
 design against the source code and header file. Any change to the tree,
 including saving the tree, will mark the code 'outdated'. Generating source
 code and header files will clear this flag until the next modification.

 \param[in] mf 0 to clear the modflag, 1 to mark the design "modified", -1 to
 ignore this parameter
 \param[in] mfc default -1 to let \c mf control \c modflag_c, 0 to mark the
 code files current, 1 to mark it out of date. -2 to ignore changes to mf.
 */
void Project::set_modflag(int mf, int mfc) {
  const char *code_ext = nullptr;
  char new_title[FL_PATH_MAX];

  // Update the modflag_c to the worst possible condition. We could be a bit
  // more graceful and compare modification times of the files, but C++ has
  // no API for that until C++17.
  if (mf!=-1) {
    modflag = mf;
    if (mfc==-1 && mf==1)
      mfc = mf;
  }
  if (mfc>=0) {
    modflag_c = mfc;
  }

  if (Fluid.main_window) {
    std::string basename;
    if (proj_filename.empty()) basename = "Untitled.fl";
    else basename = fl_filename_name_str(proj_filename);
    code_ext = fl_filename_ext(code_file_name.c_str());
    char mod_star = modflag ? '*' : ' ';
    char mod_c_star = modflag_c ? '*' : ' ';
    snprintf(new_title, sizeof(new_title), "%s%c  %s%c",
             basename.c_str(), mod_star, code_ext, mod_c_star);
    const char *old_title = Fluid.main_window->label();
    // only update the title if it actually changed
    if (!old_title || strcmp(old_title, new_title))
      Fluid.main_window->copy_label(new_title);
  }
  // if the UI was modified in any way, update the Code View panel
  if (codeview_panel && codeview_panel->visible() && cv_autorefresh->value())
    codeview_defer_update();
}

/**
 Give the user the opportunity to save a project before clearing it.

 If the project has unsaved changes, this function pops up a dialog, that
 allows the user to save the project, continue without saving the project,
 or to cancel the operation.

 If the user chooses to save, and no filename was set, a file dialog allows
 the user to pick a name and location, or to cancel the operation.

 \return false if the user aborted the operation and the calling function
 should abort as well
 */
bool Project::confirm_clear() {
  if (modflag == 0) return true;
  switch (fluid_choice("This project has unsaved changes. Do you want to save\n"
                    "the project file before proceeding?",
                    "Cancel", "Save", "Don't Save"))
  {
    case 0 : /* Cancel */
      return false;
    case 1 : /* Save */
      save();
      if (modflag) return false;  // user canceled the "Save As" dialog
  }
  return true;
}

/**
 Load a project from the give file name and path.

 The project file is inserted at the currently selected type.

 If no filename is given, FLUID will open a file chooser dialog.

 \param[in] filename_arg path and name of the new project file
 \return false if the operation failed
 */
bool Project::load_or_merge(const std::string &filename_arg) {
  bool is_a_merge = (!tree.empty());
  std::string title = is_a_merge ? "Merge Project File" : "Load Project File";

  // ask for a filename if none was given
  std::string new_filename = filename_arg;
  if (new_filename.empty()) {
    new_filename = fluid::io::filechooser(
      fluid::io::FileChooserType::LOAD_FILE,
      fluid::io::FileChooserPath::ABSOLUTE_PATH,
      title,
      "Can't open project file:\n%s.",
      Fluid.history.latest_project_path(),
      Fluid.launch_path(),
      "Fluid Project Files\t*.f[ld]"
    );
    if (new_filename.empty()) {
      return false;
    }
  }

  const char *c = new_filename.c_str();
  std::string oldfilename = proj_filename;
  proj_filename.clear();
  set_filename(c);
  if (is_a_merge)
    undo.checkpoint();
  undo.suspend();
  if (!fluid::io::read_file(*this, c, is_a_merge)) {
    undo.resume();
    widget_browser->rebuild();
    update_settings_dialog();
    fluid_message("Can't read %s: %s", c, strerror(errno));
    proj_filename.clear();
    proj_filename = oldfilename;
    if (Fluid.main_window)
      set_modflag(modflag);
    return false;
  }
  undo.resume();
  widget_browser->rebuild();
  if (is_a_merge) {
    // Inserting a file; restore the original filename...
    set_filename(oldfilename);
    set_modflag(1);
  } else {
    // Loaded a file; free the old filename...
    set_modflag(0, 0);
    undo.clear();
  }
  update_settings_dialog();
  return true;
}

