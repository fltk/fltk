// Turning the border on/off by changing the motif_wm_hints property
// works on Irix 4DWM.  Does not appear to work for any other window
// manager.  Fullscreen still works on some window managers (fvwm is one)
// because they allow the border to be placed off-screen.

// Unfortunatly most X window managers ignore changes to the border
// and refuse to position the border off-screen, so attempting to make
// the window full screen will lose the size of the border off the
// bottom and right.

#include <FL/Fl.H>
#include <FL/x.H>

void Fl_Window::border(int b) {
  if (b) {
    if (border()) return;
    clear_flag(FL_NOBORDER);
  } else {
    if (!border()) return;
    set_flag(FL_NOBORDER);
  }
#ifdef WIN32
  // not yet implemented, but it's possible
  // for full fullscreen we have to make the window topmost as well
#else
  if (shown()) Fl_X::i(this)->sendxjunk();
#endif
}

void Fl_Window::fullscreen() {
  border(0);
  if (!x()) x(1); // force it to call XResizeWindow()
  resize(0,0,Fl::w(),Fl::h());
}

void Fl_Window::fullscreen_off(int X,int Y,int W,int H) {
#ifdef WIN32
  border(1);
  resize(X,Y,W,H);
#else
  // this order produces less blinking on IRIX:
  resize(X,Y,W,H);
  border(1);
#endif
}

