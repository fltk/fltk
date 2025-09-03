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
#include <FL/fl_callback_macros.H>

Fl_Window *window = NULL;


/*
 Here is a list of callback functions that can take custom parameters and are
 not limited to FLTK's built-in `void*` or `long` user_data.
 */
void hello_0_args_cb() {
  fl_message("Hello with 0 arguments");
}

void hello_2_args_cb(const char *text, int number) {
  fl_message("Hello with 2 arguments,\n\"%s\" and '%d'", text, number);
}

void hello_4_args_cb(int a1, int a2, int a3, int a4) {
  fl_message("Hello with 4 arguments:\n%d %d %d %d", a1, a2, a3, a4);
}

/*
 We create our own little class here that uses method callbacks.
 */
class MyButton : public Fl_Button {
  // id will be set in the constructor
  int id_;
public:
  // create a simple push button
  MyButton(int x, int y, int w, int h, const char *label, int id)
  : Fl_Button(x, y, w, h, label), id_(id)
  { }
  // public non-static callback method
  void hello(int a, int b, int c) {
    // it's not a static method, so we have full access to all members, i.e. 'id_'
    fl_message("MyButton has the id %d\nand was called with the custom parameters\n%d, %d, and %d.", id_, a, b, c);
  }
};

/*
 Whenever the code created by the macro is called, custom parameters are
 duplicated. This ensures that each widget created dynamically with the same
 function has its own separate set of user data at runtime. Consequently,
 multiple widgets can be created dynamically, and each widget will have its
 own unique set of parameters.
 */
void make_button(Fl_Window *win, int set) {
  int y_lut[] = { 60, 90 };
  const char *label_lut[] = { "id 2 (5, 6, 7)", "id 3 (6, 7, 8)" };
  MyButton *btn = new MyButton(200, y_lut[set], 180, 25, label_lut[set], set+2);
  FL_METHOD_CALLBACK_3(btn, MyButton, btn, hello, int, set+5, int, set+6, int, set+7);
}


int main(int argc, char ** argv) {
  window = new Fl_Window(580, 120);

  /* -- testing function callbacks with multiple arguments
   These buttons demo the use of the CALLBACK macro to call standard C style
   functions with up to five custom parameters.
   */

  new Fl_Box(10, 5, 180, 25, "Function Callbacks:");

  Fl_Button *func_cb_btn_0 = new Fl_Button(10, 30, 180, 25, "0 args");
  FL_FUNCTION_CALLBACK_0(func_cb_btn_0, hello_0_args_cb);

  Fl_Button *func_cb_btn_2 = new Fl_Button(10, 60, 180, 25, "2 args");
  FL_FUNCTION_CALLBACK_2(func_cb_btn_2, hello_2_args_cb, const char *, "FLTK", int, 2);

  Fl_Button *func_cb_btn_4 = new Fl_Button(10, 90, 180, 25, "4 args");
  FL_FUNCTION_CALLBACK_4(func_cb_btn_4, hello_4_args_cb, int, 1, int, 2, int, 3, int, 4);

  /* -- testing non-static method callbacks with multiple arguments
   The following buttons call non-static class methods with custom parameters.
   Check the class above to see how this is implemented.
   */

  new Fl_Box(200, 5, 180, 25, "Method Callbacks:");

  MyButton *meth_cb_btn_0 = new MyButton(200, 30, 180, 25, "id 1 (1, 2, 3)", 1);
  // 1: the macro needs a pointer to the button first
  // 2: we can call a method in any class, but here we call ourselves
  // 3: call a method in our own class, so we need to set 'meth_cb_btn_0' again
  //    Note: we could just as well call a method in a different class.
  // 4: this is the method that we want to call; it must be "public"
  // 5: add zero to five parameter triplets, note the comma placement
  FL_METHOD_CALLBACK_3(meth_cb_btn_0, MyButton, meth_cb_btn_0, hello, int, 1, int, 2, int, 3);

  // call the same FL_METHOD_CALLBACK macro multiple times to ensure we get
  // individual parameter sets
  make_button(window, 0);
  make_button(window, 1);

  /* -- testing inline callback functions
   Adding a simple Lambda style functionality to FLTK without actually using
   lambdas and staying C++99 compatible.
   */

  new Fl_Box(390, 5, 180, 25, "Inline Callbacks:");

  Fl_Button *inline_cb_btn_0 = new Fl_Button(390, 30, 180, 25, "0 args");
  FL_INLINE_CALLBACK_0(inline_cb_btn_0,
                       { fl_message("Inline callback with 0 args."); }
                       );

  Fl_Button *inline_cb_btn_2 = new Fl_Button(390, 60, 180, 25, "2 args");
  FL_INLINE_CALLBACK_2(inline_cb_btn_2,
                       const char *, text, "FLTK", int, number, 2,
                       { fl_message("We received the message %s with %d!", text, number); }
                       );

  Fl_Button *inline_cb_btn_4 = new Fl_Button(390, 90, 180, 25, "4 args");
  FL_INLINE_CALLBACK_4(inline_cb_btn_4,
                       int, x, window->x(),
                       int, y, window->y(),
                       int, w, window->w(),
                       int, h, window->h(),
                       { fl_message("The main window was at\nx:%d, y:%d, w:%d, h:%d\n"
                                    "when the callback was created\n"
                                    "and is now at x:%d, y:%d", x, y, w, h,
                                    window->x(), window->y());
                       }
                       );

  window->end();
  window->show(argc,argv);
  return Fl::run();
}
