//
// Unit tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

#include "unittests.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Simple_Terminal.H>
#include <FL/Fl_String.H>

/* Test Fl_String constructor and assignment. */
TEST(Fl_String, Assignment) {
  Fl_String null;
  EXPECT_STREQ(null.c_str(), "");   // default initialisation is an empty string
  EXPECT_TRUE(null.empty());

  Fl_String null2(NULL);
  EXPECT_STREQ(null2.c_str(), "");  // initialise with a NULL pointer gets an empty string
  EXPECT_TRUE(null2.empty());

  Fl_String empty("");
  EXPECT_STREQ(empty.c_str(), "");  // also, empty CString make empty Fl_String
  EXPECT_TRUE(empty.empty());

  Fl_String text("hello");
  EXPECT_STREQ(text.c_str(), "hello");  // Load some text from a CString
  EXPECT_EQ(text.size(), 5);        // did we get the size right?
  EXPECT_EQ(text.strlen(), 5);      // do we have a trailing 0
  EXPECT_GE(text.capacity(), 5);    // do we have the capacity
  EXPECT_TRUE(!text.empty());       // test the empty() method

  Fl_String text2("abcdef", 3);
  EXPECT_STREQ(text2.c_str(), "abc");
  EXPECT_EQ(text2.size(), 3);

  Fl_String text3("abc\0def", 7);
  EXPECT_EQ(text3.strlen(), 3);
  EXPECT_EQ(text3.size(), 7);

  Fl_String text4(text);
  EXPECT_STREQ(text4.c_str(), "hello");

  Fl_String text5 = text;
  EXPECT_STREQ(text5.c_str(), "hello");

  Fl_String text6 = "yoohoo";
  EXPECT_STREQ(text6.c_str(), "yoohoo");

  return true;
}

/* Test methods that access Fl_String content and parts of it. */
TEST(Fl_String, Access) {
  Fl_String hello = "hello";
  EXPECT_STREQ(hello.c_str(), "hello");
  EXPECT_STREQ(hello.data(), "hello");
  EXPECT_EQ(hello[1], 'e');
  EXPECT_EQ(hello[hello.size()], 0);
  EXPECT_EQ(hello.at(1), 'e');
  EXPECT_EQ(hello.at(-1), 0);
  EXPECT_EQ(hello.at(11), 0);

  hello[1] = 'a';
  EXPECT_STREQ(hello.c_str(), "hallo");

  hello.data()[1] = 'e';
  EXPECT_STREQ(hello.c_str(), "hello");

  return true;
}

/* Test the Fl_String capacity management. */
TEST(Fl_String, Capacity) {
  Fl_String hello;
  EXPECT_EQ(hello.capacity(), 0);

  hello = "hi";
  EXPECT_STREQ(hello.c_str(), "hi");
  EXPECT_GE(hello.capacity(), 2);

  hello = "the quick brown fox jumps over the lazy dog";
  EXPECT_STREQ(hello.c_str(), "the quick brown fox jumps over the lazy dog");
  EXPECT_GE(hello.capacity(), 41);

  int c = hello.capacity();
  hello.reserve(c+100);
  EXPECT_STREQ(hello.c_str(), "the quick brown fox jumps over the lazy dog");
  EXPECT_GE(hello.capacity(), 141);

  hello = "hi";
  hello.shrink_to_fit();
  EXPECT_EQ(hello.capacity(), 2);

  return true;
}

/* Test all methods that operate on Fl_String. */
TEST(Fl_String, Operations) {
  Fl_String empty;
  Fl_String hello = "Hello", world = "World";
  hello.resize(4);
  EXPECT_STREQ(hello.c_str(), "Hell");

  hello.clear();
  EXPECT_TRUE(hello.empty());

  hello = "Hello";
  hello.insert(3, "-");
  EXPECT_STREQ(hello.c_str(), "Hel-lo");
  hello = "Hello";
  hello.erase(2, 2);
  EXPECT_STREQ(hello.c_str(), "Heo");

  hello = "Hello";
  hello.push_back('!');
  EXPECT_STREQ(hello.c_str(), "Hello!");
  hello.pop_back();
  EXPECT_STREQ(hello.c_str(), "Hello");
  hello.append(world);
  EXPECT_STREQ(hello.c_str(), "HelloWorld");
  hello.append("!");
  EXPECT_STREQ(hello.c_str(), "HelloWorld!");
  hello = "Hello";
  hello += world;
  EXPECT_STREQ(hello.c_str(), "HelloWorld");
  hello += "!";
  EXPECT_STREQ(hello.c_str(), "HelloWorld!");
  hello += '?';
  EXPECT_STREQ(hello.c_str(), "HelloWorld!?");

  hello = "Hello";
  hello.replace(0, 0, "Say ", 4);
  EXPECT_STREQ(hello.c_str(), "Say Hello");
  hello.replace(0, 4, "");
  EXPECT_STREQ(hello.c_str(), "Hello");
  hello.replace(2, 2, "bb");
  EXPECT_STREQ(hello.c_str(), "Hebbo");
  hello.replace(2, 2, "xxx");
  EXPECT_STREQ(hello.c_str(), "Hexxxo");
  hello.replace(2, 3, "ll");
  EXPECT_STREQ(hello.c_str(), "Hello");
  hello.replace(2, 0, NULL, 0);
  EXPECT_STREQ(hello.c_str(), "Hello");
  hello.replace(Fl_String::npos, Fl_String::npos, world);
  EXPECT_STREQ(hello.c_str(), "HelloWorld");

  hello = "Hello";
  Fl_String sub = hello.substr();
  EXPECT_STREQ(sub.c_str(), "Hello"); // check correct usage
  sub = hello.substr(2);
  EXPECT_STREQ(sub.c_str(), "llo");
  sub = hello.substr(2, 2);
  EXPECT_STREQ(sub.c_str(), "ll");
  sub = hello.substr(-1, 2);
  EXPECT_TRUE(sub.empty()); // check faulty values
  sub = hello.substr(20, 2);
  EXPECT_TRUE(sub.empty());
  sub = empty.substr(0, 2);
  EXPECT_TRUE(sub.empty());

  return true;
}

/* Test all Fl_String functions that are no part of the class. */
TEST(Fl_String, Non-Member Functions) {
  Fl_String a = "a", b = "b", empty = "", result;
  result = a + b;
  EXPECT_STREQ(result.c_str(), "ab");
  result = a + empty;
  EXPECT_STREQ(result.c_str(), "a");
  result = a + "c";
  EXPECT_STREQ(result.c_str(), "ac");
  result = empty + "x";
  EXPECT_STREQ(result.c_str(), "x");
  EXPECT_TRUE(!(a == b));
  EXPECT_TRUE(a == a);
  EXPECT_TRUE(empty == empty);
  EXPECT_TRUE(a+b == "ab");
  EXPECT_TRUE(a+"b" == "a" + b);

  return true;
}

/* Test additions to Fl_Preferences. */
TEST(Fl_Preferences, Strings) {
  {
    Fl_Preferences prefs(Fl_Preferences::USER_L, "fltk.org", "unittests");
    prefs.set("a", Fl_String());
    prefs.set("b", Fl_String("Hello"));
    prefs.set("c", Fl_String("Hel\\l\nö"));
  }
  {
    Fl_Preferences prefs(Fl_Preferences::USER_L, "fltk.org", "unittests");
    Fl_String r;
    prefs.get("a", r, "x");
    EXPECT_STREQ(r.c_str(), "");
    prefs.get("b", r, "x");
    EXPECT_STREQ(r.c_str(), "Hello");
    prefs.get("c", r, "x");
    EXPECT_STREQ(r.c_str(), "Hel\\l\nö");
    prefs.get("d", r, "x");
    EXPECT_STREQ(r.c_str(), "x");
  }
  return true;
}

//
//------- test aspects of the FLTK core library ----------
//

/*
 Create a tab with only a terminal window in it. When shown for the first time,
 unittest will visualize progress by runing all regsitered tests one-by-one
 every few miliseconds.

 When run in command line mode (option `--core`), all tests are executed
 at full speed.
 */
class Ut_Core_Test : public Fl_Group {

  Fl_Simple_Terminal *tty;
  bool suite_ran_;

public:

  // Create the tab
  static Fl_Widget *create() {
    return new Ut_Core_Test(UT_TESTAREA_X, UT_TESTAREA_Y, UT_TESTAREA_W, UT_TESTAREA_H);
  }

  // Constructor for this tab
  Ut_Core_Test(int x, int y, int w, int h)
  : Fl_Group(x, y, w, h),
    tty(NULL),
    suite_ran_(false)
  {
    tty = new Fl_Simple_Terminal(x+4, y+4, w-8, h-8, "Unittest Log");
    tty->ansi(true);
    end();
    Ut_Suite::tty = tty;
  }

  // Run one single test and repeat calling this until all tests are done
  static void timer_cb(void*) {
    // Run a test every few miliseconds to visualize the progress
    if (Ut_Suite::run_next_test())
      Fl::repeat_timeout(0.2, timer_cb);
  }

  // Showing this tab for the first time will trigger the tests
  void show() FL_OVERRIDE {
    Fl_Group::show();
    if (!suite_ran_) {
      Fl::add_timeout(0.5, timer_cb);
      suite_ran_ = true;
    }
  }
};

// Register this tab with the unittest app.
UnitTest core(UT_TEST_CORE, "Core Functionality", Ut_Core_Test::create);



