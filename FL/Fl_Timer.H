// Fl_Timer.H

// Emulate the forms Timer widget

#ifndef Fl_Timer_H
#define Fl_Timer_H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

// values for type():
#define FL_NORMAL_TIMER		0
#define FL_VALUE_TIMER		1
#define FL_HIDDEN_TIMER		2

class Fl_Timer : public Fl_Widget {
  static void stepcb(void *);
  void step();
  char on, direction_;
  double delay, total;
  long lastsec,lastusec;
protected:
  void draw();
public:
  int handle(int);
  Fl_Timer(uchar t,int x,int y,int w,int h, const char *l);
  ~Fl_Timer();
  void value(double);
  double value() const {return delay>0.0?delay:0.0;}
  char direction() const {return direction_;}
  void direction(char d) {direction_ = d;}
  char suspended() const {return !on;}
  void suspended(char d);
};

#endif

