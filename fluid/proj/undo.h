//
// Fluid Undo header for the Fast Light Tool Kit (FLTK).
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

#ifndef undo_h
#define undo_h

#include <FL/filename.H>

class Fl_Widget;

namespace fld {

class Project;

namespace proj {

class Undo
{
public:
  
  enum class OnceType {
    ALWAYS = 0,
    WINDOW_RESIZE
  };

  /// Link Undo class to this project.
  Project &proj_;
  /// Current undo level in buffer
  int current_ = 0;
  /// Last undo level in buffer
  int last_ = 0;
  // Maximum undo level used
  int max_ = 0;
  /// Last undo level that was saved
  int save_ = -1;
  // Undo checkpointing paused?
  int paused_ = 0;
  // Undo file path
  char path_[FL_PATH_MAX] { };
  // length w/o filename
  unsigned int path_len_ = 0;
  /// Suspend further undos of the same type
  OnceType once_type_ = OnceType::ALWAYS;

public:

  // Constructor.
  Undo(Project &p);
  // Destructor.
  ~Undo();

  // Save current file to undo buffer
  void checkpoint();
  // Save undo buffer once until a different checkpoint type is called
  int checkpoint(OnceType type);
  // Clear undo buffer
  void clear();
  // Resume undo checkpoints
  void resume();
  // Suspend undo checkpoints
  void suspend();
  // Return the undo filename.
  char *filename(int level);

  // Redo menu callback
  void redo();
  // Undo menu callback
  void undo();

  // Redo menu callback
  static void redo_cb(Fl_Widget *, void *);
  // Undo menu callback
  static void undo_cb(Fl_Widget *, void *);
};

} // namespace fld
} // namespace proj


#endif // !undo_h
