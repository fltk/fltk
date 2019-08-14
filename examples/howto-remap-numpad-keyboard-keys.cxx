// vim: autoindent tabstop=8 shiftwidth=2 expandtab softtabstop=2
//
// "$Id$"
//
// Demonstrate keyboard remapping: Force number pad to type numbers even if NumLock off
//
// DESCRIPTION
// -----------
//    This demonstrates several important things:
//
//         o How to use the global event dispatcher function Fl::event_dispatch()
//
//         o How to remap keyboard events
//
//         o How to use Fl::event_original_key() to easily ensure the dual purpose
//           numeric keypad keys generate only numbers even if NumLock is off,
//           without affecting the dedicated Home/End/etc keys.
//
//    Use of Fl::event_original_key() ensures someone hitting e.g. "7/Home" key
//    returns FL_KP+'7', and not FL_Home, while allowing the dedicated "Home" key
//    to still generate FL_Home. This allows users to use the "Home" and "End"
//    keys to navigate the buffer, while "7/Home" and "1/End" always type numbers.
//
// This demo based on fltk.general thread entitled "keyboard mapping", Aug 2019.
//
// * * *
//
// Copyright 1998-2017 by Bill Spitzak and others.
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
#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>

//
Fl_Check_Button *G_checkbut = 0;

// Global event handler: FLTK calls this after event translation. It's up to us
// to call Fl::handle_(e,w) to actually deliver the event to the widgets. If we
// don't and just return, the event will be dropped. See docs for more.
// 
int MyHandler(int e, Fl_Window *w) {
  // Remapping disabled? Early exit..
  if ( G_checkbut->value() == 0 ) return Fl::handle_(e, w);
  // Keyboard key pressed? See if we should remap..
  if ( e == FL_KEYDOWN || e == FL_KEYUP) {
    // Get FLTK keycode /before/ NumLock state is applied (see above DESCRIPTION)
    int keycode = Fl::event_original_key();            // get keycode before FLTK applies NumLock
    if ( keycode >= FL_KP && keycode <= FL_KP_Last ) { // keypad key pressed?
      static char buf[2];                              // static: we don't want buffer to go out of scope
      buf[0] = char(keycode - FL_KP);                  // convert keypad keycode -> ascii
      buf[1] = 0;                                      // terminate string (for safety)
      Fl::e_text   = buf;                              // point to our static buffer
      Fl::e_length = 1;                                // only first char relevant
    }
  }
  return Fl::handle_(e, w);                            // let FLTK deliver event to widgets
}

int main(int argc, char *argv[]) {
  Fl_Double_Window *win = new Fl_Double_Window(400, 150, "Keyboard Translation Test");
  win->begin();
  {
    new Fl_Input(100, 10, 200, 25, "Input:");
    G_checkbut = new Fl_Check_Button(100,40,280,25," Force numeric keypad to type numbers");
    G_checkbut->labelsize(12);
    G_checkbut->set();
  }
  win->end();
  win->resizable(win);
  win->show(argc, argv);
  win->tooltip("Turn NumLock OFF, then type into Input:\nusing numeric keypadt to test translation");
  // Set up our event handler to manage events
  Fl::event_dispatch(MyHandler);
  return(Fl::run());
}
