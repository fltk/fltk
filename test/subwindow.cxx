//
// "$Id"
//
// Nested window test program for the Fast Light Tool Kit (FLTK).
//
// Test to make sure nested windows work.
// Events should be reported for enter/exit and all mouse operations
// Buttons and pop-up menu should work, indicating that mouse positions
// are being correctly translated.
//
// Copyright 1998 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#include <stdlib.h>
#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>

class testwindow : public Fl_Window {
  int handle(int);
  void draw();
public:
  testwindow(Fl_Boxtype b,int x,int y,const char *l)
    : Fl_Window(x,y,l) {box(b);}
  testwindow(Fl_Boxtype b,int x,int y,int w,int h,const char *l)
    : Fl_Window(x,y,w,h,l) {box(b);}
};

void testwindow::draw() {
#ifdef DEBUG
  printf("%s : draw\n",label());
#endif
  Fl_Window::draw();
}

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

#ifdef DEBUG
const char *eventnames[] = {
"zero",
"FL_PUSH",
"FL_RELEASE",
"FL_ENTER",
"FL_LEAVE",
"FL_DRAG",
"FL_FOCUS",
"FL_UNFOCUS",
"FL_KEYBOARD",
"9",
"FL_MOVE",
"FL_SHORTCUT",
"12",
"FL_DEACTIVATE",
"FL_ACTIVATE",
"FL_HIDE",
"FL_SHOW",
"FL_PASTE",
"FL_SELECTIONCLEAR",
};
#endif

Fl_Menu_Button* popup;

int testwindow::handle(int e) {
#ifdef DEBUG
  if (e != FL_MOVE) printf("%s : %s\n",label(),eventnames[e]);
#endif
  if (Fl_Window::handle(e)) return 1;
  //  if (e==FL_PUSH) return popup->handle(e);
  return 0;
}

int main(int, char **) {
  testwindow *window =
    new testwindow(FL_UP_BOX,400,400,"outer");
  new Fl_Toggle_Button(310,310,80,80,"&outer");
  new EnterExit(10,310,80,80,"enterexit");
  new Fl_Input(150,310,150,25,"input:");
  (new Fl_Menu_Button(5,150,80,25,"menu&1"))->add("this|is|only|a test");
  testwindow *subwindow =
    new testwindow(FL_DOWN_BOX,100,100,200,200,"inner");
  new Fl_Toggle_Button(110,110,80,80,"&inner");
  new EnterExit(10,110,80,80,"enterexit");
  (new Fl_Menu_Button(50,50,80,25,"menu&2"))->add("this|is|only|a test");
  new Fl_Input(45,80,150,25,"input:");
  subwindow->resizable(subwindow);
  window->resizable(subwindow);
  subwindow->end();
  (new Fl_Box(FL_NO_BOX,0,0,400,100,
	     "A child Fl_Window with children of it's own may "
	     "be useful for imbedding controls into a GL or display "
	     "that needs a different visual.  There are bugs with the "
	     "origins being different between drawing and events, "
	     "which I hope I have solved."
	     )) -> align(FL_ALIGN_WRAP);
  popup = new Fl_Menu_Button(0,0,400,400);
  popup->type(Fl_Menu_Button::POPUP3);
  popup->add("This|is|a popup|menu");
  window->show();
  return Fl::run();
}

//
// End of "$Id: subwindow.cxx,v 1.2 1998/10/20 13:25:24 mike Exp $".
//
