// Fl_Scrollbar.H

#ifndef Fl_Scrollbar_H
#define Fl_Scrollbar_H

#include "Fl_Slider.H"

class Fl_Scrollbar : public Fl_Slider {

  int linesize_;
  int pushed_;
  void draw();
  int handle(int);
  static void timeout_cb(void*);
  void increment_cb();

public:

  Fl_Scrollbar(int x,int y,int w,int h, const char *l = 0);

  int value() {return int(Fl_Slider::value());}
  int value(int position, int size, int top, int total) {
    return scrollvalue(position, size, top, total);
  }
  int linesize() const {return linesize_;}
  void linesize(int i) {linesize_ = i;}

};

#endif
