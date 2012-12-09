//
// "$Id$"
//
// Output test program for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Value_Input.H> // necessary for bug in mingw32?
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Input.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Multiline_Output.H>

Fl_Output *text;
Fl_Multiline_Output *text2;
Fl_Input *input;
Fl_Value_Slider *fonts;
Fl_Value_Slider *sizes;
Fl_Double_Window *window;

void font_cb(Fl_Widget *,void *) {
  text->textfont(int(fonts->value()));
  text->redraw();
  text2->textfont(int(fonts->value()));
  text2->redraw();
}

void size_cb(Fl_Widget *,void *) {
  text->textsize(int(sizes->value()));
  text->redraw();
  text2->textsize(int(sizes->value()));
  text2->redraw();
}

void input_cb(Fl_Widget *,void *) {
  text->value(input->value());
  text2->value(input->value());
}

int main(int argc, char **argv) {
  window = new Fl_Double_Window(400,400);

  input = new Fl_Input(50,375,350,25);
  input->static_value("The quick brown fox\njumped over\nthe lazy dog.");
  input->when(FL_WHEN_CHANGED);
  input->callback(input_cb);

  sizes = new Fl_Hor_Value_Slider(50,350,350,25,"Size");
  sizes->align(FL_ALIGN_LEFT);
  sizes->bounds(1,64);
  sizes->step(1);
  sizes->value(14);
  sizes->callback(size_cb);

  fonts = new Fl_Hor_Value_Slider(50,325,350,25,"Font");
  fonts->align(FL_ALIGN_LEFT);
  fonts->bounds(0,15);
  fonts->step(1);
  fonts->value(0);
  fonts->callback(font_cb);

  text2 = new Fl_Multiline_Output(100,150,200,100,"Fl_Multiline_Output");
  text2->value(input->value());
  text2->align(FL_ALIGN_BOTTOM);
  text2->tooltip("This is an Fl_Multiline_Output widget.");
  window->resizable(text2);

  text = new Fl_Output(100,90,200,30,"Fl_Output");
  text->value(input->value());
  text->align(FL_ALIGN_BOTTOM);
  text->tooltip("This is an Fl_Output widget.");

  window->end();
  window->show(argc,argv);
  return Fl::run();
}

//
// End of "$Id$".
//
