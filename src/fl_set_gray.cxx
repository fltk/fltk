// fl_set_gray.C

// -fg, -bg, and -bg2 switches

#include <FL/Fl.H>

void Fl::background(uchar r, uchar g, uchar b) {
  // replace the gray ramp so that color 47 is this color
  int i;
  for (i = 32; i <= 47; i++) {
    int m = (i-32)*255/23;
    Fl::set_color(i,r*m/166,g*m/166,b*m/166);
  }
  for (; i < 56; i++) {
    int m = 255-(i-32)*255/23;
    Fl::set_color(i,255-(255-r)*m/89,255-(255-g)*m/89,255-(255-b)*m/89);
  }
}

static void set_others() {
  uchar r,g,b; Fl::get_color(FL_BLACK,r,g,b);
  uchar r1,g1,b1; Fl::get_color(FL_WHITE,r1,g1,b1);
  Fl::set_color(FL_INACTIVE_COLOR,(2*r+r1)/3, (2*g+g1)/3, (2*b+b1)/3);
  Fl::set_color(FL_SELECTION_COLOR,(2*r1+r)/3, (2*g1+g)/3, (2*b1+b)/3);
}

void Fl::foreground(uchar r, uchar g, uchar b) {
  Fl::set_color(FL_BLACK,r,g,b);
  set_others();
}

void Fl::background2(uchar r, uchar g, uchar b) {
  Fl::set_color(FL_WHITE,r,g,b);
  set_others();
}
