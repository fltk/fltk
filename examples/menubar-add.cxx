//
// "$Id$"
//
//	An example of using Fl_Menu_Bar's add() to dynamically create menubars
//
//	Menu bars can be created several ways. Using add() allows
//	dynamically creating a menubar using a 'pathname' syntax.
//	Use if you're creating items dynamically, or if you're making 
//	menubars by hand (as opposed to using fluid), as it's easier
//	to type and read. 
//
//	In this case we're using one callback for all items, 
//	but you can make unique callbacks for each item if needed.
//
// Copyright 2010 Greg Ercolano.
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
#include <stdio.h>			// fprintf()
#include <stdlib.h>			// exit()
#include <string.h>			// strcmp()
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/filename.H>		// fl_open_uri()

// This callback is invoked whenever the user clicks an item in the menu bar
static void MyMenuCallback(Fl_Widget *w, void *) {
  Fl_Menu_Bar *bar = (Fl_Menu_Bar*)w;				// Get the menubar widget
  const Fl_Menu_Item *item = bar->mvalue();			// Get the menu item that was picked

  char ipath[256]; bar->item_pathname(ipath, sizeof(ipath));	// Get full pathname of picked item

  fprintf(stderr, "callback: You picked '%s'", item->label());	// Print item picked
  fprintf(stderr, ", item_pathname() is '%s'", ipath);		// ..and full pathname

  if ( item->flags & (FL_MENU_RADIO|FL_MENU_TOGGLE) ) {		// Toggle or radio item?
    fprintf(stderr, ", value is %s", item->value()?"on":"off");	// Print item's value
  }
  fprintf(stderr, "\n");
  if ( strcmp(item->label(), "Google") == 0 ) { fl_open_uri("http://google.com/"); }
  if ( strcmp(item->label(), "&Quit") == 0 ) { exit(0); }
}

int main() {
  Fl::scheme("gtk+");
  Fl_Window *win = new Fl_Window(400,200, "menubar-simple");	// Create window
  Fl_Menu_Bar *menu = new Fl_Menu_Bar(0,0,400,25);		// Create menubar, items..
  menu->add("&File/&Open",  "^o", MyMenuCallback);
  menu->add("&File/&Save",  "^s", MyMenuCallback, 0, FL_MENU_DIVIDER);
  menu->add("&File/&Quit",  "^q", MyMenuCallback);
  menu->add("&Edit/&Copy",  "^c", MyMenuCallback);
  menu->add("&Edit/&Paste", "^v", MyMenuCallback, 0, FL_MENU_DIVIDER);
  menu->add("&Edit/Radio 1",   0, MyMenuCallback, 0, FL_MENU_RADIO);
  menu->add("&Edit/Radio 2",   0, MyMenuCallback, 0, FL_MENU_RADIO|FL_MENU_DIVIDER);
  menu->add("&Edit/Toggle 1",  0, MyMenuCallback, 0, FL_MENU_TOGGLE);			// Default: off 
  menu->add("&Edit/Toggle 2",  0, MyMenuCallback, 0, FL_MENU_TOGGLE);			// Default: off
  menu->add("&Edit/Toggle 3",  0, MyMenuCallback, 0, FL_MENU_TOGGLE|FL_MENU_VALUE);	// Default: on
  menu->add("&Help/Google",    0, MyMenuCallback);

  // Example: show how we can dynamically change the state of item Toggle #2 (turn it 'on')
  {
      Fl_Menu_Item *item = (Fl_Menu_Item*)menu->find_item("&Edit/Toggle 2");	// Find item
      if ( item ) item->set();							// Turn it on
      else fprintf(stderr, "'Toggle 2' item not found?!\n");			// (optional) Not found? complain!
  }

  win->end();
  win->show();
  return(Fl::run());
}

//
// End of "$Id$".
//
