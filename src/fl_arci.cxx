// fl_arci.C

// "integer" circle drawing functions.  These draw the limited
// circle types provided by X and NT graphics.  The advantage of
// these is that small ones draw quite nicely (probably due to stored
// hand-drawn bitmaps of small circles!) and may be implemented by
// hardware and thus are fast.

// Probably should add fl_chord.

// 3/10/98: created

#include <FL/fl_draw.H>
#include <FL/x.H>
#ifdef WIN32
#include <FL/math.h>
#endif

void fl_arc(int x,int y,int w,int h,double a1,double a2) {
  if (w <= 0 || h <= 0) return;
#ifdef WIN32
  int xa = x+w/2+int(w*cos(a1/180.0*M_PI));
  int ya = y+h/2-int(h*sin(a1/180.0*M_PI));
  int xb = x+w/2+int(w*cos(a2/180.0*M_PI));
  int yb = y+h/2-int(h*sin(a2/180.0*M_PI));
  Arc(fl_gc, x, y, x+w, y+h, xa, ya, xb, yb); 
#else
  XDrawArc(fl_display, fl_window, fl_gc, x,y,w-1,h-1, int(a1*64),int((a2-a1)*64));
#endif
}

void fl_pie(int x,int y,int w,int h,double a1,double a2) {
  if (w <= 0 || h <= 0) return;
#ifdef WIN32
  if (a1 == a2) return;
  int xa = x+w/2+int(w*cos(a1/180.0*M_PI));
  int ya = y+h/2-int(h*sin(a1/180.0*M_PI));
  int xb = x+w/2+int(w*cos(a2/180.0*M_PI));
  int yb = y+h/2-int(h*sin(a2/180.0*M_PI));
  SelectObject(fl_gc, fl_brush());
  Pie(fl_gc, x, y, x+w, y+h, xa, ya, xb, yb); 
#else
  XFillArc(fl_display, fl_window, fl_gc, x,y,w,h, int(a1*64),int((a2-a1)*64));
#endif
}
