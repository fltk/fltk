//
// Fluid Undo code for the Fast Light Tool Kit (FLTK).
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

#include "proj/undo.h"

#include "Fluid.h"
#include "Project.h"
#include "io/Project_Reader.h"
#include "io/Project_Writer.h"
#include "nodes/Node.h"
#include "nodes/Widget_Node.h"
#include "widgets/Node_Browser.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_ask.H>
#include "tools/filename.h"
#include "../src/flstring.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <io.h>
#  include <windows.h>
#  define getpid (int)GetCurrentProcessId
#else
#  include <unistd.h>
#endif // _WIN32 && !__CYGWIN__

// This file implements an undo system using temporary files; ideally
// we'd like to do this in memory, however the current data structures
// and design aren't well-suited...  Instead, we save and restore
// checkpoint files.

extern Fl_Window* the_panel;

using namespace fld;
using namespace fld::proj;


Undo::Undo(Project &p)
: proj_( p )
{ }

Undo::~Undo() {
  // TODO: delete old undo files when calling the destructor.
}


// Return the undo filename.
// The filename is constructed in a static internal buffer and
// this buffer is overwritten by every call of this function.
// The return value is a pointer to this internal string.
char *Undo::filename(int level) {
  if (!path_len_) {
    Fluid.preferences.getUserdataPath(path_, sizeof(path_));
    path_len_ = (unsigned int)strlen(path_);
  }

  // append filename: "undo_PID_LEVEL.fl"
  snprintf(path_ + path_len_,
           sizeof(path_) - path_len_ - 1,
           "undo_%d_%d.fl", getpid(), level);
  return path_;
}


// Redo menu callback
void Undo::redo() {
  // int undo_item = main_menubar->find_index(undo_cb);
  // int redo_item = main_menubar->find_index(redo_cb);
  once_type_ = OnceType::ALWAYS;

  if (current_ >= last_) {
    fl_beep();
    return;
  }

  suspend();
  if (widget_browser) {
    widget_browser->save_scroll_position();
    widget_browser->new_list();
  }
  int reload_panel = (the_panel && the_panel->visible());
  if (!fld::io::read_file(proj_, filename(current_ + 1), 0)) {
    // Unable to read checkpoint file, don't redo...
    widget_browser->rebuild();
    proj_.update_settings_dialog();
    resume();
    return;
  }
  if (reload_panel) {
    for (auto w: Fluid.proj.tree.all_selected_widgets()) {
      w->open();
    }
  }
  if (widget_browser) widget_browser->restore_scroll_position();

  current_ ++;

  // Update modified flag...
  proj_.set_modflag(current_ != save_);
  widget_browser->rebuild();
  proj_.update_settings_dialog();

  // Update undo/redo menu items...
  // if (current_ >= last_) main_menu[redo_item].deactivate();
  // main_menu[undo_item].activate();
  resume();
}

// Undo menu callback
void Undo::undo() {
  // int undo_item = main_menubar->find_index(undo_cb);
  // int redo_item = main_menubar->find_index(redo_cb);
  once_type_ = OnceType::ALWAYS;

  if (current_ <= 0) {
    fl_beep();
    return;
  }

  if (current_ == last_) {
    fld::io::write_file(proj_, filename(current_));
  }

  suspend();
  // Undo first deletes all widgets which resets the widget_tree browser.
  // Save the current scroll position, so we don't scroll back to 0 at undo.
  // TODO: make the scroll position part of the .fl project file
  if (widget_browser) {
    widget_browser->save_scroll_position();
    widget_browser->new_list();
  }
  int reload_panel = (the_panel && the_panel->visible());
  if (!fld::io::read_file(proj_, filename(current_ - 1), 0)) {
    // Unable to read checkpoint file, don't undo...
    widget_browser->rebuild();
    proj_.update_settings_dialog();
    proj_.set_modflag(0, 0);
    resume();
    return;
  }
  if (reload_panel) {
    for (Node *t = Fluid.proj.tree.first; t; t=t->next) {
      if (t->is_widget() && t->selected) {
        t->open();
        break;
      }
    }
  }
  // Restore old browser position.
  // Ideally, we would save the browser position inside the undo file.
  if (widget_browser) widget_browser->restore_scroll_position();

  current_ --;

  // Update modified flag...
  proj_.set_modflag(current_ != save_);

  // Update undo/redo menu items...
  // if (current_ <= 0) main_menu[undo_item].deactivate();
  // main_menu[redo_item].activate();
  widget_browser->rebuild();
  proj_.update_settings_dialog();
  resume();
}

/**
 \param[in] type set a new type, or set to 0 to clear the once_type without setting a checkpoint
 \return 1 if the checkpoint was set, 0 if this is a repeating event
 */
int Undo::checkpoint(OnceType type) {
  if (type == OnceType::ALWAYS) {
    once_type_ = OnceType::ALWAYS;
    return 0;
  }
  if (paused_) return 0;
  if (once_type_ != type) {
    checkpoint();
    once_type_ = type;
    return 1;
  } else {
    // do not add more checkpoints for the same undo type
    return 0;
  }
}

// Save current file to undo buffer
void Undo::checkpoint() {
  //  printf("checkpoint(): current_=%d, paused_=%d, modflag=%d\n",
  //         current_, paused_, modflag);

  // Don't checkpoint if suspend() has been called...
  if (paused_) return;

  // int undo_item = main_menubar->find_index(undo_cb);
  // int redo_item = main_menubar->find_index(redo_cb);
  once_type_ = OnceType::ALWAYS;

  // Save the current UI to a checkpoint file...
  const char *file = filename(current_);
  if (!fld::io::write_file(proj_, file)) {
    // Don't attempt to do undo stuff if we can't write a checkpoint file...
    perror(file);
    return;
  }

  // Update the saved level...
  if (proj_.modflag && current_ <= save_) save_ = -1;
  else if (!proj_.modflag) save_ = current_;

  // Update the current undo level...
  current_ ++;
  last_ = current_;
  if (current_ > max_) max_ = current_;

  // Enable the Undo and disable the Redo menu items...
  // main_menu[undo_item].activate();
  // main_menu[redo_item].deactivate();
}

// Clear undo buffer
void Undo::clear() {
  // int undo_item = main_menubar->find_index(undo_cb);
  // int redo_item = main_menubar->find_index(redo_cb);
  // Remove old checkpoint files...
  for (int i = 0; i <= max_; i ++) {
    fl_unlink(filename(i));
  }

  // Reset current, last, and save indices...
  current_ = last_ = max_ = 0;
  if (proj_.modflag) save_ = -1;
  else save_ = 0;

  // Disable the Undo and Redo menu items...
  // main_menu[undo_item].deactivate();
  // main_menu[redo_item].deactivate();
}

// Resume undo checkpoints
void Undo::resume() {
  paused_--;
}

// Suspend undo checkpoints
void Undo::suspend() {
  paused_++;
}

void Undo::undo_cb(Fl_Widget *, void *) {
  Fluid.proj.undo.undo();
}

void Undo::redo_cb(Fl_Widget *, void *) {
  Fluid.proj.undo.redo();
}
