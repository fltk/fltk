//
// Fluid Project header for the Fast Light Tool Kit (FLTK).
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


#ifndef FLUID_PROJECT_H
#define FLUID_PROJECT_H

#include "proj/undo.h"
#include "proj/i18n.h"
#include "nodes/Tree.h"

#include <string>

// ---- project class declaration


namespace fld {

namespace app {
  class Layout_Preset;
  extern Layout_Preset *default_layout_preset;
} // namespace app

/**
 Data and settings for a FLUID project file.
 */
class Project
{
public: // Member Variables
  // Undo actions for this Project.
  proj::Undo undo { *this };

  // Manage the node tree of the project.
  node::Tree tree { *this };

  // Project internationalization.
  proj::I18n i18n { *this };

  /// If set, generate code to include the header file form the c++ file
  int include_H_from_C = 1;
  /// If set, handle keyboard shortcut Ctrl on macOS using Cmd instead
  int use_FL_COMMAND = 0;
  /// Clear if UTF-8 characters in statics texts are written as escape sequences
  int utf8_in_src = 0;
  /// If set, <FL/Fl.H> will not be included from the header code before anything else
  int avoid_early_includes = 0;
  /// If set, command line overrides header file name in .fl file.
  int header_file_set = 0;
  ///  If set, command line overrides source code file name in .fl file.
  int code_file_set = 0;
  /// later
  int write_mergeback_data = 0;
  /// Filename of the current .fl project file
  const char *proj_filename { nullptr };
  /// Hold the default extension for header files, or the entire filename if set via command line.
  std::string header_file_name = ".h";
  /// Hold the default extension for source code  files, or the entire filename if set via command line.
  std::string code_file_name = ".cxx";
  /// Macro used in header file for #ifdef MACRO \n #defined MACRO \n ... \n #endif
  std::string include_guard = "";

  /// Used as a counter to set the .fl project dir as the current directory.
  int in_project_dir { 0 };
  /// Application work directory, stored here when temporarily changing to the source code directory.
  std::string app_work_dir = "";

  /// Set if the current design has been modified compared to the associated .fl design file.
  int modflag { 0 };
  /// Set if the code files are older than the current design.
  int modflag_c { 0 };

  /// Currently used layout preset.
  app::Layout_Preset *layout { app::default_layout_preset };

public: // Methods
  Project();
  ~Project();
  void reset();
  void update_settings_dialog();

  std::string projectfile_path() const;
  std::string projectfile_name() const;
  std::string codefile_path() const;
  std::string codefile_name() const;
  std::string headerfile_path() const;
  std::string headerfile_name() const;
  std::string stringsfile_path() const;
  std::string stringsfile_name() const;
  std::string basename() const;

  void enter_project_dir();
  void leave_project_dir();

  void set_filename(std::nullptr_t);
  void set_filename(const std::string &c);
  void write_strings();

  void set_modflag(int mf, int mfc = -1);
};

} // namespace fld

#endif // FLUID_PROJECT_H


