//
// "$Id: browser.cxx,v 1.5.2.2 2000/01/20 06:14:34 bill Exp $"
//
// Browser test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

/*
This is a test of how the browser draws lines.
This is a second line.
This is a third.

That was a blank line above this.

@r@_Right justify
@c@_Center justify
@_Left justify

@bBold text
@iItalic text
@b@iBold Italic
@fFixed width
@f@bBold Fixed
@f@iItalic Fixed
@f@i@bBold Italic Fixed
@lLarge
@l@bLarge bold
@sSmall
@s@bSmall bold
@s@iSmall italic
@s@i@bSmall italic bold
@uunderscore
@C1RED
@C2Green
@C4Blue

	You should try different browser types:
	Fl_Browser
	Fl_Select_Browser
	Fl_Hold_Browser
	Fl_Multi_Browser
*/

#include <FL/Fl.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Double_Window.H>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

void b_cb(Fl_Widget* o, void*) {
  printf("callback, selection = %d, event_clicks = %d\n",
	 ((Fl_Browser*)o)->value(), Fl::event_clicks());
}

int main(int argc, char **argv) {
  int i;
  if (!Fl::args(argc,argv,i)) Fl::fatal(Fl::help);
  const char* fname = (i < argc) ? argv[i] : "browser.cxx";
  Fl_Window window(400,400,fname);
  window.box(FL_NO_BOX); // because it is filled with browser
  Fl_Select_Browser browser(0,0,400,400,0);
  browser.type(FL_MULTI_BROWSER);
  //browser.type(FL_HOLD_BROWSER);
  //browser.color(42);
  browser.callback(b_cb);
  // browser.scrollbar_right();
  //browser.has_scrollbar(Fl_Browser::BOTH_ALWAYS);
  if (!browser.load(fname)) {
    printf("Can't load %s, %s\n", fname, strerror(errno));
    exit(1);
  }
  browser.position(0);
  window.resizable(&browser);
  window.show(argc,argv);
  return Fl::run();
}

//
// End of "$Id: browser.cxx,v 1.5.2.2 2000/01/20 06:14:34 bill Exp $".
//

