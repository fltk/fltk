Reminder for present and potential future considerations 

From Bill:
First there is the HAVE_CAIRO configuration option. This indicates that
any cairo calls are available. In this case you get something like this:

// static variable holding the last cairo context fltk set:
cairo_t* Fl::cr;

// Make cr draw in this window. This hides the ugly platform-dependent
// part of getting cairo going:
void Fl::cairo_make_current(Fl_Window*)

*** POST 1.3 potential cairo use:
// Set cr to something you made yourself. This lets you reuse functions
// that use cr, and also tells fltk that cr is not one of it's own and
// thus cannot be destroyed or reused for a different window:
void Fl::cairo_make_current(cairo_t*)

Second there is the USE_CAIRO configuration option. This means that all
drawing is done using Cairo. In this case when a widget draw() method is
called, it is exactly as though cairo_make_current(window) has been done.
*** 

Note that it should be possible to compile so HAVE_CAIRO works even if
USE_CAIRO does not, and so that turning on USE_CAIRO does not break any
programs written for HAVE_CAIRO.

From Fabien
We will provide some service for users to use cairo with fltk 1.3 :
A HAVE_CAIRO configuration option will be created.
A simple Fl_Cairo_Window will be implemented and will provide basic 
cairo surface transparent handling (hiding non portable cairo context creation and so on).
A demo program called cairo_test will be added in demo,  providing a new test case.
This program will raise an alert dialog box in case HAVE_CAIRO is not set, indicating that cairo lib is not available.
