// forms_free.C

// Emulation of the Forms "free" widget.
// This emulation allows the free demo to run, and has allowed
// me to port several other programs, but it is in no way
// complete.

#include <FL/Fl.H>
#include <FL/Fl_Free.H>

void Fl_Free::step(void *v) {
  Fl_Free *f = (Fl_Free *)v;
  f->handle(FL_STEP);
  Fl::add_timeout(.01,step,v);
}

Fl_Free::Fl_Free(uchar t,int x,int y,int w,int h,const char *l,
		 FL_HANDLEPTR hdl) :
Fl_Widget(x,y,w,h,l) {
  type(t);
  hfunc = hdl;
  if (t == FL_SLEEPING_FREE) deactivate();
  if (t == FL_CONTINUOUS_FREE || t == FL_ALL_FREE)
    Fl::add_timeout(.01,step,this);
}

Fl_Free::~Fl_Free() {
  Fl::remove_timeout(step,this);
  hfunc(this,FL_FREEMEM,0,0,0);
}

void Fl_Free::draw() {hfunc(this,FL_DRAW,0,0,0);}

int Fl_Free::handle(int e) {
  char key = Fl::event_key();
  switch (e) {
  case FL_FOCUS:
    if (type()!=FL_INPUT_FREE && type()!=FL_ALL_FREE) return 0;
    break;
  case FL_PUSH:
  case FL_DRAG:
  case FL_RELEASE:
    key = 4-Fl::event_button();
    break;
  case FL_SHORTCUT:
    return 0;
  }
  if (hfunc(this, e, float(Fl::event_x()), float(Fl::event_y()), key)) do_callback();
  return 1;
}
