#ifndef Fl_Hor_Slider_H
#define Fl_Hor_Slider_H

#include "Fl_Slider.H"

class Fl_Hor_Slider : public Fl_Slider {
public:
    Fl_Hor_Slider(int x,int y,int w,int h,const char *l=0)
	: Fl_Slider(x,y,w,h,l) {type(FL_HOR_SLIDER);}
};

#endif
