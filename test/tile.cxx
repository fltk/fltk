//
// "$Id$"
//
// Fl_Tile test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Box.H>

//#define TEST_INACTIVE

int main(int argc, char** argv) {
  Fl_Double_Window window(300,300);
  window.box(FL_NO_BOX);
  window.resizable(window);

  int dx = 20, dy = dx; // border width of resizable() - see below
  Fl_Tile tile(0,0,300,300);

  // create the symmetrical resize box with dx and dy pixels distance, resp.
  // from the borders of the Fl_Tile widget before all other children
  Fl_Box r(tile.x()+dx,tile.y()+dy,tile.w()-2*dx,tile.h()-2*dy);
  tile.resizable(r);

  Fl_Box box0(0,0,150,150,"0");
  box0.box(FL_DOWN_BOX);
  box0.color(9);
  box0.labelsize(36);
  box0.align(FL_ALIGN_CLIP);

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
  //tile2.end();

  //Fl_Tile tile3(150,150,150,150);
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
  //tile3.end();

  tile.end();
  window.end();

#ifdef TEST_INACTIVE // test inactive case
  tile.deactivate();
#endif

  w1.show();
  window.show(argc,argv);
  return Fl::run();
}

//
// End of "$Id$".
//
