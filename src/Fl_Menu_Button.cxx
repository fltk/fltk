// Fl_Menu_Button.C

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/fl_draw.H>

void Fl_Menu_Button::draw() {
  if (!box() || type()) return;
  draw_box(box(), color());
  draw_label();
  if (box() == FL_FLAT_BOX) return; // for XForms compatability
  int H = (labelsize()-3)&-2;
  int X = x()+w()-H*2;
  int Y = y()+(h()-H)/2;
  fl_color(FL_DARK3); fl_line(X+H/2, Y+H, X, Y, X+H, Y);
  fl_color(FL_LIGHT3); fl_line(X+H, Y, X+H/2, Y+H);
}

const Fl_Menu_Item* Fl_Menu_Button::popup() {
  const Fl_Menu_Item* m;
  if (!box() || type()) {
    m = menu()->popup(Fl::event_x(), Fl::event_y(), label(), mvalue(), this);
  } else {
    m = menu()->pulldown(x(), y(), w(), h(), 0, this);
  }
  picked(m);
  return m;
}

int Fl_Menu_Button::handle(int e) {
  if (!menu() || !menu()->text) return 0;
  switch (e) {
  case FL_ENTER:
  case FL_LEAVE:
    return (box() && !type()) ? 1 : 0;
  case FL_PUSH:
    if (!box()) {
      if (Fl::event_button() != 3) return 0;
    } else if (type()) {
      if (!(type() & (1 << (Fl::event_button()-1)))) return 0;
    }
    popup();
    return 1;
  case FL_SHORTCUT:
    if (Fl_Widget::test_shortcut()) {popup(); return 1;}
    return test_shortcut() != 0;
  default:
    return 0;
  }
}

Fl_Menu_Button::Fl_Menu_Button(int X,int Y,int W,int H,const char *l)
: Fl_Menu_(X,Y,W,H,l) {
  down_box(FL_NO_BOX);
}

// end of Fl_Menu_Button.C
