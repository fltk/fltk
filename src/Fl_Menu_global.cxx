// Fl_Menu_global.C

// Make all the shortcuts in this menu global.
// Currently only one menu at a time and you cannot destruct the menu,
// is this sufficient?

#include <FL/Fl.H>
#include <FL/Fl_Menu_.H>

static Fl_Menu_* the_widget;

static int handler(int e) {
  if (e != FL_SHORTCUT || Fl::modal()) return 0;
  return the_widget->test_shortcut() != 0;
}

void Fl_Menu_::global() {
  if (!the_widget) Fl::add_handler(handler);
  the_widget = this;
}
