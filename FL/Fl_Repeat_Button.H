// Fl_Repeat_Button.H

// When held down, this generates callbacks repeatedly.
// Forms called this a "touch" button.

#ifndef Fl_Repeat_Button_H
#define Fl_Repeat_Button_H
#include "Fl_Button.H"

class Fl_Repeat_Button : public Fl_Button {
  static void repeat_callback(void *);
public:
  int handle(int);
  Fl_Repeat_Button(int x,int y,int w,int h,const char *l=0)
    : Fl_Button(x,y,w,h,l) {}
};

#endif
