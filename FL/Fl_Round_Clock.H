#ifndef Fl_Round_Clock_H
#define Fl_Round_Clock_H

#include "Fl_Clock.H"

class Fl_Round_Clock : public Fl_Clock {
public:
    Fl_Round_Clock(int x,int y,int w,int h, const char *l = 0)
	: Fl_Clock(x,y,w,h,l) {type(FL_ROUND_CLOCK); box(FL_NO_BOX);}
};

#endif
