// Fl_Slider.H

#ifndef Fl_Slider_H
#define Fl_Slider_H

#ifndef Fl_Valuator_H
#include "Fl_Valuator.H"
#endif

// values for type(), lowest bit indicate horizontal:
#define FL_VERT_SLIDER		0
#define FL_HOR_SLIDER		1
#define FL_VERT_FILL_SLIDER	2
#define FL_HOR_FILL_SLIDER	3
#define FL_VERT_NICE_SLIDER	4
#define FL_HOR_NICE_SLIDER	5

class Fl_Slider : public Fl_Valuator {

  float slider_size_;
  uchar slider_;
  void _Fl_Slider();
  void draw_bg(int, int, int, int);

protected:

  // these allow subclasses to put the slider in a smaller area:
  void draw(int, int, int, int);
  int handle(int, int, int, int, int);

public:

  void draw();
  int handle(int);
  Fl_Slider(int x,int y,int w,int h, const char *l = 0);
  Fl_Slider(uchar t,int x,int y,int w,int h, const char *l);

  int scrollvalue(int windowtop,int windowsize,int first,int totalsize);
  void bounds(double a, double b);
  float slider_size() const {return slider_size_;}
  void slider_size(double v);
  Fl_Boxtype slider() const {return (Fl_Boxtype)slider_;}
  void slider(Fl_Boxtype c) {slider_ = c;}
};

#endif
