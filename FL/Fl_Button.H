// Fl_Button.H

#ifndef Fl_Button_H
#define Fl_Button_H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

// values for type()
#define FL_TOGGLE_BUTTON	1
#define FL_RADIO_BUTTON		(FL_RESERVED_TYPE+2)
#define FL_HIDDEN_BUTTON	3 // for Forms compatability

extern int fl_old_shortcut(const char*);

class Fl_Button : public Fl_Widget {

  int shortcut_;
  char value_;
  char oldval;
  uchar down_box_;

protected:

  virtual void draw();

public:

  virtual int handle(int);
  Fl_Button(int,int,int,int,const char * = 0);
  int value(int);
  char value() const {return value_;}
  int set() {return value(1);}
  int clear() {return value(0);}
  void setonly(); // this should only be called on FL_RADIO_BUTTONs
  int shortcut() const {return shortcut_;}
  void shortcut(int s) {shortcut_ = s;}
  Fl_Boxtype down_box() const {return (Fl_Boxtype)down_box_;}
  void down_box(Fl_Boxtype b) {down_box_ = b;}

  // back compatability:
  void shortcut(const char *s) {shortcut(fl_old_shortcut(s));}
  Fl_Color down_color() const {return selection_color();}
  void down_color(uchar c) {selection_color(c);}
};

#endif
