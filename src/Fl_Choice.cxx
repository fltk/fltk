// Fl_Choice.C

// Emulates the Forms choice widget.  This is almost exactly the same
// as an Fl_Menu_Button.  The only difference is the appearance of the
// button: it draws the text of the current pick and a down-arrow.

#include <FL/Fl.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_draw.H>

extern char fl_draw_shortcut;

void Fl_Choice::draw() {
  draw_box();
  if (box() == FL_FLAT_BOX) return; // for XForms compatability
  int H = labelsize()/2+1;
  draw_box(FL_THIN_UP_BOX,x()+w()-3*H,y()+(h()-H)/2,2*H,H,color());
  fl_font(textfont(),textsize(),default_font(),default_size());
  fl_color(active_r() ? textcolor() : inactive(textcolor()));
  fl_draw_shortcut = 2; // hack value to make '&' disappear
  fl_draw(text(),x()+6,y(),w()-6,h(),FL_ALIGN_LEFT);
  fl_draw_shortcut = 0;
  draw_label();
}

Fl_Choice::Fl_Choice(int x,int y,int w,int h, const char *l)
: Fl_Menu_(x,y,w,h,l) {
  align(FL_ALIGN_LEFT);
  when(FL_WHEN_RELEASE);
  textfont(FL_HELVETICA);
  down_box(FL_NO_BOX);
}

int Fl_Choice::value(int v) {
  if (!Fl_Menu_::value(v)) return 0;
  redraw();
  return 1;
}

int Fl_Choice::handle(int e) {
  if (!menu() || !menu()->text) return 0;
  const Fl_Menu_Item* v;
  switch (e) {
  case FL_PUSH:
  J1:
    v = menu()->pulldown(x(), y(), w(), h(), mvalue(), this);
    if (!v || v->submenu()) return 1;
    if (v != mvalue()) redraw();
    picked(v);
    return 1;
  case FL_SHORTCUT:
    if (Fl_Widget::test_shortcut()) goto J1;
    v = menu()->test_shortcut();
    if (!v) return 0;
    if (v != mvalue()) redraw();
    picked(v);
    return 1;
  default:
    return 0;
  }
}

// end of Fl_Choice.C
