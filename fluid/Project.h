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
#include "nodes/Tree.h"

#include <string>

// ---- project class declaration


namespace fld {

namespace app {
  class Layout_Preset;
  extern Layout_Preset *default_layout_preset;
} // namespace app

/**
 Enumeration of available internationalization types.
 */
enum class I18n_Type {
  NONE = 0, ///< No i18n, all strings are litearals
  GNU,      ///< GNU gettext internationalization
  POSIX     ///< Posix catgets internationalization
};


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

  /// One of the available internationalization types.
  fld::I18n_Type i18n_type = I18n_Type::NONE;
  /// Include file for GNU i18n, writes an #include statement into the source
  /// file. This is usually `<libintl.h>` or `"gettext.h"` for GNU gettext.
  std::string i18n_gnu_include = "<libintl.h>";
  // Optional name of a macro for conditional i18n compilation.
  std::string i18n_gnu_conditional = "";
  /// For the gettext/intl.h options, this is the function that translates text
  /// at runtime. This is usually "gettext" or "_".
  std::string i18n_gnu_function = "gettext";
  /// For the gettext/intl.h options, this is the function that marks the translation
  /// of text at initialisation time. This is usually "gettext_noop" or "N_".
  std::string i18n_gnu_static_function = "gettext_noop";

  /// Include file for Posix i18n, write a #include statement into the source
  /// file. This is usually `<nl_types.h>` for Posix catgets.
  std::string i18n_pos_include = "<nl_types.h>";
  // Optional name of a macro for conditional i18n compilation.
  std::string i18n_pos_conditional = "";
  /// Name of the nl_catd database
  std::string i18n_pos_file = "";
  /// Message set ID for the catalog.
  std::string i18n_pos_set = "1";

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
  
  void set_filename(const char *c);
  void write_strings();

  void set_modflag(int mf, int mfc = -1);
};

} // namespace fld

#endif // FLUID_PROJECT_H


