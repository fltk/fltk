//
// Mandelbrot set header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>

class Drawing_Area : public Fl_Box {
  void draw() FL_OVERRIDE;
public:
  uchar *buffer;
  int W,H;
  int dx, dy;         // drawing box offsets
  int nextline;
  int drawn;
  int julia;
  int iterations;
  int brightness;
  double jX, jY;
  double X,Y,scale;
  int sx, sy, sw, sh; // selection box
  void erase_box();
  int handle(int) FL_OVERRIDE;
  void resize(int,int,int,int) FL_OVERRIDE;
  void new_display();
  enum {
    MAX_BRIGHTNESS = 16,
    DEFAULT_BRIGHTNESS = 16,
    MAX_ITERATIONS = 14,
    DEFAULT_ITERATIONS = 7
  };
  Drawing_Area(int x,int y,int w,int h) : Fl_Box(x,y,w,h) {
    buffer = 0;
    W = w;
    H = h;
    dx = 0; // NOTE: as the box type is set *after* the constructor
    dy = 0; //       the actual offsets are determined in draw()
    nextline = 0;
    drawn = 0;
    julia = 0;
    X = Y = 0;
    scale = 4.0;
    iterations = 1<<DEFAULT_ITERATIONS;
    brightness = DEFAULT_BRIGHTNESS;
  }
  int idle();
};
