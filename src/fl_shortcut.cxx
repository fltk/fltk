// fl_shortcut.C

// Code to test and parse fltk shortcut numbers.

// A shortcut is a keysym or'd with shift flags.  In the simplest
// sense a shortcut is matched if the shift state is exactly as
// given and the key returning that keysym is pressed.

// To make it easier to match some things it is more complex:

// Only FL_META, FL_ALT, FL_SHIFT, and FL_CTRL must be "off".  A
// zero in the other shift flags indicates "dont care".

// It also checks against the first character of Fl::event_text(),
// and zero for FL_SHIFT means "don't care".
// This allows punctuation shortcuts like "#" to work (rather than
// calling it "shift+3")

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <ctype.h>
#include <string.h>
#ifndef WIN32
#include <X11/Xlib.h>
#endif

int Fl::test_shortcut(int shortcut) {
  if (!shortcut) return 0;

  int shift = Fl::event_state();
  // see if any required shift flags are off:
  if ((shortcut&shift) != (shortcut&0x7fff0000)) return 0;
  // record shift flags that are wrong:
  int mismatch = (shortcut^shift)&0x7fff0000;
  // these three must always be correct:
  if (mismatch&(FL_META|FL_ALT|FL_CTRL)) return 0;

  int key = shortcut & 0xffff;

  // if shift is also correct, check for exactly equal keysyms:
  if (!(mismatch&(FL_SHIFT)) && key == Fl::event_key()) return 1;

  // try matching ascii, ignore shift:
  if (key == event_text()[0]) return 1;

  // kludge so that Ctrl+'_' works (as opposed to Ctrl+'^_'):
  if ((shift&FL_CTRL) && key >= 0x3f && key <= 0x5F
      && event_text()[0]==(key^0x40)) return 1;
  return 0;
}

const char * fl_shortcut_label(int shortcut) {
  static char buf[20];
  char *p = buf;
  if (!shortcut) {*p = 0; return buf;}
  if (shortcut & FL_META) {strcpy(p,"Meta+"); p += 5;}
  if (shortcut & FL_ALT) {strcpy(p,"Alt+"); p += 4;}
  if (shortcut & FL_SHIFT) {strcpy(p,"Shift+"); p += 6;}
  if (shortcut & FL_CTRL) {strcpy(p,"Ctrl+"); p += 5;}
  int key = shortcut & 0xFFFF;
#ifdef WIN32
  if (key >= FL_F && key <= FL_F_Last) {
    *p++ = 'F';
    if (key > FL_F+9) *p++ = (key-FL_F)/10+'0';
    *p++ = (key-FL_F)%10 + '0';
  } else {
    if (key == FL_Enter || key == '\r') {strcpy(p,"Enter"); return buf;}
    *p++ = uchar(key);
  }
  *p = 0;
  return buf;
#else
  const char* q;
  if (key == FL_Enter || key == '\r') q="Enter"; // don't use Xlib's "Return"
  else if (key > 32 && key < 0x100) q = 0;
  else q = XKeysymToString(key);
  if (!q) {*p++ = uchar(key); *p = 0; return buf;}
  if (p > buf) {strcpy(p,q); return buf;} else return q;
#endif
}

// Tests for &x shortcuts in button labels:

int Fl_Widget::test_shortcut(const char *label) {
  char c = Fl::event_text()[0];
  if (!c || !label) return 0;
  for (;;) {
    if (!*label) return 0;
    if (*label++ == '&' && *label) {
      if (*label == '&') label++;
      else if (*label == c) return 1;
      else return 0;
    }
  }
}

int Fl_Widget::test_shortcut() {
  if (!(flags()&SHORTCUT_LABEL)) return 0;
  return test_shortcut(label());
}
