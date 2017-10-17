//
// "$Id$"
//
//      Simple Example app using Fl_Simple_Terminal. - erco 10/12/2017
//
// Copyright 2017 Greg Ercolano.
// Copyright 1998-2016 by Bill Spitzak and others.
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

#include <time.h>		//START
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Simple_Terminal.H>

#define TERMINAL_HEIGHT 120

// Globals
Fl_Double_Window   *G_win = 0;
Fl_Box             *G_box = 0;
Fl_Simple_Terminal *G_tty = 0;

// Append a date/time message to the terminal every 2 seconds
void tick_cb(void *data) {
  time_t lt = time(NULL);
  G_tty->printf("Timer tick: \033[32m%s\033[0m\n", ctime(&lt));
  Fl::repeat_timeout(2.0, tick_cb, data);
}

int main(int argc, char **argv) {
  G_win = new Fl_Double_Window(500, 200+TERMINAL_HEIGHT, "Your App");
  G_win->begin();

    G_box = new Fl_Box(0, 0, G_win->w(), 200,
                       "Your app GUI in this area.\n\n"
                       "Your app's debugging output in tty below");

    // Add simple terminal to bottom of app window for scrolling history of status messages.
    G_tty = new Fl_Simple_Terminal(0,200,G_win->w(),TERMINAL_HEIGHT);
    G_tty->ansi(true);  // enable use of "\033[32m"

  G_win->end();
  G_win->resizable(G_win);
  G_win->show();
  Fl::add_timeout(0.5, tick_cb);
  return Fl::run();
}				//END

//
// End of "$Id$".
//
