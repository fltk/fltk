// X-specific code that is not called by all programs, so it is put
// here in it's own source file to make programs smaller.

#include <FL/x.H>

extern char fl_show_iconic; // in Fl_x.C

void Fl_Window::iconize() {
  if (!shown()) {
    fl_show_iconic = 1;
    show();
  } else {
#ifdef WIN32
    ShowWindow(i->xid, SW_SHOWMINNOACTIVE);
#else
    XIconifyWindow(fl_display, i->xid, fl_screen);
#endif
  }
}
