//	produce images for documentation

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Clock.H>
#include <FL/Fl_Round_Clock.H>

int main(int argc, char **argv) {
  Fl_Window window(220,220,"Fl_Clock");
  Fl_Clock c1(0,0,220,220); // c1.color(2,1);
  window.resizable(c1);
  window.end();
  Fl_Window window2(220,220,"Fl_Round_Clock");
  Fl_Round_Clock c2(0,0,220,220); // c2.color(3,4);
  window2.resizable(c2);
  window2.end();
  // my machine had a clock* Xresource set for another program, so
  // I don't want the class to be "clock":
  window.xclass("Fl_Clock");
  window2.xclass("Fl_Clock");
  window.show(argc,argv);
  window2.show();
  return Fl::run();
}
