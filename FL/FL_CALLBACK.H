/*
 * Macros for easy callback for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 2023 by Bill Spitzak and others.
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file.  If this
 * file is missing or damaged, see the license at:
 *
 *     https://www.fltk.org/COPYING.php
 *
 * Please see the following page on how to report bugs and issues:
 *
 *     https://www.fltk.org/bugs.php
 */

/**
 \file FL_CALLBACK.H
 This file provides macros for easy callbacks of functions and methods
 with multiple a type safe arguments.
*/

#ifdef FL_DOXYGEN

/**
 \brief Declare a plain C or static method callback with custom parameters.

 This macro makes it easy to call arbitray functions with up to five custom
 parameters. It expands FLTK's limitation of passing a single `void*` argument
 and is type safe.

 The first argument is the widget that will handle this callback. The second
 argument is a regular function or a static method in any class. Up to five
 triplets can now follow, where each triplet is a comma separated list of
 a type, the parameter name, and the value, for example: `int, x, 3`. Passing
 two arguments is done in two triplets: `int, x, 3, int, y, 4`.

 \code{.cpp}
 #include <FL/FL_CALLBACK.H>
 ...
 Fl_Button *btn1 = new Fl_Button(10, 10, 100, 20, "Beep");
 FL_FUNCTION_CALLBACK(btn1, fl_beep);
 ...
 Fl_Button *btn2 = new Fl_Button(10, 40, 100, 20, "Hello");
 FL_FUNCTION_CALLBACK(btn2,
   fl_message,
   const char *, text, "Hello\n%d %d %d %d",
   int, a, 1,  int, b, 2,  int, c, 3,  int, d, 4
 );
 \endcode

 There is a little demo program for FL_*_*CALLBACK in `examples/callbacks.cxx`.

 \param WIDGET the widget that will call the callback
 \param FUNC a C/C++ function or a static class method
 \param ... a list of zero to five triples declaring type, name, and value

 \see FL_METHOD_CALLBACK, FL_INLINE_CALLBACK
 */
#define FL_FUNCTION_CALLBACK(WIDGET, FUNC, ...)

/**
 \brief Declare a non-static class method callback with custom parameters.

 This macro makes it easy to call arbitray methods in any class with up to five
 custom parameters. It expands FLTK's limitation of passing a single `void*`
 argument and is type safe.

 The first argument is the widget that will handle this callback. The second
 argument indicates the class type that will be called. The third argument
 must be a pointer to an instance of that class. The fourth argument is the
 method in that class. It must be public and should not be static.

 Up to five triplets can now follow, where each triplet is a comma separated
 list of a type, the parameter name, and the value, for example: `int, x, 3`.
 Passing two arguments is done in two triplets: `int, x, 3, int, y, 4`.

 \code{.cpp}
 #include <FL/FL_CALLBACK.H>
 ...
 Fl_Button *btn = new Fl_Button(10, 10, 100, 20, "Test");
 FL_METHOD_CALLBACK(btn, Fl_Button, btn, color, Fl_Color, c, FL_GREEN);
 \endcode

 There is a little demo program for FL_*_*CALLBACK in `examples/callbacks.cxx`.

 \param WIDGET the widget that will call the callback
 \param CLASS the class type
 \param SELF a pointer to an instance of the class
 \param METH a C++ class method that must be public and not static
 \param ... a list of zero to five triples declaring type, name, and value

 \see FL_METHOD_CALLBACK, FL_INLINE_CALLBACK
 */
#define FL_METHOD_CALLBACK(WIDGET, CLASS, SELF, METH, ...)

/**
 \brief Inline a function as a callback with custom parameters.

 This macro makes it possible to write a callback function right where the
 widget and callback is declared, somewhat similar to a Lambda function. The
 function body is limited to a syntacx that the macro preprocessor tollerates.

 The first argument is the widget that will handle this callback. The second
 argument is the function body itself. Up to five triplets can now follow,
 where each triplet is a comma separated list of a type, the parameter name,
 and the value, for example: `int, x, 3`. Passing two arguments is done in two
 triplets: `int, x, 3, int, y, 4`.

 \code{.cpp}
 #include <FL/FL_CALLBACK.H>
 ...
 Fl_Button *btn = new Fl_Button(10, 10, 100, 20, "Test");
 FL_INLINE_CALLBACK(btn,
   { fl_message("Greetings for the %s button", name); },
   const char*, name, btn->label()
 );
 \endcode

 There is a little demo program for FL_*_*CALLBACK in `examples/callbacks.cxx`.

 \param WIDGET the widget that will call the callback
 \param FUNC the function body within the limits of the C macro preprocessor
 \param ... a list of zero to five triples declaring type, name, and value

 \see FL_METHOD_CALLBACK, FL_INLINE_CALLBACK
 */
#define FL_INLINE_CALLBACK(WIDGET, FUNC, ...)

#else // FL_DOXYGEN

#ifndef _FL_FL_CALLBACK_H_
#define _FL_FL_CALLBACK_H_

#define _FL_LIST_MEMBERS_5(i00, i01, i02, i10, i11, i12, i20, i21, i22, i30, i31, i32, i40, i41, i42) i00 i01; i10 i11; i20 i21; i30 i31; i40 i41;
#define _FL_LIST_FN_ARGS_5(i00, i01, i02, i10, i11, i12, i20, i21, i22, i30, i31, i32, i40, i41, i42) i00 i01, i10 i11, i20 i21, i30 i31, i40 i41
#define _FL_LIST_ARGS_5(i00, i01, i02, i10, i11, i12, i20, i21, i22, i30, i31, i32, i40, i41, i42) cbdata->i01, cbdata->i11, cbdata->i21, cbdata->i31, cbdata->i41
#define _FL_LIST_VALUES_5(i00, i01, i02, i10, i11, i12, i20, i21, i22, i30, i31, i32, i40, i41, i42) i02, i12, i22, i32, i42

#define _FL_LIST_MEMBERS_4(i00, i01, i02, i10, i11, i12, i20, i21, i22, i30, i31, i32) i00 i01; i10 i11; i20 i21; i30 i31;
#define _FL_LIST_FN_ARGS_4(i00, i01, i02, i10, i11, i12, i20, i21, i22, i30, i31, i32) i00 i01, i10 i11, i20 i21, i30 i31
#define _FL_LIST_ARGS_4(i00, i01, i02, i10, i11, i12, i20, i21, i22, i30, i31, i32) cbdata->i01, cbdata->i11, cbdata->i21, cbdata->i31
#define _FL_LIST_VALUES_4(i00, i01, i02, i10, i11, i12, i20, i21, i22, i30, i31, i32) i02, i12, i22, i32

#define _FL_LIST_MEMBERS_3(i00, i01, i02, i10, i11, i12, i20, i21, i22) i00 i01; i10 i11; i20 i21;
#define _FL_LIST_FN_ARGS_3(i00, i01, i02, i10, i11, i12, i20, i21, i22) i00 i01, i10 i11, i20 i21
#define _FL_LIST_ARGS_3(i00, i01, i02, i10, i11, i12, i20, i21, i22) cbdata->i01, cbdata->i11, cbdata->i21
#define _FL_LIST_VALUES_3(i00, i01, i02, i10, i11, i12, i20, i21, i22) i02, i12, i22

#define _FL_LIST_MEMBERS_2(i00, i01, i02, i10, i11, i12) i00 i01; i10 i11;
#define _FL_LIST_FN_ARGS_2(i00, i01, i02, i10, i11, i12) i00 i01, i10 i11
#define _FL_LIST_ARGS_2(i00, i01, i02, i10, i11, i12) cbdata->i01, cbdata->i11
#define _FL_LIST_VALUES_2(i00, i01, i02, i10, i11, i12) i02, i12

#define _FL_LIST_MEMBERS_1(i00, i01, i02) i00 i01;
#define _FL_LIST_FN_ARGS_1(i00, i01, i02) i00 i01
#define _FL_LIST_ARGS_1(i00, i01, i02) cbdata->i01
#define _FL_LIST_VALUES_1(i00, i01, i02) i02

#define _FL_LIST_MEMBERS_0()
#define _FL_LIST_FN_ARGS_0()
#define _FL_LIST_ARGS_0()
#define _FL_LIST_VALUES_0()

#define FL_COUNT_BY_3(i00, i01, i02, i10, i11, i12, i20, i21, i22, i30, i31, i32, i40, i41, i42, cnt, ...) cnt

#define FL_CONCAT_IMPL(x, y) x##y
#define FL_CONCAT(x, y) FL_CONCAT_IMPL(x, y)

#define FL_FUNCTION_CALLBACK(WIDGET, FUNC, ...) do {\
typedef struct FL_CONCAT(cbtype_,__LINE__) { \
FL_CONCAT(_FL_LIST_MEMBERS_, FL_COUNT_BY_3(__VA_ARGS__, 5, err, err, 4, err, err, 3, err, err, 2, err, err, 1, err, 0, 0))(__VA_ARGS__) \
static void cb(Fl_Widget *w, void *user_data) { \
FL_CONCAT(cbtype_,__LINE__) *cbdata = (FL_CONCAT(cbtype_,__LINE__)*)user_data; \
FUNC( \
FL_CONCAT(_FL_LIST_ARGS_, FL_COUNT_BY_3(__VA_ARGS__, 5, err, err, 4, err, err, 3, err, err, 2, err, err, 1, err, 0, 0))(__VA_ARGS__) \
); }; \
} FL_CONCAT(cbtype_,__LINE__); \
static FL_CONCAT(cbtype_,__LINE__) FL_CONCAT(cbd_,__LINE__) = { \
FL_CONCAT(_FL_LIST_VALUES_, FL_COUNT_BY_3(__VA_ARGS__, 5, err, err, 4, err, err, 3, err, err, 2, err, err, 1, err, 0, 0))(__VA_ARGS__) \
}; \
WIDGET->callback(FL_CONCAT(cbd_,__LINE__).cb, &FL_CONCAT(cbd_,__LINE__)); } while(0)

#define FL_METHOD_CALLBACK(WIDGET, CLASS, SELF, METH, ...) do {\
typedef struct FL_CONCAT(cbtype_,__LINE__) { \
CLASS *self; \
FL_CONCAT(_FL_LIST_MEMBERS_, FL_COUNT_BY_3(__VA_ARGS__, 5, err, err, 4, err, err, 3, err, err, 2, err, err, 1, err, 0, 0))(__VA_ARGS__) \
static void cb(Fl_Widget *w, void *user_data) { \
FL_CONCAT(cbtype_,__LINE__) *cbdata = (FL_CONCAT(cbtype_,__LINE__)*)user_data; \
cbdata->self->METH( \
FL_CONCAT(_FL_LIST_ARGS_, FL_COUNT_BY_3(__VA_ARGS__, 5, err, err, 4, err, err, 3, err, err, 2, err, err, 1, err, 0, 0))(__VA_ARGS__) \
); }; \
} FL_CONCAT(cbtype_,__LINE__); \
static FL_CONCAT(cbtype_,__LINE__) FL_CONCAT(cbd_,__LINE__) = { \
SELF, FL_CONCAT(_FL_LIST_VALUES_, FL_COUNT_BY_3(__VA_ARGS__, 5, err, err, 4, err, err, 3, err, err, 2, err, err, 1, err, 0, 0))(__VA_ARGS__) \
}; \
WIDGET->callback(FL_CONCAT(cbd_,__LINE__).cb, &FL_CONCAT(cbd_,__LINE__)); } while(0)

#define FL_INLINE_CALLBACK(WIDGET, FUNC, ...) do {\
typedef struct FL_CONCAT(cbtype_,__LINE__) { \
FL_CONCAT(_FL_LIST_MEMBERS_, FL_COUNT_BY_3(__VA_ARGS__, 5, err, err, 4, err, err, 3, err, err, 2, err, err, 1, err, 0, 0))(__VA_ARGS__) \
static void func(\
FL_CONCAT(_FL_LIST_FN_ARGS_, FL_COUNT_BY_3(__VA_ARGS__, 5, err, err, 4, err, err, 3, err, err, 2, err, err, 1, err, 0, 0))(__VA_ARGS__) \
) FUNC; \
static void cb(Fl_Widget *w, void *user_data) { \
FL_CONCAT(cbtype_,__LINE__) *cbdata = (FL_CONCAT(cbtype_,__LINE__)*)user_data; \
cbdata->func( \
FL_CONCAT(_FL_LIST_ARGS_, FL_COUNT_BY_3(__VA_ARGS__, 5, err, err, 4, err, err, 3, err, err, 2, err, err, 1, err, 0, 0))(__VA_ARGS__) \
); }; \
} FL_CONCAT(cbtype_,__LINE__); \
static FL_CONCAT(cbtype_,__LINE__) FL_CONCAT(cbd_,__LINE__) = { \
FL_CONCAT(_FL_LIST_VALUES_, FL_COUNT_BY_3(__VA_ARGS__, 5, err, err, 4, err, err, 3, err, err, 2, err, err, 1, err, 0, 0))(__VA_ARGS__) \
}; \
WIDGET->callback(FL_CONCAT(cbd_,__LINE__).cb, &FL_CONCAT(cbd_,__LINE__)); } while(0)

//  The macro:
//  FL_FUNCTION_CALLBACK(func_cb_btn_4, hello_4_args_cb, int, a1, 1, int, a2, 2, int, a3, 3, int, a4, 4);
//  expands to:
//  do {
//    typedef struct cbtype_114 {
//      int a1; int a2; int a3; int a4;
//      static void cb(Fl_Widget *w, void *user_data) {
//        cbtype_114 *cbdata = (cbtype_114*)user_data;
//        hello_4_args_cb( cbdata->a1, cbdata->a2, cbdata->a3, cbdata->a4 );
//      };
//    } cbtype_114;
//    static cbtype_114 cbd_114 = { 1, 2, 3, 4 };
//    func_cb_btn_4->callback(cbd_114.cb, &cbd_114);
//  } while(0)

//  The macro:
//  FL_METHOD_CALLBACK(this, MyButton2, this, hello_2, const char *, text, "CALLBACK", float, v, 3.141592654);
//  expands to:
//  do {
//    typedef struct cbtype_60 {
//      MyButton2 *self; const char * text; float v;
//      static void cb(Fl_Widget *w, void *user_data) {
//        cbtype_60 *cbdata = (cbtype_60*)user_data;
//        cbdata->self->hello_2( cbdata->text, cbdata->v );
//      };
//    } cbtype_60;
//    static cbtype_60 cbd_60 = { this, "CALLBACK", 3.141592654 };
//    this->callback(cbd_60.cb, &cbd_60);
//  } while(0);

//  The macro:
//  FL_INLINE_CALLBACK(inline_cb_btn_2,
//                     { fl_message("We received the message %s with %d!", text, number); },
//                     const char *, text, "FLTK", int, number, 2);
//  expands to:
//  do {
//    typedef struct cbtype_145 {
//      const char * text; int number;
//      static void func(const char * text, int number ) {
//        fl_message("We received the message %s with %d!", text, number);
//      };
//      static void cb(Fl_Widget *w, void *user_data) {
//        cbtype_145 *cbdata = (cbtype_145*)user_data; cbdata->func( cbdata->text, cbdata->number );
//      };
//    } cbtype_145;
//    static cbtype_145 cbd_145 = { "FLTK", 2 };
//    inline_cb_btn_2->callback(cbd_145.cb, &cbd_145);
//  } while(0);

#endif // FL_DOXYGEN

#endif /* !_FL_FL_CALLBACK_H_ */
