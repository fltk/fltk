// Fl_Round_Button.H

// A Fl_Light_Button with a round down_box() and a red color.

#ifndef Fl_Round_Button_H
#define Fl_Round_Button_H

#include "Fl_Light_Button.H"

class Fl_Round_Button : public Fl_Light_Button {
public:
  Fl_Round_Button(int x,int y,int w,int h,const char *l = 0);
};

#endif
