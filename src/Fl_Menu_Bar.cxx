#include <FL/Fl.H>
#include <FL/Fl_Menu_Bar.H>

void Fl_Menu_Bar::draw() {
  draw_box();
  if (!menu() || !menu()->text) return;
  const Fl_Menu_Item* m;
  int X = x()+9;
  for (m=menu(); m->text; m = m->next()) {
    m->draw(X, y(), 0, h(), this);
    X += m->measure(0,this) + 16;
  }
}

int Fl_Menu_Bar::handle(int event) {
  const Fl_Menu_Item* v;
  if (menu() && menu()->text) switch (event) {
  case FL_PUSH:
    v = 0;
  J1:
    v = menu()->pulldown(x(), y(), w(), h(), v, this, 0, 1);
    picked(v);
    return 1;
  case FL_SHORTCUT:
    v = menu()->test_shortcut();
    if (v) {picked(v); return 1;}
    v = menu()->find_shortcut();
    if (v) goto J1;
    return 0;
  }
  return 0;
}

