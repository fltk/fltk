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
#include <FL/Fl_Button.H>
#include <FL/Fl_Terminal.H>
#include <FL/Fl_Preferences.H>
#include <FL/fl_callback_macros.H>
#include <FL/filename.H>
#include <FL/fl_utf8.h>

#include <string>


/* Test additions to Fl_Preferences. */
TEST(Fl_Preferences, Strings) {
  {
    Fl_Preferences prefs(Fl_Preferences::USER_L, "fltk.org", "unittests");
    prefs.set("a", std::string());
    prefs.set("b", std::string("Hello"));
    prefs.set("c", std::string("Hel\\l\nö"));
  }
  {
    Fl_Preferences prefs(Fl_Preferences::USER_L, "fltk.org", "unittests");
    std::string r;
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

#if 0

TEST(fl_filename, ext) {
  std::string r = fl_filename_ext("test.txt");
  EXPECT_STREQ(r.c_str(), ".txt");
  r = fl_filename_ext("test");
  EXPECT_STREQ(r.c_str(), "");
  r = fl_filename_ext("");
  EXPECT_STREQ(r.c_str(), "");
  return true;
}

TEST(fl_filename, setext) {
  std::string r = fl_filename_setext(std::string("test.txt"), ".rtf");
  EXPECT_STREQ(r.c_str(), "test.rtf");
  r = fl_filename_setext(std::string("test"), ".rtf");
  EXPECT_STREQ(r.c_str(), "test.rtf");
  r = fl_filename_setext(std::string("test.txt"), "");
  EXPECT_STREQ(r.c_str(), "test");
  r = fl_filename_setext(std::string(""), ".rtf");
  EXPECT_STREQ(r.c_str(), ".rtf");
  r = fl_filename_setext(std::string("path/test"), ".rtf");
  EXPECT_STREQ(r.c_str(), "path/test.rtf");
  return true;
}

TEST(fl_filename, relative) {
  std::string base = "/var/tmp/somedir";
  std::string r = fl_filename_relative("/var/tmp/somedir/foo.txt", base);
  EXPECT_STREQ(r.c_str(), "foo.txt");
  r = fl_filename_relative("/var/tmp/foo.txt", base);
  EXPECT_STREQ(r.c_str(), "../foo.txt");
  r = fl_filename_relative("./foo.txt", base);
  EXPECT_STREQ(r.c_str(), "./foo.txt");
  r = fl_filename_relative("../foo.txt", base);
  EXPECT_STREQ(r.c_str(), "../foo.txt");
  return true;
}

TEST(fl_filename, absolute) {
  std::string base = "/var/tmp/somedir";
  std::string r = fl_filename_absolute("foo.txt", base);
  EXPECT_STREQ(r.c_str(), "/var/tmp/somedir/foo.txt");
  r = fl_filename_absolute("/var/tmp/foo.txt", base);
  EXPECT_STREQ(r.c_str(), "/var/tmp/foo.txt");
  r = fl_filename_absolute("./foo.txt", base);
  EXPECT_STREQ(r.c_str(), "/var/tmp/somedir/foo.txt");
  r = fl_filename_absolute("../foo.txt", base);
  EXPECT_STREQ(r.c_str(), "/var/tmp/foo.txt");
  return true;
}


bool cb1a_ok = false, cb1b_ok = false, cb1c_ok = false;
int cb1_alloc = 0;
class MyString : public std::string {
public:
  MyString() : std::string() { cb1_alloc++; }
  MyString(const MyString &str) : std::string(str) { cb1_alloc++; }
  MyString(const char *t) : std::string(t) { cb1_alloc++; }
  ~MyString() { cb1_alloc--; }
};
void cb1(MyString a, int b) {
  cb1a_ok = true;
  if (strcmp(a.c_str(),"FLTK")==0) cb1b_ok = true;
  if (b==4) cb1c_ok = true;
}

/* Test callback macros. */
TEST(Fl_Callback_Macros, FL_FUNCTION_CALLBACK) {
  Fl_Group::current(NULL);
  Fl_Button *btn = new Fl_Button(10, 10, 100, 100);
  FL_FUNCTION_CALLBACK_2(btn, cb1, MyString, "FLTK", int, 4);

  do { class Fl_Callback_User_Data_240 : public Fl_Callback_User_Data {
    public: MyString a_; int b_;
    static void cb(Fl_Widget *w, void *user_data) {
      Fl_Callback_User_Data_240 *cbdata = (Fl_Callback_User_Data_240*)user_data; (void)cbdata; cb1(cbdata->a_, cbdata->b_); }; Fl_Callback_User_Data_240(MyString a, int b) : a_(a), b_(b) { } }; btn->callback(Fl_Callback_User_Data_240::cb, new Fl_Callback_User_Data_240("FLTK", 4), true); } while(0);

  btn->do_callback();
  delete btn;
  EXPECT_TRUE(cb1a_ok); // callback called
  EXPECT_TRUE(cb1b_ok); // string stored correctly
  EXPECT_TRUE(cb1c_ok); // integer stored correctly
  EXPECT_TRUE(cb1_alloc==0);  // string destroyed correctly (allocated as often as deallocated)
  return true;
}

TEST(Fl_Callback_Macros, FL_METHOD_CALLBACK) {
  Fl_Group::current(NULL);
  std::string *str = new std::string("FLTK");
  Fl_Button *btn = new Fl_Button(10, 10, 100, 100);
  FL_METHOD_CALLBACK_2(btn, std::string, str, insert, int, 2, const char*, "XX");
  btn->do_callback();
  EXPECT_STREQ(str->c_str(), "FLXXTK");
  delete btn;
  delete str;
  return true;
}

int cb3a = 0, cb3b = 0;
TEST(Fl_Callback_Macros, FL_INLINE_CALLBACK) {
  Fl_Group::current(NULL);
  Fl_Button *btn = new Fl_Button(10, 10, 100, 100);
  FL_INLINE_CALLBACK_2(btn,
                       int, a, 42,  int, b, 16,
                       { cb3a = a; cb3b = b; }
                       );
  btn->do_callback();
  EXPECT_EQ(cb3a, 42);
  EXPECT_EQ(cb3b, 16);
  delete btn;
  return true;
}

#endif // FIXME - Fl_String

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

  Fl_Terminal *tty;
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
    tty = new Fl_Terminal(x+4, y+4, w-8, h-8, "Unittest Log");
    tty->ansi(true);
    end();
    Ut_Suite::tty = tty;
  }

  // Run one single test and repeat calling this until all tests are done
  static void timer_cb(void*) {
    // Run a test every few milliseconds to visualize the progress
    if (Ut_Suite::run_next_test())
      Fl::repeat_timeout(0.15, timer_cb);
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



