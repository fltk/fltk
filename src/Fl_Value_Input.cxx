// Fl_Value_Input.C

// Fltk widget for drag-adjusting a floating point value.

// Warning: this works by making a child Fl_Input object, even
// though this object is *not* an Fl_Group.  May be a kludge?

#include <FL/Fl.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Group.H>
#include <stdlib.h>

void Fl_Value_Input::input_cb(Fl_Widget*, void* v) {
  Fl_Value_Input& t = *(Fl_Value_Input*)v;
  double nv;
  if (t.step()>=1.0) nv = strtol(t.input.value(), 0, 0);
  else nv = strtod(t.input.value(), 0);
  t.handle_push();
  t.handle_drag(nv);
  t.handle_release();
}

void Fl_Value_Input::draw() {
  if (damage()&~1) input.clear_damage(~0);
  input.box(box());
  input.color(color(), selection_color());
  input.draw();
  input.clear_damage();
}

void Fl_Value_Input::resize(int X, int Y, int W, int H) {
  Fl_Valuator::resize(X, Y, W, H);
  input.resize(X, Y, W, H);
}

void Fl_Value_Input::value_damage() {
  char buf[128];
  format(buf);
  input.value(buf);
  input.mark(input.position()); // turn off selection highlight
}

int Fl_Value_Input::handle(int event) {
  double v;
  int delta;
  int mx = Fl::event_x();
  static int ix, drag;
  switch (event) {
  case FL_PUSH:
    if (!step()) goto DEFAULT;
    ix = mx;
    drag = Fl::event_button();
    handle_push();
    return 1;
  case FL_DRAG:
    if (!step()) goto DEFAULT;
    delta = Fl::event_x()-ix;
    if (delta > 5) delta -= 5;
    else if (delta < -5) delta += 5;
    else delta = 0;
    switch (drag) {
    case 3: v = increment(previous_value(), delta*100); break;
    case 2: v = increment(previous_value(), delta*10); break;
    default:v = increment(previous_value(), delta); break;
    }
    v = round(v);
    handle_drag(soft()?softclamp(v):clamp(v));;
    return 1;
  case FL_RELEASE:
    if (!step()) goto DEFAULT;
    if (value() != previous_value() || !Fl::event_is_click())
      handle_release();
    else {
      input.handle(FL_PUSH);
      input.handle(FL_RELEASE);
    }
    return 1;
  case FL_FOCUS:
    return input.take_focus();
  default:
  DEFAULT:
    input.type(step()>=1.0 ? FL_INT_INPUT : FL_FLOAT_INPUT);
    return input.handle(event);
  }
}

Fl_Value_Input::Fl_Value_Input(int x, int y, int w, int h, const char* l)
: Fl_Valuator(x, y, w, h, l), input(x, y, w, h, 0) {
  soft_ = 0;
  if (input.parent())  // defeat automatic-add
    ((Fl_Group*)input.parent())->remove(input);
  input.parent(this); // kludge!
  input.callback(input_cb, this);
  input.when((Fl_When)(FL_WHEN_RELEASE|FL_WHEN_ENTER_KEY));
  box(input.box());
  color(input.color());
  selection_color(input.selection_color());
  align(FL_ALIGN_LEFT);
  value_damage();
}
