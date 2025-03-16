//
// Fluid Project File History code for the Fast Light Tool Kit (FLTK).
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

#include "app/history.h"

#include "Fluid.h"

#include "../src/flstring.h"

using namespace fld;
using namespace fld::app;


/**
 Load file history from preferences.

 This loads the absolute filepaths of the last 10 used design files.
 It also computes and stores the relative filepaths for display in
 the main menu.
 */
void History::load() {
  int   i;              // Looping var
  int   max_files;

  Fluid.preferences.get("recent_files", max_files, 5);
  if (max_files > 10) max_files = 10;

  for (i = 0; i < max_files; i ++) {
    Fluid.preferences.get( Fl_Preferences::Name("file%d", i), abspath[i], "", sizeof(abspath[i]));
    if (abspath[i][0]) {
      // Make a shortened version of the filename for the menu...
      std::string fn = fl_filename_shortened(abspath[i], 48);
      strncpy(relpath[i], fn.c_str(), sizeof(relpath[i]) - 1);
      if (i == 9) Fluid.history_item[i].flags = FL_MENU_DIVIDER;
      else Fluid.history_item[i].flags = 0;
    } else break;
  }

  for (; i < 10; i ++) {
    if (i) Fluid.history_item[i-1].flags |= FL_MENU_DIVIDER;
    Fluid.history_item[i].hide();
  }
}

/**
 Update file history from preferences.

 Add this new filepath to the history and update the main menu.
 Writes the new file history to the app preferences.

 \param[in] project_file path and filename of .fl project file, will be
    converted into an absolute file path based on the current working directory.
 */
void History::update(std::string project_file) {
  int   i;              // Looping var
  int   max_files;

  Fluid.preferences.get("recent_files", max_files, 5);
  if (max_files > 10) max_files = 10;

  std::string absolute = fld::fix_separators(fl_filename_absolute_str(project_file));
  for (i = 0; i < max_files; i ++)
#if defined(_WIN32) || defined(__APPLE__)
    if (!strcasecmp(absolute.c_str(), abspath[i])) break;
#else
    if (!strcmp(absolute.c_str(), abspath[i])) break;
#endif // _WIN32 || __APPLE__

  // Does the first entry match?
  if (i == 0)
    return;

  // Was there no match?
  if (i >= max_files)
    i = max_files - 1;

  // Move the other filenames down in the list...
  memmove(abspath + 1, abspath, i * sizeof(abspath[0]));
  memmove(relpath + 1, relpath, i * sizeof(relpath[0]));

  // Put the new file at the top...
  strlcpy(abspath[0], absolute.c_str(), sizeof(abspath[0]));
  std::string fn = fl_filename_shortened(absolute, 48);
  strncpy(relpath[0], fn.c_str(), sizeof(relpath[0]) - 1);

  // Update the menu items as needed...
  for (i = 0; i < max_files; i ++) {
    Fluid.preferences.set( Fl_Preferences::Name("file%d", i), abspath[i]);
    if (abspath[i][0]) {
      if (i == 9) Fluid.history_item[i].flags = FL_MENU_DIVIDER;
      else Fluid.history_item[i].flags = 0;
    } else break;
  }

  for (; i < 10; i ++) {
    Fluid.preferences.set( Fl_Preferences::Name("file%d", i), "");
    if (i) Fluid.history_item[i-1].flags |= FL_MENU_DIVIDER;
    Fluid.history_item[i].hide();
  }
  Fluid.preferences.flush();
}

