// Fl_Adjuster.H

// 3-button "slider", made for Nuke

#ifndef Fl_Adjuster_H
#define Fl_Adjuster_H

#ifndef Fl_Valuator_H
#include "Fl_Valuator.H"
#endif

class Fl_Adjuster : public Fl_Valuator {
  int drag;
  int ix;
  int soft_;
protected:
  void draw();
  int handle(int);
  void value_damage();
public:
  Fl_Adjuster(int x,int y,int w,int h,const char *l=0);
  void soft(int x) {soft_ = x;}
  int soft() const {return soft_;}
};

#endif
