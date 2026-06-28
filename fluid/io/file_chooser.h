//
// Fluid C++ File Chooser header for the Fast Light Tool Kit (FLTK).
//
// Copyright 2026 by Bill Spitzak and others.
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

#ifndef FLUID_IO_FILE_CHOOSER_H
#define FLUID_IO_FILE_CHOOSER_H

#include <string>

namespace fluid {

namespace io {

// ✔︎ Load Inline Data
// ✔︎ Pick a comment
// ✔︎ ui_find_image (the offender here, I guess)

// Application::save_project_file
// ✔︎ Application::open_project_filechooser
// Fd_Shell_Command_List::export_selected
// Fd_Shell_Command_List::import_from_file
// Load Layout Settings
// Save Layout Settings

extern std::string load_project_filechooser(
  const std::string& title,
  const std::string& preset_path,
  const std::string& fallback_path,
  const std::string& filter);

extern std::string load_inline_data_filechooser(
  const std::string& title,
  const std::string& preset_path,
  const std::string& fallback_path,
  const std::string& filter);

extern std::string load_comment_filechooser(
  const std::string& title,
  const std::string& preset_path,
  const std::string& fallback_path,
  const std::string& filter);

extern std::string load_image_filechooser(
  const std::string& title,
  const std::string& preset_path,
  const std::string& fallback_path,
  const std::string& filter);

} // namespace io

} // namespace fluid

#endif // FLUID_IO_FILE_CHOOSER_H
