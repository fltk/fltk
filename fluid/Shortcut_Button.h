#include <FL/Fl_Button.H>

class Shortcut_Button : public Fl_Button {
public:
  int svalue;
  int handle(int);
  void draw();
  Shortcut_Button(int x, int y, int w, int h, const char* l = 0) :
    Fl_Button(x,y,w,h,l) {svalue = 0;}
};
