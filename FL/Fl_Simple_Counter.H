#ifndef Fl_Simple_Counter_H
#define Fl_Simple_Counter_H

#include "Fl_Counter.H"

class Fl_Simple_Counter : public Fl_Counter {
public:
    Fl_Simple_Counter(int x,int y,int w,int h, const char *l = 0)
	: Fl_Counter(x,y,w,h,l) {type(FL_SIMPLE_COUNTER);}
};

#endif
