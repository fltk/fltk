//
// "$Id$"
//
// Simple Fl_Tree widget example. - erco 06/05/2010
//
// Copyright 1998-2010 by Bill Spitzak and others.
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
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//
#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tree.H>

// Tree's callback
//    Invoked whenever someone clicks an item.
//
void TreeCallback(Fl_Widget *w, void *data) {
  Fl_Tree      *tree = (Fl_Tree*)w;
  Fl_Tree_Item *item = (Fl_Tree_Item*)tree->item_clicked();
  fprintf(stderr, "TreeCallback: item clicked='%s'\n", (item)?item->label():"???");
}

int main(int argc, char *argv[]) {
  Fl::scheme("gtk+");
  Fl_Double_Window *win = new Fl_Double_Window(250, 400, "Simple Tree");
  win->begin();
  {
    // Create the tree
    Fl_Tree *tree = new Fl_Tree(10, 10, win->w()-20, win->h()-20);
    tree->showroot(0);				// don't show root of tree
    tree->callback(TreeCallback);		// setup a callback for the tree

    // Add some items
    tree->add("Flintstones/Fred");
    tree->add("Flintstones/Wilma");
    tree->add("Flintstones/Pebbles");
    tree->add("Simpsons/Homer");
    tree->add("Simpsons/Marge");
    tree->add("Simpsons/Bart");
    tree->add("Simpsons/Lisa");

    // Start with one of the items closed
    tree->close("Simpsons");
  }
  win->end();
  win->resizable(win);
  win->show(argc, argv);
  return(Fl::run());
}

//
// End of "$Id$".
//
