//
// FLTK native file chooser widget : Zenity version
//
// Copyright 2021-2022 by Bill Spitzak and others.
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

#ifndef FL_ZENITY_NATIVE_FILE_CHOOSER_H
#define FL_ZENITY_NATIVE_FILE_CHOOSER_H 1

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

#include "Fl_Native_File_Chooser_Kdialog.H"

class Fl_Zenity_Native_File_Chooser_Driver : public Fl_Kdialog_Native_File_Chooser_Driver {
  friend class Fl_Native_File_Chooser;
  static bool did_find_zenity;
  static bool have_looked_for_zenity;
  Fl_Zenity_Native_File_Chooser_Driver(int val);
  void append_filter(std::string& command);
  void build_command(std::string& command) FL_OVERRIDE;
};

/**
\}
\endcond
*/

#endif // FL_ZENITY_NATIVE_FILE_CHOOSER_H
