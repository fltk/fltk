// Fl_Light_Button.H

// Subclass of Fl_Button where the "box" indicates whether it is
// pushed or not, and the "down box" is drawn small and square on
// the left to indicate the current state.

// The default down_box of zero draws a rectangle designed to look
// just like Flame's buttons.

#ifndef Fl_Light_Button_H
#define Fl_Light_Button_H

#include "Fl_Button.H"

class Fl_Light_Button : public Fl_Button {
protected:
    virtual void draw();
public:
    virtual int handle(int);
    Fl_Light_Button(int x,int y,int w,int h,const char *l = 0);
};

#endif
