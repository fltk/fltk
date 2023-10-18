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

#include "unittests.h"

#include <time.h>
#include <FL/Fl_Group.H>
#include <FL/Fl_Terminal.H>

//
//------- test the Fl_Terminal drawing capabilities ----------
//
class Ut_Terminal_Test : public Fl_Group {
  Fl_Terminal *tty1;
  Fl_Terminal *tty2;
  void ansi_test_pattern(Fl_Terminal *tty) {
    tty->append("\033[30mBlack          Courier 14\033[0m Normal text\n"
                "\033[31mRed            Courier 14\033[0m Normal text\n"
                "\033[32mGreen          Courier 14\033[0m Normal text\n"
                "\033[33mYellow         Courier 14\033[0m Normal text\n"
                "\033[34mBlue           Courier 14\033[0m Normal text\n"
                "\033[35mMagenta        Courier 14\033[0m Normal text\n"
                "\033[36mCyan           Courier 14\033[0m Normal text\n"
                "\033[37mWhite          Courier 14\033[0m Normal text\n"
                "\033[1;30mBright Black   Courier 14\033[0m Normal text\n"
                "\033[1;31mBright Red     Courier 14\033[0m Normal text\n"
                "\033[1;32mBright Green   Courier 14\033[0m Normal text\n"
                "\033[1;33mBright Yellow  Courier 14\033[0m Normal text\n"
                "\033[1;34mBright Blue    Courier 14\033[0m Normal text\n"
                "\033[1;35mBright Magenta Courier 14\033[0m Normal text\n"
                "\033[1;36mBright Cyan    Courier 14\033[0m Normal text\n"
                "\033[1;37mBright White   Courier 14\033[0m Normal text\n"
                "\n"
                "\033[31mRed\033[32mGreen\033[33mYellow\033[34mBlue\033[35mMagenta\033[36mCyan\033[37mWhite\033[0m - "
                "\033[31mX\033[32mX\033[33mX\033[34mX\033[35mX\033[36mX\033[37mX\033[0m\n"
                "\033[1;31mRed\033[1;32mGreen\033[1;33mYellow\033[34mBlue\033[35mMagenta\033[36mCyan\033[1;37mWhite\033[1;0m - "
                "\033[1;31mX\033[1;32mX\033[1;33mX\033[1;34mX\033[1;35mX\033[1;36mX\033[1;37mX\033[0m\n");
  }
  void gray_test_pattern(Fl_Terminal *tty) {
    tty->append("Grayscale Test Pattern\n"
                "--------------------------\n"
                "\033[38;2;255;255;255m 100% white     Courier 14\n"  // ESC xterm codes for setting r;g;b colors
                "\033[38;2;230;230;230m 90%  white     Courier 14\n"
                "\033[38;2;205;205;205m 80%  white     Courier 14\n"
                "\033[38;2;179;179;179m 70%  white     Courier 14\n"
                "\033[38;2;154;154;154m 60%  white     Courier 14\n"
                "\033[38;2;128;128;128m 50%  white     Courier 14\n"
                "\033[38;2;102;102;102m 40%  white     Courier 14\n"
                "\033[38;2;77;77;77m" " 30%  white     Courier 14\n"
                "\033[38;2;51;51;51m" " 20%  white     Courier 14\n"
                "\033[38;2;26;26;26m" " 10%  white     Courier 14\n"
                "\033[38;2;0;0;0m"    "  0%  white     Courier 14\n"
                "\033[0m");
  }
  static void date_timer_cb(void *data) {
    Fl_Terminal *tty = (Fl_Terminal*)data;
    time_t lt = time(NULL);
    tty->printf("The time and date is now: %s", ctime(&lt));
    Fl::repeat_timeout(3.0, date_timer_cb, data);
  }
public:
  static Fl_Widget *create() {
    return new Ut_Terminal_Test(UT_TESTAREA_X, UT_TESTAREA_Y, UT_TESTAREA_W, UT_TESTAREA_H);
  }
  Ut_Terminal_Test(int x, int y, int w, int h)
    : Fl_Group(x, y, w, h) {
    int tty_h = (int)(h/2.25+.5);
    int tty_y1 = y+(tty_h*0)+20;
    int tty_y2 = y+(tty_h*1)+40;

    // TTY1
    tty1 = new Fl_Terminal(x, tty_y1, w, tty_h,"Tty 1: Colors");
    ansi_test_pattern(tty1);
    Fl::add_timeout(0.5, date_timer_cb, (void*)tty1);

    // TTY2
    tty2 = new Fl_Terminal(x, tty_y2, w, tty_h,"Tty 2: Grayscale");
    gray_test_pattern(tty2);
    Fl::add_timeout(0.5, date_timer_cb, (void*)tty2);

    end();
  }
};

UnitTest simple_terminal(UT_TEST_SIMPLE_TERMINAL, "Terminal", Ut_Terminal_Test::create);
