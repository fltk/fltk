// Fl_Clock.H

#ifndef Fl_Clock_H
#define Fl_Clock_H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

// values for type:
#define FL_SQUARE_CLOCK		0
#define FL_ROUND_CLOCK		1
#define FL_ANALOG_CLOCK FL_SQUARE_CLOCK
#define FL_DIGITAL_CLOCK FL_SQUARE_CLOCK // nyi

class Fl_Clock : public Fl_Widget {
  int hour_, minute_, second_;
  ulong value_;
  void drawhands(Fl_Color,Fl_Color); // part of draw
protected:
  void draw(int, int, int, int);
  void draw();
  void _Fl_Clock();
  int handle(int);
public:
  Fl_Clock(int x,int y,int w,int h, const char *l = 0);
  Fl_Clock(uchar t,int x,int y,int w,int h, const char *l);
  ~Fl_Clock();
  void value(ulong v);	// set to this Unix time
  void value(int,int,int);	// set hour, minute, second
  ulong value() const {return value_;}
  int hour() const {return hour_;}
  int minute() const {return minute_;}
  int second() const {return second_;}
};

#endif

