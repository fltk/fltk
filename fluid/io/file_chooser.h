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

enum class FileChooserType {
  LOAD_FILE,
  SAVE_FILE
};

enum class FileChooserPath {
  ABSOLUTE,
  RELATIVE
};

extern std::string filechooser(
  FileChooserType type,
  FileChooserPath path_type,
  const std::string& title,
  const std::string& error_message,
  const std::string& preset_path,
  const std::string& fallback_path,
  const std::string& filter
);

} // namespace io

} // namespace fluid

#endif // FLUID_IO_FILE_CHOOSER_H
