//
// "$Id$"
//
// Unit tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2017 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <time.h>
#include <FL/Fl_Group.H>
#include <FL/Fl_Simple_Terminal.H>

//
//------- test the Fl_Simple_Terminal drawing capabilities ----------
//
class SimpleTerminal : public Fl_Group {
  Fl_Simple_Terminal *tty1;
  Fl_Simple_Terminal *tty2;
  Fl_Simple_Terminal *tty3;
  void AnsiTestPattern(Fl_Simple_Terminal *tty) {
    tty->append("\033[30mBlack          Courier 14\033[0m Normal text\n"
                "\033[31mRed            Courier 14\033[0m Normal text\n"
                "\033[32mGreen          Courier 14\033[0m Normal text\n"
                "\033[33mYellow         Courier 14\033[0m Normal text\n"
                "\033[34mBlue           Courier 14\033[0m Normal text\n"
                "\033[35mMagenta        Courier 14\033[0m Normal text\n"
                "\033[36mCyan           Courier 14\033[0m Normal text\n"
                "\033[37mWhite          Courier 14\033[0m Normal text\n"
                "\033[40mBright Black   Courier 14\033[0m Normal text\n"
                "\033[41mBright Red     Courier 14\033[0m Normal text\n"
                "\033[42mBright Green   Courier 14\033[0m Normal text\n"
                "\033[43mBright Yellow  Courier 14\033[0m Normal text\n"
                "\033[44mBright Blue    Courier 14\033[0m Normal text\n"
                "\033[45mBright Magenta Courier 14\033[0m Normal text\n"
                "\033[46mBright Cyan    Courier 14\033[0m Normal text\n"
                "\033[47mBright White   Courier 14\033[0m Normal text\n"
                "\n"
                "\033[31mRed\033[32mGreen\033[33mYellow\033[34mBlue\033[35mMagenta\033[36mCyan\033[37mWhite\033[0m - "
                "\033[31mX\033[32mX\033[33mX\033[34mX\033[35mX\033[36mX\033[37mX\033[0m\n"
                "\033[41mRed\033[42mGreen\033[43mYellow\033[44mBlue\033[45mMagenta\033[46mCyan\033[47mWhite\033[0m - "
                "\033[41mX\033[42mX\033[43mX\033[44mX\033[45mX\033[46mX\033[47mX\033[0m\n");
  }
  void GrayTestPattern(Fl_Simple_Terminal *tty) {
    tty->append("Grayscale Test Pattern\n"
                "--------------------------\n"
                "\033[0m 100% white     Courier 14\n"
                "\033[1m 90%  white     Courier 14\n"
                "\033[2m 80%  white     Courier 14\n"
                "\033[3m 70%  white     Courier 14\n"
                "\033[4m 60%  white     Courier 14\n"
                "\033[5m 50%  white     Courier 14\n"
                "\033[6m 40%  white     Courier 14\n"
                "\033[7m 30%  white     Courier 14\n"
                "\033[8m 20%  white     Courier 14\n"
                "\033[9m 10%  white     Courier 14\n"
		"\033[0m");
  }
  static void DateTimer_CB(void *data) {
    Fl_Simple_Terminal *tty = (Fl_Simple_Terminal*)data;
    time_t lt = time(NULL);
    tty->printf("The time and date is now: %s", ctime(&lt));
    Fl::repeat_timeout(3.0, DateTimer_CB, data);
  }
public:
  static Fl_Widget *create() {
    return new SimpleTerminal(TESTAREA_X, TESTAREA_Y, TESTAREA_W, TESTAREA_H);
  }
  SimpleTerminal(int x, int y, int w, int h) : Fl_Group(x, y, w, h) {
    static Fl_Text_Display::Style_Table_Entry my_stable[] = {	// 10 entry grayscale
      // Font Color Font Face        Font Size ANSI Sequence
      // ---------- ---------------- --------- -------------
      { 0xffffff00, FL_COURIER_BOLD, 14 },  // "\033[0m"      0   white 100%
      { 0xe6e6e600, FL_COURIER_BOLD, 14 },  // "\033[1m"      1   white 90%
      { 0xcccccc00, FL_COURIER_BOLD, 14 },  // "\033[2m"      2   white 80%
      { 0xb3b3b300, FL_COURIER_BOLD, 14 },  // "\033[3m"      3   white 70%
      { 0x99999900, FL_COURIER_BOLD, 14 },  // "\033[4m"      4   white 60%
      { 0x80808000, FL_COURIER_BOLD, 14 },  // "\033[5m"      5   white 50% "\033[0m"
      { 0x66666600, FL_COURIER_BOLD, 14 },  // "\033[6m"      6   white 40%
      { 0x4d4d4d00, FL_COURIER_BOLD, 14 },  // "\033[7m"      7   white 30%
      { 0x33333300, FL_COURIER_BOLD, 14 },  // "\033[8m"      8   white 20%
      { 0x1a1a1a00, FL_COURIER_BOLD, 14 },  // "\033[9m"      9   white 10%
    };
    int tty_h = (h/3.5);
    int tty_y1 = y+(tty_h*0)+20;
    int tty_y2 = y+(tty_h*1)+40;
    int tty_y3 = y+(tty_h*2)+60;

    // TTY1
    tty1 = new Fl_Simple_Terminal(x, tty_y1, w, tty_h,"Tty 1: ANSI off");
    tty1->ansi(false);
    Fl::add_timeout(0.5, DateTimer_CB, (void*)tty1);

    // TTY2
    tty2 = new Fl_Simple_Terminal(x, tty_y2, w, tty_h,"Tty 2: ANSI on");
    tty2->ansi(true);
    AnsiTestPattern(tty2);
    Fl::add_timeout(0.5, DateTimer_CB, (void*)tty2);

    // TTY3
    tty3 = new Fl_Simple_Terminal(x, tty_y3, w, tty_h, "Tty 3: Grayscale Style Table");
    tty3->style_table(my_stable, sizeof(my_stable), 0);
    tty3->ansi(true);
    GrayTestPattern(tty3);
    Fl::add_timeout(0.5, DateTimer_CB, (void*)tty3);

    end();
  }
};

UnitTest simple_terminal("simple terminal", SimpleTerminal::create);

//
// End of "$Id$"
//
