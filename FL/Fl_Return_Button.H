// Fl_Return_Button.H

// Just like a normal button except it has a built-in shortcut for
// the Enter key and it draws a user-friendly arrow on the button.

#ifndef Fl_Return_Button_H
#define Fl_Return_Button_H
#include "Fl_Button.H"

class Fl_Return_Button : public Fl_Button {
protected:
  void draw();
public:
  int handle(int);
  Fl_Return_Button(int x,int y,int w,int h,const char *l=0)
    : Fl_Button(x,y,w,h,l) {}
};

#endif
