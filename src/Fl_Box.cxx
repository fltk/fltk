// Fl_Box.C

// The box widget.  An almost non-functional subclass of Fl_Widget.

#include <FL/Fl_Widget.H>
#include <FL/Fl_Box.H>

void Fl_Box::draw() {
  draw_box();
  draw_label();
}
