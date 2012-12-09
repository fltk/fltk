//
// "$Id$"
//
// Unit tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl_Help_View.H>

//
//------- Introduction to FLTK drawing test -------
//
class About : public Fl_Help_View {
public:
  static Fl_Widget *create() {
    return new About(TESTAREA_X, TESTAREA_Y, TESTAREA_W, TESTAREA_H);
  }
  About(int x, int y, int w, int h) : Fl_Help_View(x, y, w, h) {
    value(
"<htmL><body><h2>About Unit Testing...</h2>\n"
"The Unit Testing application can be used to verify correct graphics rendering "
"on the current platform. The core developer team uses this program to make sure that the "
"FLTK user experience is identical on all supported graphics systems."
"<h3>the UI Designer</h3>\n"
"<p>Designing a good user interface is an art. Widgets must be selected and carefully positioned "
"to create a consistent look and feel for the user. Text must fit into given boxes and graphic "
"elements must be correctly aligned. A good UI library will give consistent results on any "
"supported platform and render all graphics in the way the UI designer intended.</p>\n"
"<p>FLTK supports a large collection of platforms and graphics drivers. This unit testing "
"application contains modules which will test rendering and alignment for most "
"FLTK core graphics functions.</p>\n"
"<h3>the Developer</h3>\n"
"<p>Unittest is also a great help when implementing new graphics drivers. The tests are sorted "
"in the same order in which a new graphics driver could be implemented. Most tests rely "
"on the previous test to function correctly, so sticking to the given order is a good idea.</p>\n"
"<h3>Conventions</h3>\n"
"<p>Two layers of graphics are drawn for most tests. The lower layer contains "
"red and green pixels. The upper layer contains black pixels. The test is rendered correctly "
"if all red pixels are covered, but none of the green pixels. The top graphics layer can be "
"switched on and off.</p>"
"</body></html>");
  }
};

UnitTest about("About...", About::create);

//
// End of "$Id$".
//
