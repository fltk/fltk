#ifndef Fl_Radio_Round_Button_H
#define Fl_Radio_Round_Button_H

#include "Fl_Round_Button.H"

class Fl_Radio_Round_Button : public Fl_Round_Button {
public:
    Fl_Radio_Round_Button(int x,int y,int w,int h,const char *l=0)
	: Fl_Round_Button(x,y,w,h,l) {type(FL_RADIO_BUTTON);}
};

#endif
