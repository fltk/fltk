//
// Test program for Fl_Input_Choice and Fl_Choice
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Scheme_Choice.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Terminal.H>

#define TERMINAL_HEIGHT 150

// Globals
Fl_Terminal *G_tty = 0;

void active_cb(Fl_Widget *w, void *data) {
  Fl_Check_Button *b = (Fl_Check_Button *)w;
  Fl_Group *g = (Fl_Group *)data;
  if (b->value()) {
    g->activate();
    G_tty->printf("activate group\n");
  } else {
    g->deactivate();
    G_tty->printf("deactivate group\n");
  }
  g->redraw();
  if (b->changed()) {
    G_tty->printf("Callback: changed() is set\n");
    b->clear_changed();
  }
}

void input_choice_cb(Fl_Widget *, void *data) {
  Fl_Input_Choice *in = (Fl_Input_Choice *)data;
  G_tty->printf("Fl_Input_Choice value='%s'\n", (const char *)in->value());
}

void choice_cb(Fl_Widget *, void *data) {
  Fl_Choice *in = (Fl_Choice *)data;
  G_tty->printf("Fl_Choice       value='%d'\n", in->value());
}

int main(int argc, char **argv) {
  Fl_Window *win = new Fl_Double_Window(300, 200 + TERMINAL_HEIGHT);

  G_tty = new Fl_Terminal(0, 200, win->w(), TERMINAL_HEIGHT);

  // this group can be activated and deactivated:
  Fl_Group *active_group = new Fl_Group(0, 0, 300, 120);

  // all *_Choice widgets must be aligned for easier visual comparison:

  Fl_Input_Choice *in = new Fl_Input_Choice(180, 40, 100, 25, "Fl_Input_Choice:");
  in->callback(input_choice_cb, (void *)in);
  in->add("one");
  in->add("two");
  in->add("three");
  in->value(0);

  Fl_Choice *choice = new Fl_Choice(180, 70, 100, 25, "Fl_Choice:");
  choice->callback(choice_cb, (void *)choice);
  choice->add("aaa");
  choice->add("bbb");
  choice->add("ccc");
  choice->value(1);

  active_group->end();

  // Interactive control of scheme
  Fl_Scheme_Choice *sch = new Fl_Scheme_Choice(180, 120, 100, 25, "Choose scheme:");
  sch->visible_focus(0);

  Fl_Check_Button *active = new Fl_Check_Button(50, 160, 160, 30, "Activate/deactivate");
  active->callback(active_cb, (void *)active_group);
  active->value(1);

  win->end();
  win->resizable(win);
  win->size_range(200, 160);
  win->show(argc, argv);
  Fl::run();
  delete win;
  return 0;
}
