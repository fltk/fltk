// Fl_Output.H
// a non-editable subclass of Fl_Input_

#ifndef Fl_Output_H
#define Fl_Output_H

#include "Fl_Input_.H"

class Fl_Output : public Fl_Input_ {
protected:
  void draw();
public:
  int handle(int);
  Fl_Output(int x, int y, int w, int h, const char *l = 0)
    : Fl_Input_(x, y, w, h, l) {}
};

#endif 
