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
  const char* fname = (i < argc) ? argv[i] : "browser.C";
  Fl_Window window(400,400,fname);
  window.box(FL_NO_BOX); // because it is filled with browser
  Fl_Select_Browser browser(0,0,400,400,0);
  browser.type(FL_MULTI_BROWSER);
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


