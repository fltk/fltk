// Fl_Positioner.H

// The positioner widget from Forms, gives 2D input

#ifndef Fl_Positioner_H
#define Fl_Positioner_H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

class Fl_Positioner : public Fl_Widget {

  double xmin, ymin;
  double xmax, ymax;
  double xvalue_, yvalue_;
  double xstep_, ystep_;

protected:

  // these allow subclasses to put the dial in a smaller area:
  void draw(int, int, int, int);
  int handle(int, int, int, int, int);
  void draw();

public:

  int handle(int);
  Fl_Positioner(int x,int y,int w,int h, const char *l=0);
  double xvalue() const {return xvalue_;}
  double yvalue() const {return yvalue_;}
  int xvalue(double);
  int yvalue(double);
  int value(double,double);
  void xbounds(double, double);
  double xminimum() const {return xmin;}
  void xminimum(double a) {xbounds(a,xmax);}
  double xmaximum() const {return xmax;}
  void xmaximum(double a) {xbounds(xmin,a);}
  void ybounds(double, double);
  double yminimum() const {return ymin;}
  void yminimum(double a) {ybounds(a,ymax);}
  double ymaximum() const {return ymax;}
  void ymaximum(double a) {ybounds(ymin,a);}
  void xstep(double a) {xstep_ = a;}
  void ystep(double a) {ystep_ = a;}

};

#endif

