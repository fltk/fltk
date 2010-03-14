//
// "$Id: curve.cxx 6615 2009-01-01 16:35:13Z matt $"
//
// Curve test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
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
#include <FL/fl_ask.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Toggle_Button.H>
//#include <FL/Fl_Printer.H>
#include <FL/Fl_Device.H>
#include <FL/Fl_Pixmap.H>
#include "pixmaps/porsche.xpm"
Fl_Pixmap porsche (porsche_xpm);
double args[9] = {
  20,20, 50,200, 100,20, 200,200, 0};
const char* name[9] = {
  "X0", "Y0", "X1", "Y1", "X2", "Y2", "X3", "Y3", "rotate"};

int points;

#include <FL/Fl_Box.H>
class Drawing : public Fl_Window {
  void draw() {
    fl_push_clip(0,0,w(),h());
    fl_color(FL_DARK3);
    fl_rectf(0,0,w(),h());
    fl_color(FL_RED);
    fl_font(FL_HELVETICA, 14);
    //                        tau    epsilon  chi    tau
    static unsigned utfs[4] = {0x3c4, 0x3b5, 0x3c7, 0x3c4};
    char utf8[40] = "test UTF ";
    for(int i = 0; i < 4; i++) {
      char buf[5];
      int l = fl_utf8encode(utfs[i], buf); buf[l] = 0;
      strcat(utf8, buf);
      }
    fl_draw(utf8,  5,  15);
    fl_color(FL_BLACK);
    fl_font(FL_HELVETICA, 24);
    fl_draw("bottom clipped text", 5, h() + 4);
    fl_line_style(FL_SOLID, 0);
    fl_rect(x()+200,y()+10,64,64);
    porsche.draw(x()+200,y()+10,64,64);
    fl_push_matrix();
    if (args[8]) {
      fl_translate(w()/2.0, h()/2.0);
      fl_rotate(args[8]);
      fl_translate(-(w()/2.0), -(h()/2.0));
    }
    fl_translate(x(),y());
    if (!points) {
    fl_color(FL_WHITE);
    fl_begin_complex_polygon();
    fl_curve(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]);
    fl_end_complex_polygon();
    }
    fl_color(FL_BLACK);
    fl_begin_line();
    fl_vertex(args[0],args[1]);
    fl_vertex(args[2],args[3]);
    fl_vertex(args[4],args[5]);
    fl_vertex(args[6],args[7]);
    fl_end_line();
    fl_color(points ? FL_WHITE : FL_RED);
    points ? fl_begin_points() : fl_begin_line();
    fl_curve(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]);
    points ? fl_end_points() : fl_end_line();
    fl_pop_matrix();
    fl_pop_clip();
  }
public:
  Drawing(int X,int Y,int W,int H) : Fl_Window(X,Y,W,H) {}
};

Drawing *d;

void points_cb(Fl_Widget* o, void*) {
  points = ((Fl_Toggle_Button*)o)->value();
  d->redraw();
}

void slider_cb(Fl_Widget* o, void* v) {
  Fl_Slider* s = (Fl_Slider*)o;
  args[long(v)] = s->value();
  d->redraw();
}

#include <FL/Fl_Printer.H>
void print_cb(Fl_Widget* o, void* v) {
  int w, h;
  Drawing *d = (Drawing *)v;
  Fl_Window *win = o->window();
  // start print job and one page
  Fl_Printer printer;
  if ( printer.start_job(1) ) {
#if defined(__APPLE__) || defined(WIN32)
    fl_alert("error starting print job");
#else
    fl_alert("error starting print job\n"
    	     "this platform doesn't support printing yet");
#endif
    return;
  }
  if (printer.start_page() ) return;
  // draw the printable area border
  printer.printable_rect(&w, &h);
  fl_color(FL_GRAY);
  fl_line_style(FL_DOT, 0);
  fl_rect(0,0,w,h);  
  fl_line_style(FL_SOLID, 0);
  //print the full window at top left of page
  printer.print_widget(win);
  //print the shrinked Drawing custom widget at right of page
  printer.scale(.6,.6);
  printer.printable_rect(&w, &h);
  printer.origin(w - d->w(), 100);
  printer.print_widget(d);
  //print the print button at bottom left
  printer.scale(1,1);
  printer.printable_rect(&w, &h);
  printer.print_widget(o, 0, h - o->h() );
  //print the scaled window at bottom right
  printer.scale(.5,.5);
  printer.printable_rect(&w, &h);
  printer.print_widget(win, w - win->w(), h - win->h());

  // close page and print job
  printer.end_page();
  printer.end_job();
}

int main(int argc, char** argv) {
  Fl_Double_Window window(300,555);
  Fl_Double_Window window2(5,5,290,290);
  Drawing drawing(5,5,280,280);
  drawing.end();
  window2.end();
  window2.box(FL_DOWN_BOX);
  d = &drawing;

  int y = 300;
  for (int n = 0; n<9; n++) {
    Fl_Slider* s = new Fl_Hor_Value_Slider(50,y,240,25,name[n]); y += 25;
    s->minimum(0); s->maximum(280);
    if (n == 8) s->maximum(360);
    s->step(1);
    s->value(args[n]);
    s->align(FL_ALIGN_LEFT);
    s->callback(slider_cb, (void*)n);
  }
  Fl_Toggle_Button but(50,y,50,25,"points");
  but.callback(points_cb);
  
  Fl_Button pbut(110, y, 50, 25, "Print");
  pbut.callback(print_cb, d);

  window.end();
  window.show(argc,argv);
  return Fl::run();
}

//
// End of "$Id: curve.cxx 6615 2009-01-01 16:35:13Z matt $".
//
