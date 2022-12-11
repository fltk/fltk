HOW TO CREATE A NEW UNIT TEST
-----------------------------

    1) Create your new test/unittest_xxx.cxx file (or use an existing one)

    2) In your new cxx file, define a class derived from Fl_Group
       for your test (e.g. TestFoo).

       The following should be a good starting template for the new file:

                                       * * *

           // ..standard fltk src header goes here..

           #include "unittests.h"      // Needed for UnitTest class and enum constant

           // Widgets this test will use
           #include <FL/Fl.H>
           #include <FL/Fl_Group.H>

           // Your class to do the test
           //     Your test must do its work within the TESTAREA_XYWH area.
           //
           class TestFoo : public Fl_Group {
           public:
             static Fl_Widget *create() {
               return new TestFoo(TESTAREA_X, TESTAREA_Y, TESTAREA_W, TESTAREA_H);
             }
             TestFoo(int x, int y, int w, int h) : Fl_Group(x, y, w, h) { .. }
           };

           // Create an instance of your class and register it with the main app
           UnitTest testfoo(kTestFoo, "My foo tester", TestFoo::create);

                                       * * *

       Note that the last line in the above is what "registers" your new test
       with the unittests main application:

           UnitTest testfoo(kTestFoo, "My foo tester", TestFoo::create);
                    ------- --------   -------------   ---------------
                     |      |          |               |
                     |      |          |               Your class's static create() method
                     |      |          |
                     |      |          Text name for your test that shows up in unittests browser
                     |      |
                     |      Just put 'k' in front of your class name.
                     |      (This will be defined as an enum constant in the next step)
                     |
                     The global instance name for your test.

    3) Take the 'k' name you used above, e.g. kTestFoo, and add it to the enum {}
       at the top of the unittests.h file. Example:

           enum {
             kTestAbout = 0,
             kTestPoints,
             ...
             kTestFoo,         <-- ADD YOUR TEST CLASS WITH THE 'k' PREFIX
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
    TESTAREA_X, Y, W, and H will be the position and size of the Group,
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
