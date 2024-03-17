//
// Standard dialog test program for the Fast Light Tool Kit (FLTK).
//
// This also demonstrates how to trap attempts by the user to
// close the last window by overriding Fl::exit
//
// Copyright 1998-2024 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Box.H>

#include <FL/fl_ask.H>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Button callback: v == 0 ("input") or 1 ("password")

void rename_button(Fl_Widget *o, void *v) {
  int what = fl_int(v);
  int ret = 0;
#if (FLTK_USE_STD)
  std::string input;
  if (what == 0) {
    fl_message_icon_label("§");
    input = fl_input_str(ret, 0, "Input (no size limit, use ctrl/j for newline):", o->label());
  } else {
    fl_message_icon_label("€");
    input = fl_password_str(ret, 20, "Enter password (max. 20 characters):", o->label());
  }
  if (ret == 0) {
    o->copy_label(input.c_str());
    o->redraw();
  }
#else
  const char *input;
  if (what == 0) {
    fl_message_icon_label("§");
    input = fl_input("Input (no size limit, use ctrl/j for newline):", o->label());
    if (!input) ret = 1;
  } else {
    fl_message_icon_label("€");
    input = fl_password(20, "Enter password (max. 20 characters):", o->label());
    if (!input) ret = 1;
  }
  if (ret == 0) {
    o->copy_label(input);
    o->redraw();
  }
#endif // FLTK_USE_STD
}

void window_callback(Fl_Widget *win, void *) {
  int hotspot = fl_message_hotspot();
  fl_message_hotspot(0);
  fl_message_title("note: no hotspot set for this dialog");
  int rep = fl_choice("Are you sure you want to quit?", "Cancel", "Quit", "Dunno");
  fl_message_hotspot(hotspot);
  if (rep == 1)
    exit(0);
  else if (rep == 2) { // (Dunno)
    fl_message_position(win);
    fl_message_title("This dialog must be centered over the main window");
    fl_message("Well, maybe you should know before we quit.");
  }
}

/*
  This timer callback shows a message dialog (fl_choice) window
  every 5 seconds to test "recursive" (aka nested) common dialogs.

  The timer can be stopped by clicking the button "Stop these funny popups"
  or pressing the Enter key. As it is currently implemented, clicking the
  "Close" button will reactivate the popups (only possible if "recursive"
  dialogs are enabled, see below).

  Note 1: This dialog box had been blocked in FLTK 1.3 if another common
  dialog was already open because the used window was a static (i.e.
  permanently allocated) Fl_Window instance. This has been fixed in FLTK 1.4.
  See STR #334 ("technical change : remove statics in fl_ask") and also
  STR #2751 ("Limit input field characters").
*/
void timer_cb(void *) {

  static int stop = 0;
  static int n = 0;
  const double delta = 5.0;   // delay of popups
  const int nmax = 10;        // limit no. of popups

  n++;
  if (n >= nmax)
    stop = 1;
  Fl_Box *message_icon = (Fl_Box *)fl_message_icon();

  if (stop) {
    message_icon->color(FL_WHITE);
    return;
  }

  Fl::repeat_timeout(delta, timer_cb);

  // Change the icon box color:
  Fl_Color c = message_icon->color();
  c = (c + 1) % 32;
  if (c == message_icon->labelcolor())
    c++;
  message_icon->color((Fl_Color)c);

  // test message title assignment with a local buffer
  {                                 // local scope for buf
    char buf[40];                   // test: use local variable
    snprintf(buf, 40, "Message #%d", n); // fill message title
    fl_message_title(buf);          // set message title
    strcpy(buf, "** void **");      // overwrite buffer to be sure
  }                                 // buf goes out of scope here

  // pop up a message:
  stop |= fl_choice(
          "Timeout. Click the 'Close' button or press Escape.\n"
          "Note: this message had been blocked in FLTK 1.3\n"
          "and earlier if another message window was open.\n"
          "This message should pop up every 5 seconds (max. 10 times)\n"
          "in FLTK 1.4 and later until stopped by clicking the button\n"
          "below or by pressing the Enter (Return) key.\n",
          "Close", "Stop these funny popups", NULL);
}

int main(int argc, char **argv) {
  char buffer[128] = "Test text";
  char buffer2[128] = "MyPassword";

  // This is a test to make sure automatic destructors work. Pop up
  // the question dialog several times and make sure it doesn't crash.

  Fl_Double_Window window(200, 105);
  Fl_Return_Button b(20, 10, 160, 35, buffer);
  b.callback(rename_button, (void *)(0));
  Fl_Button b2(20, 50, 160, 35, buffer2);
  b2.callback(rename_button, (void *)(1));
  window.end();
  window.resizable(&b);
  window.show(argc, argv);

  // Also we test to see if the exit callback works:
  window.callback(window_callback);

  // Test: set default message window title:
  // fl_message_title_default("Default Message Title");

  // Test: multiple (nested, aka "recursive") popups (see timer_cb())
  Fl::add_timeout(5.0, timer_cb);

  return Fl::run();
}
