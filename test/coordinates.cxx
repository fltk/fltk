//
// Coordinate demonstration program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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
#include <FL/Fl_Window.H>
#include "../src/flstring.h" // fl_snprintf()
#include <stdio.h>

class Box : public Fl_Box {
public:
  Box(int X, int Y, int W, int H, Fl_Color C, const char* T)
  : Fl_Box(X, Y, W, H, T) {
    align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    box(FL_DOWN_BOX);
    labelcolor(C);
    labelsize(11);
  }
};

class Title : public Fl_Box {
public:
  Title(int X, int Y, int W, int H, Fl_Color C, const char* T)
  : Fl_Box(X, Y, W, H, T) {
    align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_TOP);
    box(FL_NO_BOX);
    labelcolor(C);
    labelsize(12);
  }
};

class MainWindow : public Fl_Window {
public:
  MainWindow(int X, int Y, const char* T)
  : Fl_Window(X, Y, T) {

    Fl_Window* tl_window = new Fl_Window(0, 0, 250, 100);
    tl_window->box(FL_ENGRAVED_BOX);
    /* Title* tl_title = */ new Title(10, 10, 230, 40, FL_RED,
      "Fl_Window TL(0, 0, 250, 100)\nx, y relative to main window");
    /* Box* tl_box = */ new Box(25, 50, 200, 40, FL_RED,
      "Fl_Box tl(25, 50, 200, 40)\nx, y relative to TL window");
    tl_window->end();

    Fl_Window* br_window = new Fl_Window(250, 100, 250, 100);
    br_window->box(FL_ENGRAVED_BOX);
    /* Title* br_title = */ new Title(10, 10, 230, 40, FL_MAGENTA,
      "Fl_Window BR(250, 100, 250, 100)\nx, y relative to main window");
    /* Box* br_box = */ new Box(25, 50, 200, 40, FL_MAGENTA,
      "Fl_Box br(25, 50, 200, 40)\nx, y relative to BR window");
    br_window->end();

    Fl_Group* tr_group = new Fl_Group(250, 0, 250, 100);
    tr_group->box(FL_ENGRAVED_BOX);
    /* Title* tr_title = */ new Title(260, 10, 230, 40, FL_BLUE,
      "Fl_Group TR(250, 0, 250, 100)\nx, y relative to main window");
    /* Box* tr_box = */ new Box(275, 50, 200, 40, FL_BLUE,
      "Fl_Box tr(275, 50, 200, 40)\nx, y relative to main window");
    tr_group->end();

    Fl_Group* bl_group = new Fl_Group(0, 100, 250, 100);
    bl_group->box(FL_ENGRAVED_BOX);
    /* Title* bl_title = */ new Title(10, 110, 230, 40, FL_BLACK,
      "Fl_Group BL(0, 100, 250, 100)\nx, y relative to main window");
    /* Box* bl_box = */ new Box(25, 150, 200, 40, FL_BLACK,
      "Fl_Box bl(25, 150, 200, 40)\nx, y relative to main window");
    bl_group->end();

    // member variable
    message_box = new Fl_Box(0, 201, 500, 30);
    message_box->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    message_box->box(FL_ENGRAVED_BOX);
    message_box->labelfont(FL_COURIER);
    message_box->labelsize(12);

    end();
  }

protected:
  int handle(int event) FL_OVERRIDE {
    static char buffer[128];
    static const char* fmt = "Mouse position relative to main window: %3d,%3d";
    int result = Fl_Window::handle(event);
    switch (event) {
      case FL_ENTER:
      case FL_LEAVE:
        result = 1;
        message_box->copy_label("");
        break;
      case FL_MOVE:
      case FL_DRAG:
        result = 1;
        if (0 < Fl::event_x() && Fl::event_x() < w() &&
            0 < Fl::event_y() && Fl::event_y() < h()) {
          fl_snprintf(buffer, 128-1, fmt, Fl::event_x(), Fl::event_y());
          message_box->copy_label(buffer);
        } else message_box->copy_label("");
        break;
      default:
        break;
    }
    return result;
  }

private:
  Fl_Box* message_box;
};

int main(int argc, char** argv) {
  MainWindow window(500, 232, "FLTK Coordinate Systems");
  window.show(argc, argv);
  return Fl::run();
}
