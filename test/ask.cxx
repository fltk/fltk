/* 	ask.C

	Demonstrates how to use readqueue to see if a button has been
	pushed, and to see if a window has been closed, thus avoiding
	the need to define callbacks.

	This also demonstrates how to trap attempts by the user to
	close the last window by overriding Fl::exit

*/

#include <stdio.h>
#include <string.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>

int get_string(char*buffer) {
  Fl_Window window(320,75);
  Fl_Input input(60, 10, 250, 25, "Input:");
  input.value(buffer);
  Fl_Button cancel(60, 40, 80, 25, "cancel");
  Fl_Return_Button ok(150, 40, 80, 25, "OK");
  window.hotspot(&cancel); // you must position modal windows
  window.end();
  window.set_modal();
  window.show();
  for (;;) {
    Fl::wait();
    Fl_Widget *o;
    while ((o = Fl::readqueue())) {
      if (o == &ok) {
	strcpy(buffer,input.value());
	return 1;
      } else if (o == &cancel || o == &window) {
	return 0;
      }
    }
  }
}

void rename_me(Fl_Widget*o) {
  if (get_string((char*)(o->label()))) o->redraw();
}

#if 1
#include <FL/fl_ask.H>
#include <stdlib.h>

void window_callback(Fl_Widget*, void*) {
  if (!fl_ask("Are you sure you want to quit?")) return;
  exit(0);
}
#endif

int main(int argc, char **argv) {
  char buffer[128] = "test text";

#if 1
// this is a test to make sure automatic destructors work.  Pop up
// the question dialog several times and make sure it don't crash.

  Fl_Window window(200, 55);
  Fl_Return_Button b(20, 10, 160, 35, buffer); b.callback(rename_me);
  window.add(b);
  window.resizable(&b);
  window.show(argc, argv);

// Also we test to see if the exit callback works:
  window.callback(window_callback);

  return Fl::run();

#else
// This is the demo as written in the documentation, it only creates
// the popup window once:

  if (get_string(buffer)) {
    puts(buffer);
  } else {
    puts("cancel");
  }
  return 0;

#endif

}
    
