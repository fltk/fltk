#ifndef Fl_Nice_Slider_H
#define Fl_Nice_Slider_H

#include "Fl_Slider.H"

class Fl_Nice_Slider : public Fl_Slider {
public:
    Fl_Nice_Slider(int x,int y,int w,int h,const char *l=0)
	: Fl_Slider(x,y,w,h,l) {type(FL_VERT_NICE_SLIDER); box(FL_FLAT_BOX);}
};

#endif
