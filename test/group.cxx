//
// Fl_Group manipulation test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 2024 by Bill Spitzak and others.
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

// This test program creates two groups and moves widgets around
// between groups to test insert, append, remove, etc..

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Terminal.H>

// #include <vector>

// Globals for easier testing

const int ww = 520; // window width
const int wh = 200; // window height (w/o button and terminal)
const int th = 200; // initial terminal (tty) height

Fl_Double_Window *window;
Fl_Flex *g1, *g2;
Fl_Box *b1, *b2, *b3, *b4;
Fl_Terminal *tty;

void debug_group(Fl_Group *g) {
  const int nc = g->children();
  tty->printf("%s has %2d child%s\n", g->label(), nc, nc == 1 ? "" : "ren");
  for (int i = 0; i < g->children(); i++) {
    Fl_Widget *w = g->child(i);
    const char *lbl = w ? w->label() : "";
#if (0) // full info with child address
    tty->printf("    Child[%2d] (%p) = %s\n", i, (void *)w, lbl);
#else // short info
    tty->printf("    Child[%2d] = %s\n", i, lbl);
#endif
  }
}

// This button callback exercises multiple Fl_Group operations in a circle.
// Note to devs: make sure that the last action restores the original layout.

void button_cb(Fl_Widget *, void *) {
  static int move = 0;
  move++;
  tty->printf("\nMove %2d:\n", move);
  switch(move) {
    case  1: g1->insert(*b3, 0);    break;
    case  2: g1->insert(*b3, 2);    break;
    case  3: g1->insert(*b2, 1);    break;
    case  4: g1->insert(*b1, 5);    break;
    case  5: g1->insert(*b1, 1);    break;
    case  6: g1->insert(*b4, 3);    break;
    case  7: g1->insert(*b3, 2);    break; // no-op (same position)
    case  8: g2->add(b3);           break;
    case  9: g2->add(b3);           break; // no-op (same position)
    case 10: g2->add(b4);           break;
    case 11: g1->remove(b2);        break;
    case 12: g1->add(b2); move = 0; break; // last move: reset counter
    default:              move = 0; break; // safety: reset counter
  }
  debug_group(g1);
  debug_group(g2);
  g1->layout();
  g2->layout();
  window->redraw();
}

int main(int argc, char **argv) {
  window = new Fl_Double_Window(ww, wh + th + 100, "Fl_Group Test");

  g1 = new Fl_Flex(50, 20, ww - 80, wh / 2 - 20, "g1: ");
  g1->type(Fl_Flex::HORIZONTAL);
  g1->box(FL_FLAT_BOX);
  g1->color(FL_WHITE);
  g1->align(FL_ALIGN_LEFT);

  b1 = new Fl_Box(0, 0, 0, 0, "b1");
  b1->box(FL_FLAT_BOX);
  b1->color(FL_RED);

  b2 = new Fl_Box(0, 0, 0, 0, "b2");
  b2->box(FL_FLAT_BOX);
  b2->color(FL_GREEN);

  g1->end();

  g2 = new Fl_Flex(50, wh / 2 + 20, ww - 80, wh / 2 - 20, "g2: ");
  g2->type(Fl_Flex::HORIZONTAL);
  g2->box(FL_FLAT_BOX);
  g2->color(FL_WHITE);
  g2->align(FL_ALIGN_LEFT);

  b3 = new Fl_Box(0, 0, 0, 0, "b3");
  b3->box(FL_FLAT_BOX);
  b3->color(FL_BLUE);
  b3->labelcolor(FL_WHITE);

  b4 = new Fl_Box(0, 0, 0, 0, "b4");
  b4->box(FL_FLAT_BOX);
  b4->color(FL_YELLOW);

  g2->end();

  auto bt = new Fl_Button(10, wh + 20, ww - 20, 40, "Move children ...");
  bt->callback(button_cb);

  tty = new Fl_Terminal(10, wh + 80, ww - 20, th);

  window->end();
  window->resizable(tty);
  window->size_range(window->w(), window->h());
  window->show(argc, argv);

  tty->printf("sizeof(Fl_Widget)    = %3d\n", (int)sizeof(Fl_Widget));
  tty->printf("sizeof(Fl_Box)       = %3d\n", (int)sizeof(Fl_Box));
  tty->printf("sizeof(Fl_Button)    = %3d\n", (int)sizeof(Fl_Button));
  tty->printf("sizeof(Fl_Group)     = %3d\n", (int)sizeof(Fl_Group));
  tty->printf("sizeof(Fl_Window)    = %3d\n", (int)sizeof(Fl_Window));

  int idx = g2->children() + 3;
  tty->printf("child(n) out of range: g2->child(%d) = %p, children = %d\n",
              idx, (void *)g2->child(idx), g2->children());

  int ret = Fl::run();

  // reset pointers to give memory checkers a chance to test for leaks
  g1 = g2 = nullptr;
  tty = nullptr;
  b1 = b2 = b3 = b4 = nullptr;
  bt = nullptr;
  delete window;

  return ret;
}
