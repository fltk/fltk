HOW TO CREATE A NEW UNIT TEST
-----------------------------

    1) Create your new test/unittest_xxx.cxx file (or use an existing one)

    2) In your new cxx file, define a class derived from Fl_Group
       for your test (e.g. Ut_Test_Foo).

       The following should be a good starting template for the new file:

                                       * * *

           // ..standard fltk src header goes here..

           #include "unittests.h"      // Needed for UnitTest class and enum constant

           // Widgets this test will use
           #include <FL/Fl.H>
           #include <FL/Fl_Group.H>

           // Your class to do the test
           //     Your test must do its work within the UT_TESTAREA_XYWH area.
           //
           class Ut_Test_Foo : public Fl_Group {
           public:
             static Fl_Widget *create() {
               return new Ut_Test_Foo(UT_TESTAREA_X, UT_TESTAREA_Y, UT_TESTAREA_W, UT_TESTAREA_H);
             }
             Ut_Test_Foo(int x, int y, int w, int h) : Fl_Group(x, y, w, h) { .. }
           };

           // Create an instance of your class and register it with the main app
           UnitTest testfoo(UT_TEST_FOO, "My foo tester", Ut_Test_Foo::create);

                                       * * *

       Note that the last line in the above is what "registers" your new test
       with the unittests main application:

           UnitTest testfoo(UT_TEST_FOO, "My foo tester", Ut_Test_Foo::create);
                    ------- -----------   -------------   -------------------
                     |      |             |               |
                     |      |             |               Your class's static create() method
                     |      |             |
                     |      |             Text name for your test that shows up in unittests browser
                     |      |
                     |      This will be defined as an enum constant in the next step
                     |
                     The global instance name for your test.

    3) Add an entry anywhere to the enum {} at the top of the unittests.h file. Example:

           enum {
             UT_TEST_ABOUT = 0,
             UT_TEST_POINTS,
             ...
             UT_TEST_FOO,    <-- ADD YOUR TEST CLASS NAME IN ALL CAPS
             ...
           };

    4) Add your new unittest_xxx.cxx file to:

                a) test/CMakeLists.txt          -- cmake needs this
                b) test/Makefile                -- configure needs this (?)

       You should then be able to use cmake to create a build directory that
       builds with your new test. e.g.

           mkdir build
           cmake ..

        ..and from there, you should be able to go through the common
        edit/make/run development cycle:

           make -j 4                    <-- should build bin/test/unittests with your test
           bin/test/unittests           <-- run unittests to check your work

      5) That's it!

GENERAL TEST PRACTICES
----------------------
    UT_TESTAREA_X, Y, W, and H will be the position and size of the Group,
    and that the Group must expect to be resized, but not any smaller than
    that area.

    Try to include documentation that briefly recommends to the other devs
    what this test does and what to watch out for. If docs won't fit into
    the test area, either add a "?" button, or use a tooltip().

TEST REGISTRATION NOTES
-----------------------
    The mechanism of using an enum and the UnitTest instance allows us
    to ignore a test by simply not linking it (e.g. by choice with cmake).
    No changes to the source code are needed. The enumeration still needs
    an entry for everything that *may* be linked, even if it is not.
