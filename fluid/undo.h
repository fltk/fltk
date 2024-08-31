//
// FLUID undo definitions for the Fast Light Tool Kit (FLTK).
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

#ifndef undo_h
#define undo_h

class Fl_Widget;

#define kUndoWindowResize 1

extern int undo_current;                // Current undo level in buffer
extern int undo_last;                   // Last undo level in buffer
extern int undo_save;                   // Last undo level that was saved
extern int undo_once_type;              // Suspend further undos of the same type

void redo_cb(Fl_Widget *, void *);      // Redo menu callback
void undo_cb(Fl_Widget *, void *);      // Undo menu callback
void undo_checkpoint();                 // Save current file to undo buffer
void undo_checkpoint_once(int type);    // Save undo buffer once until a different checkpoint type is called
void undo_clear();                      // Clear undo buffer
void undo_resume();                     // Resume undo checkpoints
void undo_suspend();                    // Suspend undo checkpoints

#endif // !undo_h
