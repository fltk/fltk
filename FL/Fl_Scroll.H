// Fl_Scroll.H
// A scrolling area of child widgets.

#ifndef Fl_Scroll_H
#define Fl_Scroll_H

#include "Fl_Group.H"
#include "Fl_Scrollbar.H"

class Fl_Scroll : public Fl_Group {

  int xposition_, yposition_;
  int width_, height_;
  int oldx, oldy;
  static void hscrollbar_cb(Fl_Widget*, void*);
  static void scrollbar_cb(Fl_Widget*, void*);
  void fix_scrollbar_order();
  static void draw_clip(void*,int,int,int,int);
  void bbox(int&,int&,int&,int&);

protected:

  void draw();

public:

  Fl_Scrollbar scrollbar;
  Fl_Scrollbar hscrollbar;

  void resize(int,int,int,int);
  int handle(int);

  Fl_Scroll(int X,int Y,int W,int H,const char*l=0);

  enum { // values for type()
    HORIZONTAL = 1,
    VERTICAL = 2,
    BOTH = 3,
    ALWAYS_ON = 4,
    HORIZONTAL_ALWAYS = 5,
    VERTICAL_ALWAYS = 6,
    BOTH_ALWAYS = 7
  };

  int xposition() const {return xposition_;}
  int yposition() const {return yposition_;}
  void position(int, int);
};

#endif
