// Fl_Input.H

// This is the "user interface", it decodes user actions into what to
// do to the text.  See also Fl_Input_.H for text manipulation functions.

#ifndef Fl_Input_H
#define Fl_Input_H

#include "Fl_Input_.H"

class Fl_Input : public Fl_Input_ {
  int handle_key();
  int shift_position(int p);
  int shift_up_down_position(int p);
  void handle_mouse(int keepmark=0);
public:
  void draw();
  int handle(int);
  Fl_Input(int,int,int,int,const char * = 0);
};

#endif 
