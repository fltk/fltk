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

#include "app/project.h"

#include "nodes/Fl_Type.h"
#include "panels/settings_panel.h"

// ---- project settings

/// The current project, possibly a new, empty roject
Fluid_Project g_project;

/**
 Initialize a new project.
 */
Fluid_Project::Fluid_Project() :
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
Fluid_Project::~Fluid_Project() {
}

/**
 Reset all project setting to create a new empty project.
 */
void Fluid_Project::reset() {
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
void Fluid_Project::update_settings_dialog() {
  if (settings_window) {
    w_settings_project_tab->do_callback(w_settings_project_tab, LOAD);
    w_settings_i18n_tab->do_callback(w_settings_i18n_tab, LOAD);
  }
}

/**
 Get the absolute path of the project file, for example `/Users/matt/dev/`.
 \return the path ending in '/'
 */
std::string Fluid_Project::projectfile_path() const {
  return end_with_slash(fl_filename_absolute_str(fl_filename_path_str(proj_filename), g_launch_path));
}

/**
 Get the project file name including extension, for example `test.fl`.
 \return the file name without path
 */
std::string Fluid_Project::projectfile_name() const {
  return fl_filename_name(proj_filename);
}

/**
 Get the absolute path of the generated C++ code file, for example `/Users/matt/dev/src/`.
 \return the path ending in '/'
 */
std::string Fluid_Project::codefile_path() const {
  std::string path = fl_filename_path_str(code_file_name);
  if (batch_mode)
    return end_with_slash(fl_filename_absolute_str(path, g_launch_path));
  else
    return end_with_slash(fl_filename_absolute_str(path, projectfile_path()));
}

/**
 Get the generated C++ code file name including extension, for example `test.cxx`.
 \return the file name without path
 */
std::string Fluid_Project::codefile_name() const {
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
std::string Fluid_Project::headerfile_path() const {
  std::string path = fl_filename_path_str(header_file_name);
  if (batch_mode)
    return end_with_slash(fl_filename_absolute_str(path, g_launch_path));
  else
    return end_with_slash(fl_filename_absolute_str(path, projectfile_path()));
}

/**
 Get the generated C++ header file name including extension, for example `test.cxx`.
 \return the file name without path
 */
std::string Fluid_Project::headerfile_name() const {
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
std::string Fluid_Project::stringsfile_path() const {
  if (batch_mode)
    return g_launch_path;
  else
    return projectfile_path();
}

/**
 Get the generated i18n text file name including extension, for example `test.po`.
 \return the file name without path
 */
std::string Fluid_Project::stringsfile_name() const {
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
std::string Fluid_Project::basename() const {
  return fl_filename_setext_str(fl_filename_name(proj_filename), "");
}

