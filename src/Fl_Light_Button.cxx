// Fl_Light_Button.C

// Subclass of Fl_Button where the "box" indicates whether it is
// pushed or not, and the "down box" is drawn small and square on
// the left to indicate the current state.

// The default down_box of zero draws a rectangle designed to look
// just like Flame's buttons.

#include <FL/Fl.H>
#include <FL/Fl_Light_Button.H>
#include <FL/fl_draw.H>

void Fl_Light_Button::draw() {
  if (box()) draw_box(this==Fl::pushed() ? down(box()) : box(), color());
  Fl_Color col = value() ? selection_color() : color();
  int d = h()/6;
  int W = w()<h() ? w() : h();
  if (down_box()) {
    // draw other down_box() styles:
    draw_box(down_box(), x()+d, y()+d+1, W-2*d-2, W-2*d-2, col);
  } else {
    // if down_box() is zero, draw light button style:
    int hh = h()-2*d;
    int ww = hh/2+1;
    int xx = d*2;
    if (w()<ww+2*xx) xx = (w()-ww)/2;
    draw_box(FL_THIN_DOWN_BOX, x()+xx, y()+d, ww, hh, col);
  }
  draw_label(x()+W-d, y(), w()-W+d, h());
}

int Fl_Light_Button::handle(int event) {
  switch (event) {
  case FL_RELEASE:
    if (box()) redraw();
  default:
    return Fl_Button::handle(event);
  }
}

Fl_Light_Button::Fl_Light_Button(int x, int y, int w, int h, const char* l)
: Fl_Button(x, y, w, h, l) {
  type(FL_TOGGLE_BUTTON);
  selection_color(FL_YELLOW);
  align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
}
