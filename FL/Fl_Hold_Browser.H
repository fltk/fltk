#ifndef Fl_Hold_Browser_H
#define Fl_Hold_Browser_H

#include "Fl_Browser.H"

class Fl_Hold_Browser : public Fl_Browser {
public:
    Fl_Hold_Browser(int x,int y,int w,int h,const char *l=0)
	: Fl_Browser(x,y,w,h,l) {type(FL_HOLD_BROWSER);}
};

#endif
