#ifndef Fl_Radio_Button_H
#define Fl_Radio_Button_H

#include "Fl_Button.H"

class Fl_Radio_Button : public Fl_Button {
public:
    Fl_Radio_Button(int x,int y,int w,int h,const char *l=0)
	: Fl_Button(x,y,w,h,l) {type(FL_RADIO_BUTTON);}
};

#endif
