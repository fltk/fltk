//
// Resize example for use in the Fast Light Tool Kit (FLTK) documentation.
//
//     See Article #415: How does resizing work?
//     https://www.fltk.org/articles.php?L415
//
// Copyright 1998-2020 by Bill Spitzak and others.
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

#ifndef RESIZE_ARROWS_H
#define RESIZE_ARROWS_H

#include <FL/Fl.H>
#include <FL/Fl_Box.H>

/** Harrow is an Fl_Box with a horizontal arrow drawn across the middle.

    The arrow is drawn in black on a white background.
    By default, the box has no border, and the label is below the box.
 */
class Harrow : public Fl_Box {
public:
  Harrow(int X, int Y, int W, int H, const char *T = 0);

  void draw() FL_OVERRIDE;
};

/** Varrow is an Fl_Box with a vertical arrow drawn down the middle.

    The arrow is drawn in black on a white background.
    By default, the box has no border, and the label is to the right of the box.
 */
class Varrow : public Fl_Box {
public:
  Varrow(int X, int Y, int W, int H, const char *T = 0);

  void draw() FL_OVERRIDE;
};

#endif // RESIZE_ARROWS_H
