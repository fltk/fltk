// Fl_Menu_Button.C

#ifndef Fl_Menu_Button_H
#define Fl_Menu_Button_H

#include "Fl_Menu_.H"

class Fl_Menu_Button : public Fl_Menu_ {
protected:
  void draw();
public:
  // values for type:
  enum {POPUP1 = 1, POPUP2, POPUP12, POPUP3, POPUP13, POPUP23, POPUP123};
  int handle(int);
  const Fl_Menu_Item* popup();
  Fl_Menu_Button(int,int,int,int,const char * =0);
};

#endif
