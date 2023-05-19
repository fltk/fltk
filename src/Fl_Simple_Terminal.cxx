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
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
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

// --- Fl_Escape_Seq ----------------------------------------------------------

// Append char to buf[] safely (with bounds checking)
//    Returns:
//      success - ok
//      fail    - buffer full/overflow
//
int Fl_Simple_Terminal::Fl_Escape_Seq::append_buf(char c) {
  if ( bufp_ >= bufendp_ ) return fail;   // end of buffer reached?
  *bufp_++ = c;
  *bufp_   = 0;                           // keep buf[] null terminated
  return success;
}

// Append whatever integer string is at valbufp into vals_[] safely w/bounds checking
//    Assumes valbufp points to a null terminated string.
//    Returns:
//       success - parsed ok
//       fail    - error occurred (non-integer, or vals_[] full)
//
int Fl_Simple_Terminal::Fl_Escape_Seq::append_val() {
  if ( vali_ == maxvals ) {               // too many vals_[] already?
    return fail;                          // fail if vals_[] full
  }
  if ( !valbufp_ || (*valbufp_ == 0) ) {  // no integer to parse? e.g. ESC[;m
    vals_[vali_] = 0;                     // handle as if it was zero, e.g. ESC[0;
  } else if ( sscanf(valbufp_, "%d", &vals_[vali_]) != 1 ) { // Parse integer into vals_[]
    return fail;                          // fail if parsed a non-integer
  }
  if ( ++vali_ >= maxvals ) {             // advance val index, fail if too many vals
    vali_ = maxvals-1;                    // clamp
    return fail;                          // fail
  }
  valbufp_ = 0;                           // parsed val ok, reset valbufp to NULL
  return success;
}

// Ctor
Fl_Simple_Terminal::Fl_Escape_Seq::Fl_Escape_Seq() {
  reset();
}

// Reset the class
void Fl_Simple_Terminal::Fl_Escape_Seq::reset() {
  esc_mode_ = 0;                   // disable ESC mode, so parse_in_progress() returns false
  bufp_     = buf_;                // point to beginning of buffer
  bufendp_ = buf_ + (maxbuf - 1);  // point to end of buffer
  valbufp_  = 0;                   // disable val ptr (no vals parsed yet)
  vali_    = 0;                    // zero val index
  buf_[0]  = 0;                    // null terminate buffer
  vals_[0] = 0;                    // first val[] 0
}

// Return current escape mode.
//    This is really only valid after parse() returns 'completed'.
//    After a reset() this will return 0.
//
char Fl_Simple_Terminal::Fl_Escape_Seq::esc_mode() const {
  return esc_mode_;
}

// Set current escape mode.
void Fl_Simple_Terminal::Fl_Escape_Seq::esc_mode(char val) {
  esc_mode_ = val;
}

// Return the total vals parsed.
//    This is really only valid after parse() returns 'completed'.
//
int Fl_Simple_Terminal::Fl_Escape_Seq::total_vals() const {
  return vali_;
}

// Return the value at index i.
//    i is not range checked; it's assumed 0 <= i < total_vals().
//    It is only valid to call this after parse() returns 'completed'.
//
int Fl_Simple_Terminal::Fl_Escape_Seq::val(int i) const {
  return vals_[i];
}

// See if we're in the middle of parsing an ESC sequence
bool Fl_Simple_Terminal::Fl_Escape_Seq::parse_in_progress() const {
  return (esc_mode_ == 0) ? false : true;
}

// Handle parsing an escape sequence.
//
//    Call this only if parse_in_progress() is true.
//    Passing ESC does a reset() and sets esc_mode() to ESC.
//    When a full escape sequence has been parsed, 'completed' is returned (see below)
//
//    Typical use pattern (this is unverified code, shown just to give a general gist):
//
//       while ( *s ) {                                // walk text that may contain ESC sequences
//         if ( *s == 0x1b ) {
//           escseq.parse(*s++);                       // start parsing ESC seq (does a reset())
//           continue;
//         } else if ( escseq.parse_in_progress() ) {  // continuing to parse an ESC seq?
//           switch (escseq.parse(*s++)) {             // parse char, advance s..
//             case fail:    escseq.reset(); continue; // failed? reset, continue..
//             case success: continue;                 // keep parsing..
//             case completed:                         // parsed complete esc sequence?
//               break;
//           }
//           // Handle parsed esc sequence here..
//           switch ( escseq.esc_mode() ) {
//             case 'm':                               // ESC[...m?
//               for ( int i=0; i<escseq.total_vals(); i++ ) {
//                 int val = escseq.val(i);
//                 ..handle values here..
//               }
//               break;
//             case 'J':                               // ESC[#J?
//               ..handle..
//               break;
//           }
//           escseq.reset();   // done handling escseq, reset()
//           continue;
//         } else {
//           ..handle non-escape chars here..
//         }
//         ++s;    // advance thru string
//       }
//
// Returns:
//   fail      - error occurred: escape sequence invalid, class is reset()
//   success   - parsing ESC sequence OK so far, still in progress/not done yet
//   completed - complete ESC sequence was parsed, esc_mode() will be, e.g.
//               'm' - ESC[#;#..m sequence parsed, val() has value(s) parsed
//               'J' - ESC[#J sequence parsed, val() has value(s) parsed
//               'A' thru 'D' - cursor up/down/right/left movement (ESC A/B/C/D)
//
int Fl_Simple_Terminal::Fl_Escape_Seq::parse(char c) {
  // NOTE: During parsing esc_mode() will be:
  //             0 - reset/not parsing
  //          0x1b - ESC received, expecting next one of A/B/C/D or '['
  //           '[' - actively parsing CSI sequence, e.g. ESC[
  //
  //       At the /end/ of parsing, after 'completed' is returned,
  //       esc_mode() will be the mode setting char, e.g. 'm' for 'ESC[0m', etc.
  //
  if ( c == 0 ) {                          // NULL? (caller should really never send us this)
    return success;                        // do nothing -- leave state unchanged, return 'success'
  } else if ( c == 0x1b ) {                // ESC at ANY time resets class/begins new ESC sequence
    reset();
    esc_mode(0x1b);
    if ( append_buf(c) < 0 ) goto pfail;   // save ESC in buf
    return success;
  } else if ( c < ' ' || c >= 0x7f ) {     // any other control or binary characters?
    goto pfail;                            // reset + fail out of esc sequence parsing
  }

  // Whatever the character is, handle it depending on esc_mode..
  if ( esc_mode() == 0x1b ) {              // in ESC mode?
    if ( c == '[' ) {                      // ESC[?
      esc_mode(c);                         // switch to parsing mode for ESC[#;#;#..
      vali_    = 0;                        // zero vals_[] index
      valbufp_  = 0;                       // valbufp NULL (no vals yet)
      if ( append_buf(c) < 0 ) goto pfail; // save '[' in buf
      return success;                      // success
    } else if ( c >= 'A' && c <= 'D' ) {   // ESC A/B/C/D? (cursor movement?)
      esc_mode(c);                         // use as mode
      vali_    = 0;
      if ( append_buf(c) < 0 ) goto pfail; // save A/B/C/D in buf
      return success;                      // success
    } else {                               // ESCx? not supported
      goto pfail;
    }
  } else if ( esc_mode() == '[' ) {        // '[' mode? e.g. ESC[...
    if ( c == ';' ) {                      // ';' indicates end of a value, e.g. ESC[0;2..
      if (append_val() < 0 ) goto pfail;   // append value parsed so far, vali gets inc'ed
      if (append_buf(c) < 0 ) goto pfail;  // save ';' in buf
      return success;
    }
    if ( isdigit(c) ) {                    // parsing an integer?
      if ( !valbufp_ )                     // valbufp not set yet?
        { valbufp_ = bufp_; }              // point to first char in integer string
      if ( append_buf(c) < 0 ) goto pfail; // add value to buffer
      return success;
    }
    // Not a ; or digit? fall thru to [A-Z,a-z] check
  } else {                                 // all other esc_mode() chars are fail/unknown
    goto pfail;
  }
  if ( ( c >= 'A' && c<= 'Z') ||           // ESC#X or ESC[...X, where X is [A-Z,a-z]?
       ( c >= 'a' && c<= 'z') ) {
    if (append_val() < 0 ) goto pfail;     // append any trailing vals just before letter
    if (append_buf(c) < 0 ) goto pfail;    // save letter in buffer
    esc_mode(c);                           // change mode to the mode setting char
    if ( vali_ == 0 ) {                    // no vals were specified? assume 0 (e.g. ESC[J? assume ESC[0J)
      vals_[vali_++] = 0;                  // force vals_[0] to be 0, and vali = 1;
    }
    return completed;                      // completed/done
  }
  // Any other chars? reset+fail
pfail:
  reset();
  return fail;
}

//
// --- Fl_Simple_Terminal -----------------------------------------------------
//

// Vertical scrollbar callback intercept
void Fl_Simple_Terminal::vscroll_cb2(Fl_Widget *w, void*) {
  scrolling_ = 1;
  orig_vscroll_cb_(w, orig_vscroll_data_);
  scrollaway_ = (mVScrollBar->value() != mVScrollBar->maximum());
  scrolling_  = 0;
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
  lines_ = 0;                    // note: lines!=mNBufferLines when lines are wrapping
  scrollaway_ = false;
  scrolling_ = false;
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
  stable_              = &builtin_stable[0];
  stable_size_         = builtin_stable_size;
  normal_style_index_  = builtin_normal_index;
  current_style_index_ = builtin_normal_index;
  current_style_       = 'A' + 0;
  // ANSI escape seq
  ansi_ = false;
  ansi_show_unknown_ = false;
  // Intercept vertical scrolling
  orig_vscroll_cb_     = mVScrollBar->callback();
  orig_vscroll_data_   = mVScrollBar->user_data();
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
 See if we should show unknown ANSI sequences with '¿' or not.
 \see ansi_show_unknown(bool)
*/
bool Fl_Simple_Terminal::ansi_show_unknown(void) const {
  return ansi_show_unknown_;
}

/**
 Enable showing unknown ESC sequences with the '¿' character.
 By default this is off, and unknown escape sequences are silently ignored.
 \see ansi_show_unknown()
*/
void Fl_Simple_Terminal::ansi_show_unknown(bool val) {
  ansi_show_unknown_ = val;
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
  current_style_       = 'A' + current_style_index_;
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
 Get the current style char used for style buffer.
 This character appends in parallel with any text in the text buffer
 to specify the per-character styling. This is typically 'A' for the
 first entry, 'B' for the second entry, etc.

 This value is changed by current_style_index(int).
 \see current_style_index(int)
*/
int Fl_Simple_Terminal::current_style() const {
  return current_style_;
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
  if ( stay_at_bottom_ && buffer() && !scrollaway_ ) {
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
  if ( history_lines() > -1 && lines_ > history_lines() ) {
    int trimlines = lines_ - history_lines();
    remove_lines(0, trimlines);                         // remove lines from top
  }
}

/**
 Destructive backspace from end of existing buffer() for specified \p count characters.
 Takes into account multi-byte (Unicode) chars. So if count is 3, last 3 chars
 are deleted from end of buffer.
*/
void Fl_Simple_Terminal::backspace_buffer(unsigned int count) {
  if ( count == 0 ) return;
  int end = buf->length();      // find end of buffer
  int pos = end;                // pos index starts at end, walks backwards
  while ( count-- ) {           // repeat backspace operation until done
    pos = buf->prev_char(pos);  // move back one full char (unicode safe)
    if ( pos < 0 ) {            // bs beyond beginning of buffer? done
      pos = 0;
      break;
    }
  }
  // Remove chars we backspaced over
  buf->remove(pos, end);
  sbuf->remove(pos, end);
}

/**
 Handle a Unicode aware backspace.
 This flushes the string parsed so far to Fl_Text_Display,
 then lets Fl_Text_Display handle the unicode aware backspace.
*/
void Fl_Simple_Terminal::handle_backspace() {
  // FLUSH TEXT TO TEXT DISPLAY
  //   This prevents any Unicode multibyte char split across
  //   our string buffer and Fl_Text_Display, and also allows
  //   Fl_Text_Display to handle unicode aware backspace.

  // 1) Null temrinate buffers
  *ntp_ = 0;
  *nsp_ = 0;
  // 2) Flush text to Fl_Text_Display
  buf->append(ntm_);    // flush text to FTD
  sbuf->append(nsm_);   // flush style to FTD
  // 3) Rewind buffer and sp, restore saved chars
  ntp_ = ntm_;
  nsp_ = nsm_;
  // 4) Let Fl_Text_Display handle unicode aware backspace
  backspace_buffer(1);
}

// Handle unknown esc sequences
void Fl_Simple_Terminal::unknown_escape() {
  if ( ansi_show_unknown() ) {
    for ( const char *s = "¿"; *s; s++ ) {
      *ntp_++ = *s;                        // emit utf-8 encoded char
      *nsp_++ = current_style();           // use current style
    }
  }
}

/**
 Handle appending string with ANSI escape sequences, and other 'special'
 character processing (such as backspaces).
*/
void Fl_Simple_Terminal::append_ansi(const char *s, int len) {
    int nstyles = stable_size_ / STE_SIZE;
    if ( len < 0 ) len = (int)strlen(s);
    // ntm/tsm - new text buffer (after ansi codes parsed+removed)
    ntm_ = (char*)malloc(len+1);            // new text memory
    ntp_ = ntm_;                            // new text ptr
    nsm_ = (char*)malloc(len+1);            // new style memory
    nsp_ = nsm_;                            // new style ptr
    // Walk user's string looking for codes, modify new text/style text as needed
    const char *sp = s;
    while ( *sp ) {
      if (*sp == 0x1b ) {                   // start of ESC sequence?
        escseq.parse(*sp++);                // start parsing..
        continue;
      }
      if ( escseq.parse_in_progress() ) {   // ESC sequence in progress?
        switch ( escseq.parse(*sp) ) {      // parse until completed or fail
          case Fl_Escape_Seq::fail:         // parsing error?
            unknown_escape();
            escseq.reset();                 // ..reset and continue
            ++sp;
            continue;
          case Fl_Escape_Seq::success:      // parsed ok / still in progress
            ++sp;
            continue;
          case Fl_Escape_Seq::completed:    // completed parsing ESC sequence?
            break;
        }
        // Escape sequence completed ok? handle it
        //     Walk all values parsed from ESC[#;#;#..x
        //
        for ( int i=0; i<escseq.total_vals(); i++ ) {
          int val = escseq.val(i);
          switch (escseq.esc_mode() ) {
            // ERASE IN DISPLAY (ED)
            case 'J':                       // ESC[#J
              switch (val) {
                case 0:                     // ESC[0J -- clear to eol
                  unknown_escape();
                  break;
                case 1:                     // ESC[1J -- clear to sol
                  unknown_escape();
                  break;
                case 2:                     // ESC[2J -- clear visible screen
                  // NOTE: Currently we clear the /entire screen history/, and
                  //       moves cursor to the top of buffer.
                  //
                  //       ESC[2J should really only clear the /visible display/
                  //       not affecting screen history or cursor position.
                  //
                  clear();                  // clear text buffer
                  ntp_ = ntm_;              // clear text contents accumulated so far
                  nsp_ = nsm_;              // clear style contents ""
                  break;
                default:                    // all other ESC[#J unsupported
                  unknown_escape();
                  break;
              }
              break;
            // SELECT GRAPHIC RENDITION (SGR)
            case 'm':
              if ( val == 0 ) {             // ESC[0m? (reset color)
                // Switch to "normal color"
                current_style_index(normal_style_index_);
              } else {                      // ESC[#m? (set some specific color)
                // Use modulus to map into styles[] buffer
                current_style_index(val % nstyles);
              }
              break;
            // UNSUPPORTED MODES
            default:
              unknown_escape();
              break;                         // unsupported
          }
        }
        escseq.reset();                      // reset after handling escseq
        ++sp;                                // advance thru string
        continue;
      } else if ( *sp == 8 ) {               // backspace?
        handle_backspace();
        sp++;
      } else {                               // Not ANSI or backspace? append to display
        if ( *sp == '\n' ) ++lines_;         // crlf? keep track of #lines
        *ntp_++ = *sp++;                     // pass char thru
        *nsp_++ = current_style();           // use current style
      }
    } // while
    *ntp_ = 0;
    *nsp_ = 0;
    //::printf("  RESULT: ntm='%s'\n", ntm_);
    //::printf("  RESULT: nsm='%s'\n", nsm_);
    buf->append(ntm_);                       // new text memory
    sbuf->append(nsm_);                      // new style memory
    free(ntm_);
    free(nsm_);
}

/**
 Appends new string 's' to terminal.

 The string can contain UTF-8, crlf's.
 And if ansi(bool) is set to 'true', ANSI 'ESC' sequences (such as ESC[1m)
 and other control characters (such as backspace) are handled.

 \param s string to append.

 \param len optional length of string can be specified if known
            to save the internals from having to call strlen()

 \see printf(), vprintf(), text(), clear()
*/
void Fl_Simple_Terminal::append(const char *s, int len) {
  // Remove ansi codes and adjust style buffer accordingly.
  if ( ansi() ) {
    append_ansi(s, len);
  } else {
    // raw append
    buf->append(s, len);
    lines_ += ::strcnt(s, '\n');  // count total line feeds in string added
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
  lines_ = 0;
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
  lines_ -= count;
  if ( lines_ < 0 ) lines_ = 0;
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
