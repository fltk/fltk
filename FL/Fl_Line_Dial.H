#ifndef Fl_Line_Dial_H
#define Fl_Line_Dial_H

#include "Fl_Dial.H"

class Fl_Line_Dial : public Fl_Dial {
public:
    Fl_Line_Dial(int x,int y,int w,int h, const char *l = 0)
	: Fl_Dial(x,y,w,h,l) {type(FL_LINE_DIAL);}
};

#endif
