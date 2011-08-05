//
// "$Id$"
//
// Label test program for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_draw.H>

#include <math.h>
#ifndef M_PI
#define M_PI 3.141592654
#endif

Fl_Toggle_Button *leftb,*rightb,*clipb;
//Fl_Box *text;
Fl_Input *input;
Fl_Hor_Value_Slider *fonts;
Fl_Hor_Value_Slider *sizes;
Fl_Hor_Value_Slider *angles;
Fl_Double_Window *window;

//code taken from fl_engraved_label.cxx
class Rotated_Label_Box : public Fl_Widget{
  protected:
  void draw(){
    draw_box();
    fl_font(labelfont(), labelsize());
    fl_color(labelcolor());
    int dx(0),dy(0);
     
    if(rt_align&FL_ALIGN_CLIP)fl_push_clip(x(),y(),w(),h());
    else fl_push_no_clip();
    fl_measure(rt_text,dx,dy);
    if(rt_align&FL_ALIGN_LEFT){
      dx=dy=0;
    }else if(rt_align&FL_ALIGN_RIGHT){
      dy=(int)(-sin(M_PI*(double)(rt_angle+180)/180.)*(double)dx);
      dx=(int)(cos(M_PI*(double)(rt_angle+180)/180.)*(double)dx);
    }else{
      dy=(int)(sin(M_PI*(double)rt_angle/180.)*(double)dx);
      dx=(int)(-cos(M_PI*(double)rt_angle/180.)*(double)dx);
      dx/=2;dy/=2;
    }
    if(labeltype()==FL_SHADOW_LABEL)shadow_label(x()+w()/2+dx,y()+h()/2+dy);
    else if(labeltype()==FL_ENGRAVED_LABEL)engraved_label(x()+w()/2+dx,y()+h()/2+dy);
    else if(labeltype()==FL_EMBOSSED_LABEL)embossed_label(x()+w()/2+dx,y()+h()/2+dy);
    else{
     fl_draw(rt_angle,rt_text,x()+w()/2+dx,y()+h()/2+dy);
    }
    fl_pop_clip();
    draw_label();
  }
  void innards(int X, int Y, int data[][3], int n){
    for (int i = 0; i < n; i++) {
      fl_color((Fl_Color)(i < n-1 ? data[i][2] : labelcolor()));
      fl_draw(rt_angle,rt_text, X+data[i][0], Y+data[i][1]);
    }
  }

  void shadow_label(int X, int Y){
    static int data[2][3] = {{2,2,FL_DARK3},{0,0,0}};
    innards(X, Y, data, 2);
  }

  void engraved_label(int X, int Y){
    static int data[7][3] = {
      {1,0,FL_LIGHT3},{1,1,FL_LIGHT3},{0,1,FL_LIGHT3},
      {-1,0,FL_DARK3},{-1,-1,FL_DARK3},{0,-1,FL_DARK3},
      {0,0,0}};
    innards(X, Y, data, 7);
  }

  void embossed_label(int X, int Y){
    static int data[7][3] = {
      {-1,0,FL_LIGHT3},{-1,-1,FL_LIGHT3},{0,-1,FL_LIGHT3},
      {1,0,FL_DARK3},{1,1,FL_DARK3},{0,1,FL_DARK3},
      {0,0,0}};
    innards(X, Y, data, 7);
  }

  public:
  Rotated_Label_Box(int X, int Y, int W, int H, const char*L=0):
    Fl_Widget(X,Y,W,H,L),rt_angle(0),rt_align((Fl_Align)0){
      rt_text=input->value();
    };
  int rt_angle;
  const char* rt_text;
  Fl_Align rt_align;
}*text;


void button_cb(Fl_Widget *,void *) {
  int i = 0;
  if (leftb->value()) i |= FL_ALIGN_LEFT;
  if (rightb->value()) i |= FL_ALIGN_RIGHT;
  if (clipb->value()) i |= FL_ALIGN_CLIP;
  text->rt_align=(Fl_Align)i;
  window->redraw();
}

void font_cb(Fl_Widget *,void *) {
  text->labelfont(int(fonts->value()));
  window->redraw();
}

void size_cb(Fl_Widget *,void *) {
  text->labelsize(int(sizes->value()));
  window->redraw();
}
void angle_cb(Fl_Widget *,void *) {
  text->rt_angle=(int)angles->value();
  window->redraw();
}

void input_cb(Fl_Widget *,void *) {
  text->rt_text=input->value();
  window->redraw();
}

void normal_cb(Fl_Widget *,void *) {
  text->labeltype(FL_NORMAL_LABEL);
  window->redraw();
}

void shadow_cb(Fl_Widget *,void *) {
  text->labeltype(FL_SHADOW_LABEL);
  window->redraw();
}

void embossed_cb(Fl_Widget *,void *) {
  text->labeltype(FL_EMBOSSED_LABEL);
  window->redraw();
}

void engraved_cb(Fl_Widget *,void *) {
  text->labeltype(FL_ENGRAVED_LABEL);
  window->redraw();
}

Fl_Menu_Item choices[] = {
  {"FL_NORMAL_LABEL",0,normal_cb},
  {"FL_SHADOW_LABEL",0,shadow_cb},
  {"FL_ENGRAVED_LABEL",0,engraved_cb},
  {"FL_EMBOSSED_LABEL",0,embossed_cb},
  {0}};

int main(int argc, char **argv) {
  window = new Fl_Double_Window(400,425);

  angles= new Fl_Hor_Value_Slider(50,400,350,25,"Angle:");
  angles->align(FL_ALIGN_LEFT);
  angles->bounds(-360,360);
  angles->step(1);
  angles->value(0);
  angles->callback(angle_cb);

  input = new Fl_Input(50,375,350,25);
  input->static_value("Rotate Me!!!");
  input->when(FL_WHEN_CHANGED);
  input->callback(input_cb);

  sizes= new Fl_Hor_Value_Slider(50,350,350,25,"Size:");
  sizes->align(FL_ALIGN_LEFT);
  sizes->bounds(1,64);
  sizes->step(1);
  sizes->value(14);
  sizes->callback(size_cb);

  fonts=new Fl_Hor_Value_Slider(50,325,350,25,"Font:");
  fonts->align(FL_ALIGN_LEFT);
  fonts->bounds(0,15);
  fonts->step(1);
  fonts->value(0);
  fonts->callback(font_cb);

  Fl_Group *g = new Fl_Group(50,300,350,25);
  leftb = new Fl_Toggle_Button(50,300,50,25,"left");
  leftb->callback(button_cb);
  rightb = new Fl_Toggle_Button(100,300,50,25,"right");
  rightb->callback(button_cb);
  clipb = new Fl_Toggle_Button(350,300,50,25,"clip");
  clipb->callback(button_cb);
  g->resizable(rightb);
  g->end();

  Fl_Choice *c = new Fl_Choice(50,275,200,25);
  c->menu(choices);

  text= new Rotated_Label_Box(100,75,200,100,"Widget with rotated text");
  text->box(FL_FRAME_BOX);
  text->align(FL_ALIGN_BOTTOM);
  window->resizable(text);
  window->end();
  window->show(argc,argv);
  return Fl::run();
}

//
// End of "$Id$".
//
