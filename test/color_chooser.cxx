//
// "$Id$"
//
// Color chooser test program for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/fl_show_colormap.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Image.H>
#include <FL/x.H>
#include <FL/fl_draw.H>

#include <stdlib.h>
#include <stdio.h>
#if !defined(WIN32) && !defined(__APPLE__)
#include "list_visuals.cxx"
#endif

int width = 100;
int height = 100;
uchar *image;
Fl_Box *hint;

void make_image() {
  image = new uchar[3*width*height];
  uchar *p = image;
  for (int y = 0; y < height; y++) {
    double Y = double(y)/(height-1);
    for (int x = 0; x < width; x++) {
      double X = double(x)/(width-1);
      *p++ = uchar(255*((1-X)*(1-Y))); // red in upper-left
      *p++ = uchar(255*((1-X)*Y));	// green in lower-left
      *p++ = uchar(255*(X*Y));	// blue in lower-right
    }
  }
}

class Pens : public Fl_Box {
  void draw();
public:
  Pens(int X, int Y, int W, int H, const char* L)
  : Fl_Box(X,Y,W,H,L) {}
};
void Pens::draw() {
  // use every color in the gray ramp:
  for (int i = 0; i < 3*8; i++) {
    fl_color((Fl_Color)(FL_GRAY_RAMP+i));
    fl_line(x()+i, y(), x()+i, y()+h());
  }
}

Fl_Color c = FL_GRAY;
#define fullcolor_cell (FL_FREE_COLOR)

void cb1(Fl_Widget *, void *v) {
  c = fl_show_colormap(c);
  Fl_Box* b = (Fl_Box*)v;
  b->color(c);
  hint->labelcolor(fl_contrast(FL_BLACK,c));
  b->parent()->redraw();
}

void cb2(Fl_Widget *, void *v) {
  uchar r,g,b;
  Fl::get_color(c,r,g,b);
  if (!fl_color_chooser("New color:",r,g,b,3)) return;
  c = fullcolor_cell;
  Fl::set_color(fullcolor_cell,r,g,b);
  Fl_Box* bx = (Fl_Box*)v;
  bx->color(fullcolor_cell);
  hint->labelcolor(fl_contrast(FL_BLACK,fullcolor_cell));
  bx->parent()->redraw();
}

int main(int argc, char ** argv) {
  Fl::set_color(fullcolor_cell,145,159,170);
  Fl_Window window(400,400);
  Fl_Box box(30,30,340,340);
  box.box(FL_THIN_DOWN_BOX);
  c = fullcolor_cell;
  box.color(c);
  Fl_Box hintbox(40,40,320,30,"Pick background color with buttons:");
  hintbox.align(FL_ALIGN_INSIDE);
  hint = &hintbox;
  Fl_Button b1(120,80,180,30,"fl_show_colormap()");
  b1.callback(cb1,&box);
  Fl_Button b2(120,120,180,30,"fl_color_chooser()");
  b2.callback(cb2,&box);
  Fl_Box image_box(160,190,width,height,0);
  make_image();
  (new Fl_RGB_Image(image, width, height))->label(&image_box);
  Fl_Box b(160,310,120,30,"Example of fl_draw_image()");
  Pens p(60,180,3*8,120,"lines");
  p.align(FL_ALIGN_TOP);
  int i = 1;
  if (!Fl::args(argc,argv,i) || i < argc-1) {
    printf("usage: %s <switches> visual-number\n"
           " - : default visual\n"
           " r : call Fl::visual(FL_RGB)\n"
           " c : call Fl::own_colormap()\n",argv[0]);
#if !defined(WIN32) && !defined(__APPLE__)
    printf(" # : use this visual with an empty colormap:\n");
    list_visuals();
#endif
    puts(Fl::help);
    exit(1);
  }
  if (i!=argc) {
    if (argv[i][0] == 'r') {
      if (!Fl::visual(FL_RGB)) printf("Fl::visual(FL_RGB) returned false.\n");
    } else if (argv[i][0] == 'c') {
      Fl::own_colormap();
    } else if (argv[i][0] != '-') {
#if !defined(WIN32) && !defined(__APPLE__)
      int visid = atoi(argv[i]);
      fl_open_display();
      XVisualInfo templt; int num;
      templt.visualid = visid;
      fl_visual = XGetVisualInfo(fl_display, VisualIDMask, &templt, &num);
      if (!fl_visual) Fl::fatal("No visual with id %d",visid);
      fl_colormap = XCreateColormap(fl_display, RootWindow(fl_display,fl_screen),
                                    fl_visual->visual, AllocNone);
      fl_xpixel(FL_BLACK); // make sure black is allocated
#else
      Fl::fatal("Visual id's not supported on MSWindows or MacOS.");
#endif
    }
  }
  window.show(argc,argv);
  return Fl::run();
}

//
// End of "$Id$".
//
