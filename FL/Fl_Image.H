/*	Fl_Image.H	*/

#ifndef Fl_Image_H
#define Fl_Image_H

class Fl_Widget;
struct Fl_Menu_Item;

struct Fl_Image {
  const uchar *array;
  int w, h, d, ld;
  ulong id; // for internal use
  Fl_Image(const uchar *bits, int W, int H, int D=3, int LD=0) :
    array(bits), w(W), h(H), d(D), ld(LD), id(0) {}
  ~Fl_Image();
  void label(Fl_Widget*);
  void label(Fl_Menu_Item*);
  void draw(int X, int Y, int W, int H, int cx=0, int cy=0);
  void draw(int X, int Y) {draw(X, Y, w, h, 0, 0);}
};

#endif
