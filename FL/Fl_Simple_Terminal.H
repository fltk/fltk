//
// "$Id$"
//
// A simple terminal widget for Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2011 by Bill Spitzak and others.
// Copyright 2017 by Greg Ercolano.
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

/* \file
   Fl_Simple_Terminal widget . */

#ifndef Fl_Simple_Terminal_H
#define Fl_Simple_Terminal_H

#include "Fl_Export.H"
#include <FL/Fl_Text_Display.H>

/**
  This is a continuous text scroll widget for logging and debugging
  output, much like a terminal.  Includes printf() for appending messages,
  a line limit for the screen history size, ANSI sequences to control
  text color, font face, font weight and font size.

  This is useful in place of using stdout/stderr for logging messages
  when no terminal is available, such as when an application is invoked
  from a desktop shortcut, dock, or file browser.

  Like a regular console terminal, the vertical scrollbar 'tracks'
  the bottom of the buffer as new output is added. If the user scrolls
  away from the bottom, this 'tracking' feature is temporarily suspended,
  so the user can browse the terminal history without fighting the scrollbar
  when new text is added asynchronously.  When the user returns the
  scroller to the bottom of the display, the scrollbar's tracking resumes.

  Features include:

    - history_lines(int) can define a maximum size for the terminal screen history
    - stay_at_bottom(bool) can be used to cause the terminal to keep scrolled to the bottom
    - ansi(bool) enables ANSI sequences within the text to control text colors
    - style_table() can be used to define custom color/font/weight/size combinations

  What this widget is NOT is a full terminal emulator; it does NOT
  handle stdio redirection, pipes, pseudo ttys, termio character cooking,
  keyboard input processing, screen addressing, random cursor positioning,
  curses(3) compatibility, or VT100/xterm emulation.

  It is a simple text display widget that leverages the features of the
  Fl_Text_Display base class to handle terminal-like behavior, such as
  logging events or debug information.

  Example use:
  \code

      #include <FL/Fl_Simple_Terminal.H>
      :
        tty = new Fl_Simple_Terminal(...);
        tty->ansi(true);      // enable use of "\033[#m"
        :
        tty->printf("The time is now: \033[32m%s\033[0m", date_time_str);

  \endcode

  Example application:
  \dontinclude simple-terminal.cxx
  \skip  //START
  \until //END

  Style Tables For Color/Font/Fontsize Control
  --------------------------------------------
  Internally this widget derives from Fl_Text_Display, and therefore
  inherits some of its idiosyncracies. In particular, when colors
  are used, the base class's concept of a 'style table' is used.

  The 'style table' is similar to a color mapped image; where each
  pixel is a single value that is an index into a table of colors
  to minimize per-pixel memory use.

  The style table has a similar goal; since every character in the
  terminal can potentially be a different color, instead of managing
  several integer attribute values per-character, a single character
  for each character is used as an index into the style table, choosing
  one of the available color/font/weight/size values available.
  This saves on as much as 3 to 4 times the memory use, useful when
  there's a large amount of text.

  When ansi() is set to 'true', ANSI sequences of the form "\033[#m"
  can be used to select different colors, font faces, font weights (bold,italic..),
  and font sizes, where '#' is the index number into the style table. Example:

  \code
      "\033[0mThis text uses the 1st entry in the style table\n"
      "\033[1mThis text uses the 2nd entry in the style table\n"
      "\033[2mThis text uses the 3rd entry in the style table\n"
      etc..
  \endcode

  There is a built-in style table that provides some
  commonly used ANSI colors for "\033[30m" through "\033[37m"
  (blk,red,grn,yel,blu,mag,cyn,wht), and a brighter version of those
  colors for "\033[40" through "\033[47m". See ansi(bool) for more info.

  You can also supply a custom style table using
  style_table(Style_Table_Entry*,int,int), allowing you to define
  your own color/font/weight/size combinations. See that method's docs
  for more info.

  All style index numbers are rounded to the size of the style table
  (via modulus) to protect the style array from overruns.

*/
class FL_EXPORT Fl_Simple_Terminal : public Fl_Text_Display {
protected:
  Fl_Text_Buffer *buf;      // text buffer
  Fl_Text_Buffer *sbuf;     // style buffer

private:
  int history_lines_;       // max lines allowed in screen history
  bool stay_at_bottom_;     // lets scroller chase last line in buffer
  bool ansi_;               // enables ANSI sequences
  // scroll management
  int lines;                // #lines in buffer (optimization: Fl_Text_Buffer slow to calc this)
  bool scrollaway;          // true when user changed vscroll away from bottom
  bool scrolling;           // true while scroll callback active
  // Fl_Text_Display vscrollbar's callback+data
  Fl_Callback *orig_vscroll_cb;
  void *orig_vscroll_data;
  // Style table
  const Fl_Text_Display::Style_Table_Entry *stable_;  // the active style table
  int stable_size_;         // active style table size (in bytes)
  int normal_style_index_;  // "normal" style used by "\033[0m" reset sequence
  int current_style_index_; // current style used for drawing text

public:
  Fl_Simple_Terminal(int X,int Y,int W,int H,const char *l=0);
  ~Fl_Simple_Terminal();

  // Terminal options
  void stay_at_bottom(bool);
  bool stay_at_bottom() const;
  void history_lines(int);
  int  history_lines() const;
  void ansi(bool val);
  bool ansi() const;
  void style_table(Fl_Text_Display::Style_Table_Entry *stable, int stable_size, int normal_style_index=0);
  const Fl_Text_Display::Style_Table_Entry *style_table() const;
  int style_table_size() const;
  void normal_style_index(int);
  int  normal_style_index() const;
  void current_style_index(int);
  int  current_style_index() const;

  // Terminal text management
  void append(const char *s, int len=-1);
  void text(const char *s, int len=-1);
  const char* text() const;
  void printf(const char *fmt, ...);
  void vprintf(const char *fmt, va_list ap);
  void clear();
  void remove_lines(int start, int count);

private:
  // Methods blocking public access to the subclass
  //    These are subclass methods that would give unexpected
  //    results if used. By making them private, we effectively
  //    "block" them.
  //
  //    TODO: There are probably other Fl_Text_Display methods that
  //          need to be blocked.
  //
  void insert(const char*) { }

protected:
  // Fltk
  virtual void draw();

  // Internal methods
  void enforce_stay_at_bottom();
  void enforce_history_lines();
  void vscroll_cb2(Fl_Widget*, void*);
  static void vscroll_cb(Fl_Widget*, void*);
};

#endif

//
// End of "$Id$".
//
