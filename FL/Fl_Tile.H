// Fl_Tile.H - Group of 2,3 or 4 "tiles" that can be resized by
// dragging the borders between them.  May be improved in the future
// to handle more tiles in arbitrary layout.

#ifndef Fl_Tile_H
#define Fl_Tile_H

#include "Fl_Group.H"

class Fl_Tile : public Fl_Group {
  int handle(int);
public:
  Fl_Tile(int X,int Y,int W,int H,const char*l=0) : Fl_Group(X,Y,W,H,l) {}
  void resize(int, int, int, int);
  void position(int, int, int, int);
};

#endif
