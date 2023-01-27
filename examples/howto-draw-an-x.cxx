//
// Demonstrate how to draw an 'X' in fltk
//
//     Create a custom widget that draws an 'X' to the corners of the window,
//     even when window is resized. Here we subclass Fl_Widget, the lowest level
//     FLTK widget object. Origin: http://seriss.com/people/erco/fltk/#FltkX
//
// Copyright 2005 by Greg Ercolano.
// Copyright 1998-2017 by Bill Spitzak and others.
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
#include <FL/fl_draw.H>
#include <FL/Fl_Double_Window.H>

class DrawX : public Fl_Widget {
public:
    DrawX(int X, int Y, int W, int H, const char*L=0) : Fl_Widget(X,Y,W,H,L) {
    }
    virtual void draw() FL_OVERRIDE {
        // Draw background - a white filled rectangle
        fl_color(FL_WHITE); fl_rectf(x(),y(),w(),h());
        // Draw black 'X' over base widget's background
        fl_color(FL_BLACK);
        int x1 = x(),       y1 = y();
        int x2 = x()+w()-1, y2 = y()+h()-1;
        fl_line(x1, y1, x2, y2);
        fl_line(x1, y2, x2, y1);
    }
};
int main() {
    Fl_Double_Window win(200,200,"Draw X");
    DrawX draw_x(10, 10, win.w()-20, win.h()-20);       // put our widget 10 pixels within window edges
    draw_x.color(FL_WHITE);                             // make widget's background white
    win.resizable(draw_x);
    win.show();
    return(Fl::run());
}
