//
// "$Id$"
//
//	Simple Fl_Tabs widget example. 
//	Originally from erco's cheat sheet 06/05/2010, permission by author.
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
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
//
// Simple tabs example
//      _____  _____
//   __/ Aaa \/ Bbb \______________________
//  |    _______                           |
//  |   |_______|                          |
//  |    _______                           |
//  |   |_______|                          |
//  |    _______                           |
//  |   |_______|                          |
//  |______________________________________|
//
int main(int argc, char *argv[]) {
  Fl::scheme("gtk+");
  Fl_Window *win = new Fl_Window(500,200,"Tabs Example");
  {
    // Create the tab widget
    Fl_Tabs *tabs = new Fl_Tabs(10,10,500-20,200-20);
    {
      // ADD THE "Aaa" TAB
      //   We do this by adding a child group to the tab widget.
      //   The child group's label defined the label of the tab.
      //
      Fl_Group *aaa = new Fl_Group(10,35,500-20,200-45,"Aaa");
      {
	// Put some different buttons into the group, which will be shown
	// when the tab is selected.
	Fl_Button *b1 = new Fl_Button(50, 60,90,25,"Button A1"); b1->color(88+1);
	Fl_Button *b2 = new Fl_Button(50, 90,90,25,"Button A2"); b2->color(88+2);
	Fl_Button *b3 = new Fl_Button(50,120,90,25,"Button A3"); b3->color(88+3);
      }
      aaa->end();

      // ADD THE "Bbb" TAB
      //   Same details as above.
      //
      Fl_Group *bbb = new Fl_Group(10,35,500-10,200-35,"Bbb");
      {
	// Put some different buttons into the group, which will be shown
	// when the tab is selected.
	Fl_Button *b1 = new Fl_Button( 50,60,90,25,"Button B1"); b1->color(88+1);
	Fl_Button *b2 = new Fl_Button(150,60,90,25,"Button B2"); b2->color(88+3);
	Fl_Button *b3 = new Fl_Button(250,60,90,25,"Button B3"); b3->color(88+5);
	Fl_Button *b4 = new Fl_Button( 50,90,90,25,"Button B4"); b4->color(88+2);
	Fl_Button *b5 = new Fl_Button(150,90,90,25,"Button B5"); b5->color(88+4);
	Fl_Button *b6 = new Fl_Button(250,90,90,25,"Button B6"); b6->color(88+6);
      }
      bbb->end();
    }
    tabs->end();
  }
  win->end();
  win->show(argc, argv);
  return(Fl::run());
}

//
// End of "$Id$".
//
