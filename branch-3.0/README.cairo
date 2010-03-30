Cairo Support for fltk 1.3
===========================

It is now possible to integrate cairo rendering in your fltk application
more easily and transparently.
In 1.3, we provide minimum support for Cairo,
In particular, no "total" cairo rendering layer support is achieved,
as in fltk2.

Configuration:
-------------
All the changes are *inactive*  as long as the new configuration
option --enable-cairo is not added to the configure command.
For non configure based platforms/ide, the HAVE_CAIRO preprocess var.
has to be defined.
All configure based build files has now this feature integrated,
also vc2005 build files have 2 new build modes "Release Cairo" and 
"Debug Cairo".
Others IDE's will be updated progressively.

The current support consist in :
-------------------------------
- Adding a new Fl_Cairo_Window class permitting transparent and easy
integration of a Cairo draw callback without the need to achieve subclassing.

- Adding a Fl::cairo_make_current(Fl_Window*) function only providing
transparently a cairo context to your custom Fl_Window derived class.
This function is intended to be used in your overloaded draw() method.

- Adding an optional cairo autolink context mode support (disabled by default)
  which permits complete & automatic synchronization of OS dependent graphical 
  context and cairo contexts, thus furthering a valid cairo context anytime, 
  in any current window. 
  This feature should be only necessary in the following cases:
  -  Intensive and almost systematic use of cairo contexts in an fltk application
  -  Creation of a new cairo based scheme for fltk ...
  -  Other uses of cairo necessiting the flk internals instrumentation 
     to automatically making possible the use of a cairo context 
     in any fltk window.

- A new cairo demo that is available in the test subdirectory and has been
   used as a testcase durings the multiplatform tests.

For more details, please have a look to the doxygen documentation,
in the Modules section.

-----------------------------------------------------------------------
Reminder for potential future considerations 

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
