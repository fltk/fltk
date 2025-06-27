//
// Fluid Project Internationalization header for the Fast Light Tool Kit (FLTK).
//
// Copyright 2025 by Bill Spitzak and others.
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

#ifndef FLUID_PROJ_I18N_H
#define FLUID_PROJ_I18N_H

#include <string>

namespace fld {

class Project;

/**
 Enumeration of available internationalization types.
 */
enum class I18n_Type {
  NONE = 0, ///< No i18n, all strings are litearals
  GNU,      ///< GNU gettext internationalization
  POSIX     ///< Posix catgets internationalization
};

namespace io {
class Project_Reader;
class Project_Writer;
}

namespace proj {

/**
 Data and settings for a FLUID project file.
 */
class I18n
{
public:
  Project &project_;

  /// One of the available internationalization types.
  fld::I18n_Type type = I18n_Type::NONE;
  /// Include file for GNU i18n, writes an #include statement into the source
  /// file. This is usually `<libintl.h>` or `"gettext.h"` for GNU gettext.
  std::string gnu_include = "<libintl.h>";
  // Optional name of a macro for conditional i18n compilation.
  std::string gnu_conditional = "";
  /// For the gettext/intl.h options, this is the function that translates text
  /// at runtime. This is usually "gettext" or "_".
  std::string gnu_function = "gettext";
  /// For the gettext/intl.h options, this is the function that marks the translation
  /// of text at initialisation time. This is usually "gettext_noop" or "N_".
  std::string gnu_static_function = "gettext_noop";

  /// Include file for Posix i18n, write a #include statement into the source
  /// file. This is usually `<nl_types.h>` for Posix catgets.
  std::string posix_include = "<nl_types.h>";
  // Optional name of a macro for conditional i18n compilation.
  std::string posix_conditional = "";
  /// Name of the nl_catd database
  std::string posix_file = "";
  /// Message set ID for the catalog.
  std::string posix_set = "1";

public: // Methods
  I18n(Project &p) : project_(p) {};
  ~I18n() = default;
  void reset();
  void read(io::Project_Reader &f, const char *key);
  void write(io::Project_Writer &f) const;
};

} // namespace proj

} // namespace fld

#endif // FLUID_PROJ_I18N_H


