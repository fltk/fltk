//
// "$Id$"
//
// Tile test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
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
  Fl_Window window(300,300);
  window.box(FL_NO_BOX);
  window.resizable(window);
  Fl_Tile tile(0,0,300,300);
  Fl_Box box0(0,0,150,150,"0");
  box0.box(FL_DOWN_BOX);
  box0.color(9);
  box0.labelsize(36);
  box0.align(FL_ALIGN_CLIP);
  Fl_Window w1(150,0,150,150,"1");
  w1.box(FL_NO_BOX);
  Fl_Box box1(0,0,150,150,"1\nThis is a\nchild\nwindow");
  box1.box(FL_DOWN_BOX);
  box1.color(19);
  box1.labelsize(18);
  box1.align(FL_ALIGN_CLIP);
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
  
  Fl_Box r(10,0,300-10,300-10);
  tile.resizable(r);
  // r.box(FL_BORDER_FRAME);

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
