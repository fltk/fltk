// Fl_Pack.H

#ifndef Fl_Pack_H
#define Fl_Pack_H

#include <FL/Fl_Group.H>

class Fl_Pack : public Fl_Group {
  int spacing_;
public:
  enum { // values for type(int)
    VERTICAL = 0,
    HORIZONTAL = 1
  };
  void draw();
  Fl_Pack(int x,int y,int w ,int h,const char *l = 0);
  int spacing() const {return spacing_;}
  void spacing(int i) {spacing_ = i;}
  uchar horizontal() const {return type();}
};

#endif
