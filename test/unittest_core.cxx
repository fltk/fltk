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
#include "../src/Fl_String.H"
#include <FL/Fl_Preferences.H>
#include <FL/fl_callback_macros.H>
#include <FL/filename.H>
#include <FL/fl_utf8.h>

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

#if (0) // FIXME - Fl_String

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
  EXPECT_FALSE((a != a));              // neq -erco
  EXPECT_TRUE((a != b));               // neq -erco
  EXPECT_TRUE(empty == empty);
  EXPECT_TRUE(a+b == "ab");
  EXPECT_TRUE(a+"b" == "a" + b);

  return true;
}

/* Test additions to Fl_Preferences. */
TEST(Fl_String, fl_filename_...) {
  const Fl_String ref = "/test/me.txt";
  Fl_String name = fl_filename_name(ref);
  EXPECT_STREQ(name.c_str(), "me.txt");
  name = fl_filename_name(Fl_String("/test/"));
  EXPECT_STREQ(name.c_str(), "");
  Fl_String path = fl_filename_path(ref);
  EXPECT_STREQ(path.c_str(), "/test/");
  Fl_String ext = fl_filename_ext(ref);
  EXPECT_STREQ(ext.c_str(), ".txt");
  ext = fl_filename_setext(ref, ".rtf");
  EXPECT_STREQ(ext.c_str(), "/test/me.rtf");
  fl_putenv("FL_UNITTEST=unit/test");
  name = fl_filename_expand(Fl_String("abc/$FL_UNITTEST/xyz"));
  EXPECT_STREQ(name.c_str(), "abc/unit/test/xyz");
  Fl_String abs = fl_filename_absolute(Fl_String("./abc/def.txt"));
  Fl_String rel = fl_filename_relative(abs);
  EXPECT_STREQ(rel.c_str(), "abc/def.txt");
  EXPECT_STREQ(ref.c_str(), "/test/me.txt");
  return true;
}

/* Test additions to Fl_Preferences. */
//TEST(Fl_Preferences, Strings) {
//  {
//    Fl_Preferences prefs(Fl_Preferences::USER_L, "fltk.org", "unittests");
//    prefs.set("a", Fl_String());
//    prefs.set("b", Fl_String("Hello"));
//    prefs.set("c", Fl_String("Hel\\l\nö"));
//  }
//  {
//    Fl_Preferences prefs(Fl_Preferences::USER_L, "fltk.org", "unittests");
//    Fl_String r;
//    prefs.get("a", r, "x");
//    EXPECT_STREQ(r.c_str(), "");
//    prefs.get("b", r, "x");
//    EXPECT_STREQ(r.c_str(), "Hello");
//    prefs.get("c", r, "x");
//    EXPECT_STREQ(r.c_str(), "Hel\\l\nö");
//    prefs.get("d", r, "x");
//    EXPECT_STREQ(r.c_str(), "x");
//  }
//  return true;
//}

TEST(fl_filename, ext) {
  Fl_String r = fl_filename_ext("test.txt");
  EXPECT_STREQ(r.c_str(), ".txt");
  r = fl_filename_ext("test");
  EXPECT_STREQ(r.c_str(), "");
  r = fl_filename_ext("");
  EXPECT_STREQ(r.c_str(), "");
  return true;
}

TEST(fl_filename, setext) {
  Fl_String r = fl_filename_setext(Fl_String("test.txt"), ".rtf");
  EXPECT_STREQ(r.c_str(), "test.rtf");
  r = fl_filename_setext(Fl_String("test"), ".rtf");
  EXPECT_STREQ(r.c_str(), "test.rtf");
  r = fl_filename_setext(Fl_String("test.txt"), "");
  EXPECT_STREQ(r.c_str(), "test");
  r = fl_filename_setext(Fl_String(""), ".rtf");
  EXPECT_STREQ(r.c_str(), ".rtf");
  r = fl_filename_setext(Fl_String("path/test"), ".rtf");
  EXPECT_STREQ(r.c_str(), "path/test.rtf");
  return true;
}

TEST(fl_filename, relative) {
  Fl_String base = "/var/tmp/somedir";
  Fl_String r = fl_filename_relative("/var/tmp/somedir/foo.txt", base);
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
  Fl_String base = "/var/tmp/somedir";
  Fl_String r = fl_filename_absolute("foo.txt", base);
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
class MyString : public Fl_String {
public:
  MyString() : Fl_String() { cb1_alloc++; }
  MyString(const MyString &str) : Fl_String(str) { cb1_alloc++; }
  MyString(const char *t) : Fl_String(t) { cb1_alloc++; }
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
  Fl_String *str = new Fl_String("FLTK");
  Fl_Button *btn = new Fl_Button(10, 10, 100, 100);
  FL_METHOD_CALLBACK_2(btn, Fl_String, str, insert, int, 2, const char*, "XX");
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



