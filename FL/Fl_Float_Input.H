#ifndef Fl_Float_Input_H
#define Fl_Float_Input_H

#include "Fl_Input.H"

class Fl_Float_Input : public Fl_Input {
public:
    Fl_Float_Input(int x,int y,int w,int h,const char *l = 0)
	: Fl_Input(x,y,w,h,l) {type(FL_FLOAT_INPUT);}
};

#endif
