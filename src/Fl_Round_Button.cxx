// Fl_Round_Button.C

// A subclass of Fl_Button that always draws as a round circle.  This
// circle is smaller than the widget size and can be surrounded by
// another box type, for compatability with Forms.

#include <FL/Fl.H>
#include <FL/Fl_Round_Button.H>

Fl_Round_Button::Fl_Round_Button(int x,int y,int w,int h, const char *l)
: Fl_Light_Button(x,y,w,h,l) {
  box(FL_NO_BOX);
  down_box(FL_ROUND_DOWN_BOX);
  selection_color(FL_RED);
}
