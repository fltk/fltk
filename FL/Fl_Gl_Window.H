#ifndef Fl_Gl_Window_H
#define Fl_Gl_Window_H

#include "Fl_Window.H"

class Fl_Gl_Choice; // structure to hold result of glXChooseVisual

class Fl_Gl_Window : public Fl_Window {

  int mode_;
  const int *alist;
  Fl_Gl_Choice *g;
  void * context; // actually a GLXContext
  char valid_;
  char damage1_; // damage() of back buffer
  virtual void draw_overlay();
  void init();

  void *overlay;
  void make_overlay();
  friend class _Fl_Gl_Overlay;

  static int can_do(int, const int *);
  int mode(int, const int *);

public:

  void show();
  void show(int a, char **b) {Fl_Window::show(a,b);}
  void flush();
  void hide();
  void resize(int,int,int,int);

  char valid() const {return valid_;}
  void valid(char i) {valid_ = i;}
  void invalidate();

  static int can_do(int i) {return can_do(i,0);}
  static int can_do(const int *i) {return can_do(0, i);}
  int can_do() {return can_do(mode_,alist);}
  Fl_Mode mode() const {return (Fl_Mode)mode_;}
  int mode(int a) {return mode(a,0);}
  int mode(const int *a) {return mode(0, a);}

  int can_do_overlay();
  void redraw_overlay();
  void hide_overlay();

  void make_current();
  void make_overlay_current();
  void swap_buffers();
  void ortho();

  ~Fl_Gl_Window();
  Fl_Gl_Window(int W, int H, const char *l=0) : Fl_Window(W,H,l) {init();}
  Fl_Gl_Window(int X, int Y, int W, int H, const char *l=0)
    : Fl_Window(X,Y,W,H,l) {init();}
};

#endif
