/*	Fl_Bitmap.H	*/

#ifndef Fl_Bitmap_H
#define Fl_Bitmap_H

class Fl_Widget;
struct Fl_Menu_Item;

struct Fl_Bitmap {
  const uchar *array;
  int w, h;
  ulong id; // for internal use
  Fl_Bitmap(const uchar *bits, int W, int H) :
    array(bits), w(W), h(H), id(0) {}
  Fl_Bitmap(const char *bits, int W, int H) :
    array((const uchar *)bits), w(W), h(H), id(0) {}
  ~Fl_Bitmap();
  void label(Fl_Widget*);
  void label(Fl_Menu_Item*);
  void draw(int X, int Y, int W, int H, int cx=0, int cy=0);
  void draw(int X, int Y) {draw(X, Y, w, h, 0, 0);}
};

#endif
