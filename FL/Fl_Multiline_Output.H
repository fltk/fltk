#ifndef Fl_Multiline_Output_H
#define Fl_Multiline_Output_H

#include "Fl_Output.H"

class Fl_Multiline_Output : public Fl_Output {
public:
    Fl_Multiline_Output(int x,int y,int w,int h,const char *l = 0)
	: Fl_Output(x,y,w,h,l) {type(FL_MULTILINE_INPUT);}
};

#endif
