/*	fl_round_box.C

	Box drawing code for an obscure box type.
	These box types are in seperate files so they are not linked
	in if not used.

	3/8/99: Complete rewrite to use XDrawArc
*/

#include <FL/Fl.H>
#include <FL/fl_draw.H>

// A compiler from a certain very large software company will not compile
// the function pointer assignment due to the name conflict with fl_arc.
// This function is to fix that:
void fl_arc_i(int x,int y,int w,int h,double a1,double a2) {
  fl_arc(x,y,w,h,a1,a2);
}

enum {UPPER_LEFT, LOWER_RIGHT, CLOSED, FILL};

static void draw(int which, int x,int y,int w,int h, int inset, uchar color)
{
  if (inset*2 >= w) inset = (w-1)/2;
  if (inset*2 >= h) inset = (h-1)/2;
  x += inset;
  y += inset;
  w -= 2*inset;
  h -= 2*inset;
  int d = w <= h ? w : h;
  if (d <= 1) return;
  fl_color((Fl_Color)color);
  void (*f)(int,int,int,int,double,double) =
    (which==FILL) ? fl_pie : fl_arc_i;
  if (which >= CLOSED) {
    f(x+w-d, y, d, d, w<=h ? 0 : -90, w<=h ? 180 : 90);
    f(x, y+h-d, d, d, w<=h ? 180 : 90, w<=h ? 360 : 270);
  } else if (which == UPPER_LEFT) {
    f(x+w-d, y, d, d, 45, w<=h ? 180 : 90);
    f(x, y+h-d, d, d, w<=h ? 180 : 90, 225);
  } else { // LOWER_RIGHT
    f(x, y+h-d, d, d, 225, w<=h ? 360 : 270);
    f(x+w-d, y, d, d, w<=h ? 360 : 270, 360+45);
  }
  if (which == FILL) {
    if (w < h)
      fl_rectf(x, y+d/2, w, h-(d&-2));
    else if (w > h)
      fl_rectf(x+d/2, y, w-(d&-2), h);
  } else {
    if (w < h) {
      if (which != UPPER_LEFT) fl_yxline(x+w-1, y+d/2, y+h-d/2);
      if (which != LOWER_RIGHT) fl_yxline(x, y+d/2, y+h-d/2);
    } else if (w > h) {
      if (which != UPPER_LEFT) fl_xyline(x+d/2, y+h-1, x+w-d/2);
      if (which != LOWER_RIGHT) fl_xyline(x+d/2, y, x+w-d/2);
    }
  }
}

extern uchar* Fl_Gray_Ramp;

static void fl_round_down_box(int x, int y, int w, int h, Fl_Color bgcolor) {
  draw(FILL,	    x,   y, w,   h, 2, bgcolor);
  draw(UPPER_LEFT,  x+1, y, w-2, h, 0, Fl_Gray_Ramp['N']);
  draw(UPPER_LEFT,  x+1, y, w-2, h, 1, Fl_Gray_Ramp['H']);
  draw(UPPER_LEFT,  x,   y, w,   h, 0, Fl_Gray_Ramp['N']);
  draw(UPPER_LEFT,  x,   y, w,   h, 1, Fl_Gray_Ramp['H']);
  draw(LOWER_RIGHT, x,   y, w,   h, 0, Fl_Gray_Ramp['S']);
  draw(LOWER_RIGHT, x+1, y, w-2, h, 0, Fl_Gray_Ramp['U']);
  draw(LOWER_RIGHT, x,   y, w,   h, 1, Fl_Gray_Ramp['U']);
  draw(LOWER_RIGHT, x+1, y, w-2, h, 1, Fl_Gray_Ramp['W']);
  draw(CLOSED,	    x,   y, w,   h, 2, Fl_Gray_Ramp['A']);
}

static void fl_round_up_box(int x, int y, int w, int h, Fl_Color bgcolor) {
  draw(FILL,	    x,   y, w,   h, 2, bgcolor);
  draw(LOWER_RIGHT, x+1, y, w-2, h, 0, Fl_Gray_Ramp['H']);
  draw(LOWER_RIGHT, x+1, y, w-2, h, 1, Fl_Gray_Ramp['N']);
  draw(LOWER_RIGHT, x,   y, w,   h, 1, Fl_Gray_Ramp['H']);
  draw(LOWER_RIGHT, x,   y, w,   h, 2, Fl_Gray_Ramp['N']);
  draw(UPPER_LEFT,  x,   y, w,   h, 2, Fl_Gray_Ramp['U']);
  draw(UPPER_LEFT,  x+1, y, w-2, h, 1, Fl_Gray_Ramp['S']);
  draw(UPPER_LEFT,  x,   y, w,   h, 1, Fl_Gray_Ramp['W']);
  draw(UPPER_LEFT,  x+1, y, w-2, h, 0, Fl_Gray_Ramp['U']);
  draw(CLOSED,	    x,   y, w,   h, 0, Fl_Gray_Ramp['A']);
}

extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*);
Fl_Boxtype define_FL_ROUND_UP_BOX() {
  fl_internal_boxtype(_FL_ROUND_DOWN_BOX, fl_round_down_box);
  fl_internal_boxtype(_FL_ROUND_UP_BOX, fl_round_up_box);
  return _FL_ROUND_UP_BOX;
}
