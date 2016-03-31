//
// "$Id$"
//
// shapedwindow example source file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2014 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Tiled_Image.H>
#include <FL/Fl_Image_Surface.H>
#include "test/pixmaps/tile.xpm"


void cb(Fl_Widget *w, void *) {
  w->window()->hide();
}

class dragbox : public Fl_Box {
public:
  dragbox(int x, int y, int w, int h, const char *t=0) : Fl_Box(x,y,w,h,t) {};
  int handle(int event) {
    static int fromx, fromy, winx, winy;
    if (event == FL_PUSH) {
      fromx = Fl::event_x_root();
      fromy = Fl::event_y_root();
      winx = window()->x_root();
      winy = window()->y_root();
      return 1;
    }
    else if (event == FL_DRAG) {
      int deltax = Fl::event_x_root() - fromx;
      int deltay = Fl::event_y_root() - fromy;
      window()->position(winx + deltax, winy + deltay);
      return 1;
    }
    return Fl_Box::handle(event);
  }
};

const float factor = 1.3;

void shrink(Fl_Widget *wdgt, void *data)
{
  Fl_Window *win = wdgt->window();
  int old = win->w();
  win->size(old/factor, old/factor);
  if (win->w() <= *(int*)data) wdgt->deactivate();
}

void enlarge(Fl_Widget *wdgt, void *data)
{
  Fl_Window *win = wdgt->window();
  int old = win->w();
  win->size(old*factor, old*factor);
  ((Fl_Widget*)data)->activate();
}

Fl_RGB_Image* prepare_shape(int w)
{
  // draw a white circle with a hole in it on black background
  Fl_Image_Surface *surf = new Fl_Image_Surface(w, w);
  Fl_Surface_Device* current = Fl_Surface_Device::surface();
  surf->set_current();
  fl_color(FL_BLACK);
  fl_rectf(-1, -1, w+2, w+2);
  fl_color(FL_WHITE);
  fl_pie(2,2,w-4,w-4,0,360);
  fl_color(FL_BLACK);
  fl_pie(0.7*w,w/2,w/4,w/4,0,360);
  Fl_RGB_Image* img = surf->image();
  delete surf;
  current->set_current();
  return img; // return white image on black background
}

int main(int argc, char **argv) {
  int dim = 200;
  Fl_Double_Window *win = new Fl_Double_Window(100, 100, dim, dim, "Testing1");
  Fl_RGB_Image *img = prepare_shape(dim);
  win->shape(img);
  dragbox *box = new dragbox(0, 0, win->w(), win->h());
  box->image(new Fl_Tiled_Image(new Fl_Pixmap((const char * const *)tile_xpm)));
  Fl_Group *g = new Fl_Group(10, 20, 80, 20);
  g->box(FL_NO_BOX);
  Fl_Button *b = new Fl_Button(10, 20, 80, 20, "Close");
  b->callback(cb);
  g->end();
  g->resizable(NULL);
  g = new Fl_Group(60, 70, 80, 40, "Drag me");
  g->box(FL_NO_BOX);
  g->align(FL_ALIGN_TOP);
  Fl_Button *bs = new Fl_Button(60, 70, 80, 20, "Shrink");
  bs->callback(shrink, &dim);
  bs->deactivate();
  Fl_Button *be = new Fl_Button(60, 90, 80, 20, "Enlarge");
  be->callback(enlarge, bs);
  g->end();
  g->resizable(NULL);
  win->end();
  win->resizable(win);
  win->show(argc, argv);
  Fl::run();
  delete win;
  return 0;
}

//
// End of "$Id$".
//
