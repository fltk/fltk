//
// "$Id$"
//
// Test program for Fl_Input_Choice
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

#include <stdio.h>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input_Choice.H>

void buttcb(Fl_Widget*,void*data) {
    Fl_Input_Choice *in=(Fl_Input_Choice *)data;
    static int flag = 1;
    flag ^= 1;
    if ( flag ) in->activate();
    else        in->deactivate();
    if (in->changed()) {
        printf("Callback: changed() is set\n");
        in->clear_changed();
    }
}

void input_choice_cb(Fl_Widget*,void*data) {
    Fl_Input_Choice *in=(Fl_Input_Choice *)data;
    fprintf(stderr, "Value='%s'\n", (const char*)in->value());
}

int main(int argc, char **argv) {
    Fl_Double_Window win(300, 200);

    Fl_Input_Choice in(40,40,100,28,"Test");
    in.callback(input_choice_cb, (void*)&in);
    in.add("one");
    in.add("two");
    in.add("three");
    in.value(1);

    Fl_Button onoff(40,150,200,28,"Activate/Deactivate");
    onoff.callback(buttcb, (void*)&in);

    win.end();
    win.resizable(win);
    win.show(argc, argv);
    return Fl::run();
}


//
// End of "$Id$".
//
