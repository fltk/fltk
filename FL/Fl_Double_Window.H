// Fl_Double_Window.H

#ifndef Fl_Double_Window_H
#define Fl_Double_Window_H

#include "Fl_Window.H"

class Fl_Double_Window : public Fl_Window {
public:
  void show();
  void show(int a, char **b) {Fl_Window::show(a,b);}
  void flush();
  void resize(int,int,int,int);
  void hide();
  ~Fl_Double_Window();
  Fl_Double_Window(int W, int H, const char *l = 0) : Fl_Window(W,H,l) {}
  Fl_Double_Window(int X, int Y, int W, int H, const char *l = 0)
    : Fl_Window(X,Y,W,H,l) {}
};

#endif
