#ifndef Fl_Fill_Slider_H
#define Fl_Fill_Slider_H

#include "Fl_Slider.H"

class Fl_Fill_Slider : public Fl_Slider {
public:
    Fl_Fill_Slider(int x,int y,int w,int h,const char *l=0)
	: Fl_Slider(x,y,w,h,l) {type(FL_VERT_FILL_SLIDER);}
};

#endif
