// forms_pixmap.C
// Forms compatability widget to draw a pixmap

#include <FL/forms.H>

Fl_FormsPixmap::Fl_FormsPixmap(
  Fl_Boxtype t, int x, int y, int w, int h, const char* l)
: Fl_Widget(x, y, w, h, l) {
  box(t);
  b = 0;
  color(FL_BLACK);
  align(FL_ALIGN_BOTTOM);
}

void Fl_FormsPixmap::set(char*const* bits) {
  delete b;
  b = new Fl_Pixmap(bits);
}

void Fl_FormsPixmap::draw() {
  draw_box(box(), selection_color());
  if (b) {fl_color(color()); b->draw(x(), y(), w(), h());}
  draw_label();
}
