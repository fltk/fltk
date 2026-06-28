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


static constexpr int load_type = Fl_Native_File_Chooser::BROWSE_FILE;
static constexpr int save_type = Fl_Native_File_Chooser::BROWSE_SAVE_FILE;
static constexpr int load_options = Fl_Native_File_Chooser::PREVIEW;
#ifdef __APPLE__
static constexpr int save_options = Fl_Native_File_Chooser::NEW_FOLDER|Fl_Native_File_Chooser::SAVEAS_CONFIRM;
#else
static constexpr int save_options = Fl_Native_File_Chooser::NEW_FOLDER|Fl_Native_File_Chooser::SAVEAS_CONFIRM;
#endif

static void set_paths(
  Fl_Native_File_Chooser& fnfc,
  const std::string& preset_path,
  const std::string& fallback_path,
  std::string& preset_directory,
  std::string& preset_filename)
{
  std::string preset;

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
}

/**
 Lets the user choose a project file to open.
 \param title the dialog window's title
 \param preset_path the path and name to a file that should be preselected in the dialog, or an empty string to use the fallback path
 \param fallback_path the path to use if no preset path is provided
 \param filter the file filter to apply in the dialog
 \return the absolute path to the chosen file, or an empty string if the operation was canceled
 */
std::string fluid::io::load_project_filechooser(
  const std::string& title,
  const std::string& preset_path,
  const std::string& fallback_path,
  const std::string& filter)
{
  std::string preset_directory;
  std::string preset_filename;

  Fl_Native_File_Chooser fnfc;
  fnfc.type(load_type);
  fnfc.options(load_options);
  fnfc.title(title.c_str());
  fnfc.filter(filter.c_str());
  set_paths(fnfc, preset_path, fallback_path, preset_directory, preset_filename);

  switch (fnfc.show()) {
    case -1: // Error
      fl_alert("Can't open project:\n%s", fnfc.errmsg());
      return "";
    case 1: // Cancelled
      return "";
    default: // Success
      return fl_filename_absolute_str(fnfc.filename());
  }
}

/**
 Lets the user choose a data file to inline into the code.
 \param title the dialog window's title
 \param preset_path the path and name to a file that should be preselected in the dialog, or an empty string to use the fallback path
 \param fallback_path the path to use if no preset path is provided
 \param filter the file filter to apply in the dialog
 \return the relative path to the chosen file, or an empty string if the operation was canceled
 */
std::string fluid::io::load_inline_data_filechooser(
  const std::string& title,
  const std::string& preset_path,
  const std::string& fallback_path,
  const std::string& filter)
{
  std::string preset_directory;
  std::string preset_filename;

  Fl_Native_File_Chooser fnfc;
  fnfc.type(load_type);
  fnfc.options(load_options);
  fnfc.title(title.c_str());
  fnfc.filter(filter.c_str());
  set_paths(fnfc, preset_path, fallback_path, preset_directory, preset_filename);

  switch (fnfc.show()) {
    case -1: // Error
      fl_alert("Can't open data file:\n%s", fnfc.errmsg());
      return "";
    case 1: // Cancelled
      return "";
    default: // Success
      return fl_filename_relative_str(fnfc.filename());
  }
}

/**
 Lets the user choose a comment file to load.
 \param title the dialog window's title
 \param preset_path not useful here, we never save the path to the comment file, just import it immediately
 \param fallback_path the path to use if no preset path is provided
 \param filter the file filter to apply in the dialog
 \return the absolute path to the chosen file, or an empty string if the operation was canceled
 */
std::string fluid::io::load_comment_filechooser(
  const std::string& title,
  const std::string& preset_path,
  const std::string& fallback_path,
  const std::string& filter)
{
  std::string preset_directory;
  std::string preset_filename;

  Fl_Native_File_Chooser fnfc;
  fnfc.type(load_type);
  fnfc.options(load_options);
  fnfc.title(title.c_str());
  fnfc.filter(filter.c_str());
  set_paths(fnfc, preset_path, fallback_path, preset_directory, preset_filename);

  switch (fnfc.show()) {
    case -1: // Error
      fl_alert("Can't open comment file:\n%s", fnfc.errmsg());
      return "";
    case 1: // Cancelled
      return "";
    default: // Success
      return fl_filename_absolute_str(fnfc.filename());
  }
}

/**
 Lets the user choose an image file to load.
 \param title the dialog window's title
 \param preset_path the path and name to a file that should be preselected in the dialog, or an empty string to use the fallback path
 \param fallback_path the path to use if no preset path is provided
 \param filter the file filter to apply in the dialog
 \return the relative path to the chosen file, or an empty string if the operation was canceled
 */
std::string fluid::io::load_image_filechooser(
  const std::string& title,
  const std::string& preset_path,
  const std::string& fallback_path,
  const std::string& filter)
{
  std::string preset_directory;
  std::string preset_filename;

  Fl_Native_File_Chooser fnfc;
  fnfc.type(load_type);
  fnfc.options(load_options);
  fnfc.title(title.c_str());
  fnfc.filter(filter.c_str());
  set_paths(fnfc, preset_path, fallback_path, preset_directory, preset_filename);

  switch (fnfc.show()) {
    case -1: // Error
      fl_alert("Can't open image file:\n%s", fnfc.errmsg());
      return "";
    case 1: // Cancelled
      return "";
    default: // Success
      return fl_filename_relative_str(fnfc.filename());
  }
}
