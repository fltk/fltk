//
// "$Id$"
//
//    Demonstrate creating an Fl_Browser with icons - Greg Ercolano 10/07/09 (STR#1739)
//
//    Shows how one can add icons to items in a browser.
//
// Copyright 2009,2013 Greg Ercolano.
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
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Pixmap.H>

static const char *big[] = {                       // XPM
  "50 34 4 1",
  "  c #000000",
  "o c #ff9900",
  "@ c #ffffff",
  "# c None",
  "##################################################",
  "###      ##############################       ####",
  "### ooooo  ###########################  ooooo ####",
  "### oo  oo  #########################  oo  oo ####",
  "### oo   oo  #######################  oo   oo ####",
  "### oo    oo  #####################  oo    oo ####",
  "### oo     oo  ###################  oo     oo ####",
  "### oo      oo                     oo      oo ####",
  "### oo       oo  ooooooooooooooo  oo       oo ####",
  "### oo        ooooooooooooooooooooo        oo ####",
  "### oo     ooooooooooooooooooooooooooo    ooo ####",
  "#### oo   ooooooo ooooooooooooo ooooooo   oo #####",
  "####  oo oooooooo ooooooooooooo oooooooo oo  #####",
  "##### oo oooooooo ooooooooooooo oooooooo oo ######",
  "#####  o ooooooooooooooooooooooooooooooo o  ######",
  "###### ooooooooooooooooooooooooooooooooooo #######",
  "##### ooooooooo     ooooooooo     ooooooooo ######",
  "##### oooooooo  @@@  ooooooo  @@@  oooooooo ######",
  "##### oooooooo @@@@@ ooooooo @@@@@ oooooooo ######",
  "##### oooooooo @@@@@ ooooooo @@@@@ oooooooo ######",
  "##### oooooooo  @@@  ooooooo  @@@  oooooooo ######",
  "##### ooooooooo     ooooooooo     ooooooooo ######",
  "###### oooooooooooooo       oooooooooooooo #######",
  "###### oooooooo@@@@@@@     @@@@@@@oooooooo #######",
  "###### ooooooo@@@@@@@@@   @@@@@@@@@ooooooo #######",
  "####### ooooo@@@@@@@@@@@ @@@@@@@@@@@ooooo ########",
  "######### oo@@@@@@@@@@@@ @@@@@@@@@@@@oo ##########",
  "########## o@@@@@@ @@@@@ @@@@@ @@@@@@o ###########",
  "########### @@@@@@@     @     @@@@@@@ ############",
  "############  @@@@@@@@@@@@@@@@@@@@@  #############",
  "##############  @@@@@@@@@@@@@@@@@  ###############",
  "################    @@@@@@@@@    #################",
  "####################         #####################",
  "##################################################",
};


static const char *med[] = {                       // XPM
  "14 14 2 1",
  "# c #000000",
  "  c #ffffff",
  "##############",
  "##############",
  "##          ##",
  "##  ##  ##  ##",
  "##  ##  ##  ##",
  "##   ####   ##",
  "##    ##    ##",
  "##    ##    ##",
  "##   ####   ##",
  "##  ##  ##  ##",
  "##  ##  ##  ##",
  "##          ##",
  "##############",
  "##############",
};

static const char *sml[] = {                       // XPM
  "9 11 5 1",
  ".  c None",
  "@  c #000000",
  "+  c #808080",
  "r  c #802020",
  "#  c #ff8080",
  ".........",
  ".........",
  "@+.......",
  "@@@+.....",
  "@@r@@+...",
  "@@##r@@+.",
  "@@####r@@",
  "@@##r@@+.",
  "@@r@@+...",
  "@@@+.....",
  "@+.......",
};

// Create a custom browser
//
//    You don't *have* to derive a class just to control icons in a browser,
//    but in final apps it's something you'd do to keep the implementation clean.
//
//    All it really comes down to is calling browser->icon() to define icons
//    for the items you want.
//
class MyBrowser : public Fl_Browser {
  Fl_Image *big_icon;
  Fl_Image *med_icon;
  Fl_Image *sml_icon;

public:
  MyBrowser(int X,int Y,int W,int H,const char *L=0) : Fl_Browser(X,Y,W,H,L) {

    // Create icons (these could also be pngs, jpegs..)
    big_icon = new Fl_Pixmap(big);
    med_icon = new Fl_Pixmap(med);
    sml_icon = new Fl_Pixmap(sml);

    // Normal browser initialization stuff
    textfont(FL_COURIER);
    textsize(14);
    type(FL_MULTI_BROWSER);
    add("One");
    add("Two");
    add("Three");
    add("Four");
    add("Five");
    add("Six");
    add("Seven");
  }
  static void Choice_CB(Fl_Widget*w, void *d) {
    MyBrowser *mb = (MyBrowser*)d;
    Fl_Choice *ch = (Fl_Choice*)w;

    // See which icon the user picked
    Fl_Image *i = 0;
	 if ( strcmp(ch->text(), "None"  ) == 0 ) { i = 0; }
    else if ( strcmp(ch->text(), "Small" ) == 0 ) { i = mb->sml_icon; }
    else if ( strcmp(ch->text(), "Medium") == 0 ) { i = mb->med_icon; }
    else if ( strcmp(ch->text(), "Large" ) == 0 ) { i = mb->big_icon; }

    // Change the icon of three browser items to what the user picked
    //    This is all you have to do to change a browser item's icon.
    //    The browser will automatically resize the items if need be.
    mb->icon(3,i);
    mb->icon(4,i);
    mb->icon(5,i);
  }
};
int main() {
  Fl_Double_Window *w = new Fl_Double_Window(400,300);

  // Create a browser
  MyBrowser *b = new MyBrowser(10,40,w->w()-20,w->h()-50);

  // Create a chooser to let the user change the icons
  Fl_Choice *choice = new Fl_Choice(60,10,140,25,"Icon:");
  choice->add("None");
  choice->add("Small");
  choice->add("Medium");
  choice->add("Large");
  choice->callback(MyBrowser::Choice_CB, (void*)b);
  choice->take_focus();
  choice->value(1); choice->do_callback();

  w->end();
  w->show();
  return(Fl::run());
} 

//
// End of "$Id$".
//
