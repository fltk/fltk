// Fl_Menu_Window.H

// This is the window type used by Fl_Menu to make the pop-ups.  It
// draws in the overlay planes, if possible, and does other fun things
// such as turning on override_redirect.

#ifndef Fl_Menu_Window_H
#define Fl_Menu_Window_H

#include "Fl_Single_Window.H"

class Fl_Menu_Window : public Fl_Single_Window {
  enum {NO_OVERLAY = 128};
public:
  void show();
  void erase();
  void flush();
  void hide();
  int overlay() {return !(flags()&NO_OVERLAY);}
  void set_overlay() {clear_flag(NO_OVERLAY);}
  void clear_overlay() {set_flag(NO_OVERLAY);}
  ~Fl_Menu_Window();
  Fl_Menu_Window(int W, int H, const char *l = 0)
    : Fl_Single_Window(W,H,l) {}
  Fl_Menu_Window(int X, int Y, int W, int H, const char *l = 0)
    : Fl_Single_Window(X,Y,W,H,l) {}
};

#endif
