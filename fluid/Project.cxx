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

#include "Project.h"

#include "nodes/Fl_Type.h"
#include "panels/settings_panel.h"

using namespace fld;

// ---- project settings

/**
 Initialize a new project.
 */
Project::Project() :
i18n_type(FD_I18N_NONE),
include_H_from_C(1),
use_FL_COMMAND(0),
utf8_in_src(0),
avoid_early_includes(0),
header_file_set(0),
code_file_set(0),
write_mergeback_data(0),
header_file_name(".h"),
code_file_name(".cxx")
{ }

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
  ::delete_all();
  i18n_type = FD_I18N_NONE;

  i18n_gnu_include = "<libintl.h>";
  i18n_gnu_conditional = "";
  i18n_gnu_function = "gettext";
  i18n_gnu_static_function = "gettext_noop";

  i18n_pos_include = "<nl_types.h>";
  i18n_pos_conditional = "";
  i18n_pos_file = "";
  i18n_pos_set = "1";

  include_H_from_C = 1;
  use_FL_COMMAND = 0;
  utf8_in_src = 0;
  avoid_early_includes = 0;
  header_file_set = 0;
  code_file_set = 0;
  header_file_name = ".h";
  code_file_name = ".cxx";
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
  return end_with_slash(fl_filename_absolute_str(fl_filename_path_str(proj_filename), Fluid.launch_path));
}

/**
 Get the project file name including extension, for example `test.fl`.
 \return the file name without path
 */
std::string Project::projectfile_name() const {
  return fl_filename_name(proj_filename);
}

/**
 Get the absolute path of the generated C++ code file, for example `/Users/matt/dev/src/`.
 \return the path ending in '/'
 */
std::string Project::codefile_path() const {
  std::string path = fl_filename_path_str(code_file_name);
  if (Fluid.batch_mode)
    return end_with_slash(fl_filename_absolute_str(path, Fluid.launch_path));
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
    return fl_filename_setext_str(fl_filename_name(proj_filename), ".cxx");
  } else if (name[0] == '.') {
    return fl_filename_setext_str(fl_filename_name(proj_filename), code_file_name);
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
    return end_with_slash(fl_filename_absolute_str(path, Fluid.launch_path));
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
    return fl_filename_setext_str(fl_filename_name_str(proj_filename), ".h");
  } else if (name[0] == '.') {
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
    return Fluid.launch_path;
  else
    return projectfile_path();
}

/**
 Get the generated i18n text file name including extension, for example `test.po`.
 \return the file name without path
 */
std::string Project::stringsfile_name() const {
  switch (i18n_type) {
    default: return fl_filename_setext_str(fl_filename_name(proj_filename), ".txt");
    case FD_I18N_GNU: return fl_filename_setext_str(fl_filename_name(proj_filename), ".po");
    case FD_I18N_POSIX: return fl_filename_setext_str(fl_filename_name(proj_filename), ".msg");
  }
}

/**
 Get the name of the project file without the filename extension.
 \return the file name without path or extension
 */
std::string Project::basename() const {
  return fl_filename_setext_str(fl_filename_name(proj_filename), "");
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
  if (!Fluid.proj.proj_filename || !*Fluid.proj.proj_filename) {
    fprintf(stderr, "** Fluid internal error: enter_project_dir() no filename set\n");
    return;
  }
  // store the current working directory for later
  app_work_dir = fl_getcwd_str();
  // set the current directory to the path of our .fl file
  std::string project_path = fl_filename_path_str(fl_filename_absolute_str(Fluid.proj.proj_filename));
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

