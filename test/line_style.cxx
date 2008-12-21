//
// "$Id$"
//
// Line style demo for the Fast Light Tool Kit (FLTK).
//
// Copyright 2000-2008 by Bill Spitzak and others.
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
#include <FL/Fl_Window.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Box.H>

Fl_Window *form;
Fl_Slider *sliders[8];
Fl_Choice *choice[3];
Fl_Check_Button *draw_line;

class test_box: public Fl_Window {
  void draw();
public:
  test_box(int x,int y,int w,int h,const char *l=0)
    : Fl_Window(x,y,w,h,l) {}
}*test;

void test_box::draw() {
  Fl_Window::draw();
  fl_color((uchar)(sliders[0]->value()),
	   (uchar)(sliders[1]->value()),
	   (uchar)(sliders[2]->value()));
  char buf[5];
  buf[0] = char(sliders[4]->value());
  buf[1] = char(sliders[5]->value());
  buf[2] = char(sliders[6]->value());
  buf[3] = char(sliders[7]->value());
  buf[4] = 0;
  fl_line_style(
    (long)(choice[0]->mvalue()->user_data()) +
    (long)(choice[1]->mvalue()->user_data()) +
    (long)(choice[2]->mvalue()->user_data()),
    (long)(sliders[3]->value()),
    buf);
  
  // draw the defined fl_rect and fl_vertex first and then
  // the additional one-pixel line, if enabled

  for (int i=0; i<draw_line->value()+1; i++) {
    fl_rect(10,10,w()-20,h()-20);
    fl_begin_line();
    fl_vertex(35, 35);
    fl_vertex(w()-35, h()-35);
    fl_vertex(w()-40, 35);
    fl_vertex(35, h()/2);
    fl_end_line();
    // you must reset the line type when done:
    fl_line_style(FL_SOLID);
    fl_color(FL_BLACK);
  }
}

Fl_Menu_Item style_menu[] = {
  {"FL_SOLID",	0, 0, (void*)FL_SOLID},
  {"FL_DASH",	0, 0, (void*)FL_DASH},
  {"FL_DOT",	0, 0, (void*)FL_DOT},
  {"FL_DASHDOT",0, 0, (void*)FL_DASHDOT},
  {"FL_DASHDOTDOT", 0, 0, (void*)FL_DASHDOTDOT},
  {0}
};

Fl_Menu_Item cap_menu[] = {
  {"default",		0, 0, 0},
  {"FL_CAP_FLAT",	0, 0, (void*)FL_CAP_FLAT},
  {"FL_CAP_ROUND",	0, 0, (void*)FL_CAP_ROUND},
  {"FL_CAP_SQUARE",	0, 0, (void*)FL_CAP_SQUARE},
  {0}
};

Fl_Menu_Item join_menu[] = {
  {"default",		0, 0, 0},
  {"FL_JOIN_MITER",	0, 0, (void*)FL_JOIN_MITER},
  {"FL_JOIN_ROUND",	0, 0, (void*)FL_JOIN_ROUND},
  {"FL_JOIN_BEVEL",	0, 0, (void*)FL_JOIN_BEVEL},
  {0}
};

void do_redraw(Fl_Widget*,void*)
{
    test->redraw();
}

void makeform(const char *) {
  form = new Fl_Window(500,230,"fl_line_style() test");
  sliders[0]= new Fl_Value_Slider(280,10,180,20,"R");
  sliders[0]->bounds(0,255);
  sliders[1]= new Fl_Value_Slider(280,30,180,20,"G");
  sliders[1]->bounds(0,255);
  sliders[2]= new Fl_Value_Slider(280,50,180,20,"B");
  sliders[2]->bounds(0,255);
  choice[0]= new Fl_Choice(280,70,180,20,"Style");
  choice[0]->menu(style_menu);
  choice[1]= new Fl_Choice(280,90,180,20,"Cap");
  choice[1]->menu(cap_menu);
  choice[2]= new Fl_Choice(280,110,180,20,"Join");
  choice[2]->menu(join_menu);
  sliders[3]= new Fl_Value_Slider(280,130,180,20,"Width");
  sliders[3]->bounds(0,20);
  draw_line = new Fl_Check_Button(280,150,20,20,"Line");
  draw_line->align(FL_ALIGN_LEFT);
  new Fl_Box (300,150,160,20,"adds a 1-pixel black line");
  sliders[4] = new Fl_Slider(200,190,70,20,"Dash");
  sliders[4]->align(FL_ALIGN_TOP_LEFT);
  sliders[4]->bounds(0,40);
  sliders[5] = new Fl_Slider(270,190,70,20);
  sliders[5]->bounds(0,40);
  sliders[6] = new Fl_Slider(340,190,70,20);
  sliders[6]->bounds(0,40);
  sliders[7] = new Fl_Slider(410,190,70,20);
  sliders[7]->bounds(0,40);
  int i;
  for (i=0;i<8;i++) {
    sliders[i]->type(1);
    if (i<4) sliders[i]->align(FL_ALIGN_LEFT);
    sliders[i]->callback((Fl_Callback*)do_redraw);
    sliders[i]->step(1);
  }
  sliders[0]->value(255); // R
  sliders[1]->value(100); // G
  sliders[2]->value(100); // B
  draw_line->value(0);
  draw_line->callback((Fl_Callback*)do_redraw);
  for (i=0;i<3;i++) {
    choice[i]->value(0);
    choice[i]->callback((Fl_Callback*)do_redraw);
  }
  test=new test_box(10,10,190,190);
  test->end();
  form->resizable(test);
  form->end();
}

int main(int argc, char **argv) {
  makeform(argv[0]);
  form->show(argc,argv);
  return Fl::run();
}

//
// End of "$Id$".
//
