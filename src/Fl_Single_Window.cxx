/*	Fl_Single_Window.H

	A window with a single-buffered context

	This is provided for systems where the base class is double
	buffered.  You can turn it off using this subclass in case
	your display looks better without it.

*/

#include <FL/Fl_Single_Window.H>

void Fl_Single_Window::show() {Fl_Window::show();}
void Fl_Single_Window::flush() {Fl_Window::flush();}
