// Fl_get_key.C

#ifdef WIN32
#include "Fl_get_key_win32.C"
#else

// Return the current state of a key.  This is the X version.  I identify
// keys (mostly) by the X keysym.  So this turns the keysym into a keycode
// and looks it up in the X key bit vector, which Fl_x.C keeps track of.

#include <FL/Fl.H>
#include <FL/x.H>

extern char fl_key_vector[32]; // in Fl_x.C

int Fl::event_key(int k) {
  if (k > FL_Button && k <= FL_Button+8)
    return Fl::event_state(8<<(k-FL_Button));
  int i;
#ifdef __sgi
  // get some missing PC keyboard keys:
  if (k == FL_Meta_L) i = 147;
  else if (k == FL_Meta_R) i = 148;
  else if (k == FL_Menu) i = 149;
  else
#endif
    i = XKeysymToKeycode(fl_display, k);
  return fl_key_vector[i/8] & (1 << (i%8));
}

int Fl::get_key(int k) {
  fl_open_display();
  XQueryKeymap(fl_display, fl_key_vector);
  return event_key(k);
}

#endif
