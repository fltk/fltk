//
// Fl_Flex demo program for the Fast Light Tool Kit (FLTK).
//
// Copyright 2020 by Karsten Pedersen
// Copyright 2022 by Bill Spitzak and others.
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
#include <FL/Fl_Flex.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>

#define DEBUG_GROUP (0)

void debug_group(Fl_Group *g) {
#if (DEBUG_GROUP)
  printf("\nFl_Group (%p) has %d children:\n", g, g->children());
  for (int i = 0; i < g->children(); i++) {
    Fl_Widget *c = g->child(i);
    printf("  child %2d: hidden = %-5s, (x,y,w,h) = (%3d, %3d, %3d, %3d), label = '%s'\n",
           i, c->visible() ? "false" : "true", c->x(), c->y(), c->w(), c->h(),
           c->label() ? c->label() : "(null)");
  }
#endif
} // debug_group

Fl_Button *create_button(const char *caption) {
  Fl_Button *rtn = new Fl_Button(0, 0, 120, 30, caption);
  rtn->color(fl_rgb_color(225, 225, 225));
  return rtn;
}

void toggle_cb(Fl_Widget *w, void *v) {
  static Fl_Box *b = 0;
  Fl_Widget *o = (Fl_Widget *)v;
  Fl_Flex *flex = (Fl_Flex *)o->parent();
  if (o->visible()) {
    o->hide();
    w->label("show OK button");
    flex->child(1)->hide();         // hide Box
  } else {
    o->show();
    w->label("hide OK button");
    flex->child(1)->show();         // show Box
  }
  flex->layout();

  debug_group(flex);

  // Yet another test: modify the first (top) Fl_Flex widget

  flex = (Fl_Flex *)(flex->parent()->child(0));
  Fl_Group::current(0);
  if (!b) {
    b = new Fl_Box(0, 0, 0, 0, "Box3");
    flex->insert(*b, flex->children() - 1);
  } else {
    delete b;
    b = 0;
  }
  flex->layout();
  debug_group(flex);
}

Fl_Flex *create_row() {
  Fl_Flex *row = new Fl_Flex(Fl_Flex::ROW);
  {
    Fl_Button *toggle = create_button("hide OK button");
    toggle->tooltip("hide() or show() OK button");
    Fl_Box *box2 = new Fl_Box(0, 0, 120, 10, "Box2");
    Fl_Button * okay = create_button("OK");
    new Fl_Input(0, 0, 120, 10, "");

    toggle->callback(toggle_cb, okay);

    Fl_Flex *col2 = new Fl_Flex(Fl_Flex::COLUMN);
    {
      create_button("Top2");
      create_button("Bottom2");
      col2->end();
      col2->margin(0, 5);
      col2->box(FL_FLAT_BOX);
      col2->color(fl_rgb_color(255, 128, 128));
    }

    row->fixed(box2, 50);
    row->fixed(col2, 100);
    row->end();

    // TEST
    row->box(FL_DOWN_BOX);
    row->color(FL_GREEN);
  }

  return row;
}

int main(int argc, char **argv) {

  Fl_Window *window = new Fl_Double_Window(100, 100, "Simple GUI Example");
  Fl_Flex *col = new Fl_Flex(5, 5, 90, 90, Fl_Flex::COLUMN);
  Fl_Flex *row1 = new Fl_Flex(Fl_Flex::ROW);
  row1->color(FL_YELLOW);
  row1->box(FL_FLAT_BOX);
  create_button("Cancel");
  new Fl_Box(0, 0, 120, 10, "Box1");
  create_button("OK");
  new Fl_Input(0, 0, 120, 10, "");

  Fl_Flex *col1 = new Fl_Flex(Fl_Flex::COLUMN);
  create_button("Top1");
  create_button("Bottom1");
  col1->box(FL_FLAT_BOX);
  col1->color(fl_rgb_color(255, 128, 128));
  col1->margin(5, 5);
  col1->end();
  row1->end();

  col->fixed(create_row(), 90); // sets height of created (anonymous) row #2

  create_button("Something1"); // "row" #3

  Fl_Flex *row4 = new Fl_Flex(Fl_Flex::ROW);
  Fl_Button *cancel = create_button("Cancel");
  Fl_Button *ok = create_button("OK");
  new Fl_Input(0, 0, 120, 10, "");
  row4->fixed(cancel, 100);
  row4->fixed(ok, 100);
  row4->end();

  create_button("Something2"); // "row" #5

  col->fixed(row4, 30);
  col->margin(6, 10, 6, 10);
  col->gap(6);
  col->end();

  window->resizable(col);
  window->color(fl_rgb_color(160, 180, 240));
  window->box(FL_FLAT_BOX);
  window->end();

  window->size_range(550, 330);
  window->resize(0, 0, 640, 480);
  window->show(argc, argv);

  int ret = Fl::run();
  delete window; // not necessary but useful to test for memory leaks
  return ret;
}
