//
// "$Id$"
//
//	Simple Fl_Tree custom (numeric) sort example. - erco 12/16/2013
//      Demonstrates custom sorting of Fl_Tree items.
//
// Copyright 2013 Greg Ercolano.
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
#include <stdio.h>
#include <stdlib.h>	/* qsort(3), srand(3).. */
#include <time.h>	/* time(2) */
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Button.H>

Fl_Tree *G_tree = 0;

// Resort the tree
void MySortCallback(Fl_Widget*, void *data) {
  int dir = int(fl_intptr_t(data));		// forward or reverse
  Fl_Tree_Item *i = G_tree->root();
  // Bubble sort
  for ( int ax=0; ax<i->children(); ax++ ) {
    for ( int bx=ax+1; bx<i->children(); bx++ ) {
      long a; sscanf(i->child(ax)->label(), "%ld", &a);
      long b; sscanf(i->child(bx)->label(), "%ld", &b);
      switch ( dir ) {
        case  1: if ( a > b ) { i->swap_children(ax, bx); } break; // fwd
        case -1: if ( a < b ) { i->swap_children(ax, bx); } break; // rev
      }
    }
  }
  G_tree->redraw();
}

int main(int argc, char *argv[]) {
  // Randomize the random number generator
  time_t tval; time(&tval);
  srand((unsigned)tval);

  // Create window with tree
  Fl::scheme("gtk+");
  Fl_Double_Window *win = new Fl_Double_Window(250, 600, "Numeric Sort Tree");
  win->begin();
  {
    G_tree = new Fl_Tree(10, 10, win->w()-20, win->h()-60);
    G_tree->showroot(0);

    // Add 200 random numbers to the tree
    char word[50];
    for ( int t=0; t<200; t++ ) {
      sprintf(word, "%ld", long((float(rand()) / RAND_MAX) * 1000000));
      G_tree->add(word);
    }

    // Add some sort buttons
    Fl_Button *but;
    but = new Fl_Button(10,   win->h()-40,80,20,"Fwd"); but->callback(MySortCallback, (void*) 1);
    but = new Fl_Button(20+80,win->h()-40,80,20,"Rev"); but->callback(MySortCallback, (void*)-1);
  }
  win->end();
  win->resizable(win);
  win->show(argc, argv);
  return(Fl::run());
}

//
// End of "$Id$".
//
