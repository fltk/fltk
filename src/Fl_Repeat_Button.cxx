// Fl_Repeat_Button.C

#include <FL/Fl.H>
#include <FL/Fl_Repeat_Button.H>

#define INITIALREPEAT .5
#define REPEAT .1

void Fl_Repeat_Button::repeat_callback(void *v) {
  Fl_Button *b = (Fl_Button*)v;
  Fl::add_timeout(REPEAT,repeat_callback,b);
  b->do_callback();
}

int Fl_Repeat_Button::handle(int event) {
  int newval;
  switch (event) {
  case FL_RELEASE:
    newval = 0; goto J1;
  case FL_PUSH:
  case FL_DRAG:
    newval = Fl::event_inside(this);
  J1:
    if (value(newval)) {
      if (newval) {
	Fl::add_timeout(INITIALREPEAT,repeat_callback,this);
	do_callback();
      } else {
	Fl::remove_timeout(repeat_callback,this);
      }
    }
    return 1;
  default:
    return Fl_Button::handle(event);
  }
}
