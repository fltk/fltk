// Fl_Check_Button.C

// A subclass of Fl_Button that always draws as a diamond box.  This
// diamond is smaller than the widget size and can be surchecked by
// another box type, for compatability with Forms.

#include <FL/Fl.H>
#include <FL/Fl_Check_Button.H>

Fl_Check_Button::Fl_Check_Button(int x, int y, int w, int h, const char *l)
: Fl_Light_Button(x, y, w, h, l) {
  box(FL_NO_BOX);
  down_box(FL_DIAMOND_DOWN_BOX);
  selection_color(FL_RED);
}
