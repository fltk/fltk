//
// Label test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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
Fl_Hor_Value_Slider *h_margin, *v_margin, *img_spacing;
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

void h_margin_cb(Fl_Widget *,void *) {
  text->horizontal_label_margin(int(h_margin->value()));
  window->redraw();
}

void v_margin_cb(Fl_Widget *,void *) {
  text->vertical_label_margin(int(v_margin->value()));
  window->redraw();
}

void spacing_cb(Fl_Widget *,void *) {
  text->label_image_spacing(int(img_spacing->value()));
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

  window = new Fl_Double_Window(440,495);

  input = new Fl_Input(70,435,350,25,"Label:");
  input->static_value("The quick brown fox jumped over the lazy dog.");
  input->when(FL_WHEN_CHANGED);
  input->callback(input_cb);
  input->tooltip("label text");

  sizes= new Fl_Hor_Value_Slider(70,350,350,25,"Size:");
  sizes->align(FL_ALIGN_LEFT);
  sizes->bounds(1,64);
  sizes->step(1);
  sizes->value(14);
  sizes->callback(size_cb);

  fonts=new Fl_Hor_Value_Slider(70,325,350,25,"Font:");
  fonts->align(FL_ALIGN_LEFT);
  fonts->bounds(0,15);
  fonts->step(1);
  fonts->value(0);
  fonts->callback(font_cb);

  Fl_Box *margin = new Fl_Box(0, 380, 70, 25, "Margins");
  margin->box(FL_FLAT_BOX);
  margin->align(FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);

  h_margin = new Fl_Hor_Value_Slider(70+50,380,125,25,"Hor:");
  h_margin->align(FL_ALIGN_LEFT);
  h_margin->bounds(-25,25);
  h_margin->step(1);
  h_margin->value(0);
  h_margin->callback(h_margin_cb);

  v_margin = new Fl_Hor_Value_Slider(70+175+50,380,125,25,"Vert:");
  v_margin->align(FL_ALIGN_LEFT);
  v_margin->bounds(-25,25);
  v_margin->step(1);
  v_margin->value(0);
  v_margin->callback(v_margin_cb);

  img_spacing = new Fl_Hor_Value_Slider(70+50,405,125,25,"Image:");
  img_spacing->align(FL_ALIGN_LEFT);
  img_spacing->bounds(0,50);
  img_spacing->step(1);
  img_spacing->value(0);
  img_spacing->callback(spacing_cb);

  Fl_Group *g = new Fl_Group(70,275,350,50);
  imageb = new Fl_Toggle_Button(70,275,50,25,"image");
  imageb->callback(image_cb);
  imageb->tooltip("show image");

  imageovertextb = new Fl_Toggle_Button(120,275,50,25,"T o I");
  imageovertextb->callback(button_cb);
  imageovertextb->tooltip("FL_ALIGN_TEXT_OVER_IMAGE");

  imagenexttotextb = new Fl_Toggle_Button(170,275,50,25,"I | T");
  imagenexttotextb->callback(button_cb);
  imagenexttotextb->tooltip("FL_ALIGN_IMAGE_NEXT_TO_TEXT");

  imagebackdropb = new Fl_Toggle_Button(220,275,50,25,"back");
  imagebackdropb->callback(button_cb);
  imagebackdropb->tooltip("FL_ALIGN_IMAGE_BACKDROP");

  leftb = new Fl_Toggle_Button(70,300,50,25,"left");
  leftb->callback(button_cb);
  leftb->tooltip("FL_ALIGN_LEFT");

  rightb = new Fl_Toggle_Button(120,300,50,25,"right");
  rightb->callback(button_cb);
  rightb->tooltip("FL_ALIGN_RIGHT");

  topb = new Fl_Toggle_Button(170,300,50,25,"top");
  topb->callback(button_cb);
  topb->tooltip("FL_ALIGN_TOP");

  bottomb = new Fl_Toggle_Button(220,300,50,25,"bottom");
  bottomb->callback(button_cb);
  bottomb->tooltip("FL_ALIGN_BOTTOM");

  insideb = new Fl_Toggle_Button(270,300,50,25,"inside");
  insideb->callback(button_cb);
  insideb->tooltip("FL_ALIGN_INSIDE");

  wrapb = new Fl_Toggle_Button(320,300,50,25,"wrap");
  wrapb->callback(button_cb);
  wrapb->tooltip("FL_ALIGN_WRAP");

  clipb = new Fl_Toggle_Button(370,300,50,25,"clip");
  clipb->callback(button_cb);
  clipb->tooltip("FL_ALIGN_CLIP");

  g->resizable(insideb);
  g->end();

  Fl_Choice *c = new Fl_Choice(70,250,200,25);
  c->menu(choices);

  text = new Fl_Box(FL_FRAME_BOX,120,75,200,100,input->value());
  text->align(FL_ALIGN_CENTER);

  window->resizable(text);
  window->end();
  window->show(argc,argv);
  return Fl::run();
}
