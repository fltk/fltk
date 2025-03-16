//
// Command Line Arguments Handling header for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_APP_ARGS_H
#define FLUID_APP_ARGS_H

#include <string>

namespace fld {
namespace app {

class Args {
  // Callback.
  static int arg_cb(int argc, char** argv, int& i);
  // Handle args individually.
  int arg(int argc, char** argv, int& i);
public:
  /// Set, if Fluid was started with the command line argument -u
  int update_file { 0 };            // fluid -u
  /// Set, if Fluid was started with the command line argument -c
  int compile_file { 0 };           // fluid -c
  /// Set, if Fluid was started with the command line argument -cs
  int compile_strings { 0 };        // fluid -cs
  /// command line arguments that overrides the generate code file extension or name
  std::string code_filename { };    // fluid -o filename
  /// command line arguments that overrides the generate header file extension or name
  std::string header_filename { };  // fluid -h filename
  /// if set, generate images for automatic documentation in this directory
  std::string autodoc_path { };         // fluid --autodoc path
  /// Set, if Fluid was started with the command line argument -v
  int show_version { 0 };           // fluid -v
  /// Constructor.
  Args() = default;
  // Load args from command line into variables.
  int load(int argc, char **argv);
};

} // namespace app
} // namespace fld

#endif // FLUID_APP_ARGS_H

