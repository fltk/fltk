//
// "$Id$"
//
// Nested window test program for the Fast Light Tool Kit (FLTK).
//
// Test to make sure nested windows work.
// Events should be reported for enter/exit and all mouse operations
// Buttons and pop-up menu should work, indicating that mouse positions
// are being correctly translated.
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

#include <stdlib.h>
#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>

#ifdef DEBUG
#include <FL/names.h>
#endif

// Define DEBUG_POS for a subwindow positioning test. This will draw
// the last typed character at the cursor position, if no input widget
// currently has the focus.
// Note: The typed character is saved in the derived testwindow class,
// regardless of the definition of DEBUG_POS. Only drawing the character
// depends on this define.

#ifdef DEBUG_POS
#include <FL/fl_draw.H>
#endif

class EnterExit : public Fl_Box {
  int handle(int);
public:
  EnterExit(int x, int y, int w, int h, const char *l) : Fl_Box(FL_BORDER_BOX,x,y,w,h,l) {}
};

int EnterExit::handle(int e) {
  if (e == FL_ENTER) {color(FL_RED); redraw(); return 1;}
  else if (e == FL_LEAVE) {color(FL_GRAY); redraw(); return 1;}
  else return 0;
}

class testwindow : public Fl_Window {
  int handle(int);
  void draw();
  int cx, cy; char key;
  Fl_Cursor crsr;
public:
  testwindow(Fl_Boxtype b,int x,int y,const char *l)
    : Fl_Window(x,y,l), crsr(FL_CURSOR_DEFAULT) {box(b); key = 0;}
  testwindow(Fl_Boxtype b,int x,int y,int w,int h,const char *l)
    : Fl_Window(x,y,w,h,l) {box(b); key = 0;}
  void use_cursor(Fl_Cursor c) { crsr = c; }
};

void testwindow::draw() {
#ifdef DEBUG
  printf("%s : draw\n",label());
#endif
  Fl_Window::draw();
#ifdef DEBUG_POS
  if (key) fl_draw(&key, 1, cx, cy);
#endif
}

int testwindow::handle(int e) {
#ifdef DEBUG
  if (e != FL_MOVE) printf("%s : %s\n",label(),fl_eventnames[e]);
#endif
  if (crsr!=FL_CURSOR_DEFAULT) {
    if (e == FL_ENTER) 
      cursor(crsr);
    if (e == FL_LEAVE) 
      cursor(FL_CURSOR_DEFAULT);
  }
  if (Fl_Window::handle(e)) return 1;
  if (e == FL_FOCUS) return 1;
  if (e == FL_PUSH) {Fl::focus(this); return 1;}
  if (e == FL_KEYBOARD && Fl::event_text()[0]) {
    key = Fl::event_text()[0];
    cx = Fl::event_x();
    cy = Fl::event_y();
    redraw();
    return 1;
  }
  return 0;
}

Fl_Menu_Button* popup;

const char* bigmess =
#if 1
"this|is|only|a test"
#else
"item1|item2|item3|item4|item5|"
"submenu/item1|submenu/item2|submenu/item3|submenu/item4|"
"submenu/sub/item1|submenu/sub/item2|submenu/sub/item3|"
"item6|item7|item8|item9|item10|"
"item21|item22|item23|item24|item25|"
"submenu/item21|submenu/item22|submenu/item23|submenu/item24|"
"submenu/sub/item21|submenu/sub/item22|submenu/sub/item23|"
"item36|item37|item38|item39|item310|"
"item31|item32|item33|item34|item35|"
"submenu/item31|submenu/item32|submenu/item33|submenu/item34|"
"submenu/sub/item31|submenu/sub/item32|submenu/sub/item33|"
"item46|item47|item48|item49|item410|"
"item41|item42|item43|item44|item45|"
"submenu/item41|submenu/item42|submenu/item43|submenu/item44|"
"submenu/sub/item41|submenu/sub/item42|submenu/sub/item43|"
"item26|item27|item28|item29|item210|"
"submenu2/item1|submenu2/item2|submenu2/item3|submenu2/item4|"
"submenu2/sub/item1|submenu2/sub/item2|submenu2/sub/item3|"
"item6|item7|item8|item9|item10|"
"item21|item22|item23|item24|item25|"
"submenu2/item21|submenu2/item22|submenu2/item23|submenu2/item24|"
"submenu2/sub/item21|submenu2/sub/item22|submenu2/sub/item23|"
"item36|item37|item38|item39|item310|"
"item31|item32|item33|item34|item35|"
"submenu2/item31|submenu2/item32|submenu2/item33|submenu2/item34|"
"submenu2/sub/item31|submenu2/sub/item32|submenu2/sub/item33|"
"item46|item47|item48|item49|item410|"
"item41|item42|item43|item44|item45|"
"submenu2/item41|submenu2/item42|submenu2/item43|submenu2/item44|"
"submenu2/sub/item41|submenu2/sub/item42|submenu2/sub/item43|"
"item26|item27|item28|item29|item210|"
#endif
;

int main(int argc, char **argv) {
  testwindow *window =
    new testwindow(FL_UP_BOX,400,400,"outer");
  new Fl_Toggle_Button(310,310,80,80,"&outer");
  new EnterExit(10,310,80,80,"enterexit");
  new Fl_Input(160,310,140,25,"input1:");
  new Fl_Input(160,340,140,25,"input2:");
  (new Fl_Menu_Button(5,150,80,25,"menu&1"))->add(bigmess);
  testwindow *subwindow =
    new testwindow(FL_DOWN_BOX,100,100,200,200,"inner");
  new Fl_Toggle_Button(110,110,80,80,"&inner");
  new EnterExit(10,110,80,80,"enterexit");
  (new Fl_Menu_Button(50,20,80,25,"menu&2"))->add(bigmess);
  new Fl_Input(55,50,140,25,"input1:");
  new Fl_Input(55,80,140,25,"input2:");
  subwindow->resizable(subwindow);
  window->resizable(subwindow);
  subwindow->end();
  subwindow->use_cursor(FL_CURSOR_HAND);
  (new Fl_Box(FL_NO_BOX,0,0,400,100,
	     "A child Fl_Window with children of its own may "
	     "be useful for imbedding controls into a GL or display "
	     "that needs a different visual.  There are bugs with the "
	     "origins being different between drawing and events, "
	     "which I hope I have solved."
	     )) -> align(FL_ALIGN_WRAP);
  popup = new Fl_Menu_Button(0,0,400,400);
  popup->type(Fl_Menu_Button::POPUP3);
  popup->add("This|is|a popup|menu");
  popup->add(bigmess);
  window->show(argc, argv);
  return Fl::run();
}

//
// End of "$Id$".
//
