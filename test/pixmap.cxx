//
// Pixmap label test program for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Button.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Anim_GIF_Image.H>
#include <stdio.h>

#include "pixmaps/animated_fluid_gif.h"

#include <FL/Fl_Toggle_Button.H>

Fl_Toggle_Button *leftb,*rightb,*topb,*bottomb,*insideb,*overb,*inactb;
Fl_Button *b;
Fl_Double_Window *w;
Fl_Anim_GIF_Image *pixmap;
Fl_Anim_GIF_Image *depixmap;

void button_cb(Fl_Widget *wgt,void *) {
  int i = 0;
  if (leftb->value()) i |= FL_ALIGN_LEFT;
  if (rightb->value()) i |= FL_ALIGN_RIGHT;
  if (topb->value()) i |= FL_ALIGN_TOP;
  if (bottomb->value()) i |= FL_ALIGN_BOTTOM;
  if (insideb->value()) i |= FL_ALIGN_INSIDE;
  if (overb->value()) i |= FL_ALIGN_TEXT_OVER_IMAGE;
  b->align(i);
  if (inactb->value()) b->deactivate();
  else b->activate();
  w->redraw();
}

void play_cb(Fl_Widget *wgt,void *) {
  pixmap->start();
  depixmap->start();
}

void stop_cb(Fl_Widget *wgt,void *) {
  pixmap->stop();
  depixmap->stop();
}

void step_cb(Fl_Widget *wgt,void *) {
  pixmap->next();
  depixmap->next();
}

int dvisual = 0;
int arg(int, char **argv, int &i) {
  if (argv[i][1] == '8') {dvisual = 1; i++; return 1;}
  return 0;
}

#include <FL/Fl_Multi_Label.H>

int main(int argc, char **argv) {
  int i = 1;
  if (Fl::args(argc,argv,i,arg) < argc)
    Fl::fatal(" -8 # : use default visual\n%s\n", Fl::help);
  if (!dvisual) Fl::visual(FL_RGB);

  Fl_Double_Window window(400,440); ::w = &window;
  Fl_Button b(130,170,140,140,"Pixmap"); ::b = &b;

  Fl_Anim_GIF_Image::animate = true;
  pixmap = new Fl_Anim_GIF_Image("fluid", animated_fluid_gif, animated_fluid_gif_size,
                                 &b, Fl_Anim_GIF_Image::DONT_RESIZE_CANVAS);
  pixmap->speed(0.5);
  b.image(pixmap);

  depixmap = (Fl_Anim_GIF_Image*)pixmap->copy();
  depixmap->inactive();
  b.deimage(depixmap);

  // "bind" images to avoid memory leak reports (valgrind, asan)
  // note: these reports are benign because they appear at exit, but anyway
  b.bind_image(pixmap);
  b.bind_deimage(depixmap);

  leftb = new Fl_Toggle_Button(25,50,50,25,"left");
  leftb->callback(button_cb);
  rightb = new Fl_Toggle_Button(75,50,50,25,"right");
  rightb->callback(button_cb);
  topb = new Fl_Toggle_Button(125,50,50,25,"top");
  topb->callback(button_cb);
  bottomb = new Fl_Toggle_Button(175,50,50,25,"bottom");
  bottomb->callback(button_cb);
  insideb = new Fl_Toggle_Button(225,50,50,25,"inside");
  insideb->callback(button_cb);
  overb = new Fl_Toggle_Button(25,75,100,25,"text over");
  overb->callback(button_cb);
  inactb = new Fl_Toggle_Button(125,75,100,25,"inactive");
  inactb->callback(button_cb);

  Fl_Button* play = new Fl_Button(300, 50, 25, 25, "@>");
  play->labelcolor(FL_DARK2);
  play->callback(play_cb);
  Fl_Button* stop = new Fl_Button(325, 50, 25, 25, "@||");
  stop->labelcolor(FL_DARK2);
  stop->callback(stop_cb);
  Fl_Button* step = new Fl_Button(350, 50, 25, 25, "@|>");
  step->labelcolor(FL_DARK2);
  step->callback(step_cb);

  window.resizable(window);
  window.end();
  window.show(argc,argv);
  return Fl::run();
}
