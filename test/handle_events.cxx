//
// Event test program for the Fast Light Tool Kit (FLTK).
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

// Build: fltk-config --compile handle_events.cxx

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/names.h>
#include <stdio.h>

// define WINDOW_TYPE as either "Fl_Gl_Window" or "Fl_Double_Window"
#define WINDOW_TYPE Fl_Double_Window

// Class to handle events
class app : public WINDOW_TYPE {
protected:
  int handle(int) FL_OVERRIDE;
public:
  // storage for the last event
  int eventnum, ex, ey;
  const char *eventname;
  app(int X, int Y, int W, int H, const char *L = 0)
    : WINDOW_TYPE(X, Y, W, H, L) {
    eventname = NULL;
    eventnum = 0;
  }
  // evaluates and prints the current event
  void print_event(int ev) {
    eventnum++;
    ex = Fl::event_x();
    ey = Fl::event_y();
    int screen_num = Fl_Window::screen_num();
#if defined(FL_API_VERSION) && FL_API_VERSION >= 10400
    int scale = int(Fl::screen_scale(screen_num) * 100. + 0.5);
#else
    int scale = 100;
#endif
    eventname = fl_eventnames[ev];
    fprintf(stderr,
            "[%3d, win(%d,%d,%d,%d), screen %d, scale %3d%%] %-18.18s at (%4d, %4d)",
            eventnum, x(), y(), w(), h(), screen_num, scale, eventname, ex, ey);
    eventnum %= 999;
  }
};

// Event handling
int app::handle(int ev) {
  print_event(ev); // common for all events
  int res = WINDOW_TYPE::handle(ev);
  int buttons = Fl::event_buttons() >> 24; // bits: 1=left, 2=middle, 4=right button
  switch (ev) {
    case FL_PUSH:
      fprintf(stderr, ", button %d down, buttons = 0x%x", Fl::event_button(), buttons);
      res = 1;
      break;
    case FL_RELEASE:
      fprintf(stderr, ", button %d up,   buttons = 0x%x", Fl::event_button(), buttons);
      res = 1;
      break;
    case FL_MOUSEWHEEL:
      fprintf(stderr, ", dx = %d, dy = %d", Fl::event_dx(), Fl::event_dy());
      res = 1;
      break;
    case FL_ENTER:
    case FL_LEAVE:
      res = 1;
      break;
    case FL_MOVE:
    case FL_DRAG:
      fprintf(stderr, ",          mouse buttons = 0x%x", buttons);
      res = 1;
      break;
    case FL_KEYBOARD:
      if (Fl::event_text()[0] >= 'a' && Fl::event_text()[0] <= 'z') {
        fprintf(stderr, ", Text = '%s'", Fl::event_text());
        res = 1;
      } else { // "ignore" everything else
        fprintf(stderr, ", ignored '%s'", Fl::event_text());
      }
      break;
    case FL_KEYUP:
      res = 1;
      break;
    case FL_FOCUS:
    case FL_UNFOCUS:
      res = 1;
      break;
    default:
      break;
  }
  fprintf(stderr, "\n"); fflush(stderr);
  return res;
} /* end of handle() method */

// Quit button callback (closes the window)
void quit_cb(Fl_Button *b, void *) {
  b->window()->hide();
}

// main program
int main(int argc, char **argv) {
  app *win = new app(10, 10, 240, 240);
  Fl_Button *quit = new Fl_Button(90, 100, 60, 40, "Quit");
  quit->box(FL_THIN_UP_BOX);
  quit->callback((Fl_Callback *)quit_cb);
  win->end();
  win->resizable(win);
  win->show(argc, argv);
  return Fl::run();
}
