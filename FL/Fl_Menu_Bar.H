// Fl_Menu_Bar.H

// The implementation is in Fl_Menu.C

#ifndef Fl_Menu_Bar_H
#define Fl_Menu_Bar_H

#include "Fl_Menu_.H"

class Fl_Menu_Bar : public Fl_Menu_ {
protected:
    int handle(int);
    void draw();
public:
    Fl_Menu_Bar(int x,int y,int w,int h,const char *l=0)
      : Fl_Menu_(x,y,w,h,l) {}
};

#endif








