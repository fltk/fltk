#ifndef Fl_Select_Browser_H
#define Fl_Select_Browser_H

#include "Fl_Browser.H"

class Fl_Select_Browser : public Fl_Browser {
public:
    Fl_Select_Browser(int x,int y,int w,int h,const char *l=0)
	: Fl_Browser(x,y,w,h,l) {type(FL_SELECT_BROWSER);}
};

#endif
