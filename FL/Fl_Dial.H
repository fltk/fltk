// Fl_Dial.H

// A circular dial control, like xv uses.  From Forms.

#ifndef Fl_Dial_H
#define Fl_Dial_H

#ifndef Fl_Valuator_H
#include "Fl_Valuator.H"
#endif

// values for type():
#define FL_NORMAL_DIAL	0
#define FL_LINE_DIAL	1
#define FL_FILL_DIAL	2

#define FL_DIAL_CW	0
#define FL_DIAL_CCW	1

class Fl_Dial : public Fl_Valuator {

  short a1,a2;
  uchar direction_;

protected:

  // these allow subclasses to put the dial in a smaller area:
  void draw(int, int, int, int);
  int handle(int, int, int, int, int);
  void draw();

public:

  int handle(int);
  Fl_Dial(int x,int y,int w,int h, const char *l = 0);
  void angles(short a, short b) {a1=a; a2=b;}
  void direction(uchar d) {direction_ = d;}
  uchar direction() const {return direction_;}

};

#endif

