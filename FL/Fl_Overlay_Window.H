// Fl_Overlay_Window.H

// A window using double-buffering and able to draw an overlay
// on top of that.  Uses the hardware to draw the overlay if
// possible, otherwise it just draws in the front buffer.

#ifndef Fl_Overlay_Window_H
#define Fl_Overlay_Window_H

#include "Fl_Double_Window.H"

class Fl_Overlay_Window : public Fl_Double_Window {
  friend class _Fl_Overlay;
  virtual void draw_overlay() = 0;
  Fl_Window *overlay_;
public:
  void show();
  void flush();
  void hide();
  void resize(int,int,int,int);
  ~Fl_Overlay_Window();
  int can_do_overlay();
  void redraw_overlay();
  Fl_Overlay_Window(int W, int H, const char *l=0)
    : Fl_Double_Window(W,H,l) {overlay_ = 0;}
  Fl_Overlay_Window(int X, int Y, int W, int H, const char *l=0)
    : Fl_Double_Window(X,Y,W,H,l) {overlay_ = 0;}
  void show(int a, char **b) {Fl_Double_Window::show(a,b);}
};

#endif
