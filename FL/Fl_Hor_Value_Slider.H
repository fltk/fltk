#ifndef Fl_Hor_Value_Slider_H
#define Fl_Hor_Value_Slider_H

#include "Fl_Value_Slider.H"

class Fl_Hor_Value_Slider : public Fl_Value_Slider {
public:
    Fl_Hor_Value_Slider(int x,int y,int w,int h,const char *l=0)
	: Fl_Value_Slider(x,y,w,h,l) {type(FL_HOR_SLIDER);}
};

#endif
