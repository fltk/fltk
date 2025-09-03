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

#ifndef UNITTESTS_H
#define UNITTESTS_H 1

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>

#include <stdarg.h>

class Fl_Terminal;

// WINDOW/WIDGET SIZES
const int UT_MAINWIN_W  = 700;                                // main window w()
const int UT_MAINWIN_H  = 600;                                // main window h()
const int UT_BROWSER_X  = 10;                                 // browser x()
const int UT_BROWSER_Y  = 25;                                 // browser y()
const int UT_BROWSER_W  = 150;                                // browser w()
const int UT_BROWSER_H  = (UT_MAINWIN_H-35);                  // browser h()
const int UT_TESTAREA_X = (UT_BROWSER_W + 20);                // test area x()
const int UT_TESTAREA_Y = 25;                                 // test area y()
const int UT_TESTAREA_W = (UT_MAINWIN_W - UT_BROWSER_W - 30); // test area w()
const int UT_TESTAREA_H = UT_BROWSER_H;                       // test area h()

typedef void (*UnitTestCallback)(const char*, class Fl_Group*);

extern class Ut_Main_Window*  mainwin;
extern class Fl_Hold_Browser* browser;

enum {
  UT_TEST_ABOUT = 0,
  UT_TEST_POINTS,
  UT_TEST_FAST_SHAPES,
  UT_TEST_CIRCLES,
  UT_TEST_COMPLEX_SHAPES,
  UT_TEST_TEXT,
  UT_TEST_UNICODE,
  UT_TEST_SYBOL,
  UT_TEST_IMAGES,
  UT_TEST_VIEWPORT,
  UT_TEST_SCROLLBARSIZE,
  UT_TEST_SCHEMES,
  UT_TEST_SIMPLE_TERMINAL,
  UT_TEST_CORE
};

// This class helps to automatically register a new test with the unittest app.
// Please see the examples on how this is used.
class UnitTest {
public:
  UnitTest(int index, const char *label, Fl_Widget* (*create)());
  ~UnitTest();
  const char* label();
  void create();
  void show();
  void hide();
  static int num_tests() { return num_tests_; }
  static UnitTest* test(int i) { return test_list_[i]; }
private:
  char* label_;
  Fl_Widget* (*create_)();
  Fl_Widget* widget_;
  static void add(int index, UnitTest* t);
  static int num_tests_;
  static UnitTest* test_list_[];
};

// The following classes and macros implement a subset of the Google Test API
// without creating any external dependencies.
//
// There is nothing to initialise or set up. Just by including these classes,
// we can create tests anywhere inside the app by simply writing:
//
// TEST(Math, Addition) {
//   EXPECT_EQ(3+3, 6);
//   return true;
// }
// TEST(Math, Multiplication) {
//   EXPECT_EQ(3*3, 9);
//   return true;
// }
// RUN_ALL_TESTS();
//
// The test suite must only be run once.

typedef bool (*Ut_Test_Call)();

/**
 Implement a single test which can in turn contain many EXPECT_* macros.
 Ut_Test classes are automatically created using the TEST(suite_name, test_name)
 macro. Tests with identical suite names are grouped into a single suite.
 */
class Ut_Test {
  friend class Ut_Suite;
  const char *name_;
  Ut_Test_Call call_;
  bool failed_;
  bool done_;
public:
  Ut_Test(const char *suitename, const char *testname, Ut_Test_Call call);
  bool run(const char *suite);
  void print_failed(const char *suite);
};

/**
 Implement test registry and the grouping of tests into a suite. This class
 holds a number of static elements that register an arbitrary number of tests
 and groups them into suites via the TEST() macro.
 */
class Ut_Suite {
  static Ut_Suite **suite_list_;
  static int suite_list_size_;
  static int num_tests_;
  static int num_passed_;
  static int num_failed_;

  Ut_Test **test_list_;
  int test_list_size_;
  const char *name_;
  bool done_;
  Ut_Suite(const char *name);
public:
  void add(Ut_Test *test);
  int size() { return test_list_size_; }
  int run();
  void print_suite_epilog();
  void print_failed();
  static Ut_Suite *locate(const char *name);
  static int run_all_tests();
  static bool run_next_test();
  static void printf(const char *format, ...);
  static void log_bool(const char *file, int line, const char *cond, bool result, bool expected);
  static void log_string(const char *file, int line, const char *cond, const char *result, const char *expected);
  static void log_int(const char *file, int line, const char *cond, int result, const char *expected);
  static void print_prolog();
  static void print_epilog();
  static void color(int);
  static int failed() { return num_failed_; }
  static const char *red;
  static const char *green;
  static const char *normal;
  static Fl_Terminal *tty;
};

#define UT_CONCAT_(prefix, suffix) prefix##suffix
#define UT_CONCAT(prefix, suffix) UT_CONCAT_(prefix, suffix)

/** Create a test function and register it with the test suites.
 \param[in] SUITE naming of the test suite for grouping
 \param[in] CASE  name this test
 */
#define TEST(SUITE, CASE) \
  static bool UT_CONCAT(test_call_, __LINE__)(); \
  Ut_Test UT_CONCAT(test__, __LINE__)(#SUITE, #CASE, UT_CONCAT(test_call_, __LINE__)); \
  static bool UT_CONCAT(test_call_, __LINE__)()

/** Create a test case where the result is expected to be a boolena with the value true */
#define EXPECT_TRUE(COND) \
  bool UT_CONCAT(cond, __LINE__) = COND; \
  if (UT_CONCAT(cond, __LINE__) != true) { \
    Ut_Suite::log_bool(__FILE__, __LINE__, #COND, UT_CONCAT(cond, __LINE__), true); \
    return false; \
  }

/** Create a test case for string comparison. NULL is ok for both arguments. */
#define EXPECT_STREQ(A, B) \
  const char *UT_CONCAT(a, __LINE__) = A; \
  const char *UT_CONCAT(b, __LINE__) = B; \
  if (   (UT_CONCAT(a, __LINE__)==NULL && UT_CONCAT(b, __LINE__)!=NULL) \
      || (UT_CONCAT(a, __LINE__)!=NULL && UT_CONCAT(b, __LINE__)==NULL) \
      || (UT_CONCAT(b, __LINE__)!=NULL && strcmp(UT_CONCAT(a, __LINE__), UT_CONCAT(b, __LINE__))!=0) ) { \
  Ut_Suite::log_string(__FILE__, __LINE__, #A, UT_CONCAT(a, __LINE__), #B); \
    return false; \
  }

/** Create a test case for integer comparison. */
#define EXPECT_EQ(A, B) \
  int UT_CONCAT(a, __LINE__) = A; \
  int UT_CONCAT(b, __LINE__) = B; \
  if (UT_CONCAT(a, __LINE__) != UT_CONCAT(b, __LINE__)) { \
    Ut_Suite::log_int(__FILE__, __LINE__, #A, UT_CONCAT(a, __LINE__), #B); \
    return false; \
  }

/** Create a test case for integer comparison. */
#define EXPECT_NE(A, B) \
  int UT_CONCAT(a, __LINE__) = A; \
  int UT_CONCAT(b, __LINE__) = B; \
  if (UT_CONCAT(a, __LINE__) == UT_CONCAT(b, __LINE__)) { \
    Ut_Suite::log_int(__FILE__, __LINE__, #A, UT_CONCAT(a, __LINE__), #B); \
    return false; \
  }

/** Create a test case for integer comparison. */
#define EXPECT_LT(A, B) \
  int UT_CONCAT(a, __LINE__) = A; \
  int UT_CONCAT(b, __LINE__) = B; \
  if (UT_CONCAT(a, __LINE__) >= UT_CONCAT(b, __LINE__)) { \
    Ut_Suite::log_int(__FILE__, __LINE__, #A, UT_CONCAT(a, __LINE__), #B); \
    return false; \
  }

/** Create a test case for integer comparison. */
#define EXPECT_LE(A, B) \
  int UT_CONCAT(a, __LINE__) = A; \
  int UT_CONCAT(b, __LINE__) = B; \
  if (UT_CONCAT(a, __LINE__) > UT_CONCAT(b, __LINE__)) { \
    Ut_Suite::log_int(__FILE__, __LINE__, #A, UT_CONCAT(a, __LINE__), #B); \
    return false; \
  }

/** Create a test case for integer comparison. */
#define EXPECT_GT(A, B) \
  int UT_CONCAT(a, __LINE__) = A; \
  int UT_CONCAT(b, __LINE__) = B; \
  if (UT_CONCAT(a, __LINE__) <= UT_CONCAT(b, __LINE__)) { \
    Ut_Suite::log_int(__FILE__, __LINE__, #A, UT_CONCAT(a, __LINE__), #B); \
    return false; \
  }

/** Create a test case for integer comparison. */
#define EXPECT_GE(A, B) \
  int UT_CONCAT(a, __LINE__) = A; \
  int UT_CONCAT(b, __LINE__) = B; \
  if (UT_CONCAT(a, __LINE__) < UT_CONCAT(b, __LINE__)) { \
    Ut_Suite::log_int(__FILE__, __LINE__, #A, UT_CONCAT(a, __LINE__), #B); \
    return false; \
  }

/** Run all registered suits and their tests, and return the number of failed tests. */
#define RUN_ALL_TESTS() \
  Ut_Suite::run_all_tests()


// The main window needs an additional drawing feature in order to support
// the viewport alignment test.
class Ut_Main_Window : public Fl_Double_Window {
public:
  Ut_Main_Window(int w, int h, const char *l=0L);
  void draw_alignment_indicators();
  void draw() FL_OVERRIDE;
  void test_alignment(int v);
private:
  int draw_alignment_test_;
};

#endif
