/*	Fl_Box.H	*/

#ifndef Fl_Box_H
#define Fl_Box_H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

class Fl_Box : public Fl_Widget {
    void draw();
public:
    Fl_Box(int x, int y, int w, int h, const char *l=0)
	: Fl_Widget(x,y,w,h,l) {}
    Fl_Box(Fl_Boxtype b, int x, int y, int w, int h, const char *l)
	: Fl_Widget(x,y,w,h,l) {box(b);}
};

#endif
