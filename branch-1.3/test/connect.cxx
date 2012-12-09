//
// "$Id$"
//
// PPP example program for the Fast Light Tool Kit (FLTK).
//
// Program to make a button to turn a ppp connection on/off.
// You must chmod +s /usr/sbin/pppd, and put all the options
// into /etc/ppp/options.
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Toggle_Button.H>

int running;	// actually the pid
Fl_Toggle_Button *Button;

void sigchld(int) {
  waitpid(running, 0, 0);
  running = 0;
  Button->value(0);
}

void cb(Fl_Widget *o, void *) {
  if (((Fl_Toggle_Button*)o)->value()) {
    if (running) return;
    running = fork();
    if (!running) execl("/usr/sbin/pppd","pppd","-detach",0);
    else signal(SIGCHLD, sigchld);
  } else {
    if (!running) return;
    kill(running, SIGINT);
    waitpid(running, 0, 0);
    running = 0;
  }
}

int main(int argc, char ** argv) {
   Fl_Window window(100,50);
   Fl_Toggle_Button button(0,0,100,50,"Connect");
   Button = &button;
   button.color(1,2);
   button.callback(cb,0);
   window.show(argc,argv);
   return Fl::run();
}

//
// End of "$Id$".
//
