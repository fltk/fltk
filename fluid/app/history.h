//
// Fluid Project File History header for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_APP_HISTORY_H
#define FLUID_APP_HISTORY_H

#include <tools/filename.h>

#include <FL/filename.H>

namespace fld {
namespace app {

class History {
public:
  /// Stores the absolute filename of the last 10 project files, saved in app preferences.
  char abspath[10][FL_PATH_MAX] { };
  /// The list of filenames as displayed in the main menu.
  char relpath[10][FL_PATH_MAX] { };
  // Create the project file history.
  History() = default;
  // Load the project history from the preferences database.
  void load();
  // Add a new file to the project history using absolute paths.
  void update(std::string project_file);
};

} // namespace app
} // namespace fld

#endif // FLUID_APP_HISTORY_H

