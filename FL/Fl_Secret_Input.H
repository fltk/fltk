#ifndef Fl_Secret_Input_H
#define Fl_Secret_Input_H

#include "Fl_Input.H"

class Fl_Secret_Input : public Fl_Input {
public:
    Fl_Secret_Input(int x,int y,int w,int h,const char *l = 0)
	: Fl_Input(x,y,w,h,l) {type(FL_SECRET_INPUT);}
};

#endif
