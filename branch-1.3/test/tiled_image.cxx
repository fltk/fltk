//
// "$Id$"
//
// Fl_Tiled_Image test program for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Tiled_Image.H>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pixmaps/tile.xpm"

Fl_Button *b;
Fl_Double_Window *w;

void button_cb(Fl_Widget *,void *) {
  w->hide();
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
  Fl_Group group(0,0,400,400);
  group.image(new Fl_Tiled_Image(new Fl_Pixmap((const char * const *)tile_xpm)));
  group.align(FL_ALIGN_INSIDE);

  Fl_Button b(340,365,50,25,"Close"); ::b = &b;
  b.callback(button_cb);

  group.end();

  window.resizable(group);
  window.end();
  window.show(argc, argv);

  return Fl::run();
}

//
// End of "$Id$".
//
