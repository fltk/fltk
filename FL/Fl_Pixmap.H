/*	Fl_Pixmap.H	*/

#ifndef Fl_Pixmap_H
#define Fl_Pixmap_H

class Fl_Widget;
struct Fl_Menu_Item;

struct Fl_Pixmap {
  /*const*/ char * const * data;
  int w, h; // set by first draw...
  ulong id; // for internal use (the pixmap)
  ulong mask; // for internal use (mask bitmap)
  Fl_Pixmap(/*const*/char * const * d) : data(d), w(-1), h(0), id(0),mask(0) {}
  ~Fl_Pixmap();
  void label(Fl_Widget*);
  void label(Fl_Menu_Item*);
  void draw(int X, int Y, int W, int H, int cx=0, int cy=0);
  void draw(int X, int Y) {draw(X, Y, w, h, 0, 0);}
};

#endif
