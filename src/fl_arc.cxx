// fl_arc.C

// Utility for drawing arcs and circles.  They are added to
// the current fl_begin/fl_vertex/fl_end path.
// Incremental math implementation:

#include <FL/fl_draw.H>
#include <FL/math.h>

void fl_arc(double x, double y, double r, double start, double end) {

  // draw start point accurately:
  double A = start*(M_PI/180);
  double X = r*cos(A);
  double Y = -r*sin(A);
  fl_vertex(x+X,y+Y);

  // number of segments per radian:
  int n; {
    double x1 = fl_transform_dx(r,0);
    double y1 = fl_transform_dy(r,0);
    double r1 = x1*x1+y1*y1;
    x1 = fl_transform_dx(0,r);
    y1 = fl_transform_dy(0,r);
    double r2 = x1*x1+y1*y1;
    if (r2 < r1) r1 = r2;
    n = int(sqrt(r1)*.841471);
    if (n < 2) n = 2;
  }
  double epsilon = 1.0/n;
  double E = end*(M_PI/180);
  int i = int((E-A)*n);
  if (i < 0) {i = -i; epsilon = -epsilon;}
  double epsilon2 = epsilon/2;
  for (; i>1; i--) {
    X += epsilon*Y;
    Y -= epsilon2*X;
    fl_vertex(x+X,y+Y);
    Y -= epsilon2*X;
  }

  // draw the end point accurately:
  fl_vertex(x+r*cos(E), y-r*sin(E));
}

#if 0 // portable version.  X-specific one in fl_vertex.C
void fl_circle(double x,double y,double r) {
  _fl_arc(x, y, r, r, 0, 360);
}
#endif
