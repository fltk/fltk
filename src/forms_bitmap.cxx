// forms_bitmap.C
// Forms compatability widget to draw a bitmap

#include <FL/forms.H>

Fl_FormsBitmap::Fl_FormsBitmap(
  Fl_Boxtype t, int x, int y, int w, int h, const char* l)
: Fl_Widget(x, y, w, h, l) {
  box(t);
  b = 0;
  color(FL_BLACK);
  align(FL_ALIGN_BOTTOM);
}

void Fl_FormsBitmap::set(int W, int H, const uchar *bits) {
  delete b;
  bitmap(new Fl_Bitmap(bits, W, H));
}

void Fl_FormsBitmap::draw() {
  draw_box(box(), selection_color());
  if (b) {fl_color(color()); b->draw(x(), y(), w(), h());}
  draw_label();
}
