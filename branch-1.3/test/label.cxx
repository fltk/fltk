//
// "$Id$"
//
// Label test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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
#include <FL/Fl_Box.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Pixmap.H>
#include <FL/fl_draw.H>

#include "pixmaps/blast.xpm"

Fl_Toggle_Button *imageb, *imageovertextb, *imagenexttotextb, *imagebackdropb;
Fl_Toggle_Button *leftb,*rightb,*topb,*bottomb,*insideb,*clipb,*wrapb;
Fl_Box *text;
Fl_Input *input;
Fl_Hor_Value_Slider *fonts;
Fl_Hor_Value_Slider *sizes;
Fl_Double_Window *window;
Fl_Pixmap *img;

void button_cb(Fl_Widget *,void *) {
  int i = 0;
  if (leftb->value()) i |= FL_ALIGN_LEFT;
  if (rightb->value()) i |= FL_ALIGN_RIGHT;
  if (topb->value()) i |= FL_ALIGN_TOP;
  if (bottomb->value()) i |= FL_ALIGN_BOTTOM;
  if (insideb->value()) i |= FL_ALIGN_INSIDE;
  if (clipb->value()) i |= FL_ALIGN_CLIP;
  if (wrapb->value()) i |= FL_ALIGN_WRAP;
  if (imageovertextb->value()) i |= FL_ALIGN_TEXT_OVER_IMAGE;
  if (imagenexttotextb->value()) i |= FL_ALIGN_IMAGE_NEXT_TO_TEXT;
  if (imagebackdropb->value()) i |= FL_ALIGN_IMAGE_BACKDROP;
  text->align(i);
  window->redraw();
}

void image_cb(Fl_Widget *,void *) {
  if (imageb->value())
    text->image(img);
  else 
    text->image(0);
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

void input_cb(Fl_Widget *,void *) {
  text->label(input->value());
  window->redraw();
}

void normal_cb(Fl_Widget *,void *) {
  text->labeltype(FL_NORMAL_LABEL);
  window->redraw();
}

void symbol_cb(Fl_Widget *,void *) {
  text->labeltype(FL_SYMBOL_LABEL);
  if (input->value()[0] != '@') {
    input->static_value("@->");
    text->label("@->");
  }
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
  {"FL_SYMBOL_LABEL",0,symbol_cb},
  {"FL_SHADOW_LABEL",0,shadow_cb},
  {"FL_ENGRAVED_LABEL",0,engraved_cb},
  {"FL_EMBOSSED_LABEL",0,embossed_cb},
  {0}};

int main(int argc, char **argv) {
  img = new Fl_Pixmap(blast_xpm);
  
  window = new Fl_Double_Window(400,400);

  input = new Fl_Input(50,375,350,25);
  input->static_value("The quick brown fox jumped over the lazy dog.");
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

  Fl_Group *g = new Fl_Group(50,275,350,50);
  imageb = new Fl_Toggle_Button(50,275,50,25,"image");
  imageb->callback(image_cb);
  imageovertextb = new Fl_Toggle_Button(100,275,50,25,"I - T");
  imageovertextb->callback(button_cb);
  imagenexttotextb = new Fl_Toggle_Button(150,275,50,25,"I | T");
  imagenexttotextb->callback(button_cb);
  imagebackdropb = new Fl_Toggle_Button(200,275,50,25,"back");
  imagebackdropb->callback(button_cb);
  leftb = new Fl_Toggle_Button(50,300,50,25,"left");
  leftb->callback(button_cb);
  rightb = new Fl_Toggle_Button(100,300,50,25,"right");
  rightb->callback(button_cb);
  topb = new Fl_Toggle_Button(150,300,50,25,"top");
  topb->callback(button_cb);
  bottomb = new Fl_Toggle_Button(200,300,50,25,"bottom");
  bottomb->callback(button_cb);
  insideb = new Fl_Toggle_Button(250,300,50,25,"inside");
  insideb->callback(button_cb);
  wrapb = new Fl_Toggle_Button(300,300,50,25,"wrap");
  wrapb->callback(button_cb);
  clipb = new Fl_Toggle_Button(350,300,50,25,"clip");
  clipb->callback(button_cb);
  g->resizable(insideb);
  g->end();

  Fl_Choice *c = new Fl_Choice(50,250,200,25);
  c->menu(choices);

  text= new Fl_Box(FL_FRAME_BOX,100,75,200,100,input->value());
  text->align(FL_ALIGN_CENTER);
  window->resizable(text);
  window->end();
  window->show(argc,argv);
  return Fl::run();
}

//
// End of "$Id$".
//
