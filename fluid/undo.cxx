//
// FLUID undo support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

#include "undo.h"

#include "fluid.h"
#include "file.h"
#include "Fl_Type.h"
#include "Fl_Widget_Type.h"
#include "widget_browser.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_ask.H>
#include "fluid_filename.h"
#include "../src/flstring.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <io.h>
#  include <windows.h>
#  define getpid (int)GetCurrentProcessId
#else
#  include <unistd.h>
#endif // _WIN32 && !__CYGWIN__


//
// This file implements an undo system using temporary files; ideally
// we'd like to do this in memory, however the current data structures
// and design aren't well-suited...  Instead, we save and restore
// checkpoint files.
//

extern Fl_Window* the_panel;

int undo_current = 0;                   // Current undo level in buffer
int undo_last = 0;                      // Last undo level in buffer
int undo_max = 0;                       // Maximum undo level used
int undo_save = -1;                     // Last undo level that was saved
static int undo_paused = 0;             // Undo checkpointing paused?
int undo_once_type = 0;                 // Suspend further undos of the same type


// Return the undo filename.
// The filename is constructed in a static internal buffer and
// this buffer is overwritten by every call of this function.
// The return value is a pointer to this internal string.
static char *undo_filename(int level) {
  static char undo_path[FL_PATH_MAX] = ""; // Undo path
  static unsigned int undo_path_len = 0;   // length w/o filename

  if (!undo_path_len) {
    fluid_prefs.getUserdataPath(undo_path, sizeof(undo_path));
    undo_path_len = (unsigned int)strlen(undo_path);
  }

  // append filename: "undo_PID_LEVEL.fl"
  snprintf(undo_path + undo_path_len,
           sizeof(undo_path) - undo_path_len - 1,
           "undo_%d_%d.fl", getpid(), level);
  return undo_path;
}


// Redo menu callback
void redo_cb(Fl_Widget *, void *) {
  // int undo_item = main_menubar->find_index(undo_cb);
  // int redo_item = main_menubar->find_index(redo_cb);
  undo_once_type = 0;

  if (undo_current >= undo_last) {
    fl_beep();
    return;
  }

  undo_suspend();
  if (widget_browser) {
    widget_browser->save_scroll_position();
    widget_browser->new_list();
  }
  int reload_panel = (the_panel && the_panel->visible());
  if (!read_file(undo_filename(undo_current + 1), 0)) {
    // Unable to read checkpoint file, don't redo...
    widget_browser->rebuild();
    g_project.update_settings_dialog();
    undo_resume();
    return;
  }
  if (reload_panel) {
    for (Fl_Type *t = Fl_Type::first; t; t=t->next) {
      if (t->is_widget() && t->selected)
        t->open();
    }
  }
  if (widget_browser) widget_browser->restore_scroll_position();

  undo_current ++;

  // Update modified flag...
  set_modflag(undo_current != undo_save);
  widget_browser->rebuild();
  g_project.update_settings_dialog();

  // Update undo/redo menu items...
  // if (undo_current >= undo_last) Main_Menu[redo_item].deactivate();
  // Main_Menu[undo_item].activate();
  undo_resume();
}

// Undo menu callback
void undo_cb(Fl_Widget *, void *) {
  // int undo_item = main_menubar->find_index(undo_cb);
  // int redo_item = main_menubar->find_index(redo_cb);
  undo_once_type = 0;

  if (undo_current <= 0) {
    fl_beep();
    return;
  }

  if (undo_current == undo_last) {
    write_file(undo_filename(undo_current));
  }

  undo_suspend();
  // Undo first deletes all widgets which resets the widget_tree browser.
  // Save the current scroll position, so we don't scroll back to 0 at undo.
  // TODO: make the scroll position part of the .fl project file
  if (widget_browser) {
    widget_browser->save_scroll_position();
    widget_browser->new_list();
  }
  int reload_panel = (the_panel && the_panel->visible());
  if (!read_file(undo_filename(undo_current - 1), 0)) {
    // Unable to read checkpoint file, don't undo...
    widget_browser->rebuild();
    g_project.update_settings_dialog();
    set_modflag(0, 0);
    undo_resume();
    return;
  }
  if (reload_panel) {
    for (Fl_Type *t = Fl_Type::first; t; t=t->next) {
      if (t->is_widget() && t->selected) {
        t->open();
        break;
      }
    }
  }
  // Restore old browser position.
  // Ideally, we would save the browser position inside the undo file.
  if (widget_browser) widget_browser->restore_scroll_position();

  undo_current --;

  // Update modified flag...
  set_modflag(undo_current != undo_save);

  // Update undo/redo menu items...
  // if (undo_current <= 0) Main_Menu[undo_item].deactivate();
  // Main_Menu[redo_item].activate();
  widget_browser->rebuild();
  g_project.update_settings_dialog();
  undo_resume();
}

/**
 \param[in] type set a new type, or set to 0 to clear the once_type without setting a checkpoint
 \return 1 if the checkpoint was set, 0 if this is a repeating event
 */
int undo_checkpoint_once(int type) {
  if (type == 0) {
    undo_once_type = 0;
    return 0;
  }
  if (undo_paused) return 0;
  if (undo_once_type != type) {
    undo_checkpoint();
    undo_once_type = type;
    return 1;
  } else {
    // do not add more checkpoints for the same undo type
    return 0;
  }
}

// Save current file to undo buffer
void undo_checkpoint() {
  //  printf("undo_checkpoint(): undo_current=%d, undo_paused=%d, modflag=%d\n",
  //         undo_current, undo_paused, modflag);

  // Don't checkpoint if undo_suspend() has been called...
  if (undo_paused) return;

  // int undo_item = main_menubar->find_index(undo_cb);
  // int redo_item = main_menubar->find_index(redo_cb);
  undo_once_type = 0;

  // Save the current UI to a checkpoint file...
  const char *filename = undo_filename(undo_current);
  if (!write_file(filename)) {
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
  // Main_Menu[undo_item].activate();
  // Main_Menu[redo_item].deactivate();
}

// Clear undo buffer
void undo_clear() {
  // int undo_item = main_menubar->find_index(undo_cb);
  // int redo_item = main_menubar->find_index(redo_cb);
  // Remove old checkpoint files...
  for (int i = 0; i <= undo_max; i ++) {
    fl_unlink(undo_filename(i));
  }

  // Reset current, last, and save indices...
  undo_current = undo_last = undo_max = 0;
  if (modflag) undo_save = -1;
  else undo_save = 0;

  // Disable the Undo and Redo menu items...
  // Main_Menu[undo_item].deactivate();
  // Main_Menu[redo_item].deactivate();
}

// Resume undo checkpoints
void undo_resume() {
  undo_paused--;
}

// Suspend undo checkpoints
void undo_suspend() {
  undo_paused++;
}
