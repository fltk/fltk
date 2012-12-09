//
// "$Id$"
//
// FLUID undo support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include "Fl_Type.h"
#include "undo.h"
#include <FL/Fl_Preferences.H>
#include <FL/filename.H>
#include "../src/flstring.h"
#if defined(WIN32) && !defined(__CYGWIN__)
#  include <io.h>
#  include <windows.h>
#  define getpid (int)GetCurrentProcessId
#  ifndef __WATCOMC__
// Visual C++ 2005 incorrectly displays a warning about the use of POSIX APIs
// on Windows, which is supposed to be POSIX compliant...
#    define unlink _unlink
#  endif // !__WATCOMC__
#else
#  include <unistd.h>
#endif // WIN32 && !__CYGWIN__


extern Fl_Preferences	fluid_prefs;	// FLUID preferences
extern Fl_Menu_Item	Main_Menu[];	// Main menu

#define UNDO_ITEM	25		// Undo menu item index
#define REDO_ITEM	26		// Redo menu item index


//
// This file implements an undo system using temporary files; ideally
// we'd like to do this in memory, however the current data structures
// and design aren't well-suited...  Instead, we save and restore
// checkpoint files.
//


int undo_current = 0;			// Current undo level in buffer
int undo_last = 0;			// Last undo level in buffer
int undo_max = 0;			// Maximum undo level used
int undo_save = -1;			// Last undo level that was saved
static int undo_paused = 0;		// Undo checkpointing paused?


// Return the undo filename
static char *undo_filename(int level, char *buf, int bufsize) {
  static char	undo_path[FL_PATH_MAX] = "";	// Undo path


  if (!undo_path[0]) fluid_prefs.getUserdataPath(undo_path, sizeof(undo_path));

  snprintf(buf, bufsize, "%sundo_%d_%d.fl", undo_path, getpid(), level);
  return buf;
}


// Redo menu callback
void redo_cb(Fl_Widget *, void *) {
  char	filename[FL_PATH_MAX];			// Undo checkpoint file

  if (undo_current >= undo_last) return;

  undo_suspend();
  if (!read_file(undo_filename(undo_current + 1, filename, sizeof(filename)), 0)) {
    // Unable to read checkpoint file, don't redo...
    undo_resume();
    return;
  }

  undo_current ++;

  // Update modified flag...
  set_modflag(undo_current != undo_save);

  // Update undo/redo menu items...
  if (undo_current >= undo_last) Main_Menu[REDO_ITEM].deactivate();
  Main_Menu[UNDO_ITEM].activate();
}

// Undo menu callback
void undo_cb(Fl_Widget *, void *) {
  char	filename[FL_PATH_MAX];			// Undo checkpoint file

  if (undo_current <= 0) return;

  if (undo_current == undo_last) {
    write_file(undo_filename(undo_current, filename, sizeof(filename)));
  }

  undo_suspend();
  if (!read_file(undo_filename(undo_current - 1, filename, sizeof(filename)), 0)) {
    // Unable to read checkpoint file, don't undo...
    undo_resume();
    return;
  }

  undo_current --;

  // Update modified flag...
  set_modflag(undo_current != undo_save);

  // Update undo/redo menu items...
  if (undo_current <= 0) Main_Menu[UNDO_ITEM].deactivate();
  Main_Menu[REDO_ITEM].activate();
  undo_resume();
}

// Save current file to undo buffer
void undo_checkpoint() {
  char	filename[FL_PATH_MAX];			// Undo checkpoint filename

//  printf("undo_checkpoint(): undo_current=%d, undo_paused=%d, modflag=%d\n",
//         undo_current, undo_paused, modflag);

  // Don't checkpoint if undo_suspend() has been called...
  if (undo_paused) return;

  // Save the current UI to a checkpoint file...
  if (!write_file(undo_filename(undo_current, filename, sizeof(filename)))) {
    // Don't attempt to do undo stuff if we can't write a checkpoint file...
    perror(filename);
    return;
  }

  // Update the saved level...
  if (modflag && undo_current <= undo_save) undo_save = -1;
  else if (!modflag) undo_save = undo_current;

  // Update the current undo level...
  undo_current ++;
  undo_last = undo_current;
  if (undo_current > undo_max) undo_max = undo_current;

  // Enable the Undo and disable the Redo menu items...
  Main_Menu[UNDO_ITEM].activate();
  Main_Menu[REDO_ITEM].deactivate();
}

// Clear undo buffer
void undo_clear() {
  char	filename[FL_PATH_MAX];			// Undo checkpoint filename


  // Remove old checkpoint files...
  for (int i = 0; i <= undo_max; i ++) {
    unlink(undo_filename(i, filename, sizeof(filename)));
  }

  // Reset current, last, and save indices...
  undo_current = undo_last = undo_max = 0;
  if (modflag) undo_save = -1;
  else undo_save = 0;
}

// Resume undo checkpoints
void undo_resume() {
  undo_paused = 0;
}

// Suspend undo checkpoints
void undo_suspend() {
  undo_paused = 1;
}


//
// End of "$Id$".
//
