// Fl_Roller.H

// Rapid-App style knob

#ifndef Fl_Roller_H
#define Fl_Roller_H

#ifndef Fl_Valuator_H
#include "Fl_Valuator.H"
#endif

class Fl_Roller : public Fl_Valuator {
  void draw();
public:
  int handle(int);
  Fl_Roller(int X,int Y,int W,int H,const char* L=0);
};

#endif
