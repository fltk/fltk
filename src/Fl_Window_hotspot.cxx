// Fl_Window_hotspot.C

#include <FL/Fl.H>
#include <FL/Fl_Window.H>

void Fl_Window::hotspot(int X, int Y, int offscreen) {
  int mx,my; Fl::get_mouse(mx,my);
  X = mx-X; Y = my-Y;
  if (!offscreen) {
    if (X < 0) X = 0;
    if (X > Fl::w()-w()) X = Fl::w()-w();
    if (Y > Fl::h()-h()) Y = Fl::h()-h();
    if (Y < 0) Y = 0;
    if (border() && Y < 20) Y = 20;
  }
  position(X,Y);
}

void Fl_Window::hotspot(const Fl_Widget *o, int offscreen) {
  int X = o->w()/2;
  int Y = o->h()/2;
  while (o != this) {
    X += o->x(); Y += o->y();
    o = o->window();
  }
  hotspot(X,Y,offscreen);
}
