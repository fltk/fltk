//
// Fl_Tile test program for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Box.H>

// #define TEST_INACTIVE
// #define CLASSIC_MODE

int main(int argc, char** argv) {

#if 0 // Sample code from Fl_Tile documentation

  Fl_Window win(400, 300, "My App");

  Fl_Tile tile(0, 0, 400, 300);

  Fl_Box left_tool_box(0, 0, 100, 300, "Tools");
  left_tool_box.box(FL_DOWN_BOX);
  tile.size_range(&left_tool_box, 50, 50);

  Fl_Box document(100, 0, 200, 300, "Document");
  document.box(FL_DOWN_BOX);
  tile.size_range(&document, 100, 50);

  Fl_Box right_tool_box(300, 0, 100, 300, "More\nTools");
  right_tool_box.box(FL_DOWN_BOX);
  tile.size_range(&right_tool_box, 50, 50);

  tile.end();
  tile.resizable(document);

  win.end();
  win.resizable(tile);
  win.size_range(200, 50);
  win.show(argc,argv);

#else // new Fl_Tile test code

  Fl_Double_Window window(300, 300);
  window.box(FL_NO_BOX);
  window.resizable(window);

  Fl_Tile tile(0, 0, 300, 300);
#ifndef CLASSIC_MODE
  tile.init_size_range(30, 30); // all children's size shall be at least 30x30
#endif

  // create the symmetrical resize box with dx and dy pixels distance, resp.
  // from the borders of the Fl_Tile widget before all other children

#ifdef CLASSIC_MODE
  int dx = 20, dy = dx; // border width of resizable()
  Fl_Box r(tile.x()+dx,tile.y()+dy,tile.w()-2*dx,tile.h()-2*dy);
  tile.resizable(r);
#endif

  Fl_Box box0(0,0,150,150,"0");
  box0.box(FL_DOWN_BOX);
  box0.color(9);
  box0.labelsize(36);
  box0.align(FL_ALIGN_CLIP);
#ifndef CLASSIC_MODE
  tile.resizable(&box0);
#endif

  Fl_Double_Window w1(150,0,150,150,"1");
  w1.box(FL_NO_BOX);
  Fl_Box box1(0,0,150,150,"1\nThis is a child window");
  box1.box(FL_DOWN_BOX);
  box1.color(19);
  box1.labelsize(18);
  box1.align(FL_ALIGN_CLIP|FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
  w1.resizable(box1);
  w1.end();

  //  Fl_Tile tile2(0,150,150,150);
  Fl_Box box2a(0,150,70,150,"2a");
  box2a.box(FL_DOWN_BOX);
  box2a.color(12);
  box2a.labelsize(36);
  box2a.align(FL_ALIGN_CLIP);

  Fl_Box box2b(70,150,80,150,"2b");
  box2b.box(FL_DOWN_BOX);
  box2b.color(13);
  box2b.labelsize(36);
  box2b.align(FL_ALIGN_CLIP);
  // tile2.end();

  // Fl_Tile tile3(150,150,150,150);
  Fl_Box box3a(150,150,150,70,"3a");
  box3a.box(FL_DOWN_BOX);
  box3a.color(12);
  box3a.labelsize(36);
  box3a.align(FL_ALIGN_CLIP);

  Fl_Box box3b(150,150+70,150,80,"3b");
  box3b.box(FL_DOWN_BOX);
  box3b.color(13);
  box3b.labelsize(36);
  box3b.align(FL_ALIGN_CLIP);
  // tile3.end();

  tile.end();
  window.end();

#ifdef TEST_INACTIVE // test inactive case
  tile.deactivate();
#endif

  w1.show();
  window.size_range(90, 90);
  window.show(argc,argv);

#endif // new Fl_Tile test code

  return Fl::run();
}
