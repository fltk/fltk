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

TEST(Fl_String, Assignment) {
  Fl_String null;
  EXPECT_STREQ(null.c_str(), "");
  EXPECT_TRUE(null.empty());

  Fl_String null2(NULL);
  EXPECT_STREQ(null2.c_str(), "");
  EXPECT_TRUE(null2.empty());

  Fl_String empty("");
  EXPECT_STREQ(empty.c_str(), "");
  EXPECT_TRUE(empty.empty());

  Fl_String text("hello");
  EXPECT_STREQ(text.c_str(), "hello");
  EXPECT_EQ(text.size(), 5);
  EXPECT_EQ(text.strlen(), 5);
  EXPECT_GE(text.capacity(), 5);
  EXPECT_TRUE(!text.empty());

  Fl_String text2("abcdef", 3);
  EXPECT_STREQ(text2.c_str(), "abc");

  Fl_String text3("abc\0def", 7);
  EXPECT_EQ(text3.size(), 7);

  Fl_String text4(text);
  EXPECT_STREQ(text4.c_str(), "hello");

  Fl_String text5 = text;
  EXPECT_STREQ(text5.c_str(), "hello");

  Fl_String text6 = "yoohoo";
  EXPECT_STREQ(text6.c_str(), "yoohoo");

  return true;
}

TEST(Fl_String, Access) {

  Fl_String hello = "hello";
  EXPECT_STREQ(hello.c_str(), "hello");
  EXPECT_STREQ(hello.data(), "hello");

  return true;
}

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

  return true;
}

TEST(Fl_Int_Vector, Capacitiy) {
  // None yet
  return true;
}

//
//------- test aspects of the FLTK core library ----------
//
class Ut_Core_Test : public Fl_Group {
  Fl_Simple_Terminal *tty;
  bool suite_ran_;
public:
  static Fl_Widget *create() {
    return new Ut_Core_Test(UT_TESTAREA_X, UT_TESTAREA_Y, UT_TESTAREA_W, UT_TESTAREA_H);
  }
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
  void show() FL_OVERRIDE {
    Fl_Group::show();
    if (!suite_ran_) {
      // TODO: make this run test-by-test in an idel callback
      RUN_ALL_TESTS();
      suite_ran_ = true;
    }
  }
};

UnitTest core(UT_TEST_CORE, "Core Functionality", Ut_Core_Test::create);



