//
// Unit tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
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

// Fltk unit tests
// v0.1 - Greg combines Matthias + Ian's tests
// v0.2 - Ian's 02/12/09 fixes applied
// v0.3 - Fixes to circle desc, augmented extent tests, fixed indents, added show(argc,argv)
// v1.0 - Submit for svn
// v1.1 - Matthias seperated all tests into multiple source files for hopefully easier handling

// NOTE: See README-unittests.txt for how to add new tests.

#include "unittests.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Terminal.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>               // fl_text_extents()
#include <FL/fl_string_functions.h>   // fl_strdup()
#include <FL/fl_ask.H>                // fl_message()
#include <stdlib.h>                   // malloc, free

class Ut_Main_Window *mainwin = NULL;
class Fl_Hold_Browser *browser = NULL;

int UnitTest::num_tests_ = 0;
UnitTest *UnitTest::test_list_[200] = { 0 };

// ----- UnitTest ------------------------------------------------------ MARK: -

UnitTest::UnitTest(int index, const char* label, Fl_Widget* (*create)())
  : widget_(0L)
{
  label_ = fl_strdup(label);
  create_ = create;
  add(index, this);
}

UnitTest::~UnitTest() {
  delete widget_;
  free(label_);
}

const char *UnitTest::label() {
  return label_;
}

void UnitTest::create() {
  widget_ = create_();
  if (widget_) widget_->hide();
}

void UnitTest::show() {
  if (widget_) widget_->show();
}

void UnitTest::hide() {
  if (widget_) widget_->hide();
}

void UnitTest::add(int index, UnitTest* t) {
  test_list_[index] = t;
  if (index >= num_tests_)
    num_tests_ = index+1;
}

// ----- Ut_Main_Window ------------------------------------------------ MARK: -

Ut_Main_Window::Ut_Main_Window(int w, int h, const char *l)
  : Fl_Double_Window(w, h, l),
    draw_alignment_test_(0)
{ }

void Ut_Main_Window::draw_alignment_indicators() {
  const int SZE = 16;
  // top left corner
  fl_color(FL_GREEN); fl_yxline(0, SZE, 0, SZE);
  fl_color(FL_RED);   fl_yxline(-1, SZE, -1, SZE);
  fl_color(FL_WHITE); fl_rectf(3, 3, SZE-2, SZE-2);
  fl_color(FL_BLACK); fl_rect(3, 3, SZE-2, SZE-2);
  // bottom left corner
  fl_color(FL_GREEN); fl_yxline(0, h()-SZE-1, h()-1, SZE);
  fl_color(FL_RED);   fl_yxline(-1, h()-SZE-1, h(), SZE);
  fl_color(FL_WHITE); fl_rectf(3, h()-SZE-1, SZE-2, SZE-2);
  fl_color(FL_BLACK); fl_rect(3, h()-SZE-1, SZE-2, SZE-2);
  // bottom right corner
  fl_color(FL_GREEN); fl_yxline(w()-1, h()-SZE-1, h()-1, w()-SZE-1);
  fl_color(FL_RED);   fl_yxline(w(), h()-SZE-1, h(), w()-SZE-1);
  fl_color(FL_WHITE); fl_rectf(w()-SZE-1, h()-SZE-1, SZE-2, SZE-2);
  fl_color(FL_BLACK); fl_rect(w()-SZE-1, h()-SZE-1, SZE-2, SZE-2);
  // top right corner
  fl_color(FL_GREEN); fl_yxline(w()-1, SZE, 0, w()-SZE-1);
  fl_color(FL_RED);   fl_yxline(w(), SZE, -1, w()-SZE-1);
  fl_color(FL_WHITE); fl_rectf(w()-SZE-1, 3, SZE-2, SZE-2);
  fl_color(FL_BLACK); fl_rect(w()-SZE-1, 3, SZE-2, SZE-2);
}

void Ut_Main_Window::draw() {
  Fl_Double_Window::draw();
  if (draw_alignment_test_) {
    draw_alignment_indicators();
  }
}

void Ut_Main_Window::test_alignment(int v) {
  draw_alignment_test_ = v;
  redraw();
}

// ----- Ut_Test ------------------------------------------------------- MARK: -

/** Create a unit test that can contain many test cases using EXPECT_*.

 Ut_Test should be instantiated by using the TEST(SUITE, NAME) macro. Multiple
 TEST() macros can be called within the same source file to generate an
 arbitrary number of tests. TEST() can be called in multiple source files
 of the same application.

 The constructor also registers the test with Ut_Suite and groups it by name.
 */
Ut_Test::Ut_Test(const char *suitename, const char *testname, Ut_Test_Call call)
: name_(testname),
  call_(call),
  failed_(false),
  done_(false)
{
  Ut_Suite *suite = Ut_Suite::locate(suitename);
  suite->add(this);
}

/** Run all cases inside the test and return false when the first case fails. */
bool Ut_Test::run(const char *suite) {
  Ut_Suite::printf("%s[ RUN      ]%s %s.%s\n",
                   Ut_Suite::green, Ut_Suite::normal, suite, name_);
  bool ret = call_();
  if (ret) {
    Ut_Suite::printf("%s[       OK ]%s %s.%s\n",
                     Ut_Suite::green, Ut_Suite::normal, suite, name_);
    failed_ = false;
  } else {
    Ut_Suite::printf("%s[  FAILED  ]%s %s.%s\n",
                     Ut_Suite::red, Ut_Suite::normal, suite, name_);
    failed_ = true;
  }
  done_ = true;
  return ret;
}

/** Print a message is the test was previously marked failed. */
void Ut_Test::print_failed(const char *suite) {
  if (failed_) {
    Ut_Suite::printf("%s[  FAILED  ]%s %s.%s\n",
                     Ut_Suite::red, Ut_Suite::normal, suite, name_);
  }
}

// ----- Ut_Suite ------------------------------------------------------ MARK: -

Ut_Suite **Ut_Suite::suite_list_ = NULL;
int Ut_Suite::suite_list_size_ = 0;
int Ut_Suite::num_tests_ = 0;
int Ut_Suite::num_passed_ = 0;
int Ut_Suite::num_failed_ = 0;
const char *Ut_Suite::red = "\033[31m";
const char *Ut_Suite::green = "\033[32m";
const char *Ut_Suite::normal = "\033[0m";
Fl_Terminal *Ut_Suite::tty = NULL;

/** Switch the user of color escape sequnces in the log text. */
void Ut_Suite::color(int v) {
  if (v) {
    red = "\033[31m";
    green = "\033[32m";
    normal = "\033[0m";
  } else {
    red = "";
    green = "";
    normal = "";
  }
}

/** Create a suite that will group tests by the suite name.

 Ut_Suite is automatically instantiated by using the TEST(SUITE, NAME) macro.
 Multiple TEST() macros are grouped into suits by suite name.
 */
Ut_Suite::Ut_Suite(const char *name)
: test_list_(NULL),
  test_list_size_(0),
  name_(name),
  done_(false)
{
}

/** Add a test to the suite. This is done automatically by the TEST() macro. */
void Ut_Suite::add(Ut_Test *test) {
  if ( (test_list_size_ % 16) == 0 ) {
    test_list_ = (Ut_Test**)realloc(test_list_, (test_list_size_+16)*sizeof(Ut_Test*));
  }
  test_list_[test_list_size_++] = test;
}

/** Static method that will find or create a suite by name. */
Ut_Suite *Ut_Suite::locate(const char *name) {
  for (int i=0; i<suite_list_size_; i++) {
    if (strcmp(name, suite_list_[i]->name_)==0)
      return suite_list_[i];
  }
  if ( (suite_list_size_ % 16) == 0 ) {
    suite_list_ = (Ut_Suite**)realloc(suite_list_, (suite_list_size_+16)*sizeof(Ut_Suite*));
  }
  Ut_Suite *s = new Ut_Suite(name);
  suite_list_[suite_list_size_++] = s;
  return s;
}

/** Logs the start of a test suite run. */
void Ut_Suite::print_suite_epilog() {
  Ut_Suite::printf("%s[----------]%s %d test%s from %s\n", Ut_Suite::green, Ut_Suite::normal,
                   test_list_size_, test_list_size_ == 1 ? "" : "s", name_);
}

/** Run all tests in a single suite, returning the number of failed tests. */
int Ut_Suite::run() {
  print_suite_epilog();
  int num_tests_failed = 0;
  for (int i=0; i<test_list_size_; i++) {
    if (!test_list_[i]->run(name_)) {
      num_tests_failed++;
    }
  }
  return num_tests_failed;
}

/** Static method to log all tests that are marked failed. */
void Ut_Suite::print_failed() {
  for (int i=0; i<test_list_size_; i++) {
    test_list_[i]->print_failed(name_);
  }
}

/** Static method to log the start of a test run. */
void Ut_Suite::print_prolog() {
  int i;
  num_tests_ = 0;
  num_passed_ = 0;
  num_failed_ = 0;
  for (i=0; i<suite_list_size_; i++) {
    num_tests_ += suite_list_[i]->size();
  }
  Ut_Suite::printf("%s[==========]%s Running %d tests from %d test case%s.\n",
                   Ut_Suite::green, Ut_Suite::normal,
                   num_tests_, suite_list_size_,
                   suite_list_size_ == 1 ? "" : "s");
}

/** Static method to log the end of a test run. */
void Ut_Suite::print_epilog() {
  int i;
  Ut_Suite::printf("%s[==========]%s %d tests from %d test case%s ran.\n",
                   Ut_Suite::green, Ut_Suite::normal,
                   num_tests_, suite_list_size_,
                   suite_list_size_ == 1 ? "" : "s");
  if (num_passed_) {
    Ut_Suite::printf("%s[  PASSED  ]%s %d test%s.\n",
                     Ut_Suite::green, Ut_Suite::normal, num_passed_,
                     num_passed_ == 1 ? "" : "s");
  }
  if (num_failed_) {
    Ut_Suite::printf("%s[  FAILED  ]%s %d test%s, listed below:\n",
                     Ut_Suite::red, Ut_Suite::normal, num_failed_,
                     num_failed_ == 1 ? "" : "s");
  }
  for (i=0; i<suite_list_size_; i++) {
    suite_list_[i]->print_failed();
  }
}

/** Static method to run all tests in all test suites.
 Returns the number of failed tests.
 */
int Ut_Suite::run_all_tests() {
  print_prolog();
  // loop through all suites which then loop through all tests
  for (int i=0; i<suite_list_size_; i++) {
    int n = suite_list_[i]->run();
    num_passed_ += suite_list_[i]->size() - n;
    num_failed_ += n;
  }
  print_epilog();
  return num_failed_;
}

/** Static method to run all test, one-by-one, until done.
 Run all tests by calling `while (Ut_Suite::run_next_test()) { }`.
 This is used to visualise test progress with the terminal window by runnig test
 asynchronously and adding a noticable delay between calls.
 */
bool Ut_Suite::run_next_test() {
  // if all suites are done, print the ending text and return
  Ut_Suite *last = suite_list_[suite_list_size_-1];
  if (last->done_) {
    print_epilog();
    return false;
  }
  // if no tests ran yet, print the starting text
  Ut_Suite *first = suite_list_[0];
  if (!first->done_ && !first->test_list_[0]->done_) {
    print_prolog();
  }
  // now find the next test that hasn't ran yet
  for (int i=0; i<suite_list_size_; i++) {
    Ut_Suite *st = suite_list_[i];
    if (st->done_) continue;
    if (!st->test_list_[0]->done_)
      st->print_suite_epilog();
    for (int j=0; j<st->test_list_size_; j++) {
      if (st->test_list_[j]->done_) continue;
      if (st->test_list_[j]->run(st->name_)) num_passed_++; else num_failed_++;
      return true;
    }
    st->done_ = true;
    return true;
  }
  return true;
}

/** A printf that is redirected to the terminal or stdout. */
void Ut_Suite::printf(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  if (tty) {
    tty->vprintf(format, args);
  } else {
    vprintf(format, args);
  }
  va_end(args);
}

/** Log the result of a boolean case fail. */
void Ut_Suite::log_bool(const char *file, int line, const char *cond, bool result, bool expected) {
  Ut_Suite::printf("%s(%d): error:\n", file, line);
  Ut_Suite::printf("Value of: %s\n", cond);
  Ut_Suite::printf("Actual:   %s\n", result ? "true" : "false");
  Ut_Suite::printf("Expected: %s\n", expected ? "true" : "false");
}

/** Log the result of a string comparison case fail. */
void Ut_Suite::log_string(const char *file, int line, const char *cond, const char *result, const char *expected) {
  Ut_Suite::printf("%s(%d): error:\n", file, line);
  Ut_Suite::printf("Value of: %s\n", cond);
  Ut_Suite::printf("  Actual: %s\n", result);
  Ut_Suite::printf("Expected: %s\n", expected);
}

/** Log the result of an integer comparison case fail. */
void Ut_Suite::log_int(const char *file, int line, const char *cond, int result, const char *expected) {
  Ut_Suite::printf("%s(%d): error:\n", file, line);
  Ut_Suite::printf("Value of: %s\n", cond);
  Ut_Suite::printf("  Actual: %d\n", result);
  Ut_Suite::printf("Expected: %s\n", expected);
}

// ----- main ---------------------------------------------------------- MARK: -

// callback whenever the browser value changes
void ui_browser_cb(Fl_Widget*, void*) {
  for ( int t=1; t<=browser->size(); t++ ) {
    UnitTest* ti = (UnitTest*)browser->data(t);
    if ( browser->selected(t) ) {
      ti->show();
    } else {
      ti->hide();
    }
  }
}

static bool run_core_tests_only = false;

static int arg(int argc, char** argv, int& i) {
  if ( strcmp(argv[i], "--core") == 0 ) {
    run_core_tests_only = true;
    i++;
    return 1;
  }
  if ( strcmp(argv[i], "--color=0") == 0 ) {
    Ut_Suite::color(0);
    i++;
    return 1;
  }
  if ( strcmp(argv[i], "--color=1") == 0 ) {
    Ut_Suite::color(1);
    i++;
    return 1;
  }
  if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
    return 0;
  }
  return 0;
}

// This is the main call. It creates the window and adds all previously
// registered tests to the browser widget.
int main(int argc, char** argv) {
  int i;
  Fl::args_to_utf8(argc, argv); // for MSYS2/MinGW
  if ( Fl::args(argc,argv,i,arg) == 0 ) {   // unsupported argument found
    static const char *msg =
      "usage: %s <switches>\n"
      " --core : test core functionality only\n"
      " --color=1, --color=0 : print test output in color or plain text"
      " --help, -h : print this help page\n";
    const char *app_name = NULL;
    if ( (argc > 0) && argv[0] && argv[0][0] )
      app_name = fl_filename_name(argv[0]);
    if ( !app_name || !app_name[0])
      app_name = "unittests";
#ifdef _MSC_VER
    fl_message(msg, app_name);
#else
    fprintf(stderr, msg, app_name);
#endif
    return 1;
  }

  if (run_core_tests_only) {
    return RUN_ALL_TESTS();
  }

  Fl::get_system_colors();
  Fl::scheme(Fl::scheme()); // init scheme before instantiating tests
  Fl::visual(FL_RGB);
  Fl::use_high_res_GL(1);
  mainwin = new Ut_Main_Window(UT_MAINWIN_W, UT_MAINWIN_H, "FLTK Unit Tests");
  mainwin->size_range(UT_MAINWIN_W, UT_MAINWIN_H);
  browser = new Fl_Hold_Browser(UT_BROWSER_X, UT_BROWSER_Y, UT_BROWSER_W, UT_BROWSER_H, "Unit Tests");
  browser->align(FL_ALIGN_TOP|FL_ALIGN_LEFT);
  browser->when(FL_WHEN_CHANGED);
  browser->callback(ui_browser_cb);
  browser->linespacing(2);

  int n = UnitTest::num_tests();
  for (i=0; i<n; i++) {
    UnitTest* t = UnitTest::test(i);
    if (t) {
      mainwin->begin();
      t->create();
      mainwin->end();
      browser->add(t->label(), (void*)t);
    }
  }

  mainwin->resizable(mainwin);
  mainwin->show(argc, argv);
  // Select first test in browser, and show that test.
  browser->select(UT_TEST_ABOUT+1);
  ui_browser_cb(browser, 0);
  return Fl::run();
}
