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

#include <ctype.h>      /* isdigit */
#include <string.h>     /* memset */
#include <stdlib.h>     /* strtol */
#include <FL/Fl_Simple_Terminal.H>
#include <FL/Fl.H>
#include <stdarg.h>
#include "flstring.h"

#define STE_SIZE sizeof(Fl_Text_Display::Style_Table_Entry)

// Default style table
//    Simple ANSI style colors with an FL_COURIER font.
//    Due to how the modulo works for 20 items, the first 10 map to 40
//    and the second 10 map to 30.
//
static const Fl_Text_Display::Style_Table_Entry builtin_stable[] = {
  // FONT COLOR FONT FACE       SIZE   INDEX  COLOR NAME     ANSI     ANSI MODULO INDEX
  // ---------- --------------- ------ ------ -------------- -------- -----------------
  { 0x80808000, FL_COURIER,      14 }, // 0  - Bright Black   \033[40m 0,20,40,..
  { 0xff000000, FL_COURIER,      14 }, // 1  - Bright Red     \033[41m      ^^
  { 0x00ff0000, FL_COURIER,      14 }, // 2  - Bright Green   \033[42m
  { 0xffff0000, FL_COURIER,      14 }, // 3  - Bright Yellow  \033[43m
  { 0x0000ff00, FL_COURIER,      14 }, // 4  - Bright Blue    \033[44m
  { 0xff00ff00, FL_COURIER,      14 }, // 5  - Bright Magenta \033[45m
  { 0x00ffff00, FL_COURIER,      14 }, // 6  - Bright Cyan    \033[46m
  { 0xffffff00, FL_COURIER,      14 }, // 7  - Bright White   \033[47m
  { 0x00000000, FL_COURIER,      14 }, // 8  - x
  { 0x00000000, FL_COURIER,      14 }, // 9  - x
  { 0x00000000, FL_COURIER,      14 }, // 10 - Medium Black   \033[30m 10,30,50,..
  { 0xbb000000, FL_COURIER,      14 }, // 11 - Medium Red     \033[31m    ^^
  { 0x00bb0000, FL_COURIER,      14 }, // 12 - Medium Green   \033[32m
  { 0xbbbb0000, FL_COURIER,      14 }, // 13 - Medium Yellow  \033[33m
  { 0x0000cc00, FL_COURIER,      14 }, // 14 - Medium Blue    \033[34m
  { 0xbb00bb00, FL_COURIER,      14 }, // 15 - Medium Magenta \033[35m
  { 0x00bbbb00, FL_COURIER,      14 }, // 16 - Medium Cyan    \033[36m
  { 0xbbbbbb00, FL_COURIER,      14 }, // 17 - Medium White   \033[37m  (also "\033[0m" reset)
  { 0x00000000, FL_COURIER,      14 }, // 18 - x
  { 0x00000000, FL_COURIER,      14 }  // 19 - x
};
static const int  builtin_stable_size = sizeof(builtin_stable);
static const char builtin_normal_index = 17;        // the reset style index used by \033[0m

// Count how many times character 'c' appears in string 's'
static int strcnt(const char *s, char c) {
  int count = 0;
  while ( *s ) { if ( *s++ == c ) ++count; }
  return count;
}

// Vertical scrollbar callback intercept
void Fl_Simple_Terminal::vscroll_cb2(Fl_Widget *w, void*) {
  scrolling = 1;
  orig_vscroll_cb(w, orig_vscroll_data);
  scrollaway = (mVScrollBar->value() != mVScrollBar->maximum());
  scrolling = 0;
}
void Fl_Simple_Terminal::vscroll_cb(Fl_Widget *w, void *data) {
  Fl_Simple_Terminal *o = (Fl_Simple_Terminal*)data;
  o->vscroll_cb2(w,(void*)0);
}

/**
 Creates a new Fl_Simple_Terminal widget that can be a child of other FLTK widgets.
*/
Fl_Simple_Terminal::Fl_Simple_Terminal(int X,int Y,int W,int H,const char *l) : Fl_Text_Display(X,Y,W,H,l) {
  history_lines_ = 500;         // something 'reasonable'
  stay_at_bottom_ = true;
  ansi_ = false;
  lines = 0;                    // note: lines!=mNBufferLines when lines are wrapping
  scrollaway = false;
  scrolling = false;
  // These defaults similar to typical DOS/unix terminals
  textfont(FL_COURIER);
  color(FL_BLACK);
  textcolor(FL_WHITE);
  selection_color(FL_YELLOW);   // default dark blue looks bad for black background
  show_cursor(true);
  cursor_color(FL_GREEN);
  cursor_style(Fl_Text_Display::BLOCK_CURSOR);
  // Setup text buffer
  buf = new Fl_Text_Buffer();
  buffer(buf);
  sbuf = new Fl_Text_Buffer();  // allocate whether we use it or not
  // XXX: We use WRAP_AT_BOUNDS to prevent the hscrollbar from /always/
  //      being present, an annoying UI bug in Fl_Text_Display.
  wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
  // Style table
  stable_ = &builtin_stable[0];
  stable_size_ = builtin_stable_size;
  normal_style_index_  = builtin_normal_index;
  current_style_index_ = builtin_normal_index;
  // Intercept vertical scrolling
  orig_vscroll_cb = mVScrollBar->callback();
  orig_vscroll_data = mVScrollBar->user_data();
  mVScrollBar->callback(vscroll_cb, (void*)this);
}

/**
 Destructor for this widget; removes any internal allocations
 for the terminal, including text buffer, style buffer, etc.
*/
Fl_Simple_Terminal::~Fl_Simple_Terminal() {
  buffer(0);    // disassociate buffer /before/ we delete it
  if ( buf  ) { delete buf;  buf  = 0; }
  if ( sbuf ) { delete sbuf; sbuf = 0; }
}

/**
 Gets the current value of the stay_at_bottom(bool) flag.

 When true, the terminal tries to keep the scrollbar scrolled
 to the bottom when new text is added.

 \see stay_at_bottom(bool)
*/
bool Fl_Simple_Terminal::stay_at_bottom() const {
  return stay_at_bottom_;
}

/**
 Configure the terminal to remain scrolled to the bottom when possible,
 chasing the end of the buffer whenever new text is added.

 If disabled, the terminal behaves more like a text display widget;
 the scrollbar does not chase the bottom of the buffer.

 If the user scrolls away from the bottom, this 'chasing' feature is
 temporarily disabled. This prevents the user from having to fight
 the scrollbar chasing the end of the buffer while browsing when
 new text is also being added asynchronously. When the user returns the
 scroller to the bottom of the display, the chasing behavior resumes.

 The default is 'true'.
*/
void Fl_Simple_Terminal::stay_at_bottom(bool val) {
  if ( stay_at_bottom_ == val ) return; // no change
  stay_at_bottom_ = val;
  if ( stay_at_bottom_ ) enforce_stay_at_bottom();
}

/**
 Get the maximum number of terminal history lines last set by history_lines(int).

 -1 indicates an unlimited scroll history.

 \see history_lines(int)
*/
int Fl_Simple_Terminal::history_lines() const {
  return history_lines_;
}

/**
 Sets the maximum number of lines for the terminal history.

 The new limit value is automatically enforced on the current screen
 history, truncating off any lines that exceed the new limit.

 When a limit is set, the buffer is trimmed as new text is appended,
 ensuring the buffer never displays more than the specified number of lines.

 The default maximum is 500 lines.

 \param maxlines Maximum number of lines kept on the terminal buffer history.
                 Use -1 for an unlimited scroll history.
                 A value of 0 is not recommended.
*/
void Fl_Simple_Terminal::history_lines(int maxlines) {
  history_lines_ = maxlines;
  enforce_history_lines();
}

/**
 Get the state of the ANSI flag which enables/disables
 the handling of ANSI sequences in text.

 When true, ANSI sequences in the text stream control color, font
 and font sizes of text (e.g. "\033[41mThis is Red\033[0m").
 For more info, see ansi(bool).

 \see ansi(bool)
*/
bool Fl_Simple_Terminal::ansi() const {
  return ansi_;
}

/**
 Enable/disable support of ANSI sequences like "\033[31m", which sets the
 color/font/weight/size of any text that follows.

 If enabled, ANSI sequences of the form "\033[#m" can be used to change
 font color, face, and size, where '#' is an index number into the current
 style table. These "escape sequences" are hidden from view.

 If disabled, the textcolor() / textfont() / textsize() methods define
 the color and font for all text in the terminal. ANSI sequences are not
 handled specially, and rendered as raw text.

 A built-in style table is provided, but you can configure a custom style table
 using style_table(Style_Table_Entry*,int,int) for your own colors and fonts.

 The built-in style table supports these ANSI sequences:

     ANSI Sequence  Color Name      Font Face + Size  Remarks
     -------------  --------------  ----------------  -----------------------
     "\033[0m"      "Normal"        FL_COURIER, 14    Resets to default color/font/weight/size
     "\033[30m"     Medium Black    FL_COURIER, 14
     "\033[31m"     Medium Red      FL_COURIER, 14
     "\033[32m"     Medium Green    FL_COURIER, 14
     "\033[33m"     Medium Yellow   FL_COURIER, 14
     "\033[34m"     Medium Blue     FL_COURIER, 14
     "\033[35m"     Medium Magenta  FL_COURIER, 14
     "\033[36m"     Medium Cyan     FL_COURIER, 14
     "\033[37m"     Medium White    FL_COURIER, 14    The color when "\033[0m" reset is used
     "\033[40m"     Bright Black    FL_COURIER, 14
     "\033[41m"     Bright Red      FL_COURIER, 14
     "\033[42m"     Bright Green    FL_COURIER, 14
     "\033[43m"     Bright Yellow   FL_COURIER, 14
     "\033[44m"     Bright Blue     FL_COURIER, 14
     "\033[45m"     Bright Magenta  FL_COURIER, 14
     "\033[46m"     Bright Cyan     FL_COURIER, 14
     "\033[47m"     Bright White    FL_COURIER, 14

 Here's example code demonstrating the use of ANSI codes to select
 the built-in colors, and how it looks in the terminal:

 \image html simple-terminal-default-ansi.png "Fl_Simple_Terminal built-in ANSI sequences"
 \image latex simple-terminal-default-ansi.png "Fl_Simple_Terminal built-in ANSI sequences" width=4cm

 \note Changing the ansi(bool) value clears the buffer and forces a redraw().
 \note Enabling ANSI mode overrides textfont(), textsize(), textcolor()
       completely, which are controlled instead by current_style_index()
       and the current style_table().
 \see style_table(Style_Table_Entry*,int,int),
      current_style_index(),
      normal_style_index()
*/
void Fl_Simple_Terminal::ansi(bool val) {
  ansi_ = val;
  clear();
  if ( ansi_ ) {
    highlight_data(sbuf, stable_, stable_size_/STE_SIZE, 'A', 0, 0);
  } else {
    // XXX: highlight_data(0,0,0,'A',0,0) can crash, so to disable
    //      we use sbuf + builtin_stable but /set nitems to 0/.
    highlight_data(sbuf, builtin_stable, 0, 'A', 0, 0);
  }
  redraw();
}

/**
 Return the current style table being used.

 This is the value last passed as the 1st argument to
 style_table(Style_Table_Entry*,int,int). If no style table
 was defined, the built-in style table is returned.

 ansi(bool) must be set to 'true' for the style table to be used at all.

 \see style_table(Style_Table_Entry*,int,int)
*/
const Fl_Text_Display::Style_Table_Entry *Fl_Simple_Terminal::style_table() const {
  return stable_;
}

/**
 Return the current style table's size (in bytes).

 This is the value last passed as the 2nd argument to
 style_table(Style_Table_Entry*,int,int).
*/
int Fl_Simple_Terminal::style_table_size() const {
  return stable_size_;
}

/**
 Sets the style table index used by the ANSI terminal reset
 sequence "\033[0m", which resets the current drawing
 color/font/weight/size to "normal".

 Effective only when ansi(bool) is 'true'.

 \see ansi(bool), style_table(Style_Table_Entry*,int,int)
 \note Changing this value does *not* change the current drawing color.
       To change that, use current_style_index(int).
*/
void Fl_Simple_Terminal::normal_style_index(int val) {
  // Wrap index to ensure it's never larger than table
  normal_style_index_ = val % (stable_size_ / STE_SIZE);
}

/**
 Gets the style table index used by the ANSI terminal reset
 sequence "\033[0m".

 This is the value last set by normal_style_index(int), or as set by
 the 3rd argument to style_table(Style_Table_Entry*,int,int).

 \see normal_style_index(int), ansi(bool), style_table(Style_Table_Entry*,int,int)
*/
int Fl_Simple_Terminal::normal_style_index() const {
  return normal_style_index_;
}

/**
 Set the style table index used as the current drawing
 color/font/weight/size for new text.

 For example:
 \code
   :
   tty->ansi(true);
   tty->append("Some normal text.\n");
   tty->current_style_index(2);                           // same as "\033[2m"
   tty->append("This text will be green.\n");
   tty->current_style_index(tty->normal_style_index());   // same as "\033[0m"
   tty->append("Back to normal text.\n");
   :
 \endcode

 This value can also be changed by an ANSI sequence like "\033[#m",
 where # would be a new style index value. So if the application executes:
 <tt>term->append("\033[4mTesting")</tt>, then current_style_index()
 will be left set to 4.

 The index number specified should be within the number of items in the
 current style table. Values larger than the table will be clamped to
 the size of the table with a modulus operation.

 Effective only when ansi(bool) is 'true'.
*/
void Fl_Simple_Terminal::current_style_index(int val) {
  // Wrap index to ensure it's never larger than table
  current_style_index_ = abs(val) % (stable_size_ / STE_SIZE);
}

/**
 Get the style table index used as the current drawing
 color/font/weight/size for new text.

 This value is also controlled by the ANSI sequence "\033[#m",
 where # would be a new style index value. So if the application executes:
 <tt>term->append("\033[4mTesting")</tt>, then current_style_index()
 returns 4.

 \see current_style_index(int)
*/
int Fl_Simple_Terminal::current_style_index() const {
  return current_style_index_;
}

/**
 Set a user defined style table, which controls the font colors,
 faces, weights and sizes available for the terminal's text content.

 ansi(bool) must be set to 'true' for the defined style table
 to be used at all.

 If 'stable' is NULL, then the "built in" style table is used.
 For info about the built-in colors, see ansi(bool).

 Which style table entry used for drawing depends on the value last set
 by current_style_index(), or by the ANSI sequence "\033[#m", where '#'
 is the index into the style table array, the index limited to the size
 of the array via modulus.

 If the index# passed via "\033[#m" is larger than the number of elements
 in the table, the value is clamped via modulus. So for a 10 element table,
 the following ANSI codes would all be equivalent, selecting the 5th element
 in the table: "\033[5m", "\033[15m", "\033[25m", etc. This is because
 5==(15%10)==(25%10), etc.

 A special exception is made for "\033[0m", which is supposed to "reset"
 the current style table to default color/font/weight/size, as last set by
 \p normal_style_index, or by the API method normal_style_index(int).

 In cases like the built-in style table, where the 17th item is the
 "normal" color, the 'normal_style_index' is set to 17 so that "\033[0m"
 resets to that color, instead of the first element in the table.

 If you want "\033[0m" to simply pick the first element in the table,
 then set 'normal_style_index' to 0.

 An example of defining a custom style table (white courier 14, red courier 14,
 and white helvetica 14):
 \code
 int main() {
   :
   // Our custom style table
   Fl_Text_Display::Style_Table_Entry mystyle[] = {
     // Font Color Font Face        Font Size    Index  ANSI Sequence
     // ---------- ---------------- ---------    -----  -------------
     { FL_WHITE,   FL_COURIER_BOLD, 14 },     // 0      "\033[0m" ("default")
     { FL_RED,     FL_COURIER_BOLD, 14 },     // 1      "\033[1m"
     { FL_WHITE,   FL_HELVETICA,    14 }      // 2      "\033[2m"
   };
   // Create terminal, enable ANSI and our style table
   tty = new Fl_Simple_Terminal(..);
   tty->ansi(true);                                    // enable ANSI codes
   tty->style_table(&mystyle[0], sizeof(mystyle), 0);  // use our custom style table
   :
   // Now write to terminal, with ANSI that uses our style table
   tty->printf("\033[0mNormal Text\033[1mRed Courier Text\n");
   tty->append("\033[2mWhite Helvetica\033[0mBack to normal.\n");
   :
 \endcode

 \note Changing the style table clear()s the terminal.
 \note You currently can't control /background/ color of text,
       a limitation of Fl_Text_Display's current implementation.
 \note The caller is responsible for managing the memory of the style table.
 \note Until STR#3412 is repaired, Fl_Text_Display has scrolling bug if the
       style table's font size != textsize()

 \param stable - the style table, an array of structs of the type
                 Fl_Text_Display::Style_Table_Entry. Can be NULL
		 to use the default style table (see ansi(bool)).
 \param stable_size - the sizeof() the style table (in bytes).
                      Set this to 0 if 'stable' is NULL.
 \param normal_style_index - the style table index# used when the special
                             ANSI sequence "\033[0m" is encountered.
			     Normally use 0 so that sequence selects the
			     first item in the table. Only use different
			     values if a different entry in the table
			     should be the default. This value should
			     not be larger than the number of items in
			     the table, or it will be clamped with a
			     modulus operation. This value is ignored
			     if stable is NULL.
*/
void Fl_Simple_Terminal::style_table(Fl_Text_Display::Style_Table_Entry *stable,
                                     int stable_size, int normal_style_index) {
  // Wrap index to ensure it's never larger than table
  normal_style_index = abs(normal_style_index) % (stable_size/STE_SIZE);

  if ( stable_ == 0 ) {
    // User wants built-in style table?
    stable_ = &builtin_stable[0];
    stable_size_ = builtin_stable_size;
    normal_style_index_  = builtin_normal_index;  // set the index used by \033[0m
    current_style_index_ = builtin_normal_index;  // set the index used for drawing new text
  } else {
    // User supplying custom style table
    stable_ = stable;
    stable_size_ = stable_size;
    normal_style_index_  = normal_style_index;    // set the index used by \033[0m
    current_style_index_ = normal_style_index;    // set the index used for drawing new text
  }
  clear();            // don't take any chances with old style info
  highlight_data(sbuf, stable_, stable_size/STE_SIZE, 'A', 0, 0);
}

/**
 Scroll to last line unless someone has manually scrolled
 the vertical scrollbar away from the bottom.

 This is a protected member called automatically by the public API functions.
 Only internal methods or subclasses adjusting the internal buffer directly
 should need to call this.
*/
void Fl_Simple_Terminal::enforce_stay_at_bottom() {
  if ( stay_at_bottom_ && buffer() && !scrollaway ) {
    scroll(mNBufferLines, 0);
  }
}

/**
 Enforce 'history_lines' limit on the history buffer by trimming off
 lines from the top of the buffer.

 This is a protected member called automatically by the public API functions.
 Only internal methods or subclasses adjusting the internal buffer directly
 should need to call this.
*/
void Fl_Simple_Terminal::enforce_history_lines() {
  if ( history_lines() > -1 && lines > history_lines() ) {
    int trimlines = lines - history_lines();
    remove_lines(0, trimlines);                         // remove lines from top
  }
}

/**
 Appends new string 's' to terminal.

 The string can contain UTF-8, crlf's, and ANSI sequences are
 also supported when ansi(bool) is set to 'true'.

 \param s string to append.

 \param len optional length of string can be specified if known
            to save the internals from having to call strlen()

 \see printf(), vprintf(), text(), clear()
*/
void Fl_Simple_Terminal::append(const char *s, int len) {
  // Remove ansi codes and adjust style buffer accordingly.
  if ( ansi() ) {
    int nstyles = stable_size_ / STE_SIZE;
    if ( len < 0 ) len = strlen(s);
    // New text buffer (after ansi codes parsed+removed)
    char *ntm = (char*)malloc(len+1);       // new text memory
    char *ntp = ntm;
    char *nsm = (char*)malloc(len+1);       // new style memory
    char *nsp = nsm;
    // ANSI values
    char astyle = 'A'+current_style_index_; // the running style index
    const char *esc = 0;
    const char *sp = s;
    // Walk user's string looking for codes, modify new text/style text as needed
    while ( *sp ) {
      if ( *sp == 033 ) {        // "\033.."
        esc = sp++;
        switch (*sp) {
          case 0:                // "\033<NUL>"? stop
            continue;
          case '[': {            // "\033[.."
            ++sp;
            int vals[4], tv=0, seqdone=0;
            while ( *sp && !seqdone && isdigit(*sp) ) { // "\033[#;#.."
              char *newsp;
              long a = strtol(sp, &newsp, 10);
              sp = newsp;
              vals[tv++] = (a<0) ? 0 : a;       // prevent negative values
              if ( tv >= 4 )      // too many #'s specified? abort sequence
                { seqdone = 1; sp = esc+1; continue; }
              switch(*sp) {
                case ';':         // numeric separator
                  ++sp;
                  continue;
                case 'J':         // erase in display
                  switch (vals[0]) {
                    case 0:       // \033[0J -- clear to eol
                      // unsupported
                      break;
                    case 1:       // \033[1J -- clear to sol
                      // unsupported
                      break;
                    case 2:       // \033[2J -- clear entire screen
                      clear();    // clear text buffer
                      ntp = ntm;  // clear text contents accumulated so far
                      nsp = nsm;  // clear style contents ""
                      break;
                  }
                  ++sp;
                  seqdone = 1;
                  continue;
                case 'm':         // set color
                  if ( tv > 0 ) { // at least one value parsed?
                    current_style_index_ = (vals[0] == 0)            // "reset"?
                                             ? normal_style_index_   // use normal color for "reset"
                                             : (vals[0] % nstyles);  // use user's value, wrapped to ensure not larger than table
                    astyle = 'A' + current_style_index_;             // convert index -> style buffer char
                  }
                  ++sp;
                  seqdone = 1;
                  continue;
                case '\0':        // EOS in middle of sequence?
                  *ntp = 0;       // end of text
                  *nsp = 0;       // end of style
                  seqdone = 1;
                  continue;
                default:          // un-supported cmd?
                  seqdone = 1;
                  sp = esc+1;     // continue parsing just past esc
                  break;
              }   // switch
            }     // while
          }       // case '['
        }         // switch
      }           // \033
      else {
        // Non-ANSI character?
        if ( *sp == '\n' ) ++lines; // keep track of #lines
        *ntp++ = *sp++;             // pass char thru
        *nsp++ = astyle;            // use current style
      }
    } // while
    *ntp = 0;
    *nsp = 0;
    //::printf("  RESULT: ntm='%s'\n", ntm);
    //::printf("  RESULT: nsm='%s'\n", nsm);
    buf->append(ntm);           // new text memory
    sbuf->append(nsm);          // new style memory
    free(ntm);
    free(nsm);
  } else {
    // non-ansi buffer
    buf->append(s);
    lines += ::strcnt(s, '\n');  // count total line feeds in string added
  }
  enforce_history_lines();
  enforce_stay_at_bottom();
}

/**
 Replaces the terminal with new text content in string 's'.

 The string can contain UTF-8, crlf's, and ANSI sequences are
 also supported when ansi(bool) is set to 'true'.

 Old terminal content is completely cleared.

 \param s string to append.

 \param len optional length of string can be specified if known
            to save the internals from having to call strlen()

 \see append(), printf(), vprintf(), clear()

*/
void Fl_Simple_Terminal::text(const char *s, int len) {
  clear();
  append(s, len);
}

/**
 Returns entire text content of the terminal as a single string.

 This includes the screen history, as well as the visible
 onscreen content.
*/
const char* Fl_Simple_Terminal::text() const {
  return buf->text();
}

/**
 Appends printf formatted messages to the terminal.

 The string can contain UTF-8, crlf's, and ANSI sequences are
 also supported when ansi(bool) is set to 'true'.

 Example:
 \code
 #include <FL/Fl_Simple_Terminal.H>
 int main(..) {
     :
     // Create a simple terminal, and append some messages to it
     Fl_Simple_Terminal *tty = new Fl_Simple_Terminal(..);
     :
     // Append three lines of formatted text to the buffer
     tty->printf("The current date is: %s.\nThe time is: %s\n", date_str, time_str);
     tty->printf("The current PID is %ld.\n", (long)getpid());
     :
 \endcode
 \note See Fl_Text_Buffer::vprintf() for limitations.
 \param[in] fmt is a printf format string for the message text.
*/
void Fl_Simple_Terminal::printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  Fl_Simple_Terminal::vprintf(fmt, ap);
  va_end(ap);
}

/**
 Appends printf formatted messages to the terminal.

 Subclasses can use this to implement their own printf()
 functionality.

 The string can contain UTF-8, crlf's, and ANSI sequences are
 also supported when ansi(bool) is set to 'true'.

 \note The expanded string is currently limited to 1024 characters.
 \param fmt is a printf format string for the message text.
 \param ap is a va_list created by va_start() and closed with va_end(),
           which the caller is responsible for handling.
*/
void Fl_Simple_Terminal::vprintf(const char *fmt, va_list ap) {
  char buffer[1024];    // XXX: should be user configurable..
  ::vsnprintf(buffer, 1024, fmt, ap);
  buffer[1024-1] = 0;   // XXX: MICROSOFT
  append(buffer);
  enforce_history_lines();
}

/**
 Clears the terminal's screen and history. Cursor moves to top of window.
*/
void Fl_Simple_Terminal::clear() {
  buf->text("");
  sbuf->text("");
  lines = 0;
}

/**
 Remove the specified range of lines from the terminal, starting
 with line 'start' and removing 'count' lines.

 This method is used to enforce the history limit.

 \param start -- starting line to remove
 \param count -- number of lines to remove
*/
void Fl_Simple_Terminal::remove_lines(int start, int count) {
  int spos = skip_lines(0, start, true);
  int epos = skip_lines(spos, count, true);
  if ( ansi() ) {
    buf->remove(spos, epos);
    sbuf->remove(spos, epos);
  } else {
    buf->remove(spos, epos);
  }
  lines -= count;
  if ( lines < 0 ) lines = 0;
}

/**
  Draws the widget, including a cursor at the end of the buffer.
  This is needed since currently Fl_Text_Display doesn't provide
  a reliable way to always do this.
*/
void Fl_Simple_Terminal::draw() {
  // XXX: To do this right, we have to steal some of Fl_Text_Display's internal
  //      magic numbers to do it right, e.g. LEFT_MARGIN, RIGHT_MARGIN..  :/
  //
#define LEFT_MARGIN 3
#define RIGHT_MARGIN 3
  int buflen = buf->length();
  // Force cursor to EOF so it doesn't draw at user's last left-click
  insert_position(buflen);
  // Let widget draw itself
  Fl_Text_Display::draw();
  // Now draw cursor at the end of the buffer
  fl_push_clip(text_area.x-LEFT_MARGIN,
         text_area.y,
         text_area.w+LEFT_MARGIN+RIGHT_MARGIN,
         text_area.h);
  int X = 0, Y = 0;
  if (position_to_xy(buflen, &X, &Y)) draw_cursor(X, Y);
  fl_pop_clip();
}
