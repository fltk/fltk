// Fl_Choice.H

// Popup menu with last-picked item displayed in button

#ifndef Fl_Choice_H
#define Fl_Choice_H

#include "Fl_Menu_.H"

class Fl_Choice : public Fl_Menu_ {
protected:
  void draw();
public:
  int handle(int);
  Fl_Choice(int,int,int,int,const char * = 0);
  int value(int i);
  int value() const {return Fl_Menu_::value();}
};

#endif
