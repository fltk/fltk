//
// "$Id$"
//
// Fl_Image test program for the Fast Light Tool Kit (FLTK).
//
// Notice that Fl_Image is for a static, multiple-reuse image, such
// as an icon or postage stamp.  Use fl_draw_image to go directly
// from an buffered image that changes often.
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
#include <FL/Fl_Button.H>
#include <FL/Fl_Image.H>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int width = 100;
int height = 100;
uchar *image;

void make_image() {
  image = new uchar[4*width*height];
  uchar *p = image;
  for (int y = 0; y < height; y++) {
    double Y = double(y)/(height-1);
    for (int x = 0; x < width; x++) {
      double X = double(x)/(width-1);
      *p++ = uchar(255*((1-X)*(1-Y))); // red in upper-left
      *p++ = uchar(255*((1-X)*Y));	// green in lower-left
      *p++ = uchar(255*(X*Y));	// blue in lower-right
      X -= 0.5;
      Y -= 0.5;
      int alpha = (int)(255 * sqrt(X * X + Y * Y));
      if (alpha < 255) *p++ = uchar(alpha);	// alpha transparency
      else *p++ = 255;
      Y += 0.5;
    }
  }
}

#include <FL/Fl_Toggle_Button.H>

Fl_Toggle_Button *leftb,*rightb,*topb,*bottomb,*insideb,*overb,*inactb;
Fl_Button *b;
Fl_Double_Window *w;

void button_cb(Fl_Widget *,void *) {
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

#include <FL/x.H>
#if !defined(WIN32) && !defined(__APPLE__)
#include "list_visuals.cxx"
#endif

int visid = -1;
int arg(int argc, char **argv, int &i) {
  if (argv[i][1] == 'v') {
    if (i+1 >= argc) return 0;
    visid = atoi(argv[i+1]);
    i += 2;
    return 2;
  }
  return 0;
}

int main(int argc, char **argv) {
#if !defined(WIN32) && !defined(__APPLE__)
  int i = 1;

  Fl::args(argc,argv,i,arg);

  if (visid >= 0) {
    fl_open_display();
    XVisualInfo templt; int num;
    templt.visualid = visid;
    fl_visual = XGetVisualInfo(fl_display, VisualIDMask, &templt, &num);
    if (!fl_visual) {
      fprintf(stderr, "No visual with id %d, use one of:\n",visid);
      list_visuals();
      exit(1);
    }
    fl_colormap = XCreateColormap(fl_display, RootWindow(fl_display,fl_screen),
				fl_visual->visual, AllocNone);
    fl_xpixel(FL_BLACK); // make sure black is allocated in overlay visuals
  } else {
    Fl::visual(FL_RGB);
  }
#endif

  Fl_Double_Window window(400,400); ::w = &window;
  window.color(FL_WHITE);
  Fl_Button b(140,160,120,120,"Image w/Alpha"); ::b = &b;

  Fl_RGB_Image *rgb;
  Fl_Image *dergb;

  make_image();
  rgb = new Fl_RGB_Image(image, width, height,4);
  dergb = rgb->copy();
  dergb->inactive();

  b.image(rgb);
  b.deimage(dergb);

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
  window.resizable(window);
  window.end();
  window.show(argc, argv);
  return Fl::run();
}

//
// End of "$Id$".
//
