// Fl_Single_Window.H

#ifndef Fl_Single_Window_H
#define Fl_Single_Window_H

#include "Fl_Window.H"

class Fl_Single_Window : public Fl_Window {
public:
  void show();
  void show(int a, char **b) {Fl_Window::show(a,b);}
  void flush();
  Fl_Single_Window(int W, int H, const char *l=0)
    : Fl_Window(W,H,l) {}
  Fl_Single_Window(int X, int Y, int W, int H, const char *l=0)
    : Fl_Window(X,Y,W,H,l) {}
  int make_current();
};

#endif
