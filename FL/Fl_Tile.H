//
// Tile header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file. If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#ifndef Fl_Tile_H
#define Fl_Tile_H

#include "Fl_Group.H"

/*
  The Fl_Tile class lets you resize its children by dragging
  the border between them.
*/

class FL_EXPORT Fl_Tile : public Fl_Group {
public:
  int handle(int event) FL_OVERRIDE;
  Fl_Tile(int X, int Y, int W, int H, const char *L=0);
  void resize(int X, int Y, int W, int H) FL_OVERRIDE;
  void move_intersection(int oldx, int oldy, int newx, int newy);
  FL_DEPRECATED("in 1.4.0 - use move_intersection(p) instead",
  void position(int oldx, int oldy, int newx, int newy)) { return move_intersection(oldx, oldy, newx, newy); }
  void position(int x, int y) { Fl_Group::position(x, y); }
};

#endif
