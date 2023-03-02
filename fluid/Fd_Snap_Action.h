//
// Snap action header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2023 by Bill Spitzak and others.
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

#ifndef _FLUID_FD_SNAP_ACTION_H
#define _FLUID_FD_SNAP_ACTION_H

#include "Fl_Window_Type.h"

extern int fd_left_window_margin;
extern int fd_right_window_margin;
extern int fd_top_window_margin;
extern int fd_bottom_window_margin;
extern int fd_window_grid_x;
extern int fd_window_grid_y;

/**
 \brief Structure holding all the data to perform interactive alignment operations.
 */
typedef struct {
  int dx, dy;           ///< distance of the mouse from its initial PUSH event
  int bx, by, br, bt;   ///< bounding box of the original push event or current bounding box when drawing
  int drag;             ///< drag event mask
  int x_dist, y_dist;   ///< current closest snapping distance in x and y
  int dx_out, dy_out;   ///< current closest snapping point as a delta
  Fl_Widget_Type *wgt;  ///< first selected widget
  Fl_Window_Type *win;  ///< window that handles the drag action
} Fd_Snap_Data;

/**
 \brief Find points of interest when moving the bounding box of all selected widgets.
 */
class Fd_Snap_Action {
protected:
  void check_x_(Fd_Snap_Data &d, int x_ref, int x_snap);
  void check_y_(Fd_Snap_Data &d, int y_ref, int y_snap);
  void check_x_y_(Fd_Snap_Data &d, int x_ref, int x_snap, int y_ref, int y_snap);
public:
  int dx, dy, type, mask;
  Fd_Snap_Action() : dx(128), dy(128), type(0), mask(0) { }
  virtual void check(Fd_Snap_Data &d) = 0;
  virtual void draw(Fd_Snap_Data &d) { }
  virtual bool matches(Fd_Snap_Data &d);
public:
  static Fd_Snap_Action *list[];
  static void check_all(Fd_Snap_Data &d);
  static void draw_all(Fd_Snap_Data &d);
};

#endif // _FLUID_FD_SNAP_ACTION_H
