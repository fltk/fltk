//
// Fluid C++ File Chooser code for the Fast Light Tool Kit (FLTK).
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

#include "io/file_chooser.h"

#include "Fluid.h"

#include <FL/Fl_Native_File_Chooser.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>


static constexpr int load_type = Fl_Native_File_Chooser::BROWSE_FILE;
static constexpr int save_type = Fl_Native_File_Chooser::BROWSE_SAVE_FILE;
static constexpr int load_options = Fl_Native_File_Chooser::PREVIEW;
#ifdef __APPLE__
static constexpr int save_options = Fl_Native_File_Chooser::NEW_FOLDER|Fl_Native_File_Chooser::SAVEAS_CONFIRM;
#else
static constexpr int save_options = Fl_Native_File_Chooser::NEW_FOLDER|Fl_Native_File_Chooser::SAVEAS_CONFIRM;
#endif


std::string fluid::io::filechooser(
  fluid::io::FileChooserType type,
  fluid::io::FileChooserPath path_type,
  const std::string& title,
  const std::string& error_message,
  const std::string& preset_path,
  const std::string& fallback_path,
  const std::string& filter
) {
  std::string preset;
  std::string preset_directory;
  std::string preset_filename;

  Fl_Native_File_Chooser fnfc;
  if (type == FileChooserType::LOAD_FILE) {
    fnfc.type(load_type);
    fnfc.options(load_options);
  } else {
    fnfc.type(save_type);
    fnfc.options(save_options);
  }
  fnfc.title(title.c_str());
  fnfc.filter(filter.c_str());
  if (!preset_path.empty()) {
    preset = fl_filename_absolute_str(preset_path);
    preset_directory = fl_filename_path_str(preset);
    preset_filename = fl_filename_name_str(preset);
  } else {
    if (!fallback_path.empty()) {
      preset = fl_filename_absolute_str(fallback_path);
    } else {
      preset = fl_filename_absolute_str(Fluid.launch_path());
    }
    preset_directory = preset;
    preset_filename.clear();
  }
  fnfc.directory(preset_directory.c_str());
  fnfc.preset_file(preset_filename.c_str());

  switch (fnfc.show()) {
    case -1: // Error
      fl_alert(error_message.c_str(), fnfc.errmsg());
      return "";
    case 1: // Cancelled
      return "";
    default: // Success
      if (path_type == FileChooserPath::ABSOLUTE) {
        return fl_filename_absolute_str(fnfc.filename());
      } else {
        return fl_filename_relative_str(fnfc.filename());
      }
  }
}
