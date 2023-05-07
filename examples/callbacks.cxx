//
// Callback macros example program for the Fast Light Tool Kit (FLTK).
//
// Copyright 2023 by Bill Spitzak and others.
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

#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <FL/FL_CALLBACK.H>

Fl_Window *window = NULL;

/*
 We create our own little calls here that uses method callbacks.
 */
class MyButton0 : public Fl_Button {
  // z will be set in the constructor
  int z;
public:
  // create a simple push button
  MyButton0(int x, int y, int w, int h, const char *label)
  : Fl_Button(x, y, w, h, label), z(7)
  {
    // 1: the macro need a pointer to the button first
    // 2: we can call a method in any class, but here we call ourselves
    // 3: call a method in our own class, so we need to set 'this' again
    // 4: this is the method that we want to call; it must be "public"
    FL_METHOD_CALLBACK(this, MyButton0, this, hello_0);
  }
  // public non-static callback method
  void hello_0() {
    // its not a static method, so we have full access to all members, i.e. 'z'
    fl_message("MyButton0 has the z value %d.", z);
  }
};

/*
 This class demonstrates method callbacks with two additional callback parameters.
 */
class MyButton2 : public Fl_Button {
  int z;
public:
  MyButton2(int x, int y, int w, int h, const char *label)
  : Fl_Button(x, y, w, h, label), z(3)
  {
    FL_METHOD_CALLBACK(this, MyButton2, this, hello_2, const char *, text, "CALLBACK", float, v, 3.141592654);
  }
  void hello_2(const char *text, float v) {
    fl_message("MyButton2 has the z value %d\nand the callback says\n%s %g", z, text, v);
  }
};

/*
 This class demonstrates method callbacks with four additional callback
 parameters. The maximum is currently five.
 */
class MyButton4 : public Fl_Button {
  int z;
public:
  MyButton4(int x, int y, int w, int h, const char *label)
  : Fl_Button(x, y, w, h, label), z(1) { }
  void hello_4(int a1, int a2, int a3, int a4) {
    fl_message("MyButton4 has the z value %d.\n%d %d %d %d", z, a1, a2, a3, a4);
  }
};


// here is a list of callback functions that can take parameter lists and are
// not limited to FLTK's built-in user_data `void*`.
void hello_0_args_cb() {
  fl_message("Hello with 0 arguments");
}

void hello_2_args_cb(const char *text, int number) {
  fl_message("Hello with 2 arguments,\n\"%s\" and '%d'", text, number);
}

void hello_4_args_cb(int a1, int a2, int a3, int a4) {
  fl_message("Hello with 4 arguments:\n%d %d %d %d", a1, a2, a3, a4);
}


int main(int argc, char ** argv) {
  window = new Fl_Window(580, 120);

  /* -- testing function callbacks with multiple arguments
   These buttons demo the use of the CALLBACK macro to call standard C style
   functions with up to five custom parameters.
   */

  new Fl_Box(10, 5, 180, 25, "Function Callbacks:");

  Fl_Button *func_cb_btn_0 = new Fl_Button(10, 30, 180, 25, "0 args");
  FL_FUNCTION_CALLBACK(func_cb_btn_0, hello_0_args_cb);

  Fl_Button *func_cb_btn_2 = new Fl_Button(10, 60, 180, 25, "2 args");
  FL_FUNCTION_CALLBACK(func_cb_btn_2, hello_2_args_cb, const char *, text, "FLTK", int, number, 2);

  Fl_Button *func_cb_btn_4 = new Fl_Button(10, 90, 180, 25, "4 args");
  FL_FUNCTION_CALLBACK(func_cb_btn_4, hello_4_args_cb, int, a1, 1, int, a2, 2, int, a3, 3, int, a4, 4);

  /* -- testing non-static method callbacks with multiple arguments
   The following buttons call non-static class methods with up to five
   additioanl parametrs. Check the classes above to see how this is implemented.
   */

  new Fl_Box(200, 5, 180, 25, "Method Callbacks:");

  MyButton0 *meth_cb_btn_0 = new MyButton0(200, 30, 180, 25, "0 args");

  MyButton2 *meth_cb_btn_2 = new MyButton2(200, 60, 180, 25, "2 args");

  MyButton4 *meth_cb_btn_4 = new MyButton4(200, 90, 180, 25, "4 args");
  FL_METHOD_CALLBACK(meth_cb_btn_4, MyButton4, meth_cb_btn_4, hello_4, int, a1, 1, int, a2, 2, int, a3, 3, int, a4, 4);

  /* -- testing inline callback functions
   Adding a simple Lambda style functionality to FLTK without actually using
   lambdas and staying C99 compatible.
   */

  new Fl_Box(390, 5, 180, 25, "Inline Callbacks:");

  Fl_Button *inline_cb_btn_0 = new Fl_Button(390, 30, 180, 25, "0 args");
  FL_INLINE_CALLBACK(inline_cb_btn_0,
                     { fl_message("Inline callabck with 0 args."); }
                     );

  Fl_Button *inline_cb_btn_2 = new Fl_Button(390, 60, 180, 25, "2 args");
  FL_INLINE_CALLBACK(inline_cb_btn_2,
                     { fl_message("We received the message %s with %d!", text, number); },
                     const char *, text, "FLTK", int, number, 2);

  Fl_Button *inline_cb_btn_4 = new Fl_Button(390, 90, 180, 25, "4 args");
  FL_INLINE_CALLBACK(inline_cb_btn_4,
                     { fl_message("The mein window was at\nx:%d, y:%d, w:%d, h:%d\n"
                                  "and is now at x:%d, y:%d", x, y, w, h,
                                  window->x(), window->y()); },
                     int, x, window->x(),
                     int, y, window->y(),
                     int, w, window->w(),
                     int, h, window->h());

  window->end();
  window->show(argc,argv);
  return Fl::run();
}
