/* 	Silly test of navigation keys.
	This is not a recommended method of laying out your panels!
*/

#include <stdio.h>
#include <stdlib.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>

#define WIDTH 600
#define HEIGHT 300
#define GRID 25

int main(int argc, char **argv) {
  if (argc > 1) srand(atoi(argv[1]));
  Fl_Window window(WIDTH,HEIGHT,argv[0]);
  window.end(); // don't auto-add children
  for (int i = 0; i<10000; i++) {
    // make up a random size of widget:
    int x = rand()%(WIDTH/GRID+1) * GRID;
    int y = rand()%(HEIGHT/GRID+1) * GRID;
    int w = rand()%(WIDTH/GRID+1) * GRID;
    if (w < x) {w = x-w; x-=w;} else {w = w-x;}
    int h = rand()%(HEIGHT/GRID+1) * GRID;
    if (h < y) {h = y-h; y-=h;} else {h = h-y;}
    if (w < GRID || h < GRID || w < h) continue;
    // find where to insert it and see if it intersects something:
    Fl_Widget *i = 0;
    int n; for (n=0; n < window.children(); n++) {
      Fl_Widget *o = window.child(n);
      if (x<o->x()+o->w() && x+w>o->x() &&
	  y<o->y()+o->h() && y+h>o->y()) break;
      if (!i && (y < o->y() || y == o->y() && x < o->x())) i = o;
    }
    // skip if intersection:
    if (n < window.children()) continue;
    window.insert(*(new Fl_Input(x,y,w,h)),i);
  }
  window.show();
  return Fl::run();
}
