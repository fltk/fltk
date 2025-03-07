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


#ifndef FLUID_APP_PROJECT_H
#define FLUID_APP_PROJECT_H

#include <string>

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

  std::string projectfile_path() const;
  std::string projectfile_name() const;
  std::string codefile_path() const;
  std::string codefile_name() const;
  std::string headerfile_path() const;
  std::string headerfile_name() const;
  std::string stringsfile_path() const;
  std::string stringsfile_name() const;
  std::string basename() const;

  /// One of the available internationalization types.
  Fd_I18n_Type i18n_type;
  /// Include file for GNU i18n, writes an #include statement into the source
  /// file. This is usually `<libintl.h>` or `"gettext.h"` for GNU gettext.
  std::string i18n_gnu_include;
  // Optional name of a macro for conditional i18n compilation.
  std::string i18n_gnu_conditional;
  /// For the gettext/intl.h options, this is the function that translates text
  /// at runtime. This is usually "gettext" or "_".
  std::string i18n_gnu_function;
  /// For the gettext/intl.h options, this is the function that marks the translation
  /// of text at initialisation time. This is usually "gettext_noop" or "N_".
  std::string i18n_gnu_static_function;

  /// Include file for Posix i18n, write a #include statement into the source
  /// file. This is usually `<nl_types.h>` for Posix catgets.
  std::string i18n_pos_include;
  // Optional name of a macro for conditional i18n compilation.
  std::string i18n_pos_conditional;
  /// Name of the nl_catd database
  std::string i18n_pos_file;
  /// Message set ID for the catalog.
  std::string i18n_pos_set;

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
  /// Filename of the current .fl project file
  const char *proj_filename { nullptr };
  /// Hold the default extension for header files, or the entire filename if set via command line.
  std::string header_file_name;
  /// Hold the default extension for source code  files, or the entire filename if set via command line.
  std::string code_file_name;
};

extern Fluid_Project g_project;

#endif // FLUID_APP_PROJECT_H


