//
// Self-generate snapshots of user interface for FLUID documentation.
//
// Copyright 2023-2025 by Bill Spitzak and others.
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

/**
 \file autodoc.h
 \brief tools to take snapshots of UI elements for documentation purposes
 */

#ifndef fl_screenshot_H
#define fl_screenshot_H

#include <FL/Fl_Export.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Rect.H>

#include <string>

/** Class to initialize a Rect by providing the margin around a rect. */
class Fl_Margin : public Fl_Rect {
public:
  Fl_Margin(int dx, int dy, int dr, int db);
};

int fl_snapshot(const char *filename, Fl_Widget **w,
                const Fl_Rect &frame = Fl_Margin(4, 4, 4, 4),
                const Fl_Rect &blend = Fl_Margin(4, 4, 4, 4),
                double scale=1.0);

int fl_snapshot(const char *filename, Fl_Widget *w1, Fl_Widget *w2,
                const Fl_Rect &frame = Fl_Margin(4, 4, 4, 4),
                const Fl_Rect &blend = Fl_Margin(4, 4, 4, 4),
                double scale=1.0);

int fl_snapshot(const char *filename, Fl_Widget *w,
                const Fl_Rect &frame = Fl_Margin(4, 4, 4, 4),
                const Fl_Rect &blend = Fl_Margin(4, 4, 4, 4),
                double scale=1.0);

extern const int FL_SNAP_TO_WINDOW;

extern Fl_Widget *FL_SNAP_AREA_CLEAR;

extern void run_autodoc(const std::string &target_dir);

#endif

