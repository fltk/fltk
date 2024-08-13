//
// Fl_Terminal - A terminal widget for Fast Light Tool Kit (FLTK).
//
// Copyright 2022 by Greg Ercolano.
// Copyright 2024 by Bill Spitzak and others.
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

// TODO: double clicking text should make word selection, and drag should word-enlarge selection
// TODO: Add JG's ScrollbarStyle option to vertical scrollbar
// FIXME: While dragging a selection, hitting shift stops the selection

// This must appear above #include <assert.h>
#ifndef NDEBUG
#define NDEBUG          // comment out to enable assert()
#endif

#include <ctype.h>      // isdigit
#include <stdlib.h>     // malloc
#include <string.h>     // strlen
#include <stdarg.h>     // vprintf, va_list
#include <assert.h>

#include <FL/Fl.H>
#include <FL/Fl_Terminal.H>
#include <FL/fl_utf8.h> // fl_utf8len1
#include <FL/fl_draw.H>
#include <FL/fl_string_functions.h>
#include "Fl_String.H"

/////////////////////////////////
////// Static Functions /////////
/////////////////////////////////

#define MIN(a,b) ((a)<=(b)) ? (a) : (b)    // Return smaller of two values
#define MAX(a,b) ((a)>=(b)) ? (a) : (b)    // Return larger of two values
#define ABS(a)   ((a)<0) ? -(a) : (a)      // Return abs value

// Return val clamped between min and max
static int clamp(int val, int min, int max)
  { return (val<min) ? min : (val>max) ? max : val; }

// Swap integer values a and b
static void swap(int &a, int &b)
  { int asave = a; a = b; b = asave; }

static int normalize(int row, int maxrows) {
  row = row % maxrows;
  if (row < 0) row = maxrows + row;    // negative? index relative to end
  return row;
}

// Color channel management
static int red(Fl_Color val)      { return (val & 0xff000000) >> 24; }
static int grn(Fl_Color val)      { return (val & 0x00ff0000) >> 16; }
static int blu(Fl_Color val)      { return (val & 0x0000ff00) >>  8; }
static Fl_Color rgb(int r,int g,int b) { return (r << 24) | (g << 16) | (b << 8); }

// Return dim version of color 'val'
static Fl_Color dim_color(Fl_Color val) {
  int r = clamp(red(val) - 0x20, 0, 255);
  int g = clamp(grn(val) - 0x20, 0, 255);
  int b = clamp(blu(val) - 0x20, 0, 255);
  //DEBUG ::printf("DIM COLOR: %08x -> %08x\n", val, rgb(r,g,b));
  return rgb(r,g,b);
}

// Return bold version of color 'val'
static Fl_Color bold_color(Fl_Color val) {
  int r = clamp(red(val) + 0x20, 0, 255);
  int g = clamp(grn(val) + 0x20, 0, 255);
  int b = clamp(blu(val) + 0x20, 0, 255);
  //DEBUG ::printf("BOLD COLOR: %08x -> %08x\n", val, rgb(r,g,b));
  return rgb(r,g,b);
}

// Return an FLTK color for given XTERM foreground color index (0..7)
Fl_Color Fl_Terminal::CharStyle::fltk_fg_color(uchar ci) {
  static const Fl_Color xterm_fg_colors_[] = {
    0x00000000,       // 0
    0xd0000000,       // 1 - red
    0x00d00000,       // 2 - grn
    0xd0d00000,       // 3 - yel
    0x0000d000,       // 4 - blu
    0xd000d000,       // 5 - mag
    0x00d0d000,       // 6 - cyn
    0xd0d0d000        // 7 - white
  };
  if (ci==39) return defaultfgcolor_;   // special case for 'reset' color
  if (ci==49) return defaultbgcolor_;   // special case for 'reset' color
  ci &= 0x07;         // clamp to array size
  return xterm_fg_colors_[ci];
}

// Return an FLTK color for the given background color index (0..7) and attribute.
//    Background colors should be just a little darker than
//    the fg colors to prevent too much brightness clashing
//    for 'normal' bg vs fg colors.
//
Fl_Color Fl_Terminal::CharStyle::fltk_bg_color(uchar ci) {
  static const Fl_Color xterm_bg_colors_[] = {
    0x00000000,       // 0
    0xc0000000,       // 1 - red
    0x00c00000,       // 2 - grn
    0xc0c00000,       // 3 - yel
    0x0000c000,       // 4 - blu
    0xc000c000,       // 5 - mag
    0x00c0c000,       // 6 - cyn
    0xc0c0c000        // 7 - white
  };
  if (ci==39) return defaultfgcolor_;   // special case for 'reset' color
  if (ci==49) return defaultbgcolor_;   // special case for 'reset' color
  ci &= 0x07;         // clamp to array size
  return xterm_bg_colors_[ci];
}

// See if an Fl_Boxtype is FL_XXX_FRAME
static bool is_frame(Fl_Boxtype b) {
  if (b == FL_UP_FRAME       || b == FL_DOWN_FRAME      ||
      b == FL_THIN_UP_FRAME  || b == FL_THIN_DOWN_FRAME ||
      b == FL_ENGRAVED_FRAME || b == FL_EMBOSSED_FRAME  ||
      b == FL_BORDER_FRAME) return true;
  return false;
}

///////////////////////////////////////
////// Selection Class Methods ////////
///////////////////////////////////////

// Ctor
Fl_Terminal::Selection::Selection(Fl_Terminal *terminal)
  : terminal_(terminal)
{
  // These are used to set/get the mouse selection
  srow_ = scol_ = erow_ = ecol_ = 0;
  // FL_PUSH event row/col
  push_clear();
  selectionfgcolor_ = FL_BLACK;
  selectionbgcolor_ = FL_WHITE;
  state_              = 0;
  is_selection_       = false;
}

/**
  Return selection start/end.
  Ensures (start < end) to allow walking 'forward' thru selection,
  left-to-right, top-to-bottom.

  Returns:
  - true  -- valid selection values returned
  - false -- no selection was made, returned values undefined
*/
bool Fl_Terminal::Selection::get_selection(int &srow,int &scol,
                                           int &erow,int &ecol) const {
  srow = srow_; scol = scol_;
  erow = erow_; ecol = ecol_;
  if (!is_selection_) return false;
  // Ensure (start < end) on return
  if (srow_ == erow_ && scol_ > ecol_) swap(scol, ecol);
  if (srow_ > erow_)
    { swap(srow, erow); swap(scol, ecol); }
  return true;
}

// Start new selection at specified row,col
//    Always returns true.
//
bool Fl_Terminal::Selection::start(int row, int col, bool char_right) {
  (void) char_right;    // silence warning
  srow_ = erow_ = row;
  scol_ = ecol_ = col;
  state_ = 1;                                      // state: "started selection"
  is_selection_ = true;
  return true;
}

// Extend existing selection to row,col
//     Returns true if anything changed, false if not.
//
bool Fl_Terminal::Selection::extend(int row, int col, bool char_right) {
  // no selection started yet? start and return true
  int osrow = srow_, oerow = erow_, oscol = scol_, oecol = ecol_;
  bool oselection = is_selection_;
  if (state_ == 0) return start(row, col, char_right);
  state_ = 2;                                      // state: "extending selection"

  if ((row==push_row_) && (col+char_right==push_col_+push_char_right_)) {
    // we are in the box of the original push event
    srow_ = erow_ = row;
    scol_ = ecol_ = col;
    is_selection_ = false;
  } else if ((row>push_row_) || ((row==push_row_) && (col+char_right>push_col_+push_char_right_))) {
    // extend to the right and down
    scol_ = push_col_ + push_char_right_;
    ecol_ = col - 1 + char_right;
    is_selection_ = true;
  } else {
    // extend to the left and up
    scol_ = push_col_ - 1 + push_char_right_;
    ecol_ = col + char_right;
    is_selection_ = true;
  }

  if (scol_<0) scol_ = 0;
  if (ecol_<0) ecol_ = 0;
  int maxCol = terminal_->ring_cols()-1;
  if (scol_>maxCol) scol_ = maxCol;
  if (ecol_>maxCol) ecol_ = maxCol;
  srow_ = push_row_;
  erow_ = row;

  bool changed = (   (osrow != srow_) || (oerow != erow_)
                  || (oscol != scol_) || (oecol != ecol_)
                  || (oselection != is_selection_) );
  return !changed;
}

// End selection (turn dragging() off)
void Fl_Terminal::Selection::end(void) {
  state_ = 3;                                      // state: "finished selection"
  // Order selection
  if (erow_ < srow_)
    { swap(srow_, erow_); swap(scol_, ecol_); }
  if (erow_ == srow_ && scol_ > ecol_) swap(scol_, ecol_);
}

// Create a complete selection
void Fl_Terminal::Selection::select(int srow, int scol, int erow, int ecol) {
  srow_ = srow; scol_ = scol;
  erow_ = erow; ecol_ = ecol;
  state_ = 3;                                      // state: "finished selection"
  is_selection_ = true;
}

// Clear selection
//     Returns true if there was a selection, false if there wasn't
//
bool Fl_Terminal::Selection::clear(void) {
  bool was_selected = is_selection();              // save for return
  srow_ = scol_ = erow_ = ecol_ = 0;
  state_ = 0;
  is_selection_ = false;
  return was_selected;
}

// Scroll the selection up(+)/down(-) number of rows
void Fl_Terminal::Selection::scroll(int nrows) {
  if (is_selection()) {
    srow_ -= nrows;
    erow_ -= nrows;
    // Selection scrolled off? clear selection
    if (srow_ < 0 || erow_ < 0) clear();
  }
}

///////////////////////////////////////
////// EscapeSeq Class Methods ////////
///////////////////////////////////////

// Append char to buff[] safely (with bounds checking)
//    Returns:
//      success - ok
//      fail    - buffer full/overflow
//
int Fl_Terminal::EscapeSeq::append_buff(char c) {
  if (buffp_ >= buffendp_) return fail;   // end of buffer reached?
  *buffp_++ = c;
  *buffp_   = 0;                          // keep buff[] null terminated
  return success;
}

// Append whatever integer string is at valbuffp into vals_[] safely w/bounds checking
//    Assumes valbuffp points to a null terminated string.
//    Returns:
//       success - parsed ok
//       fail    - error occurred (non-integer, or vals_[] full)
//
int Fl_Terminal::EscapeSeq::append_val(void) {
  if (vali_ >= maxvals)                            // vals_[] full?
    { vali_ = maxvals-1; return fail; }            // clamp index, fail
  if (!valbuffp_ || (*valbuffp_ == 0))             // no integer to parse? e.g. ESC[m, ESC[;m
    { vals_[vali_] = 0; return success; }          // zero in array, do not inc vali
  if (sscanf(valbuffp_, "%d", &vals_[vali_]) != 1) // Parse integer into vals_[]
    { return fail; }                               // fail if parsed a non-integer
  vals_[vali_] &= 0x3ff;                           // sanity: enforce int in range 0 ~ 1023 (prevent DoS attack)
  if (++vali_ >= maxvals)                          // advance val index, fail if too many vals
    { vali_ = maxvals-1; return fail; }            // clamp + fail
  valbuffp_ = 0;                                   // parsed val ok, reset valbuffp to NULL
  return success;
}

// Ctor
Fl_Terminal::EscapeSeq::EscapeSeq(void) {
  reset();
  save_row_  = -1;  // only in ctor
  save_col_  = -1;
}

// Reset the class
//    Named reset to not be confused with clear() screen/line/etc
//
void Fl_Terminal::EscapeSeq::reset(void) {
  esc_mode_  = 0;                          // disable ESC mode, so parse_in_progress() returns false
  csi_       = false;                      // CSI off until '[' received
  buffp_     = buff_;                      // point to beginning of buffer
  buffendp_  = buff_ + (maxbuff - 1);      // point to end of buffer
  valbuffp_  = 0;                          // disable val ptr (no vals parsed yet)
  vali_      = 0;                          // zero val index
  buff_[0]   = 0;                          // null terminate buffer
  vals_[0]   = 0;                          // first val[] 0
  memset(vals_, 0, sizeof(vals_));
}

// Return current escape mode.
//    This is really only valid after parse() returns 'completed'.
//    After a reset() this will return 0.
//
char Fl_Terminal::EscapeSeq::esc_mode(void) const { return esc_mode_; }

// Set current escape mode.
void Fl_Terminal::EscapeSeq::esc_mode(char val) { esc_mode_ = val; }

// Return the total vals parsed.
//    This is really only valid after parse() returns 'completed'.
//
int Fl_Terminal::EscapeSeq::total_vals(void) const { return vali_; }

// Return the value at index i.
//    i is not range checked; it's assumed 0 <= i < total_vals().
//    It is only valid to call this after parse() returns 'completed'.
//
int Fl_Terminal::EscapeSeq::val(int i) const { return vals_[i]; }

// See if we're in the middle of parsing an ESC sequence
bool Fl_Terminal::EscapeSeq::parse_in_progress(void) const {
  return (esc_mode_ == 0) ? false : true;
}

// See if we're in the middle of parsing an ESC sequence
bool Fl_Terminal::EscapeSeq::is_csi(void) const { return csi_; }

// Return with default value (if none) or vals[0] (if at least one val spec'd).
//    Handles default for single values (e.g. ESC[#H vs. ESC[H)
//    vals[0] is clamped between 0 and 'max'
//
int Fl_Terminal::EscapeSeq::defvalmax(int dval, int max) const {
  if (total_vals() == 0) return dval;
  else                   return clamp(vals_[0], 0, max);
}

// Save row,col for later retrieval
void Fl_Terminal::EscapeSeq::save_cursor(int row, int col) {
  save_row_ = row;
  save_col_ = col;
}

// Restore last saved cursor position into row,col
void Fl_Terminal::EscapeSeq::restore_cursor(int &row, int &col) {
  row = save_row_;
  col = save_col_;
}

// Handle parsing an escape sequence.
//    Call this only if parse_in_progress() is true.
//    Passing ESC does a reset() and sets esc_mode() to ESC.
//    When a full escape sequence has been parsed, 'completed' is returned (see below).
//
// Returns:
//   fail      - error occurred: escape sequence invalid, class is reset()
//   success   - parsing ESC sequence OK so far, still in progress/not done yet
//   completed - complete ESC sequence was parsed, esc_mode() will be the operation, e.g.
//                  'm' - <ESC>[1m -- is_csi() will be true, val() has value(s) parsed
//                  'A' - <ESC>A   -- is_csi() will be false (no vals)
//
int Fl_Terminal::EscapeSeq::parse(char c) {
  // NOTE: During parsing esc_mode() will be:
  //             0 - reset/not parsing
  //          0x1b - ESC received, expecting next one of A/B/C/D or '['
  //           '[' - actively parsing CSI sequence, e.g. ESC[
  //
  //       At the /end/ of parsing, after 'completed' is returned,
  //       esc_mode() will be the mode setting char, e.g. 'm' for 'ESC[0m', etc.
  //
  if (c == 0) {                             // NULL? (caller should really never send us this)
    return success;                         // do nothing -- leave state unchanged, return 'success'
  } else if (c == 0x1b) {                   // ESC at ANY time resets class/begins new ESC sequence
    reset();
    esc_mode(0x1b);
    if (append_buff(c) < 0) goto pfail;     // save ESC in buf
    return success;
  } else if (c < ' ' || c >= 0x7f) {        // any other control or binary characters?
    goto pfail;                             // reset + fail out of esc sequence parsing
  }
  // Whatever the character is, handle it depending on esc_mode..
  if (esc_mode() == 0x1b) {                 // in ESC mode?
    if (c == '[') {                         // <ESC>[? CSI (Ctrl Seq Introducer)
      esc_mode(c);                          // switch to parsing mode for ESC[#;#;#..
      csi_      = true;                     // this is now a CSI sequence
      vali_     = 0;                        // zero vals_[] index
      valbuffp_ = 0;                        // valbuffp NULL (no vals yet)
      if (append_buff(c) < 0) goto pfail;   // save '[' in buf
      return success;                       // success
    } else if ((c >= '@' && c <= 'Z') ||    // C1 control code (e.g. <ESC>D, <ESC>c, etc)
               (c >= 'a' && c <= 'z')) {
      esc_mode(c);                          // save op in esc_mode() for caller to see
      csi_      = false;                    // NOT a CSI sequence
      vali_     = 0;
      valbuffp_ = 0;                        // valbuffp NULL (no vals yet)
      if (append_buff(c) < 0) goto pfail;   // save op in buf
      return completed;                     // completed sequence
    } else {                                // ESCx?
      goto pfail;                           // not supported
    }
  } else if (esc_mode() == '[') {           // '[' mode? e.g. ESC[... aka. is_csi()
    if (c == ';') {                         // ';' indicates end of a value, e.g. ESC[0;2..
      if (append_val()   < 0) goto pfail;   // append value parsed so far, vali gets inc'ed
      if (append_buff(c) < 0) goto pfail;   // save ';' in buf
      return success;
    }
    if (isdigit(c)) {                       // parsing an integer?
      if (!valbuffp_)                       // valbuffp not set yet?
        { valbuffp_ = buffp_; }             // point to first char in integer string
      if (append_buff(c) < 0) goto pfail;   // add value to buffer
      return success;
    }
    // Not a ; or digit? fall thru to [A-Z,a-z] check
  } else {                                  // all other esc_mode() chars are fail/unknown
    goto pfail;
  }
  if (( c >= '@' && c<= 'Z') ||             // ESC#X or ESC[...X, where X is [A-Z,a-z]?
      ( c >= 'a' && c<= 'z')) {
    if (append_val() < 0 ) goto pfail;      // append any trailing vals just before letter
    if (append_buff(c) < 0 ) goto pfail;    // save letter in buffer
    esc_mode(c);                            // change mode to the mode setting char
    return completed;                       // completed/done
  }
  // Any other chars? reset+fail
pfail:
  reset();
  return fail;
}

//////////////////////////////////////
///// CharStyle Class Methods ////////
//////////////////////////////////////

// Ctor
//   fontsize_defer - if true, hold off on doing fl_font() oriented calculations until
//                    just before draw(). This is for fluid's headless mode. (issue 837)
//
Fl_Terminal::CharStyle::CharStyle(bool fontsize_defer) {
  attrib_           = 0;
  charflags_        = (FG_XTERM | BG_XTERM);
  defaultfgcolor_   = 0xd0d0d000;   // off white
  defaultbgcolor_   = 0xffffffff;   // special color: doesn't draw, 'shows thru' to box()
  fgcolor_          = defaultfgcolor_;
  bgcolor_          = defaultbgcolor_;
  fontface_         = FL_COURIER;
  fontsize_         = 14;
  if (!fontsize_defer) update();       // normal behavior
  else                 update_fake();  // use fake values instead
}

// Update fontheight/descent cache whenever font changes
void Fl_Terminal::CharStyle::update(void) {
  // cache these values
  fl_font(fontface_, fontsize_);
  fontheight_  = int(fl_height()   + 0.5);
  fontdescent_ = int(fl_descent()  + 0.5);
  charwidth_   = int(fl_width("X") + 0.5);
}

// Update fontheight/descent cache with fake values (issue 837)
//   XXX: Please remove this issue 837 hack the minute fluid no longer has to
//        instance fltk classes in headless mode
//
void Fl_Terminal::CharStyle::update_fake(void) {
  fontheight_       = 99;  // Use fake values until first draw() when we call update() for real values.
  fontdescent_      = 99;  // Use absurdly large values here to make un-updated sizes clearly evident
  charwidth_        = 99;
}

// Return fg color
Fl_Color Fl_Terminal::CharStyle::fgcolor(void) const {
  return fgcolor_;
}

// Return bg color
Fl_Color Fl_Terminal::CharStyle::bgcolor(void) const {
  return bgcolor_;
}

// Return only the color bit flags
//   Only the color bits of 'inflags' are modified with our color bits.
//
uchar Fl_Terminal::CharStyle::colorbits_only(uchar inflags) const {
  return (inflags & ~COLORMASK) | (charflags_ & COLORMASK);   // add color bits only
}

void Fl_Terminal::CharStyle::fgcolor_xterm(uchar val) {
  fgcolor_ = fltk_fg_color(val);
  set_charflag(FG_XTERM);
}

void Fl_Terminal::CharStyle::bgcolor_xterm(uchar val) {
  bgcolor_ = fltk_bg_color(val);
  set_charflag(BG_XTERM);
}

///////////////////////////////////
///// Cursor Class Methods ////////
///////////////////////////////////

// Is cursor at display row,col?
bool Fl_Terminal::Cursor::is_rowcol(int drow,int dcol) const {
  return(drow == row_ && dcol == col_);
}

// Scroll (move) the cursor row up(+)/down(-) number of rows
void Fl_Terminal::Cursor::scroll(int nrows) {
  row_ = MAX(row_ - nrows, 0);   // don't let (row_<0)
}

/////////////////////////////////////
///// Utf8Char Class Methods ////////
/////////////////////////////////////

// Ctor
Fl_Terminal::Utf8Char::Utf8Char(void) {
  text_[0]   = ' ';
  len_       = 1;
  attrib_    = 0;
  charflags_ = 0;
  fgcolor_   = 0xffffff00;
  bgcolor_   = 0xffffffff;   // special color: doesn't draw, 'shows thru' to box()
}

// copy ctor
Fl_Terminal::Utf8Char::Utf8Char(const Utf8Char& src) {
  // local instance not initialized yet; init first, then copy text
  text_[0]   = ' ';
  len_       = 1;
  attrib_    = src.attrib_;
  charflags_ = src.charflags_;
  fgcolor_   = src.fgcolor_;
  bgcolor_   = src.bgcolor_;
  text_utf8_(src.text_utf8(), src.length());    // copy the src text
}

// assignment
Fl_Terminal::Utf8Char& Fl_Terminal::Utf8Char::operator=(const Utf8Char& src) {
  // local instance is already initialized, so just change its contents
  text_utf8_(src.text_utf8(), src.length());    // local copy src text
  attrib_    = src.attrib_;
  charflags_ = src.charflags_;
  fgcolor_   = src.fgcolor_;
  bgcolor_   = src.bgcolor_;
  return *this;
}

// dtor
Fl_Terminal::Utf8Char::~Utf8Char(void) {
  len_ = 0;
}

// Set 'text_' to valid UTF-8 string 'text'.
//
// text_ must not be NULL, and len must be in range: 1 <= len <= max_utf8().
// NOTE: Caller must handle such checks, and use handle_unknown_char()
// for invalid chars.
//
void Fl_Terminal::Utf8Char::text_utf8_(const char *text, int len) {
  memcpy(text_, text, len);
  len_ = len;                                    // update new length
}

// Set UTF-8 string for this char.
//
// text_ must not be NULL, and len must be in range: 1 <= len <= max_utf8().
// NOTE: Caller must handle such checks, and use handle_unknown_char()
// for invalid chars.
//
void Fl_Terminal::Utf8Char::text_utf8(const char *text,
                                      int len,
                                      const CharStyle& style) {
  text_utf8_(text, len);                       // updates text_, len_
  //issue 837 // fl_font(style.fontface(), style.fontsize()); // need font to calc UTF-8 width
  attrib_    = style.attrib();
  charflags_ = style.colorbits_only(charflags_);
  fgcolor_   = style.fgcolor();
  bgcolor_   = style.bgcolor();
}

// Set char to single printable ASCII character 'c'
//     'c' must be "printable" ASCII in the range (0x20 <= c <= 0x7e).
//     Anything outside of that is silently ignored.
//
void Fl_Terminal::Utf8Char::text_ascii(char c, const CharStyle& style) {
  // Signed char vals above 0x7f are /negative/, so <0x20 check covers those
  if (c < 0x20 || c >= 0x7e) return;           // ASCII non-printable?
  text_utf8(&c, 1, style);
}

// Set fl_font() based on specified style for this char's attribute
void Fl_Terminal::Utf8Char::fl_font_set(const CharStyle& style) const {
  int face = style.fontface() |
               ((attrib_ & Fl_Terminal::BOLD)   ? FL_BOLD   : 0) |
               ((attrib_ & Fl_Terminal::ITALIC) ? FL_ITALIC : 0);
  fl_font(face, style.fontsize());
}

// Return the foreground color as an fltk color
Fl_Color Fl_Terminal::Utf8Char::fgcolor(void) const {
  return fgcolor_;
}

// Return the background color as an fltk color
Fl_Color Fl_Terminal::Utf8Char::bgcolor(void) const {
  return bgcolor_;
}

// Return the width of this character in floating point pixels
//
//    WARNING: Uses current font, so assumes fl_font(face,size)
//             has already been set to current font!
//
double Fl_Terminal::Utf8Char::pwidth(void) const {
  return fl_width(text_, len_);
}

// Return the width of this character in integer pixels
//
//    WARNING: Uses current font, so assumes fl_font(face,size)
//             has already been set to current font!
//
int Fl_Terminal::Utf8Char::pwidth_int(void) const {
  return int(fl_width(text_, len_) + 0.5);
}

// Return color \p col, possibly influenced by BOLD or DIM attributes \p attr.
//    If a \p grp widget is specified (i.e. not NULL), don't let the color \p col be
//    influenced by the attribute bits /if/ \p col matches the \p grp widget's own color().
//
Fl_Color Fl_Terminal::Utf8Char::attr_color_(Fl_Color col, const Fl_Widget *grp) const {
  // Don't modify color if it's the special 'see thru' color 0xffffffff or widget's color()
  if (grp && ((col == 0xffffffff) || (col == grp->color()))) return grp->color();
  switch (attrib_ & (Fl_Terminal::BOLD|Fl_Terminal::DIM)) {
    case 0: return col;                                   // not bold or dim? no change
    case Fl_Terminal::BOLD: return bold_color(col);       // bold? use bold_color()
    case Fl_Terminal::DIM : return dim_color(col);        // dim?  use dim_color()
    default:                return col;                   // bold + dim? cancel out
  }
}

// Return the fg color of char \p u8c possibly influenced by BOLD or DIM.
//    If a \p grp widget is specified (i.e. not NULL), don't let the color \p col be
//    influenced by the attribute bits /if/ \p col matches the \p grp widget's own color().
//
Fl_Color Fl_Terminal::Utf8Char::attr_fg_color(const Fl_Widget *grp) const {
  if (grp && (fgcolor_ == 0xffffffff))           // see thru color?
    { return grp->color(); }                     // return grp's color()
  return (charflags_ & Fl_Terminal::FG_XTERM)    // fg is an xterm color?
           ? attr_color_(fgcolor(), grp)         // ..use attributes
           : fgcolor();                          // ..ignore attributes.
}

Fl_Color Fl_Terminal::Utf8Char::attr_bg_color(const Fl_Widget *grp) const {
  if (grp && (bgcolor_ == 0xffffffff))           // see thru color?
    { return grp->color(); }                     // return grp's color()
  return (charflags_ & Fl_Terminal::BG_XTERM)    // bg is an xterm color?
           ? attr_color_(bgcolor(), grp)         // ..use attributes
           : bgcolor();                          // ..ignore attributes.
}


////////////////////////////////////
///// RingBuffer Class Methods /////
////////////////////////////////////

// Handle adjusting 'offset_' specified number of rows to do "scrolling".
//    rows can be +/-: positive effectively scrolls "up", negative scrolls "down".
//    rows will be clamped
//
void Fl_Terminal::RingBuffer::offset_adjust(int rows) {
  if (!rows) return;                            // early exit if nothing to do
  if (rows>0) {                                 // advance?
    offset_ = (offset_ + rows) % ring_rows_;    // apply, and keep offset_ within ring_rows
  } else {
    rows = clamp(-rows, 1, ring_rows_);         // make positive, limit to ring size
    offset_ -= rows;                            // apply offset
    if (offset_<0) offset_ += ring_rows_;       // wrap underflows
  }
}

// Create a new copy of the buffer with different row/col sizes
//   Preserves old contents of display and history in use.
//
//   The old buffer might have an offset and the hist/disp might wrap
//   around the end of the ring. The NEW buffer's offset will be zero,
//   so the hist/disp do NOT wrap around, making the move operation easier to debug.
//
//   The copy preservation starts at the LAST ROW in display of both old (src) and new (dst)
//   buffers, and copies rows in reverse until hist_use_srow() reached, or if we hit top
//   of the new history (index=0), which ever comes first. So in the following where the
//   display is being enlarged, the copy preservation starts at "Line 5" (bottom of display)
//   and works upwards, ending at "Line 1" (top of history use):
//
//                   OLD (SRC)            NEW (DST)
//                 _____________        _____________   ___
//                | x x x x x x |      | x x x x x x |   ʌ                      'x' indicates
//                | Line 1      | ─┐   | x x x x x x |   |  hist_rows           unused history
//                | Line 2      |  └─> | Line 1      |   v                      buffer memory.
//                |-------------|      |-------------|  ---
//                | Line 3      |      | Line 2      |   ʌ
//                | Line 4      |      | Line 3      |   |
//                | Line 5      | ─┐   | Line 4      |   |  disp_rows
//                |_____________|  └─> | Line 5      |   |
//                                     |_____________|  _v_
//
void Fl_Terminal::RingBuffer::new_copy(int drows, int dcols, int hrows, const CharStyle& style) {
  (void)style;                                              // currently unused - need parameterized ctor (†)
  // Create new buffer
  int addhist       = disp_rows() - drows;                  // adjust history use
  int new_ring_rows = (drows+hrows);
  int new_hist_use  = clamp(hist_use_ + addhist, 0, hrows); // clamp incase new_hist_rows smaller than old
  int new_nchars    = (new_ring_rows * dcols);
  Utf8Char *new_ring_chars = new Utf8Char[new_nchars];      // Create new ring buffer (†)
  // Preserve old contents in new buffer
  int dst_cols      = dcols;
  int src_stop_row  = hist_use_srow();
  int tcols         = MIN(ring_cols(), dcols);
  int src_row       = hist_use_srow() + hist_use_ + disp_rows_ - 1; // use row#s relative to hist_use_srow()
  int dst_row       = new_ring_rows - 1;
  // Copy rows: working up from bottom of disp, stop at top of hist
  while ((src_row >= src_stop_row) && (dst_row >= 0)) {
    Utf8Char *src = u8c_ring_row(src_row);
    Utf8Char *dst = new_ring_chars + (dst_row*dst_cols);
    for (int col=0; col<tcols; col++ ) *dst++ = *src++;
    --src_row;
    --dst_row;
  }
  // Install new buffer: dump old, install new, adjust internals
  if (ring_chars_) delete[] ring_chars_;
  ring_chars_ = new_ring_chars;
  ring_rows_  = new_ring_rows;
  ring_cols_  = dcols;
  nchars_     = new_nchars;
  hist_rows_  = hrows;
  hist_use_   = new_hist_use;
  disp_rows_  = drows;
  offset_     = 0;        // for new buffer, we used a zero offset
}

// Clear the class, delete previous ring if any
void Fl_Terminal::RingBuffer::clear(void) {
  if (ring_chars_) delete[] ring_chars_; // dump our ring
  ring_chars_ = 0;
  ring_rows_  = 0;
  ring_cols_  = 0;
  nchars_     = 0;
  hist_rows_  = 0;
  hist_use_   = 0;
  disp_rows_  = 0;
  offset_     = 0;
}

// Clear history
void Fl_Terminal::RingBuffer::clear_hist(void) {
  hist_use_ = 0;
}

// Default ctor
Fl_Terminal::RingBuffer::RingBuffer(void) {
  ring_chars_ = 0;
  clear();
}

// Ctor with specific sizes
Fl_Terminal::RingBuffer::RingBuffer(int drows, int dcols, int hrows) {
  // Start with cleared buffer first..
  ring_chars_ = 0;
  clear();
  // ..then create.
  create(drows, dcols, hrows);
}

// Dtor
Fl_Terminal::RingBuffer::~RingBuffer(void) {
  if (ring_chars_) delete[] ring_chars_;
  ring_chars_ = NULL;
}

// See if 'grow' is within the history buffer
//    It's assumed grow is in the range 0 .. hist_rows()-1.
//
bool Fl_Terminal::RingBuffer::is_hist_ring_row(int grow) const {
  grow %= ring_rows_;
  grow -= offset_;                       // move into positive space
  if (grow < 0) { grow = (ring_rows_ + grow); }
  int htop = 0;
  int hbot = (hist_rows_ - 1);
  return ((grow >= htop) && (grow <= hbot));
}

// See if 'grow' is within the display buffer
//    It's assumed grow is in the range hist_rows() .. ring_rows()-1.
//
bool Fl_Terminal::RingBuffer::is_disp_ring_row(int grow) const {
  grow %= ring_rows_;
  grow -= offset_;
  if (grow < 0) { grow = (ring_rows_ + grow); }
  int dtop = hist_rows_;
  int dbot = hist_rows_ + disp_rows_ - 1;
  return ((grow >= dtop) && (grow <= dbot));
}

// Move display row from src_row to dst_row
void Fl_Terminal::RingBuffer::move_disp_row(int src_row, int dst_row) {
  Utf8Char *src = u8c_disp_row(src_row);
  Utf8Char *dst = u8c_disp_row(dst_row);
  for (int col=0; col<disp_cols(); col++) *dst++ = *src++;
}

// Clear the display rows 'sdrow' thru 'edrow' inclusive using specified CharStyle 'style'
void Fl_Terminal::RingBuffer::clear_disp_rows(int sdrow, int edrow, const CharStyle& style) {
  for (int drow=sdrow; drow<=edrow; drow++) {
    int row = hist_rows_ + drow + offset_;
    Utf8Char *u8c = u8c_ring_row(row);
    for (int col=0; col<disp_cols(); col++) u8c++->clear(style);
  }
}

// Scroll the ring buffer up or down #rows, using 'style' for empty rows
//   > Positive rows scroll "up", moves top line(s) into history, clears bot line(s)
//     Increases hist_use (unless maxed out).
//   > Negative rows scroll "down", clears top line(s), history unaffected
//
void Fl_Terminal::RingBuffer::scroll(int rows, const CharStyle& style) {
  if (rows > 0) {
    // Scroll up into history
    //   Example: scroll(2):
    //
    //              BEFORE                       AFTER
    //                 ---------------            ---------------
    //                |    H i s t    | ---      | x x x x x x x | \_ blanked rows
    //                |               |    \     | x x x x x x x | /
    //                |               |     ---> |---------------| <- disp_erow()
    //                |               |          |    H i s t    |
    //                |               |          |               |
    // disp_srow() -> |---------------| ---      | 0001          |
    //                | 0001          |    \     | 0002          |
    //                | 0002          |     ---> |---------------| <- disp_srow()
    //                | 0003          |          | 0003          |
    //                | 0004          |          | 0004          |
    //                | 0005          | ---      | 0005          |
    //                | 0006          |    \     | 0006          |
    // disp_erow() ->  ---------------      --->  ---------------
    //
    //                                  \______/
    //                                   Simple
    //                                   Offset
    rows = clamp(rows, 1, disp_rows());                        // sanity
    // Scroll up into history
    offset_adjust(rows);
    // Adjust hist_use, clamp to max
    hist_use_ = clamp(hist_use_ + rows, 0, hist_rows_);
    // Clear exposed lines at bottom
    int srow = (disp_rows() - rows) % disp_rows();
    int erow = disp_rows() - 1;
    clear_disp_rows(srow, erow, style);
  } else {
    // Scroll down w/out affecting history
    //   To leave history unaffected, we must move memory.
    //   Start at bottom row [A] and work up to top row [B].
    //
    //   Example: scroll(-2):
    //
    //       BEFORE                        AFTER
    //          -----------------            -----------------  _
    //         | 0001        [B] | ---┐     | x x x x x x x x |  \_ blanked
    //         | 0002            |    |     | x x x x x x x x | _/  rows
    //         | 0003            |    └---> | 0001            |
    //         | 0004        [A] | ---┐     | 0002            |
    //         | 0005            |    |     | 0003            |
    //         | 0006            |    └---> | 0004            |
    //          -----------------            -----------------
    //                             \______/
    //                              Memory
    //                               move
    rows = clamp(-rows, 1, disp_rows());                  // make rows positive + sane
    for (int row=disp_rows()-1; row>=0; row--) {          // start at end of disp and work up
      int src_row = (row - rows);                         // src is offset #rows being scrolled
      int dst_row = row;                                  // dst is display
      if (src_row >= 0) move_disp_row(src_row, dst_row);  // ok to move row? move row down
      else clear_disp_rows(dst_row, dst_row, style);      // hit top? blank rest of rows
    }
  }
}

// Return UTF-8 char for 'row' in the ring
// Scrolling offset is NOT applied; this is raw access to the ring's rows.
//
// Example:
//   // Walk ALL rows in the ring buffer..
//   for (int row=0; row<ring.rows(); row++) {
//     Utf8Char *u8c = ring.u8c_ring_row(row);
//     ..
//   }
//
const Fl_Terminal::Utf8Char* Fl_Terminal::RingBuffer::u8c_ring_row(int row) const {
  row = normalize(row, ring_rows());
  assert(row >= 0 && row < ring_rows_);
  return &ring_chars_[row * ring_cols()];
}

// Return UTF-8 char for beginning of 'row' in the history buffer.
//   Example:
//     // Walk ALL rows in history..
//     for (int hrow=0; hrow<ring.hrows(); hrow++) {
//       Utf8Char *u8c = u8c_hist_row(hrow);
//       ..
//     }
//
const Fl_Terminal::Utf8Char* Fl_Terminal::RingBuffer::u8c_hist_row(int hrow) const {
  int rowi = normalize(hrow, hist_rows());
  rowi = (rowi + offset_) % ring_rows_;
  assert(rowi >= 0 && rowi <= ring_rows_);
  return &ring_chars_[rowi * ring_cols()];
}

// Special case to walk the "in use" rows of the history
//   Example:
//     // Walk the "in use" rows of history..
//     for (int hrow=0; hrow<ring.hist_use(); hrow++) {
//       Utf8Char *u8c = u8c_hist_use_row(hrow);
//       ..
//     }
//
const Fl_Terminal::Utf8Char* Fl_Terminal::RingBuffer::u8c_hist_use_row(int hurow) const {
  if (hist_use_ == 0) return 0;             // history is empty! (caller is dumb to ask)
  hurow = hurow % hist_use_;                // normalize indexing within history in use
  hurow = hist_rows_ - hist_use_ + hurow;   // index hist_use rows from end history
  hurow = (hurow + offset_) % ring_rows_;   // convert to absolute index in ring_chars_[]
  assert(hurow >= 0 && hurow <= hist_use());
  return &ring_chars_[hurow * ring_cols()];
}

// Return UTF-8 char for beginning of 'row' in the display buffer
//   Example:
//     // Walk ALL rows in display..
//     for (int drow=0; drow<ring.drows(); drow++) {
//         Utf8Char *u8c = u8c_disp_row(drow);
//         ..
//     }
//
const Fl_Terminal::Utf8Char* Fl_Terminal::RingBuffer::u8c_disp_row(int drow) const {
  int rowi = normalize(drow, disp_rows());
  rowi = (hist_rows_ + rowi + offset_) % ring_rows_; // display starts at end of history
  assert(rowi >= 0 && rowi <= ring_rows_);
  return &ring_chars_[rowi * ring_cols()];
}

// non-const versions of the above ////////////////////////////////////////////////

// Return UTF-8 char for 'row' in the ring.
// Scrolling offset is NOT applied; this is raw access to the ring's rows.
// Example:
//   // Walk ALL rows in the ring buffer..
//   for (int row=0; row<ring.rows(); row++) {
//     Utf8Char *u8c = ring.u8c_ring_row(row);
//     ..
//   }
//
Fl_Terminal::Utf8Char* Fl_Terminal::RingBuffer::u8c_ring_row(int row)
  { return const_cast<Utf8Char*>(const_cast<const RingBuffer*>(this)->u8c_ring_row(row)); }

Fl_Terminal::Utf8Char* Fl_Terminal::RingBuffer::u8c_hist_row(int hrow)
  { return const_cast<Utf8Char*>(const_cast<const RingBuffer*>(this)->u8c_hist_row(hrow)); }

Fl_Terminal::Utf8Char* Fl_Terminal::RingBuffer::u8c_hist_use_row(int hurow)
  { return const_cast<Utf8Char*>(const_cast<const RingBuffer*>(this)->u8c_hist_use_row(hurow)); }

// Return UTF-8 char for beginning of 'row' in the display buffer
// Example:
//   // Walk ALL rows in display..
//   for (int drow=0; drow<ring.drows(); drow++) {
//     Utf8Char *u8c = u8c_disp_row(drow);
//     ..
//   }
Fl_Terminal::Utf8Char* Fl_Terminal::RingBuffer::u8c_disp_row(int drow)
  { return const_cast<Utf8Char*>(const_cast<const RingBuffer*>(this)->u8c_disp_row(drow)); }

// Resize ring buffer by creating new one, dumping old (if any).
// Input:
//    drows -- display height in lines of text (rows)
//    dcols -- display width in characters (columns)
//    hrows -- scrollback history size in lines of text (rows)
//
void Fl_Terminal::RingBuffer::create(int drows, int dcols, int hrows) {
  clear();
  // History
  hist_rows_  = hrows;
  hist_use_   = 0;
  // Display
  disp_rows_  = drows;
  // Ring buffer
  ring_rows_  = hist_rows_ + disp_rows_;
  ring_cols_  = dcols;
  nchars_     = ring_rows_ * ring_cols_;
  ring_chars_ = new Utf8Char[nchars_];
}

// Resize the buffer, preserve previous contents as much as possible
void Fl_Terminal::RingBuffer::resize(int drows, int dcols, int hrows, const CharStyle& style) {
  int  new_rows     = drows + hrows;              // old display + history rows
  int  old_rows     = disp_rows() + hist_rows();  // new display + history rows
  bool cols_changed = (dcols != disp_cols());     // was there a change in total #columns?
  bool rows_changed = (new_rows != old_rows);     // was there a change in total #rows?
  // If rows or cols changed, make a NEW buffer and copy old contents.
  // New copy will have disp/hist_rows/cols and nchars adjusted.
  if (cols_changed || rows_changed) {             // rows or cols changed?
    new_copy(drows, dcols, hrows, style);         // rebuild ring buffer, preserving contents
  } else {
    // Cols and total rows the same, probably just changed disp/hist ratio
    int addhist = disp_rows() - drows;            // adj hist_use smaller if disp enlarged
    hist_rows_  = hrows;                          // adj hist rows for new value
    disp_rows_  = drows;                          // adj disp rows for new value
    hist_use_   = clamp(hist_use_ + addhist, 0, hrows);
  }
}

// Change the display rows. Use style for new rows, if any.
void Fl_Terminal::RingBuffer::change_disp_rows(int drows, const CharStyle& style)
  { resize(drows, ring_cols(), hist_rows(), style); }

// Change the display columns. Use style for new columns, if any.
void Fl_Terminal::RingBuffer::change_disp_cols(int dcols, const CharStyle& style)
  { resize(disp_rows(), dcols, hist_rows(), style); }

/////////////////////////////////////
///// Fl_Terminal Class Methods /////
/////////////////////////////////////

/// See docs for non-const version of u8c_ring_row(int)
const Fl_Terminal::Utf8Char* Fl_Terminal::u8c_ring_row(int grow) const
  { return ring_.u8c_ring_row(grow); }

/// See docs for non-const version of u8c_hist_row(int)
const Fl_Terminal::Utf8Char* Fl_Terminal::u8c_hist_row(int hrow) const
  { return ring_.u8c_hist_row(hrow); }

/// See docs for non-const version of u8c_hist_use_row(int)
const Fl_Terminal::Utf8Char* Fl_Terminal::u8c_hist_use_row(int hurow) const
  { return ring_.u8c_hist_use_row(hurow); }

/// See docs for non-const version of u8c_disp_row(int)
const Fl_Terminal::Utf8Char* Fl_Terminal::u8c_disp_row(int drow) const
  { return ring_.u8c_disp_row(drow); }

// non-const versions of the above ////////////////////////////////////////////////

/**
  Return UTF-8 char for row \p grow in the ring buffer.

  \p grow is globally indexed relative to the beginning of the ring buffer,
  so this method can access ANY character in the entire ring buffer (hist or disp)
  by the global index.

  Scrolling offset is NOT applied; this is raw access to the ring's rows.

  Should really ONLY be used for making a complete copy of the ring.

  Example:
  \code
  // Walk ALL rows and cols in the ring buffer..
  for (int row=0; row<ring.rows(); row++) {
      Utf8Char *u8c = ring.u8c_ring_row(row);
      for (int col=0; col<ring.cols(); col++,u8c++) {
          ..make use of u8c->xxx() methods..
      }
  }
  \endcode
*/
Fl_Terminal::Utf8Char* Fl_Terminal::u8c_ring_row(int grow)
  { return const_cast<Utf8Char*>(const_cast<const Fl_Terminal*>(this)->u8c_ring_row(grow)); }

/**
  Return u8c for beginning of a row inside the scrollback history.
  'hrow' is indexed relative to the beginning of the scrollback history buffer.
  \see u8c_disp_row(int) for example use.
*/
Fl_Terminal::Utf8Char* Fl_Terminal::u8c_hist_row(int hrow)
  { return const_cast<Utf8Char*>(const_cast<const Fl_Terminal*>(this)->u8c_hist_row(hrow)); }

/**
  Return u8c for beginning of row \p hurow inside the 'in use' part
  of the scrollback history.

  'hurow' is indexed relative to the beginning of the 'in use' part
  of the scrollback history buffer. This may be a different from
  u8c_hist_row(int) if the history was recently cleared, and there
  aren't many (or any) rows in the history buffer that have been
  populated with scrollback text yet.

  \see u8c_disp_row(int) for example use.
*/
Fl_Terminal::Utf8Char* Fl_Terminal::u8c_hist_use_row(int hurow)
  { return const_cast<Utf8Char*>(const_cast<const Fl_Terminal*>(this)->u8c_hist_use_row(hurow)); }

/**
  Return pointer to the first u8c character in row \p drow of the display.
  - 'drow' is indexed relative to the beginning of the display buffer.
  - This can be used to walk all columns in the specfied row, e.g.
    \code
    // Print all chars in first row of display (ASCII and UTF-8)
    Utf8Char *u8c = u8c_disp_row(0);            // first char of first display row
    int scol = 0, ecol = disp_cols();           // start/end for column loop
    for (int col=scol; col<ecol; col++,u8c++) { // loop from first char to last
        char *text = u8c->text_utf8();          // text string for char
        int len    = u8c->length();             // text string length for char
        ::printf("<%.*s>", len, text);          // print potentially multibyte char
    }
    \endcode
*/
Fl_Terminal::Utf8Char* Fl_Terminal::u8c_disp_row(int drow)
  { return const_cast<Utf8Char*>(const_cast<const Fl_Terminal*>(this)->u8c_disp_row(drow)); }

// Create ring buffer.
// Input:
//    drows -- display height in lines of text (rows)
//    dcols -- display width in characters (columns)
//    hrows -- scrollback history size in lines of text (rows)
//
// NOTE: Caller should call update_screen() at some point
//       to fix the scrollbar and other things.
//
void Fl_Terminal::create_ring(int drows, int dcols, int hrows) {
  // recreate tabstops if col width being changed
  if (dcols != ring_.ring_cols()) init_tabstops(dcols);
  // recreate ring (dumps old)
  ring_.create(drows, dcols, hrows);
  // ensure cursor starts at home position
  cursor_.home();
}

/// Return the Utf8Char* for character under cursor.
Fl_Terminal::Utf8Char* Fl_Terminal::u8c_cursor(void) {
  return u8c_disp_row(cursor_.row()) + cursor_.col();
}

// Initialize tabstops for terminal
//    NOTE: 'newsize' should always be at least 'ring_cols()'..
//
void Fl_Terminal::init_tabstops(int newsize) {
  if (newsize > tabstops_size_) {                 // enlarge?
    char *oldstops = tabstops_;                   // save old stops
    int   oldsize  = tabstops_size_;              // save old size
    tabstops_ = (char*)malloc(newsize);           // alloc new
    for (int t=0; t<newsize; t++) {               // init new tabstops:
      tabstops_[t] = (oldstops && t<oldsize)
                       ? oldstops[t]              // copy old
                       : ((t % 8) == 0) ? 1 : 0;  // new defaults
    }
    if (oldstops) free((void*)oldstops);          // dump old stops
    tabstops_size_ = newsize;
  } else {
    // Same size or smaller? Do nothing -- just keep old tabstops
  }
}

// Reset all tabstops to default 8th char
void Fl_Terminal::default_tabstops(void) {
  init_tabstops(ring_cols());                // issue #882
  for (int t=1; t<tabstops_size_; t++)       // t=1: skip 0
    tabstops_[t] = ((t % 8) == 0) ? 1 : 0;   // every 8th char is a tabstop
}

// Clear all tabstops
void Fl_Terminal::clear_all_tabstops(void) {
  memset(tabstops_, 0, tabstops_size_);
}

// Set/clear tabstop at current cursor x position
//    val: 0 clears tabstop, 1 sets tabstop
//
void Fl_Terminal::set_tabstop(void) {
  int index = clamp(cursor_col(), 0, tabstops_size_-1);    // clamp cursor pos
  tabstops_[index] = 1;                                    // set/clr tabstop
}

// Clear tabstop at current cursor x position
void Fl_Terminal::clear_tabstop(void) {
  int index = clamp(cursor_col(), 0, tabstops_size_-1);    // clamp cursor pos
  tabstops_[index] = 0;                                    // clear tabstop
}

// Apply settings to scrollbar appropriate for h/v scrolling.
//    After being set, value() will return the diff between min..max.
//    Note: For vert scroll, min height of tab should be scrollbar's width,
//          but not smaller than 10 pixels, so user can grab it easily. (**)
//
void Fl_Terminal::set_scrollbar_params(Fl_Scrollbar* scroll, // scrollbar to set
                                       int min,              // min, e.g. display_rows()
                                       int max) {            // max, e.g. display_rows()+history_use()
  bool  is_hor  = (scroll->type() == FL_HORIZONTAL);
  int   diff    = max - min;
  int   length  = is_hor ? scroll->w() : scroll->h();        // long side of scrollbar in pixels
  float tabsize = min / float(max);                          // fractional size of tab
  float minpix  = float(MAX(10, scrollbar_actual_size()));   // scrollbar_size preferred, 10pix min (**)
  float minfrac = minpix / length;                           // slide_size wants a fraction
  tabsize       = MAX(minfrac, tabsize);                     // use best fractional size
  scroll->slider_size(tabsize);                              // size of slider's tab
  if (is_hor) scroll->range(0, diff);                        // range of values hscroll returns (0=left)
  else        scroll->range(diff, 0);                        // ditto, but for vscroll 0=BOTTOM
  scroll->step(0.25);                                        // Fl_Slider's resolution: 4 : 1
}

// Update both scrollbars based on screen, history buffer, etc
//    Vertical scrollbar should range from 0 (bottom) to history_use() (top),
//    the scrollbar's value being "how many lines we're scrolled back"
//    into the screen history.
//    Horizontal scrollbar should range from 0 (left) to the number of columns
//    that are offscreen.
//
void Fl_Terminal::update_scrollbar(void) {
  //
  // Vertical scrollbar
  //
  int value_before = scrollbar->value();
  {
    // Set vert scrollbar params
    int trows = disp_rows() + history_use();   // total rows we can scroll to
    int vrows = disp_rows();                   // visible rows
    set_scrollbar_params(scrollbar, vrows, trows);
  }
  if (value_before == 0) scrollbar->value(0);  // was at bottom? stay at bottom
  // Ensure scrollbar in proper position
  update_screen_xywh();                        // ensure scrn_ up to date first
  int sx = scrn_.r() + margin_.right();
  int sy = scrn_.y() - margin_.top();
  int sw = scrollbar_actual_size();
  int sh = scrn_.h() + margin_.top() + margin_.bottom();
  bool vchanged = scrollbar->x() != sx ||
                  scrollbar->y() != sy ||
                  scrollbar->w() != sw ||
                  scrollbar->h() != sh;
  if (vchanged) scrollbar->resize(sx, sy, sw, sh);

  //
  // Horizontal scrollbar
  //
  int hh;
  int hx = scrn_.x() - margin_.left();
  int hy = scrn_.b() + margin_.bottom();
  int hw = scrn_.w() + margin_.left() + margin_.right();
  unsigned int hv = hscrollbar->visible();
  // Set horiz scrollbar params
  int vcols = w_to_col(scrn_.w());    // visible cols
  int tcols = disp_cols();            // total cols we can scroll to
  if (vcols > tcols) vcols = tcols;   // don't be larger than total
  set_scrollbar_params(hscrollbar, vcols, tcols);
  // Horiz scrollbar visibility
  if (hscrollbar_style_ == SCROLLBAR_OFF) {
    hscrollbar->hide();
    hh = 0;
  } else if (vcols < tcols || hscrollbar_style_ == SCROLLBAR_ON) {
    hscrollbar->show();
    hh = scrollbar_actual_size();
  } else {
    hscrollbar->hide();
    hh = 0;
  }
  // Update system as necessary
  bool hchanged = hscrollbar->x() != hx ||
                  hscrollbar->y() != hy ||
                  hscrollbar->w() != hw ||
                  hscrollbar->h() != hh ||
                  hscrollbar->visible() != hv;
  if (hchanged) hscrollbar->resize(hx, hy, hw, hh);
  if (vchanged || hchanged) {
    init_sizes();         // tell Fl_Group child changed size..
    update_screen_xywh(); // ensure scrn_ is aware of sw change
    display_modified();   // redraw Fl_Terminal since scroller changed size
  }
  scrollbar->redraw();     // redraw scroll always
}

// Refit the display - (display_rows()/cols()) to match screen (scrn_.h()/w())
//   This implements an xterm-like resizing behavior. Refer to README-Fl_Terminal.txt,
//   section "TERMINAL RESIZING" for a diagram of cases showing how this is implemented.
//   See also issue #844.
//
void Fl_Terminal::refit_disp_to_screen(void) {
  // TODO: Needs to account for change in width too - implement dcol_diff
  int dh         = h_to_row(scrn_.h());         // disp height: in rows for tty pixel height
  int dw         = MAX(w_to_col(scrn_.w()), disp_cols()); // disp width: in cols from pixel width - enlarge only!
  int drows      = clamp(dh, 2,  dh);           // disp rows: 2 rows minimum
  int dcols      = clamp(dw, 10, dw);           // disp cols: 10 cols minimum
  int drow_diff  = drows - display_rows();      // disp row diff: change in rows?
  int is_enlarge = drows >= display_rows();     // enlarging display size?

  // NOTE: Zeroing scrollbar can be avoided if we took the scroll position
  //       into account for the below calculations. But for now..
  scrollbar->value(0);                          // force scrollbar to bottom before refit

  if (drow_diff) {                              // change in display rows means shrink|enlarge
    if (is_enlarge) {                           // enlarging widget?
      for (int i=0; i<drow_diff; i++) {         // carefully loop thru each change
        if (history_use() > 0) {                // CASE 1: Drag lines down from history
          cursor_.scroll(-1);                   // cursor chases ring_.resize()
        } else {                                // CASE 2: add blank lines below cursor
          scroll(1);                            // scroll up to create blank lines at bottom
        }
        // Handle enlarging ring's display
        ring_.resize(display_rows()+1, dcols, hist_rows(), *current_style_);
      }
    } else {                                    // shrinking widget?
      for (int i=0; i<(-drow_diff); i++) {      // carefully loop thru each row change
        int cur_row   = cursor_.row();          // cursor row
        int below_cur = (drows > cur_row);      // shrink is below cursor row?
        if (below_cur) {                        // CASE 3: shrinking below cursor? drop lines below
          ring_.disp_rows(display_rows() - 1);  // effectively "deletes" lines below cursor
        } else {                                // CASE 4: need to move cursor + lines up into hist
          cursor_up(-1, false);                 // move cursor down to follow ring_.resize()
          // Handle shrinking ring's display up into history
          ring_.resize(display_rows()-1, dcols, hist_rows(), *current_style_);
        }
      }
    }
  }
  clear_mouse_selection();
  update_screen(false);
}

// Resize the display's vertical size to (drows).
//   When enlarging / shrinking, KEEP BOTTOM OF DISPLAY THE SAME, e.g.
//
//            Display                Display
//            BEFORE:                SMALLER:
//            ___________            ___________
//           | Hist -3   | ʌ        | Hist -3   | ʌ
//           | Hist -2   | |- 3     | Hist -2   | |
//           | Hist -1   | v        | Hist -1   | |-- new hist size
//           |-----------| ---┐     | Line 1    | |
//           | Line 1    |    |     | Line 2    | v
//           | Line 2    |    └---> |-----------|
//           | Line 3    |          | Line 3    | ʌ
//           | :         |          | :         | |-- new disp size
//           | Line 25   | -------> | Line 25   | v
//            -----------            -----------
//
//            Display                 Display
//            BEFORE:                 LARGER
//            ___________             ___________
//           | Hist -3   | ʌ         | Hist -3   | --- new hist size
//           | Hist -2   | |- 3  ┌-->|-----------|
//           | Hist -1   | v     |   | Hist -2   | ʌ
//           |-----------| ------┘   | Hist -1   | |
//           | Line 1    |           | Line 1    | |
//           | Line 2    |           | Line 2    | |-- new disp size
//           | Line 3    |           | Line 3    | |
//           | :         |           | :         | |
//           | Line 25   | --------> | Line 25   | v
//            -----------             -----------
//
//
void Fl_Terminal::resize_display_rows(int drows) {
  int drow_diff = drows - ring_.disp_rows(); // Change in rows?
  if (drow_diff == 0) return;                // No changes? early exit
  int new_dcols = ring_cols();               // keep cols the same
  int new_hrows = hist_rows() - drow_diff;   // keep disp:hist ratio same
  if (new_hrows<0) new_hrows = 0;            // don't let hist be <0
  ring_.resize(drows, new_dcols, new_hrows, *current_style_);
  // ..update cursor/selections to track text position
  cursor_.scroll(-drow_diff);
  select_.clear();                  // clear any mouse selection
  // ..update scrollbar, since disp_height relative to hist_use changed
  update_scrollbar();
}

// Resize the display's columns
//    This affects the history and entire ring buffer too.
//    Make an effort to preserve previous content.
//    Up to caller to enforce any 'minimum' size for drows.
//
void Fl_Terminal::resize_display_columns(int dcols) {
  // No changes? early exit
  if (dcols == disp_cols()) return;
  // Change cols, preserves previous content if possible
  ring_.resize(disp_rows(), dcols, hist_rows(), *current_style_);
  update_scrollbar();
}

// Update only the internal terminal screen xywh and x2/y2 values
void Fl_Terminal::update_screen_xywh(void) {
  const Margin &m = margin_;
  scrn_ = *this;                                         // start with widget's current xywh
  scrn_.inset(box());                                    // apply box offset
  scrn_.inset(m.left(), m.top(), m.right(), m.bottom()); // apply margins offset
  scrn_.inset(0, 0, scrollbar_actual_size(), 0);         // apply scrollbar width
  if (hscrollbar && hscrollbar->visible())
    scrn_.inset(0, 0, 0, scrollbar_actual_size());       // apply hscrollbar height
}

// Update internals when something "global" changes
//    Call this when something important is changed:
//    Resizing screen or changing font/size affect internals globally.
//    Font change affects per-character caching of char widths.
//    Display resize affects scrn_ cache, scrollbars, etc.
//
void Fl_Terminal::update_screen(bool font_changed) {
  // current_style: update cursor's size for current font/size
  if (font_changed) {
    // Change font and current_style's font height
    if (!fontsize_defer_) { // issue 837
      //DEBUG fprintf(stderr, "update_screen(font change): row/cols=%d/%d, cs.width=%d, cs.height=%d\n", display_rows(), display_columns(), current_style_->charwidth(), current_style_->fontheight());
      fl_font(current_style_->fontface(), current_style_->fontsize());
    }
    cursor_.h(current_style_->fontheight());
  }
  // Update the scrn_* values
  update_screen_xywh();
  // Recalc the scrollbar size/position/etc
  update_scrollbar();
}

/**
  Return terminal's scrollback history buffer size in lines of text (rows).
*/
int Fl_Terminal::history_rows(void) const {
  return hist_rows();
}

/**
  Set terminal's scrollback history buffer size in lines of text (rows).
*/
void Fl_Terminal::history_rows(int hrows) {
  if (hrows == history_rows()) return;        // no change? done
  ring_.resize(disp_rows(), disp_cols(), hrows, *current_style_);
  update_screen(false);                       // false: no font change
  display_modified();
}

/**
  Returns how many lines are "in use" by the screen history buffer.

  This value will be 0 if history was recently cleared with e.g.
  clear_history() or \c "<ESC>c".

  Return value will be in the range 0 .. (history_lines()-1).
*/
int Fl_Terminal::history_use(void) const {
  return ring_.hist_use();
}

/**
  Return terminal's display height in lines of text (rows).

  This value is normally managed automatically by resize()
  based on the current font size.
*/
int Fl_Terminal::display_rows(void) const {
  return ring_.disp_rows();
}

/**
  Set terminal's display height in lines of text (rows).

  This value is normally managed automatically by resize()
  based on the current font size, and should not be changed.

  To change the display height, use resize() instead.
*/
void Fl_Terminal::display_rows(int drows) {
  if (drows == disp_rows()) return;           // no change? early exit
  ring_.resize(drows, disp_cols(), hist_rows(), *current_style_);
  update_screen(false);                       // false: no font change ?NEED?
  refit_disp_to_screen();
}

/**
  Return terminal's display width in columns of text characters.

  This value is normally managed automatically by resize()
  based on the current font size.
*/
int Fl_Terminal::display_columns(void) const {
  return ring_.disp_cols();
}

/**
  Set terminal's display width in columns of text characters.

  This value is normally managed automatically by resize()
  based on the current font size, and should not be changed.

  You CAN make the display_columns() larger than the width of
  the widget; text in the terminal will simply run off the
  screen edge and be clipped; the only way to reveal that
  text is if the user enlarges the widget, or the font size
  made smaller.

  To change the display width, it is best to use resize() instead.
*/
void Fl_Terminal::display_columns(int dcols) {
  if (dcols == disp_cols()) return;           // no change? early exit
  // Change cols, preserves previous content if possible
  ring_.resize(disp_rows(), dcols, hist_rows(), *current_style_);
  update_screen(false);                       // false: no font change ?NEED?
  refit_disp_to_screen();
}

/** Return reference to internal current style for rendering text. */
Fl_Terminal::CharStyle& Fl_Terminal::current_style(void) const {
  return *current_style_;
}

/** Set current style for rendering text. */
void Fl_Terminal::current_style(const CharStyle& sty) {
  *current_style_ = sty;
}

/**
  Set the left margin; see \ref Fl_Terminal_Margins.
*/
void Fl_Terminal::margin_left(int val) {
  val = clamp(val,0,w()-1);
  margin_.left(val);
  update_screen(true);
  refit_disp_to_screen();
}

/**
  Set the right margin; see \ref Fl_Terminal_Margins.
*/
void Fl_Terminal::margin_right(int val) {
  val = clamp(val,0,w()-1);
  margin_.right(val);
  update_screen(true);
  refit_disp_to_screen();
}

/**
  Set the top margin; see \ref Fl_Terminal_Margins.
*/
void Fl_Terminal::margin_top(int val) {
  val = clamp(val,0,h()-1);
  margin_.top(val);
  update_screen(true);
  refit_disp_to_screen();
}

/**
  Set the bottom margin; see \ref Fl_Terminal_Margins.
*/
void Fl_Terminal::margin_bottom(int val) {
  val = clamp(val,0,h()-1);
  margin_.bottom(val);
  update_screen(true);
  refit_disp_to_screen();
}

/**
  Sets the font used for all text displayed in the terminal.

  This affects all existing text (in display and history) as well
  as any newly printed text.

  Only monospace fonts are recommended, such as FL_COURIER or FL_SCREEN.
  Custom fonts configured with Fl::set_font() will also work, as long
  as they are monospace.
*/
void Fl_Terminal::textfont(Fl_Font val) {
  current_style_->fontface(val);
  update_screen(true);
  display_modified();
}

/**
  Sets the font size used for all text displayed in the terminal.

  This affects all existing text (in display and history) as well
  as any newly printed text.

  Changing this will affect the display_rows() and display_columns().
*/
void Fl_Terminal::textsize(Fl_Fontsize val) {
  current_style_->fontsize(val);
  update_screen(true);
  // Changing font size affects #lines in display, so resize it
  refit_disp_to_screen();
  display_modified();
}

/**
  Sets the foreground text color as one of the 8 'xterm color' values.

  This will be the foreground color used for all newly printed text,
  similar to the \c \<ESC\>[\#m escape sequence, where \# is between 30 and 37.

  This color will be reset to the default fg color if reset_terminal()
  is called, or by \c \<ESC\>c, \c \<ESC\>[0m, etc.

  The xterm color intensity values can be influenced by the Dim/Bold/Normal
  modes (which can be set with e.g. \c \<ESC\>[1m, textattrib(), etc), so the
  actual RGB values of these colors allow room for Dim/Bold to influence their
  brightness. For instance, "Normal Red" is not full brightness to allow
  "Bold Red" to be brighter. This goes for all colors except 'Black', which
  is not influenced by Dim or Bold; Black is always Black.

  The 8 color xterm values are:
  - 0 = Black
  - 1 = Red
  - 2 = Green
  - 3 = Yellow
  - 4 = Blue
  - 5 = Magenta
  - 6 = Cyan
  - 7 = White

  \see textfgcolor_default(Fl_Color)
*/
void Fl_Terminal::textfgcolor_xterm(uchar val) {
  current_style_->fgcolor_xterm(val);
}

/**
  Sets the background text color as one of the 8 'xterm color' values.

  This will be the foreground color used for all newly printed text,
  similar to the \c \<ESC\>[\#m escape sequence, where \# is between 40 and 47.

  This color will be reset to the default bg color if reset_terminal()
  is called, or by \c \<ESC\>c, \c \<ESC\>[0m, etc.

  The xterm color intensity values can be influenced by the Dim/Bold/Normal
  modes (which can be set with e.g. \c \<ESC\>[1m, textattrib(), etc), so the
  actual RGB values of these colors allow room for Dim/Bold to influence their
  brightness. For instance, "Normal Red" is not full brightness to allow
  "Bold Red" to be brighter. This goes for all colors except 'Black', which
  is not influenced by Dim or Bold; Black is always Black.

  The 8 color xterm values are:
  - 0 = Black
  - 1 = Red
  - 2 = Green
  - 3 = Yellow
  - 4 = Blue
  - 5 = Magenta
  - 6 = Cyan
  - 7 = White

  \see textbgcolor_default(Fl_Color)
*/
void Fl_Terminal::textbgcolor_xterm(uchar val) {
  current_style_->bgcolor_xterm(val);
}

/**
  Set the text color for the terminal.

  This is a convenience method that sets *both* textfgcolor() and textfgcolor_default(),
  ensuring both are set to the same value.

  Colors set this way will NOT be influenced by the xterm Dim/Bold color intensity attributes.
  For that, use textcolor_xterm() instead.

  \see textfgcolor(Fl_Color), textfgcolor_default(Fl_Color), textbgcolor_xterm(uchar)
*/
void Fl_Terminal::textcolor(Fl_Color val) {
  textfgcolor(val);
  textfgcolor_default(val);
}

/**
  Sets the background color for the terminal's Fl_Group::box().

  If the textbgcolor() and textbgcolor_default() are set to the special
  "see through" color 0xffffffff when any text was added, changing color()
  affects the color that shows through behind that existing text.

  Otherwise, whatever specific background color was set for existing text
  will persist after changing color().

  To see the effects of a change to color(), follow up with a call to redraw().

  The default value is 0x0.
*/
void Fl_Terminal::color(Fl_Color val) {
  Fl_Group::color(val);
}

/**
  Set text foreground drawing color to fltk color \p val used by any new text added.
  Use this for temporary color changes, similar to \<ESC\>[38;2;\<R\>;\<G\>;\<B\>m

  Colors set this way will NOT be influenced by the xterm Dim/Bold color intensity
  attributes.  For that, use textfgcolor_xterm(uchar) instead.

  This setting does _not_ affect the 'default' text colors used by \<ESC\>[0m,
  \<ESC\>c, reset_terminal(), etc. To change both the current _and_
  default fg color, also use textfgcolor_default(Fl_Color). Example:
  \par
  \code
     // Set both 'current' and 'default' colors
     Fl_Color amber = 0xd0704000;
     tty->textfgcolor(amber);         // set 'current' fg color
     tty->textfgcolor_default(amber); // set 'default' fg color used by ESC[0m reset
  \endcode
  \see textfgcolor_default(Fl_Color), textfgcolor_xterm(uchar)
*/
void Fl_Terminal::textfgcolor(Fl_Color val) {
  current_style_->fgcolor(val);         // also clears FG_XTERM charflag
}

/**
  Set text background color to fltk color \p val used by any new text added.
  Use this for temporary color changes, similar to \<ESC\>[48;2;\<R\>;\<G\>;\<B\>m

  Colors set this way will NOT be influenced by the xterm Dim/Bold color intensity
  attributes.  For that, use textbgcolor_xterm(uchar) instead.

  This setting does _not_ affect the 'default' text colors used by \<ESC\>[0m,
  \<ESC\>c, reset_terminal(), etc. To set that too, also set textbgcolor_default(Fl_Color), e.g.
  \par
  \code
     // Set both 'current' and 'default' colors
     Fl_Color darkamber = 0x20100000;
     tty->textbgcolor(darkamber);         // set 'current' bg color
     tty->textbgcolor_default(darkamber); // set 'default' bg color used by ESC[0m reset
  \endcode

  The special color value 0xffffffff (all ff's) is the "see through" color, which lets
  the widget's own Fl_Group::color() show through behind the text. This special text background
  color is the _default_, and is what most situations need.

  \see textbgcolor_default(Fl_Color), textbgcolor_xterm(uchar)
*/
void Fl_Terminal::textbgcolor(Fl_Color val) {
  current_style_->bgcolor(val);         // also clears BG_XTERM charflag
}

/**
  Set the default text foreground color used by \c \<ESC\>c, \c \<ESC\>[0m,
  and reset_terminal().

  Does not affect the 'current' text foreground color; use textfgcolor(Fl_Color) to set that.

  \see textfgcolor(Fl_Color)
*/
void Fl_Terminal::textfgcolor_default(Fl_Color val) {
  current_style_->defaultfgcolor(val);
}

/**
  Set the default text background color used by any new text added
  after a reset (\c \<ESC\>c, \c \<ESC\>[0m, or reset_terminal()).

  Does not affect the 'current' text background color; use textbgcolor(Fl_Color) to set that.

  The special color value 0xffffffff (all ff's) is the "see through" color, which lets
  the widget's own Fl_Group::color() show through behind the text. This special text background
  color is the _default_, and is what most situations need.

  \see textbgcolor(Fl_Color)
*/
void Fl_Terminal::textbgcolor_default(Fl_Color val) {
  current_style_->defaultbgcolor(val);
}

/**
  Set text attribute bits (underline, inverse, etc).
  This will be the default attribute used for all newly printed text.

 \see Fl_Terminal::Attrib
*/
void Fl_Terminal::textattrib(uchar val) {
  current_style_->attrib(val);
}

/**
  Get text attribute bits (underline, inverse, etc).
  This is the default attribute used for all newly printed text.

 \see textattrib(uchar), Fl_Terminal::Attrib
*/
uchar Fl_Terminal::textattrib() const {
  return current_style_->attrib();
}

/**
  Convert fltk window X coord to column 'gcol' on specified global 'grow'
  \returns
    - 1 if 'gcol' was found
    - 0 if X not within any char in 'grow'
*/
int Fl_Terminal::x_to_glob_col(int X, int grow, int &gcol, bool &gcr) const {
  int cx = scrn_.x();                               // leftmost char x position
  const Utf8Char *u8c = utf8_char_at_glob(grow, 0);
  for (gcol=0; gcol<ring_cols(); gcol++,u8c++) {    // walk the cols looking for X
    u8c->fl_font_set(*current_style_);              // pwidth_int() needs fl_font set
    int cx2 = cx + u8c->pwidth_int();               // char x2 (right edge of char)
    if (X >= cx && X < cx2) {
      gcr = (X > ((cx+cx2)/2));                     // X is in right half of character
      return 1;                                     // found? return with gcol and gcr set
    }
    cx += u8c->pwidth_int();                        // move cx to start x of next char
  }
  gcol = ring_cols()-1;                             // don't leave larger than #cols
  return 0;                                         // not found
}

// Convert fltk window X,Y coords to row + column indexing into ring_chars[]
// Returns:
//              1 -- found row,col
//              0 -- not found, outside display's character area
//    -1/-2/-3/-4 -- not found, off top/bot/lt/rt edge respectively
//
int Fl_Terminal::xy_to_glob_rowcol(int X, int Y, int &grow, int &gcol, bool &gcr) const {
  // X,Y outside terminal area? early exit
  if (Y<scrn_.y()) return -1;                       // up (off top edge)
  if (Y>scrn_.b()) return -2;                       // dn (off bot edge)
  if (X<scrn_.x()) return -3;                       // lt (off left edge)
  if (X>scrn_.r()) return -4;                       // rt (off right edge)
  // Find toprow of what's currently drawn on screen
  int toprow = disp_srow() - scrollbar->value();
  // Find row the 'Y' value is in
  grow = toprow + ( (Y-scrn_.y()) / current_style_->fontheight());
  return x_to_glob_col(X, grow, gcol, gcr);
}

/**
  Clears the screen to the current textbgcolor(), and homes the cursor.
  \see clear_screen(), clear_screen_home(), cursor_home()
*/
void Fl_Terminal::clear(void) {
  clear_screen_home();
}

/**
  Clears the screen to a specific color \p val and homes the cursor.
  \see clear_screen(), clear_screen_home(), cursor_home()
*/
void Fl_Terminal::clear(Fl_Color val) {
  Fl_Color save = textbgcolor();
  textbgcolor(val);
  clear_screen_home();
  textbgcolor(save);
}

/**
  Clear the terminal screen only; does not affect the cursor position.

  Also clears the current mouse selection.

  If \p 'scroll_to_hist' is true, the screen is cleared by scrolling the
  contents into the scrollback history, where it can be retrieved with the
  scrollbar. This is the default behavior. If false, the screen is cleared
  and the scrollback history is unchanged.

  Similar to the escape sequence \c "<ESC>[2J".

  \see clear_screen_home()
*/
void Fl_Terminal::clear_screen(bool scroll_to_hist) {
  if (scroll_to_hist) { scroll(disp_rows()); return; }
  for (int drow=0; drow<disp_rows(); drow++)
    for (int dcol=0; dcol<disp_cols(); dcol++)
      clear_char_at_disp(drow, dcol);
  clear_mouse_selection();
}

/**
  Clear the terminal screen and home the cursor.

  Also clears the current mouse selection.

  If \p 'scroll_to_hist' is true, the screen is cleared by scrolling the
  contents into the scrollback history, where it can be retrieved with the
  scrollbar. This is the default behavior. If false, the screen is cleared
  and the scrollback history is unchanged.

  Similar to the escape sequence \c "<ESC>[2J<ESC>[H".

  \see clear_screen()
*/
void Fl_Terminal::clear_screen_home(bool scroll_to_hist) {
  cursor_home();
  clear_screen(scroll_to_hist);
}

/// Clear from cursor to Start Of Display (EOD), like \c "<ESC>[1J".
void Fl_Terminal::clear_sod(void) {
  for (int drow=0; drow <= cursor_.row(); drow++)
    if (drow == cursor_.row())
      for (int dcol=0; dcol<=cursor_.col(); dcol++)
        plot_char(' ', drow, dcol);
    else
      for (int dcol=0; dcol<disp_cols(); dcol++)
        plot_char(' ', drow, dcol);
  //TODO: Clear mouse selection?
}

/// Clear from cursor to End Of Display (EOD), like \c "<ESC>[J<ESC>[0J".
void Fl_Terminal::clear_eod(void) {
  for (int drow=cursor_.row(); drow<disp_rows(); drow++)
    if (drow == cursor_.row())
      for (int dcol=cursor_.col(); dcol<disp_cols(); dcol++)
        plot_char(' ', drow, dcol);
    else
      for (int dcol=0; dcol<disp_cols(); dcol++)
        plot_char(' ', drow, dcol);
  //TODO: Clear mouse selection?
}

/// Clear from cursor to End Of Line (EOL), like \c "<ESC>[K".
void Fl_Terminal::clear_eol(void) {
  Utf8Char *u8c = u8c_disp_row(cursor_.row()) + cursor_.col();  // start at cursor
  for (int col=cursor_.col(); col<disp_cols(); col++)           // run from cursor to eol
    (u8c++)->clear(*current_style_);
  //TODO: Clear mouse selection?
}

/// Clear from cursor to Start Of Line (SOL), like \c "<ESC>[1K".
void Fl_Terminal::clear_sol(void) {
  Utf8Char *u8c = u8c_disp_row(cursor_.row());  // start at sol
  for (int col=0; col<=cursor_.col(); col++)    // run from sol to cursor
    (u8c++)->clear(*current_style_);
  //TODO: Clear mouse selection?
}

/// Clear entire line for specified row.
void Fl_Terminal::clear_line(int drow) {
  Utf8Char *u8c = u8c_disp_row(drow);           // start at sol
  for (int col=0; col<disp_cols(); col++)       // run to eol
    (u8c++)->clear(*current_style_);
  //TODO: Clear mouse selection?
}

/// Clear entire line cursor is currently on.
void Fl_Terminal::clear_line(void) {
  clear_line(cursor_.row());
}

/// Returns true if there's a mouse selection.
bool Fl_Terminal::is_selection(void) const {
  return select_.is_selection();
}

/**
  Walk the mouse selection one character at a time from beginning to end,
  returning a Utf8Char* to the next character in the selection, or NULL
  if the end was reached, or if there's no selection.

  This is easier to use for walking the selection than get_selection().

  \p u8c should start out as NULL, rewinding to the beginning of the selection.
  If the returned Utf8Char* is not NULL, \p row and \p col return the
  character's row/column position in the ring buffer.
  \par
  \code
  // EXAMPLE: Walk the entire mouse selection, if any
  int row,col;                                    // the returned row/col for each char
  Utf8Char *u8c = NULL;                           // start with NULL to begin walk
  while ((u8c = walk_selection(u8c, row, col))) { // loop until end char reached
    ..do something with *u8c..
  }
  \endcode

  \see get_selection(), is_selection()
*/
const Fl_Terminal::Utf8Char* Fl_Terminal::walk_selection(
    const Utf8Char *u8c, ///< NULL on first iter
    int &row,            ///< returned row#
    int &col             ///< returned col#
    ) const {
  if (u8c==NULL) {
    int erow,ecol;      // unused
    if (!get_selection(row,col,erow,ecol)) return NULL; // no selection
    u8c = u8c_ring_row(row);
  } else {
    int srow,scol,erow,ecol;
    if (!get_selection(srow,scol,erow,ecol)) return NULL; // no selection
    // At end? done
    if (row == erow && col == ecol) return NULL;
    if (++col >= ring_cols())   // advance to next char
      { col = 0; ++row; }       // wrapped to next row?
  }
  return u8c_ring_row(row) + col;
}

/**
  Return mouse selection's start/end position in the ring buffer, if any.

  Ensures (start < end) to allow walking 'forward' thru selection,
  left-to-right, top-to-bottom. The row/col values are indexes into
  the entire ring buffer.

  Example: walk the characters of the mouse selection:

  \par
  \code
  // Get selection
  int srow,scol,erow,ecol;
  if (get_selection(srow,scol,erow,ecol)) {                // mouse selection exists?
    // Walk entire selection from start to end
    for (int row=srow; row<=erow; row++) {                 // walk rows of selection
      const Utf8Char *u8c = u8c_ring_row(row);             // ptr to first character in row
      int col_start = (row==srow) ? scol : 0;              // start row? start at scol
      int col_end   = (row==erow) ? ecol : ring_cols();    // end row?   end at ecol
      u8c += col_start;                                    // include col offset (if any)
      for (int col=col_start; col<=col_end; col++,u8c++) { // walk columns
        ..do something with each char at *u8c..
      }
    }
  }
  \endcode

  Returns:
  - true  -- valid selection values returned
  - false -- no selection was made, returned values undefined

  \see walk_selection(), is_selection()
*/
bool Fl_Terminal::get_selection(int &srow,  ///< starting row for selection
                                int &scol,  ///< starting column for selection
                                int &erow,  ///< ending row for selection
                                int &ecol   ///< ending column for selection
                               ) const {
  return select_.get_selection(srow, scol, erow, ecol);
}

/**
  Is global row/column inside the current mouse selection?

  \returns
  - true  -- (\p grow, \p gcol) is inside a valid selection.
  - false -- (\p grow, \p gcol) is outside, or no valid selection.
*/
bool Fl_Terminal::is_inside_selection(int grow, int gcol) const {
  if (!is_selection()) return false;
  int ncols = ring_cols();
  // Calculate row/col magnitudes to simplify test
  int check = (grow * ncols) + gcol;
  int start = (select_.srow() * ncols) + select_.scol();
  int end   = (select_.erow() * ncols) + select_.ecol();
  if (start > end) swap(start, end);        // ensure (start < end)
  return (check >= start && check <= end);
}

// See if global row (grow) is inside the 'display' area
//
//    No wrap case:                          Wrap case:
//                  ______________                      ______________
//  ring_srow() -> |              |     ring_srow() -> | [C]          |
//                 |              |           :        |              |
//                 |              |           :        |    Display   | ...
//                 |   History    |           :        |              |   :
//                 |              |     disp_erow() -> | [C]          |   :
//                 |              |                    |--------------|   :
//                 |______________|                    |              |   :  Display
//  disp_srow() -> | [A]          |                    |    History   |   :  straddles
//        :        |              |                    |              |   :  end of ring
//        :        |   Display    |                    |--------------|   :
//        :        |              |     disp_srow() -> | [D]          | ..:
//        :        |              |           :        |    Display   |
//        :        |              |           :        |              |
//   disp_erow ┬─> | [B]          |     ring_erow() -> | [D]          |
//   ring_erow ┘    --------------                      --------------
//
bool Fl_Terminal::is_disp_ring_row(int grow) const {
  return ring_.is_disp_ring_row(grow);
}

/**
  Return byte length of all UTF-8 chars in selection, or 0 if no selection.
  NOTE: Length includes trailing white on each line.
*/
int Fl_Terminal::selection_text_len(void) const {
  int row,col,len=0;
  const Utf8Char *u8c = NULL;                     // start with NULL to begin walk
  while ((u8c = walk_selection(u8c, row, col)))   // loop until end char reached
    len += u8c->length();
  return len;
}

/**
  Return text selection (for copy()/paste() operations)
  - Returns allocated NULL terminated string for entire selection.
  - Caller must free() this memory when done.
  - Unicode safe.
*/
const char* Fl_Terminal::selection_text(void) const {
  if (!is_selection()) return fl_strdup("");             // no selection? empty string
  // Allocate buff large enough for all UTF-8 chars
  int   clen   = 0;                                      // char length
  int   buflen = selection_text_len();
  char *buf    = (char*)malloc(buflen+1);                // +1 for NULL
  char *bufp   = buf;
  char *nspc   = bufp;                                   // last 'non-space' char
  // Loop from srow,scol .. erow,ecol
  int row,col;
  const Utf8Char *u8c = NULL;                            // start with NULL to begin walk
  while ((u8c = walk_selection(u8c, row, col))) {        // loop until end char reached
    clen = u8c->length();                                // get char length
    memcpy(bufp, u8c->text_utf8(), clen);                // append UTF-8 string to buffer
    // Handle ignoring trailing whitespace
    if (!u8c->is_char(' ')) nspc = bufp + clen;          // save end pos of last non-spc
    bufp += clen;                                        // advance into buffer
    if (col >= (ring_cols()-1)) {                        // eol? handle trailing white
      if (nspc && nspc != bufp) {                        // trailing white space?
        bufp    = nspc;                                  // rewind bufp, and..
        *bufp++ = '\n';                                  // ..append crlf
        nspc    = bufp;                                  // becomes new nspc for nxt row
      }
    }
  }
  *bufp = 0;
  return buf;
}

/// Clear any current mouse selection.
void Fl_Terminal::clear_mouse_selection(void) {
  select_.clear();
}

/**
  Extend selection to FLTK coords X,Y.
  Returns true if extended, false if nothing done (X,Y offscreen)
*/
bool Fl_Terminal::selection_extend(int X,int Y) {
  if (is_selection()) {                  // selection already?
    int grow, gcol;
    bool gcr;
    if (xy_to_glob_rowcol(X, Y, grow, gcol, gcr) > 0) {
      select_.extend(grow, gcol, gcr);        // extend it
      return true;
    } else {
      // TODO: If X,Y outside row/col area and SHIFT down,
      //       extend selection to nearest edge.
    }
  }
  return false;
}

/**
 Select the word around the given row and column.
 */
void Fl_Terminal::select_word(int grow, int gcol) {
  int i, c0, c1;
  int r = grow, c = gcol;
  Utf8Char *row = u8c_ring_row(r);
  int n = ring_cols();
  if (c >= n) return;
  if (row[c].text_utf8()[0]==' ') {
    for (i=c; i>0; i--) if (row[i-1].text_utf8()[0]!=' ') break;
    c0 = i;
    for (i=c; i<n-2; i++) if (row[i+1].text_utf8()[0]!=' ') break;
    c1 = i;
  } else {
    for (i=c; i>0; i--) if (row[i-1].text_utf8()[0]==' ') break;
    c0 = i;
    for (i=c; i<n-2; i++) if (row[i+1].text_utf8()[0]==' ') break;
    c1 = i;
  }
  select_.select(r, c0, r, c1);
}

/**
 Select the entire row.
 */
void Fl_Terminal::select_line(int grow) {
  select_.select(grow, 0, grow, ring_cols()-1);
}

/**
  Scroll the display up(+) or down(-) the specified \p rows.

  - Negative value scrolls "down", clearing top line, and history unaffected.
  - Postive value scrolls "up", clearing bottom line, rotating top line into history.
*/
void Fl_Terminal::scroll(int rows) {
  // Scroll the ring
  ring_.scroll(rows, *current_style_);
  if (rows > 0) update_scrollbar();      // scroll up? changes hist, so scrollbar affected
  else          clear_mouse_selection(); // scroll dn? clear mouse select; it might wrap ring
}

/**
  Insert (count) rows at current cursor position.
  Causes rows below to scroll down, and empty lines created.
  Lines deleted by scroll down are NOT moved into the scroll history.
*/
void Fl_Terminal::insert_rows(int count) {
  int dst_drow = disp_rows()-1;                                   // dst is bottom of display
  int src_drow = clamp((dst_drow-count), 1, (disp_rows()-1));     // src is count lines up from dst
  while (src_drow >= cursor_.row()) {                             // walk srcrow upwards to cursor row
    Utf8Char *src = u8c_disp_row(src_drow--);
    Utf8Char *dst = u8c_disp_row(dst_drow--);
    for (int dcol=0; dcol<disp_cols(); dcol++) *dst++ = *src++;   // move
  }
  // Blank remaining rows upwards to and including cursor line
  while (dst_drow >= cursor_.row()) {                            // walk srcrow to curs line
    Utf8Char *dst = u8c_disp_row(dst_drow--);
    for (int dcol=0; dcol<disp_cols(); dcol++)
      dst++->clear(*current_style_);
  }
  clear_mouse_selection();
}

/**
 Delete (count) rows at cursor position.
 Causes rows to scroll up, and empty lines created at bottom of screen.
 Lines deleted by scroll up are NOT moved into the scroll history.
*/
void Fl_Terminal::delete_rows(int count) {
  int dst_drow = cursor_.row();                                   // dst is cursor row
  int src_drow = clamp((dst_drow+count), 1, (disp_rows()-1));     // src is count rows below cursor
  while (src_drow < disp_rows()) {                                // walk srcrow to EOD
    Utf8Char *src = u8c_disp_row(src_drow++);
    Utf8Char *dst = u8c_disp_row(dst_drow++);
    for (int dcol=0; dcol<disp_cols(); dcol++)
      *dst++ = *src++;                                            // move
  }
  // Blank remaining rows downwards to End Of Display
  while (dst_drow < disp_rows()) {                                // walk srcrow to EOD
    Utf8Char *dst = u8c_disp_row(dst_drow++);
    for (int dcol=0; dcol<disp_cols(); dcol++)
      dst++->clear(*current_style_);
  }
  clear_mouse_selection();
}

// Repeat printing char 'c' for 'rep' times, not to exceed end of line.
void Fl_Terminal::repeat_char(char c, int rep) {
  rep = clamp(rep, 1, disp_cols());
  while ( rep-- > 0 && cursor_.col() < disp_cols() ) print_char(c);
}

/// Insert char 'c' for 'rep' times at display row \p 'drow' and column \p 'dcol'.
void Fl_Terminal::insert_char_eol(char c, int drow, int dcol, int rep) {
  // Walk the row from the eol backwards to the col position
  //     In this example, rep=3:
  //
  //               dcol
  //               v
  // BEFORE:  |a|b|c|d|e|f|g|h|i|j| <- eol (disp_cols()-1)
  //               | | | | |_____
  //    src end -> |_____        | <-- src start
  //                     | | | | |
  //                     v v v v v
  // AFTER:   |a|b|‸|‸|‸|c|d|e|f|g|
  //               |_|_| <-- spaces added last
  //
  rep = clamp(rep, 0, disp_cols());                     // sanity
  if (rep == 0) return;
  const CharStyle &style = *current_style_;
  Utf8Char *src = u8c_disp_row(drow)+disp_cols()-1-rep; // start src at 'g'
  Utf8Char *dst = u8c_disp_row(drow)+disp_cols()-1;     // start dst at 'j'
  for (int col=(disp_cols()-1); col>=dcol; col--) {     // loop col in reverse: eol -> dcol
    if (col >= (dcol+rep)) *dst-- = *src--;             // let assignment do move
    else                   (dst--)->text_ascii(c,style);// assign chars displaced
  }
}

/**
  Insert char 'c' at the current cursor position for 'rep' times.
  Does not wrap; characters at end of line are lost.
*/
void Fl_Terminal::insert_char(char c, int rep) {
  insert_char_eol(c, cursor_.row(), cursor_.col(), rep);
}

/// Delete char(s) at (drow,dcol) for 'rep' times.
void Fl_Terminal::delete_chars(int drow, int dcol, int rep) {
  rep = clamp(rep, 0, disp_cols());                // sanity
  if (rep == 0) return;
  const CharStyle &style = *current_style_;
  Utf8Char *u8c = u8c_disp_row(drow);
  for (int col=dcol; col<disp_cols(); col++)                      // delete left-to-right
    if (col+rep >= disp_cols()) u8c[col].text_ascii(' ', style);  // blanks
    else                        u8c[col] = u8c[col+rep];          // move
}

/// Delete char(s) at cursor position for 'rep' times.
void Fl_Terminal::delete_chars(int rep) {
  delete_chars(cursor_.row(), cursor_.col(), rep);
}

/**
  Clears the scroll history buffer and adjusts scrollbar,
  forcing it to redraw().
*/
void Fl_Terminal::clear_history(void) {
  // Adjust history use
  ring_.clear_hist();
  scrollbar->value(0);   // zero scroll position
  // Clear entire history buffer
  for (int hrow=0; hrow<hist_rows(); hrow++) {
    Utf8Char *u8c = u8c_hist_row(hrow);          // walk history rows..
    for (int hcol=0; hcol<hist_cols(); hcol++) { // ..and history cols
      (u8c++)->clear(*current_style_);
    }
  }
  // Adjust scrollbar (hist_use changed)
  update_scrollbar();
}

/**
  Resets terminal to default colors, clears screen, history and
  mouse selection, homes cursor, resets tabstops. Same as \c "<ESC>c"
*/
void Fl_Terminal::reset_terminal(void) {
  current_style_->sgr_reset();        // reset current style
  clear_screen_home();                // clear screen, home cursor
  clear_history();
  clear_mouse_selection();
  default_tabstops();                 // reset tabstops to default 8 char
}

//DEBUG void Fl_Terminal::RingBuffer::show_ring_info(void) const {
//DEBUG   ::printf("\033[s"); // save cursor
//DEBUG   ::printf("\033[05C -- Ring Index\n");
//DEBUG   ::printf("\033[05C   ring_rows_: %d\n", ring_rows_);
//DEBUG   ::printf("\033[05C   ring_cols_: %d\n", ring_cols_);
//DEBUG   ::printf("\033[05C      offset_: %d\n", offset_);
//DEBUG   ::printf("\033[u"); // restore cursor
//DEBUG   ::printf("\033[30C -- History Index\n");
//DEBUG   ::printf("\033[30C   hist_rows_: %d srow=%d\n", hist_rows(), hist_srow());
//DEBUG   ::printf("\033[30C   hist_cols_: %d erow=%d\n", hist_cols(), hist_erow());
//DEBUG   ::printf("\033[30C    hist_use_: %d\n",         hist_use());
//DEBUG   ::printf("\033[u"); // restore cursor
//DEBUG   ::printf("\033[60C -- Display Index\n");
//DEBUG   ::printf("\033[60C   disp_rows_: %d srow=%d\n", disp_rows(), disp_srow());
//DEBUG   ::printf("\033[60C   disp_cols_: %d erow=%d\n", disp_cols(), disp_erow());
//DEBUG   ::printf("\n\n");
//DEBUG }

//DEBUG // Save specified row from ring buffer 'ring' to FILE*
//DEBUG void Fl_Terminal::write_row(FILE *fp, Utf8Char *u8c, int cols) const {
//DEBUG   cols = (cols != 0) ? cols : ring_cols();
//DEBUG   for (int col=0; col<cols; col++, u8c++) {
//DEBUG     ::fprintf(fp, "%.*s", u8c->length(), u8c->text_utf8());
//DEBUG   }
//DEBUG }

//DEBUG // Show two buffers side-by-side on stdout.
//DEBUG //    Second buffer can be NULL to just show the a buffer.
//DEBUG //
//DEBUG void Fl_Terminal::show_buffers(RingBuffer *a, RingBuffer *b) const {
//DEBUG   int arows = a->ring_rows(), acols = a->ring_cols();
//DEBUG   int brows = b ? b->ring_rows() : 0, bcols = b ? b->ring_cols() : 0;
//DEBUG   int trows = MAX(arows,brows);
//DEBUG   // Show header
//DEBUG   ::printf("\033[H");
//DEBUG   if (a) ::printf("SRC %d x %d huse=%d off=%d", arows,acols, a->hist_use(), a->offset());
//DEBUG   if (b) ::printf(", DST %d x %d huse=%d off=%d", brows, bcols, b->hist_use(), b->offset());
//DEBUG   ::printf("\033[K\n");
//DEBUG   Utf8Char *u8c;
//DEBUG   // Show rows
//DEBUG   for (int row=0; row < trows; row++) {
//DEBUG     // 'A' buffer
//DEBUG     if (row >= arows) {
//DEBUG       ::printf("          %*s   ", acols, "");
//DEBUG     } else {
//DEBUG       u8c = a->ring_chars()+(arows*acols);
//DEBUG       ::printf("%3d/%3d [", row, trows-1); write_row(stdout, u8c, acols); ::printf("]  ");
//DEBUG     }
//DEBUG     if (!b) { ::printf("\033[K\n"); continue; }
//DEBUG     // 'B' buffer
//DEBUG     if (row < brows) {
//DEBUG       u8c = b->ring_chars()+(brows*bcols);
//DEBUG       ::printf("["); write_row(stdout, u8c, bcols); ::printf("]");
//DEBUG     }
//DEBUG     ::printf("\033[K\n");
//DEBUG   }
//DEBUG   ::printf("--- END\033[0J\n");  // clear eos
//DEBUG   ::printf(" HIT ENTER TO CONTINUE: "); getchar();
//DEBUG   fflush(stdout);
//DEBUG }

///////////////////////////////
////// CURSOR MANAGEMENT //////
///////////////////////////////

/** Set the cursor's foreground color used for text under the cursor. */
void Fl_Terminal::cursorfgcolor(Fl_Color val) { cursor_.fgcolor(val); }
/** Set the cursor's background color used for the cursor itself. */
void Fl_Terminal::cursorbgcolor(Fl_Color val) { cursor_.bgcolor(val); }
/** Get the cursor's foreground color used for text under the cursor. */
Fl_Color Fl_Terminal::cursorfgcolor(void) const { return cursor_.fgcolor(); }
/** Get the cursor's background color used for the cursor itself. */
Fl_Color Fl_Terminal::cursorbgcolor(void) const { return cursor_.bgcolor(); }

/**
  Move cursor to the specified row \p row.
  This value is clamped to the range (0..display_rows()-1).
*/
void Fl_Terminal::cursor_row(int row) { cursor_.row( clamp(row,0,disp_rows()-1) ); }
/** Move cursor to the specified column \p col.
    This value is clamped to the range (0..display_columns()-1).
*/
void Fl_Terminal::cursor_col(int col) { cursor_.col( clamp(col,0,disp_cols()-1) ); }
/** Return the cursor's current row position on the screen. */
int  Fl_Terminal::cursor_row(void) const { return cursor_.row(); }
/** Return the cursor's current column position on the screen. */
int  Fl_Terminal::cursor_col(void) const { return cursor_.col(); }

/**
  Moves cursor up \p count lines.
  If cursor hits screen top, it either stops (does not wrap) if \p do_scroll
  is false, or scrolls down if \p do_scroll is true.
*/
void Fl_Terminal::cursor_up(int count, bool do_scroll) {
  count = clamp(count, 1, disp_rows() * 2);    // sanity (max 2 scrns)
  while (count-- > 0) {
    if (cursor_.up() <= 0) {                   // hit screen top?
      cursor_.row(0);                          // clamp cursor to top
      if (do_scroll) scroll(-1);               // scrolling on? scroll down
      else           return;                   // scrolling off? stop at top
    }
  }
}

/**
  Moves cursor down \p count lines.
  If cursor hits screen bottom, it either stops (does not wrap) if \p do_scroll
  is false, or wraps and scrolls up if \p do_scroll is true.
*/
void Fl_Terminal::cursor_down(int count,      ///< Number of lines to move cursor down
                              bool do_scroll  ///< Enable scrolling if set to true
                             ) {
  count = clamp(count, 1, ring_rows());        // sanity
  while (count-- > 0) {
    if (cursor_.down() >= disp_rows()) {       // hit screen bottom?
      cursor_.row(disp_rows() - 1);            // clamp
      if (!do_scroll) break;                   // don't scroll? done
      scroll(1);                               // scroll up 1 row to make room for new line
    }
  }
}

/**
  Moves cursor left \p count columns, and cursor stops (does not wrap)
  if it hits screen edge.
*/
void Fl_Terminal::cursor_left(int count) {
  count = clamp(count, 1, disp_cols());        // sanity
  while (count-- > 0 )
    if (cursor_.left() < 0)                    // hit left edge of screen?
      { cursor_sol(); return; }                // stop, done
}

/**
  Moves cursor right \p count columns. If cursor hits right edge of screen,
  it either stops (does not wrap) if \p do_scroll is false, or wraps and
  scrolls up one line if \p do_scroll is true.
*/
void Fl_Terminal::cursor_right(int count, bool do_scroll) {
  while (count-- > 0) {
    if (cursor_.right() >= disp_cols()) {      // hit right edge?
      if (!do_scroll)                          // no scroll?
        { cursor_eol(); return; }              // stop at EOL, done
      else
        { cursor_crlf(1); }                    // do scroll? crlf
    }
  }
}

/** Move cursor to the home position (top/left). */
void Fl_Terminal::cursor_home(void) { cursor_.col(0); cursor_.row(0); }

/** Move cursor to the last column (at the far right) on the current line. */
void Fl_Terminal::cursor_eol(void) { cursor_.col(disp_cols()-1); }

/** Move cursor to the first column (at the far left) on the current line. */
void Fl_Terminal::cursor_sol(void) { cursor_.col(0); }

/** Move cursor as if a CR (\\r) was received. Same as cursor_sol() */
void Fl_Terminal::cursor_cr(void) { cursor_sol(); }

/** Move cursor as if a CR/LF pair (\\r\\n) was received. */
void Fl_Terminal::cursor_crlf(int count) {
  const bool do_scroll = true;
  count = clamp(count, 1, ring_rows());           // sanity
  cursor_sol();
  cursor_down(count, do_scroll);
}

/// Tab right, do not wrap beyond right edge.
void Fl_Terminal::cursor_tab_right(int count) {
  count = clamp(count, 1, disp_cols());           // sanity
  int X = cursor_.col();
  while (count-- > 0) {
    // Find next tabstop
    while (++X < disp_cols()) {
      if ( (X<tabstops_size_) && tabstops_[X] )   // found?
        { cursor_.col(X); return; }               // move cur, done
    }
  }
  cursor_eol();
}

/// Tab left, do not wrap beyond left edge.
void Fl_Terminal::cursor_tab_left(int count) {
  count = clamp(count, 1, disp_cols());           // sanity
  int X = cursor_.col();
  while ( count-- > 0 )
    while ( --X > 0 )                             // search for tabstop
      if ( (X<tabstops_size_) && tabstops_[X] )   // found?
        { cursor_.col(X); return; }               // move cur, done
  cursor_sol();
}

/// Save current cursor position. Used by ESC [ s
void Fl_Terminal::save_cursor(void) {
  escseq.save_cursor(cursor_.row(), cursor_.col());
}

/// Restore previously saved cursor position, if any. Used by ESC [ u
void Fl_Terminal::restore_cursor(void) {
  int row,col;
  escseq.restore_cursor(row, col);
  if (row != -1 && col != 1)         // restore only if previously saved
    { cursor_.row(row); cursor_.col(col); }
}

//////////////////////
////// PRINTING //////
//////////////////////

// Handle '\r' output based on output translation flags
void Fl_Terminal::handle_cr(void) {
  const bool do_scroll = true;
  if (oflags_ & CR_TO_LF) cursor_down(1, do_scroll);
  else                    cursor_cr();
}

// Handle '\n' output based on output translation flags
void Fl_Terminal::handle_lf(void) {
  const bool do_scroll = true;
       if (oflags_ & LF_TO_CR  ) cursor_cr();
  else if (oflags_ & LF_TO_CRLF) cursor_crlf();
  else                           cursor_down(1, do_scroll);
}

// Handle '\e' escape character.
void Fl_Terminal::handle_esc(void) {
  if (!ansi_)                                  // not in ansi mode?
    { handle_unknown_char(); return; }         //   ..show unknown char, early exit
  if (escseq.esc_mode() == 0x1b)               // already in esc mode?
    { handle_unknown_char(); }                 //   ..show 1st esc as unknown char, parse 2nd
  if (escseq.parse(0x1b) == EscapeSeq::fail)   // parse esc
    { handle_unknown_char(); return; }         //   ..error? show unknown char
}

/**
  Sets the combined output translation flags to \p val.

  \p val can be sensible combinations of the OutFlags bit flags.

  The default is LF_TO_CRLF, so that \\n will generate both carriage-return (CR)
  and line-feed (LF).

  For \\r and \\n to be handled literally, use output_translate(Fl_Terminal::OutFlags::OFF);

  To disable all output translations, use 0 or Fl_Terminal::OutFlags::OFF.
*/
void Fl_Terminal::output_translate(Fl_Terminal::OutFlags val) {
  oflags_ = val;
}

/**
  Return the current combined output translation flags.
*/
Fl_Terminal::OutFlags Fl_Terminal::output_translate(void) const {
  return oflags_;
}

/**
  Handle the special control character 'c'.

  These are control characters that involve special terminal handling, e.g.
  \code
  \r - carriage return - default behavior for \r is CR. See output_translate()
  \n - line feed       - default behavior for \n is CRLF. See output_translate()
  \b - backspace       - cursor_left()
  \t - tab             - cursor_tab_right()
  \e - escape          - starts an ANSI or xterm escape sequence
  \endcode
*/
void Fl_Terminal::handle_ctrl(char c) {
  switch (c) {
    case '\b': cursor_left();              return;  // BS?
    case '\r': handle_cr();                return;  // CR?
    case '\n': handle_lf();                return;  // LF?
    case '\t': cursor_tab_right();         return;  // TAB?
    case 0x1b: handle_esc();               return;  // ESC?
    default:   handle_unknown_char();      return;  // Unknown ctrl char?
  }
}

// Is char printable? (0x20 thru 0x7e)
bool Fl_Terminal::is_printable(char c) {
  return ((c>=0x20) && (c<=0x7e));
}

// Is char a ctrl character? (0x00 thru 0x1f)
bool Fl_Terminal::is_ctrl(char c) {
  return ((c >= 0x00) && (c < 0x20)) ? true : false;
}

// Handle ESC[m sequences.
//    This is different from the others in that the list of vals
//    separated by ;'s can be long, to allow combining multiple mode
//    settings at once, e.g. fg and bg colors, multiple attributes, etc.
//
void Fl_Terminal::handle_SGR(void) {     // ESC[...m?
  // Shortcut varnames..
  EscapeSeq &esc = escseq;
  int tot = esc.total_vals();
  // Handle ESC[m or ESC[;m
  if (tot == 0)
    { current_style_->sgr_reset(); return; }
  // Handle ESC[#;#;#...m
  int rgbcode = 0;                   // 0=none, 38=fg, 48=bg
  int rgbmode = 0;                   // 0=none, 1="2", 2=<r>, 3=<g>, 4=<b>
  int r=0,g=0,b=0;
  for (int i=0; i<tot; i++) {        // expect possibly many values
    int val = esc.val(i);            // each val one at a time
    switch (rgbmode) {
      case 0:
         // RGB mode values?
         switch (val) {
           case 38:                  // fg RGB mode? e.g. ESC[38;2;<R>;<G>;<B>m
           case 48:                  // bg RGB mode? e.g. ESC[48;2;<R>;<G>;<B>m
             rgbmode = 1;
             rgbcode = val;
             continue;
         }
         break;
      case 1: if (val == 2) { rgbmode++; continue; }    // '2'?
              rgbcode = rgbmode = 0;                    // not '2'? cancel
              handle_unknown_char();
              break;
      case 2: r=clamp(val,0,255); ++rgbmode; continue;  // parse red value
      case 3: g=clamp(val,0,255); ++rgbmode; continue;  // parse grn value
      case 4: b=clamp(val,0,255);                       // parse blu value
        switch (rgbcode) {
          case 38: current_style_->fgcolor(r,g,b);      // Set fg rgb
                   break;
          case 48: current_style_->bgcolor(r,g,b);      // Set bg rgb
                   break;
        }
        rgbcode = rgbmode = 0;                          // done w/rgb mode parsing
        continue;                                       // continue loop to parse more vals
    }
    if (val < 10) {                                     // Set attribute? (bold,underline..)
      switch (val) {
        case 0: current_style_->sgr_reset();     break; // ESC[0m - reset
        case 1: current_style_->sgr_bold(1);     break; // ESC[1m - bold
        case 2: current_style_->sgr_dim(1);      break; // ESC[2m - dim
        case 3: current_style_->sgr_italic(1);   break; // ESC[3m - italic
        case 4: current_style_->sgr_underline(1);break; // ESC[4m - underline
        case 5: current_style_->sgr_blink(1);    break; // ESC[5m - blink
        case 6: handle_unknown_char();           break; // ESC[6m - (unused)
        case 7: current_style_->sgr_inverse(1);  break; // ESC[7m - inverse
        case 8: handle_unknown_char();           break; // ESC[8m - (unused)
        case 9: current_style_->sgr_strike(1);   break; // ESC[9m - strikeout
      }
    } else if (val >= 21 && val <= 29) {                // attribute extras
      switch (val) {
        case 21: current_style_->sgr_dbl_under(1);break; // ESC[21m - doubly underline
        case 22: current_style_->sgr_dim(0);             // ESC[22m - disable bold/dim
                 current_style_->sgr_bold(0);     break; //
        case 23: current_style_->sgr_italic(0);   break; // ESC[23m - disable italic
        case 24: current_style_->sgr_underline(0);break; // ESC[24m - disable underline
        case 25: current_style_->sgr_blink(0);    break; // ESC[25m - disable blink
        case 26: handle_unknown_char();           break; // ESC[26m - (unused)
        case 27: current_style_->sgr_inverse(0);  break; // ESC[27m - disable inverse
        case 28: handle_unknown_char();           break; // ESC[28m - disable hidden
        case 29: current_style_->sgr_strike(0);   break; // ESC[29m - disable strikeout
      }
    } else if (val >= 30 && val <= 37) {                 // Set fg color?
      uchar uval = (val - 30);
      current_style_->fgcolor_xterm(uval);
    } else if (val == 39) {                              // ESC[39m -- "normal" fg color:
      Fl_Color fg = current_style_->defaultfgcolor();    // ..get default color
      current_style_->fgcolor_xterm(fg);                 // ..set current color
    } else if (val >= 40 && val <= 47) {                 // Set bg color?
      uchar uval = (val - 40);
      current_style_->bgcolor_xterm(uval);
    } else if (val == 49) {                              // ESC[49m -- "normal" bg color:
      Fl_Color bg = current_style_->defaultbgcolor();    // ..get default bg color
      current_style_->bgcolor_xterm(bg);                 // ..set current bg color
    } else {
      handle_unknown_char();  // does an escseq.reset()  // unimplemented SGR codes
    }
  }
}

/**
  Handle the VT100 sequence <tt>ESC [ top ; lt ; bot ; rt ; att $ t</tt>

  top/lt/bot/rt is the screen area to affect, 'att' is the attrib to xor,
  i.e. 1(bold),4,5,7(inverse).

  \note
    - gnome-term doesn't support this, but xterm does.
    - Currently unsupported by Fl_Terminal
*/
void Fl_Terminal::handle_DECRARA(void) {
  // TODO: MAYBE NEVER
}

/**
  Handle an escape sequence character.

  Call this on a character only if escseq.parse_in_progress() is true.

  If this char is the end of the sequence, do the operation (if possible),
  then does an escseq.reset() to finish parsing.
*/
void Fl_Terminal::handle_escseq(char c) {
  // NOTE: Use xterm to test. gnome-terminal has bugs, even in 2022.
  const bool do_scroll = true;
  const bool no_scroll = false;
  switch (escseq.parse(c)) {                           // parse char, advance s..
    case EscapeSeq::fail:                              // failed?
      escseq.reset();                                  //   ..reset to let error_char be visible
      handle_unknown_char();                           //   ..show error char (if enabled)
      print_char(c);                                   //   ..show char we couldn't handle
      return;                                          //   ..done.
    case EscapeSeq::success:                           // success?
      return;                                          //   ..keep parsing
    case EscapeSeq::completed:                         // parsed complete esc sequence?
      break;                                           //   ..fall through to handle operation
  }
  // Shortcut varnames for escseq parsing..
  EscapeSeq &esc = escseq;
  char mode     = esc.esc_mode();
  int  tot      = esc.total_vals();
  int  val0     = (tot==0) ? 0 : esc.val(0);
  int  val1     = (tot<2)  ? 0 : esc.val(1);
  const int& dw = disp_cols();
  const int& dh = disp_rows();
  if (esc.is_csi()) {                            // Was this a CSI (ESC[..) sequence?
    switch (mode) {
      case '@':                                  // <ESC>[#@ - (ICH) Insert blank Chars (default=1)
        insert_char(' ', esc.defvalmax(1,dw));
        break;
      case 'A':                                  // <ESC>[#A - (CUU) cursor up, no scroll/wrap
        cursor_up(esc.defvalmax(1,dh));
        break;
      case 'B':                                  // <ESC>[#B - (CUD) cursor down, no scroll/wrap
        cursor_down(esc.defvalmax(1,dh), no_scroll);
        break;
      case 'C':                                  // <ESC>[#C - (CUF) cursor right, no wrap
        cursor_right(esc.defvalmax(1,dw), no_scroll);
        break;
      case 'D':                                  // <ESC>[#D - (CUB) cursor left, no wrap
        cursor_left(esc.defvalmax(1,dw));
        break;
      case 'E':                                  // <ESC>[#E - (CNL) cursor next line (crlf) xterm, !gnome
        cursor_crlf(esc.defvalmax(1,dh));
        break;
      case 'F':                                  // <ESC>[#F - (CPL) move to sol and up # lines
        cursor_cr();
        cursor_up(esc.defvalmax(1,dh));
        break;
      case 'G':                                  // <ESC>[#G - (CHA) cursor horizal absolute
        switch (clamp(tot,0,1)) {                //   │
          case 0:                                //   ├── <ESC>[G    -- move to sol
            cursor_sol();                        //   │                 default <ESC>[1G
            break;                               //   │
          case 1:                                //   └── <ESC>[#G   -- move to column
            cursor_col(clamp(val0,1,dw)-1);
            break;
        }
        break;
      case 'H':
cup:
        switch (clamp(tot,0,2)) {                // <ESC>[#H - (CUP) cursor position (#'s are 1 based)
          case 0:                                //   ├── <ESC>[H    -- no vals?
            cursor_home();                       //   │                 default <ESC>[1H
            break;                               //   │
          case 1:                                //   ├── <ESC>[#H   -- go to (row #)
            cursor_row(clamp(val0,1,dh)-1);      //   │                 NOTE: ESC[5H == ESC[5;1H
            cursor_col(0);                       //   │
            break;                               //   │
          case 2:                                //   └── <ESC>[#;#H -- go to (row# ; col#)
            cursor_row(clamp(val0,1,dh)-1);
            cursor_col(clamp(val1,1,dw)-1);
            break;
        }
        break;
      case 'I':                                  // <ESC>[#I - (CHT) cursor forward tab (default=1)
        switch (clamp(tot,0,1)) {                //   │
          case 0:                                //   ├── <ESC>[I  -- no vals
            cursor_tab_right(1);                 //   │               default <ESC>[1I
            break;                               //   │
          case 1:                                //   └── <ESC>[#I -- tab # times
            cursor_tab_right(clamp(val0,1,dw));  //
            break;
        }
        break;
      case 'J':                                  // <ESC>[#J - (ED) erase in display
        switch (clamp(tot,0,1)) {                //   │
          case 0: clear_eol(); break;            //   ├── <ESC>[J  -- no vals: default <ESC>[0J
          case 1:                                //   │
            switch (clamp(val0,0,3)) {           //   │
              case 0: clear_eod();     break;    //   ├── <ESC>[0J -- clear to end of display
              case 1: clear_sod();     break;    //   ├── <ESC>[1J -- clear to start of display
              case 2: clear_screen();  break;    //   ├── <ESC>[2J -- clear all lines
              case 3: clear_history(); break;    //   └── <ESC>[3J -- clear screen history
            }
            break;
        }
        break;
      case 'K':
        switch (clamp(tot,0,1)) {                // <ESC>[#K - (EL) Erase in Line
          case 0: clear_eol(); break;            //   ├── <ESC>[K  -- no vals
          case 1: switch (clamp(val0,0,2)) {     //   │
              case 0: clear_eol();  break;       //   ├── <ESC>[0K -- clear to end of line
              case 1: clear_sol();  break;       //   ├── <ESC>[1K -- clear to start of line
              case 2: clear_line(); break;       //   └── <ESC>[2K -- clear current line
            }
            break;
        }
        break;
      case 'L':                                  // ESC[#L - Insert # lines (def=1)
        insert_rows(esc.defvalmax(1,dh));
        break;
      case 'M':                                  // ESC[#M - Delete # lines (def=1)
        delete_rows(esc.defvalmax(1,dh));
        break;
      case 'P':                                  // ESC[#P - Delete # chars (def=1)
        delete_chars(esc.defvalmax(1,dh));
        break;
      case 'S':                                  // ESC[#S - scroll up # lines (def=1)
        scroll( +(esc.defvalmax(1,dh)) );
        //      ⮤ positive=scroll up
        break;
      case 'T':                                  // ESC[#T - scroll dn # lines (def=1)
        scroll( -(esc.defvalmax(1,dh)) );
        //      ⮤ negative=scroll down
        break;
      case 'X':                                  // <ESC>[#X - (ECH) Erase Characters (default=1)
        repeat_char(' ', esc.defvalmax(1,dw));
        break;
      case 'Z':                                  // ESC[#Z - backtab # tabs
        switch (clamp(tot,0,1)) {                //   │
          case 0:                                //   ├── <ESC>[Z  -- no vals
            cursor_tab_left(1);                  //   │               default <ESC>[1Z
            break;                               //   │
          case 1:                                //   └── <ESC>[#Z -- tab # times
            cursor_tab_left(clamp(val0,1,dw));
            break;
        }
        break;
      case 'a':  // TODO                         // ESC[#a - (HPR) move cursor relative [columns] (default=[row,col+1])
      case 'b':  // TODO                         // ESC[#b - (REP) repeat prev graphics char # times
      case 'd':  // TODO                         // ESC[#d - (VPA) line pos absolute [row]
      case 'e':  // TODO                         // ESC[#e - line pos relative [rows]
        handle_unknown_char();                   // does an escseq.reset()
        break;
      case 'f':                                  // <ESC>[#f - (CUP) cursor position (#'s 1 based)
        goto cup;                                //            (same as ESC[H)
      case 'g':                                  // ESC[...g? Tabulation Clear (TBC)
        switch (val0) {
          case  0: clear_tabstop();       break; // clears tabstop at cursor
          case  3: clear_all_tabstops();  break; // clears all tabstops
          default:
            handle_unknown_char();               // does an escseq.reset()
            break;
        }
        break;
      case 'm': handle_SGR();             break; // ESC[#m - set character attributes (SGR)
      case 's': save_cursor();            break; // ESC[s - save cur pos (xterm+gnome)
      case 'u': restore_cursor();         break; // ESC[u - restore cur pos (xterm+gnome)
      case 'q':  // TODO?                        // ESC[>#q set cursor style (block/line/blink..)
      case 'r':  // TODO                         // ESC[#;#r set scroll region top;bot
                                                 // default=full window
        handle_unknown_char();                   // does an escseq.reset()
        break;
      case 't': handle_DECRARA();         break; // ESC[#..$t -- (DECRARA)
                                                 // Reverse attribs in Rect Area (row,col)
      default:
        handle_unknown_char();                   // does an escseq.reset()
        break;
    }
  } else {
    // Not CSI? Might be C1 Control code (<ESC>D, etc)
    switch (esc.esc_mode()) {
      case 'c': reset_terminal();          break;// <ESC>c - Reset term to Initial State (RIS)
      case 'D': cursor_down(1, do_scroll); break;// <ESC>D - down line, scroll at bottom
      case 'E': cursor_crlf();             break;// <ESC>E - do a crlf
      case 'H': set_tabstop();             break;// <ESC>H - set a tabstop
      case 'M': cursor_up(1, true);        break;// <ESC>M - (RI) Reverse Index (up w/scroll)
      case '7': handle_unknown_char();     break;// <ESC>7 - Save cursor & attrs    // TODO
      case '8': handle_unknown_char();     break;// <ESC>8 - Restore cursor & attrs // TODO
      default:
        handle_unknown_char();                   // does an escseq.reset()
        break;
    }
  }
  esc.reset();   // done handling escseq, reset()
}

/**
  Clears that the display has been modified; sets internal
  redraw_modified_ to false.
*/
void Fl_Terminal::display_modified_clear(void) {
  redraw_modified_ = false;
}

/**
  Flag that the display has been modified, triggering redraws.
  Sets the internal redraw_modified_ flag to true.
*/
void Fl_Terminal::display_modified(void) {
  if (is_redraw_style(RATE_LIMITED)) {
    if (!redraw_modified_) {                         // wasn't before but now is?
      if (!redraw_timer_) {
        Fl::add_timeout(.01, redraw_timer_cb, this); // turn on timer
        redraw_timer_ = true;
      }
      redraw_modified_ = true;
    }
  } else if (is_redraw_style(PER_WRITE)) {
    if (!redraw_modified_) {
      redraw_modified_ = true;
      redraw();                      // only call redraw once
    }
  } else {                           // NO_REDRAW?
    // do nothing
  }
}

/**
  Clear the character at the specified display row and column.

  No range checking done on drow,dcol:
  - \p drow must be in range 0..(disp_rows()-1)
  - \p dcol must be in range 0..(disp_cols()-1)

  - Does not trigger redraws
*/
void Fl_Terminal::clear_char_at_disp(int drow, int dcol) {
  Utf8Char *u8c = u8c_disp_row(drow) + dcol;
  u8c->clear(*current_style_);
}

/**
  Return Utf8Char* for char at specified display row and column.
  This accesses any character in the display part of the ring buffer.

  No range checking done on drow,dcol:
  - \p drow must be in range 0..(disp_rows()-1)
  - \p dcol must be in range 0..(disp_cols()-1)

  \see u8c_disp_row()
*/
const Fl_Terminal::Utf8Char* Fl_Terminal::utf8_char_at_disp(int drow, int dcol) const {
  return u8c_disp_row(drow) + dcol;
}

/**
  Return Utf8Char* for char at specified global (grow,gcol).
  This accesses any character in the ring buffer (history + display).

  No range checking done on grow,gcol:
  - \p grow must be in range 0..(ring_rows()-1)
  - \p gcol must be in range 0..(ring_cols()-1)

  \see u8c_ring_row()
*/
const Fl_Terminal::Utf8Char* Fl_Terminal::utf8_char_at_glob(int grow, int gcol) const {
  return u8c_ring_row(grow) + gcol;
}

/**
  Plot the UTF-8 character \p text of length \p len at display position \p (drow,dcol).
  The character is displayed using the current text color/attributes.

  This is a very low level method.

  No range checking is done on drow,dcol:
  - \p drow must be in range 0..(display_rows()-1)
  - \p dcol must be in range 0..(display_columns()-1)

  - Does not trigger redraws
  - Does not handle control codes, ANSI or XTERM escape sequences.
  - Invalid UTF-8 chars show the error character (¿) depending on show_unknown(bool).

  \see handle_unknown_char()
*/
void Fl_Terminal::plot_char(const char *text, int len, int drow, int dcol) {
  Utf8Char *u8c = u8c_disp_row(drow) + dcol;
  // text_utf8() warns we must do invalid checks first
  if (!text || len<1 || len>u8c->max_utf8() || len!=fl_utf8len(*text)) {
    handle_unknown_char(drow, dcol);
    return;
  }
  u8c->text_utf8(text, len, *current_style_);
}

/**
  Plot the ASCII character \p c at the terminal's display position \p (drow,dcol).

  The character MUST be printable (in range 0x20 - 0x7e), and is displayed
  using the current text color/attributes. Characters outside that range are either
  ignored or print the error character (¿), depending on show_unknown(bool).

  This is a very low level method.

  No range checking is done on drow,dcol:
  - \p drow must be in range 0..(display_rows()-1)
  - \p dcol must be in range 0..(display_columns()-1)

  - Does not trigger redraws
  - Does NOT handle control codes, ANSI or XTERM escape sequences.

  \see show_unknown(bool), handle_unknown_char(), is_printable()
*/
void Fl_Terminal::plot_char(char c, int drow, int dcol) {
  if (!is_printable(c)) {
    handle_unknown_char(drow, dcol);
    return;
  }
  Utf8Char *u8c = u8c_disp_row(drow) + dcol;
  u8c->text_ascii(c, *current_style_);
}

/**
  Prints single UTF-8 char \p text of optional byte length \p len
  at current cursor position, and advances the cursor if the character
  is printable. Handles ASCII and control codes (CR, LF, etc).

  The character is displayed at the current cursor position
  using the current text color/attributes.

  Handles control codes and can be used to construct ANSI/XTERM
  escape sequences.

  - If optional \p len isn't specified or <0, strlen(text) is used.
  - \p text must not be NULL.
  - \p len must not be 0.
  - \p text must be a single char only (whether UTF-8 or ASCII)
  - \p text can be an ASCII character, though not as efficent as print_char()
  - Invalid UTF-8 chars show the error character (¿) depending on show_unknown(bool).
  - Does not trigger redraws

  \see show_unknown(bool), handle_unknown_char()
*/
void Fl_Terminal::print_char(const char *text, int len/*=-1*/) {
  len = len<0 ? fl_utf8len(*text) : len;       // int(strlen(text)) : len;
  const bool do_scroll = true;
  if (is_ctrl(text[0])) {                      // Handle ctrl character
    handle_ctrl(*text);
  } else if (escseq.parse_in_progress()) {     // ESC sequence in progress?
    handle_escseq(*text);
  } else {                                     // Handle printable char..
    plot_char(text, len, cursor_row(), cursor_col());
    cursor_right(1, do_scroll);
  }
}

/**
  Prints single ASCII char \p c at current cursor position, and advances the cursor.

  The character is displayed at the current cursor position
  using the current text color/attributes.

  - \p c must be ASCII, not utf-8
  - Does not trigger redraws
*/
void Fl_Terminal::print_char(char c) {
  const bool do_scroll = true;
  if (is_ctrl(c)) {                            // Handle ctrl character
    handle_ctrl(c);
  } else if (escseq.parse_in_progress()) {     // ESC sequence in progress?
    handle_escseq(c);
  } else {                                     // Handle printable char..
    plot_char(c, cursor_row(), cursor_col());
    cursor_right(1, do_scroll);
    return;
  }
}

// Clear the Partial UTF-8 Buffer cache
void Fl_Terminal::utf8_cache_clear(void) {
  pub_.clear();
}

// Flush the Partial UTF-8 Buffer cache, and clear
void Fl_Terminal::utf8_cache_flush(void) {
  if (pub_.buflen() > 0) print_char(pub_.buf(), pub_.buflen());
  pub_.clear();
}

/**
  Append NULL terminated UTF-8 string to terminal.

  - If buf is NULL, UTF-8 cache buffer is cleared
  - If optional \p len isn't specified or is -1, strlen(text) is used.
  - If \p len is 0 or <-1, no changes are made
  - Handles UTF-8 chars split across calls (e.g. block writes from pipes, etc)
  - Redraws are triggered automatically, depending on redraw_style()
*/
void Fl_Terminal::append_utf8(const char *buf, int len/*=-1*/) {
  int mod = 0;                                     // assume no modifications
  if (!buf) { utf8_cache_clear(); return; }        // clear cache, done
  if (len == -1) len = int(strlen(buf));           // len optional
  if (len<=0) return;                              // bad len? early exit

  // Handle any partial UTF-8 from last write
  //   Try to parse up rest of incomplete buffered char from end
  //   of last block, and flush it to terminal.
  //
  if (pub_.buflen() > 0) {                         // partial UTF-8 to deal with?
    while (len>0 && pub_.is_continuation(*buf)) {  // buffer 'continuation' chars
      if (pub_.append(buf, 1) == false)            // append byte to partial UTF-8 buffer
        { mod |= handle_unknown_char(); break; }   // overrun? break loop
      else { buf++; len--; }                       // shrink our buffer
    }
    if (pub_.is_complete()) utf8_cache_flush();    // complete UTF-8 captured? flush to tty
    if (len <= 0) {                                // check len again, we may have run out
      if (mod) display_modified();
      return;
    }
  }

  // For sure buf is now pointing at a valid char, so walk to end of buffer
  int clen;                                 // char length
  const char *p = buf;                      // ptr to walk buffer
  while (len>0) {
    clen = fl_utf8len(*p);                  // how many bytes long is this char?
    if (clen == -1) {                       // not expecting bad UTF-8 here
      mod |= handle_unknown_char();
      p   += 1;
      len -= 1;
    } else {
      if (len && clen>len) {                // char longer than buffer?
        if (pub_.append(p, len) == false) { // buffer it
          mod |= handle_unknown_char();
          utf8_cache_clear();
        }
        break;
      }
      print_char(p, clen);                  // write complete UTF-8 char to terminal
      p   += clen;                          // advance to next char
      len -= clen;                          // adjust len
      mod |= 1;
    }
  }
  if (mod) display_modified();
}

/**
  Append NULL terminated ASCII string to terminal,
  slightly more efficient than append_utf8().

  - If \p s is NULL, behavior is to do nothing
  - Redraws are triggered automatically, depending on redraw_style()
*/
void Fl_Terminal::append_ascii(const char *s) {
  if (!s) return;
  while ( *s ) print_char(*s++);            // handles display_modified()
  display_modified();
}

/**
  Appends string \p s to the terminal at the current cursor position
  using the current text color/attributes.

  If \p s is NULL, the UTF-8 character cache is cleared, which is
  recommended before starting a block reading loop, and again after the
  block loop has completed.

  If \p len is not specified, it's assumed \p s is a NULL terminated
  string. If \p len IS specified, it can be used for writing strings
  that aren't NULL terminated, such as block reads on a pipe, network,
  or other block oriented data source.

  Redraws of the terminal widget are by default handled automatically,
  but can be changed with redraw_rate() and redraw_style().

  <B>Block I/O</B>

  When reading block oriented sources (such as pipes), append() will
  handle partial UTF-8 chars straddling the block boundaries. It does
  this using an internal byte cache, which should be cleared before
  and after block I/O loops by calling <TT>append(NULL)</TT> as shown
  in the example below, to prevent the possibilities of partial UTF-8
  characters left behind by an interrupted or incomplete block loop.

  \par
  \code
  // Example block reading a command pipe in Unix

    // Run command and read as a pipe
    FILE *fp = popen("ls -la", "r");
    if (!fp) { ..error_handling.. }

    // Enable non-blocking I/O
    int fd = fileno(fp);
    fcntl(fd, F_SETFL, O_NONBLOCK);

    // Clear UTF-8 character cache before starting block loop
    G_tty->append(NULL);                               // prevents leftover partial UTF-8 bytes

    // Block read loop
    while (1) {
      Fl::wait(0.05);                                  // give fltk .05 secs of cpu to manage UI
      ssize_t bytes = read(fd, s, sizeof(s));          // read block from pipe
      if (bytes == -1 && errno == EAGAIN) continue;    // no data yet? continue
      if (bytes > 0) G_tty->append(s);                 // append output to terminal
      else break;                                      // end of pipe?
    }

    // Flush cache again after block loop completes
    G_tty->append(NULL);

    // Close pipe, done
    pclose(fp);

  \endcode

  \note
  - String can contain ASCII or UTF-8 chars
  - \p len is optional; if unspecified, expects \p s to be a NULL terminated string
  - Handles partial UTF-8 chars split between calls (e.g. block oriented writes)
  - If \p s is NULL, this clears the "partial UTF-8" character cache
  - Redraws are managed automatically by default; see redraw_style()
*/
void Fl_Terminal::append(const char *s, int len/*=-1*/) {
  append_utf8(s, len);
}

/**
  Handle an unknown char by either emitting an error symbol to the tty, or do nothing,
  depending on the user configurable value of show_unknown().

  This writes the "unknown" character to the output stream
  if show_unknown() is true.

  Returns 1 if tty modified, 0 if not.
  \see show_unknown()
*/
int Fl_Terminal::handle_unknown_char(void) {
  if (!show_unknown_) return 0;
  escseq.reset();               // disable any pending esc seq to prevent eating unknown char
  print_char(error_char_);
  return 1;
}

/**
  Handle an unknown char by either emitting an error symbol to the tty, or do nothing,
  depending on the user configurable value of show_unknown().

  This writes the "unknown" character to the display position \p (drow,dcol)
  if show_unknown() is true.

  Returns 1 if tty modified, 0 if not.
  \see show_unknown()
*/
int Fl_Terminal::handle_unknown_char(int drow, int dcol) {
  if (!show_unknown_) return 0;
  int len = (int)strlen(error_char_);
  Utf8Char *u8c = u8c_disp_row(drow) + dcol;
  u8c->text_utf8(error_char_, len, *current_style_);
  return 1;
}

// Handle user interactive scrolling
//    Note: this callback shared by vertical and horizontal scrollbars
//
void Fl_Terminal::scrollbar_cb(Fl_Widget*, void* userdata) {
  Fl_Terminal *o = (Fl_Terminal*)userdata;
  o->redraw();
}

// Handle mouse selection autoscrolling
void Fl_Terminal::autoscroll_timer_cb2(void) {
  // Move scrollbar
  //   NOTE: scrollbar is inverted; 0=tab at bot, so minimum() is really max
  //
  int amt  = autoscroll_amt_;                      // (amt<0):above top, (amt>0):below bottom
  int val  = scrollbar->value();
  int max  = int(scrollbar->minimum()+.5);         // NOTE: minimum() is really max
  val = (amt<0) ? (val+clamp((-amt/10),1,5)) :     // above top edge?
        (amt>0) ? (val-clamp((+amt/10),1,5)) : 0;  // below bot edge?
  val = clamp(val,0,max);                          // limit val to scroll's range
  int diff = ABS(val - scrollbar->value());        // how far scroll tab moved up/dn
  // Move scrollbar
  scrollbar->value(val);
  // Extend selection
  if (diff) {                                      // >0 if up or down
    int srow = select_.srow(), scol = select_.scol();
    int erow = select_.erow(), ecol = select_.ecol();
    int ltcol = 0, rtcol = ring_cols() - 1;
    if (amt<0) { erow -= diff; ecol = ltcol; }     // above top? use erow: reverse-selecting
    if (amt>0) { erow += diff; ecol = rtcol; }     // below bot? use erow: forward-selecting
    select_.select(srow, scol, erow, ecol);
  }
  // Restart timeout
  Fl::repeat_timeout(.1, autoscroll_timer_cb, this);
  redraw();
}

// Handle mouse selection autoscrolling
void Fl_Terminal::autoscroll_timer_cb(void *udata) {
  Fl_Terminal *tty = (Fl_Terminal*)udata;
  tty->autoscroll_timer_cb2();
}

// Handle triggering rate limited redraw() updates
//   When data comes in quickly, append() sets the redraw_modified_ flag
//   so our timer can trigger the redraw()s at a controlled rate.
//
void Fl_Terminal::redraw_timer_cb2(void) {
  //DRAWDEBUG ::printf("--- UPDATE TICK %.02f\n", redraw_rate_); fflush(stdout);
  if (redraw_modified_) {
    redraw();                                                // Timer triggered redraw
    redraw_modified_ = false;                                // acknowledge modified flag
    Fl::repeat_timeout(redraw_rate_, redraw_timer_cb, this); // restart timer
  } else {
    // Timer went off and nothing to redraw? disable
    Fl::remove_timeout(redraw_timer_cb, this);
    redraw_timer_ = false;
  }
}

void Fl_Terminal::redraw_timer_cb(void *udata) {
  Fl_Terminal *tty = (Fl_Terminal*)udata;
  tty->redraw_timer_cb2();
}

/**
  The constructor for Fl_Terminal.

  This creates an empty terminal with defaults:
  - white on black text; see textfgcolor(Fl_Color), textbgcolor(Fl_Color)
  - rows/cols based on the \p W and \p H values, see display_rows(), display_columns()
  - scrollback history of 100 lines, see history_rows()
  - redraw_style() set to RATE_LIMITED, redraw_rate() set to 0.10 seconds

  Note: While Fl_Terminal derives from Fl_Group, it's not intended for user code
  to use it as a parent for other widgets, so end() is called.

  \param[in] X,Y,W,H position and size.
  \param[in] L label string (optional), may be NULL.
*/
Fl_Terminal::Fl_Terminal(int X,int Y,int W,int H,const char*L)
  : Fl_Group(X,Y,W,H,L),
    select_(this)
{
  bool fontsize_defer = false;
  init_(X,Y,W,H,L,-1,-1,100,fontsize_defer);
}

/**
  Same as the default FLTK constructor, but lets the user force the rows, columns
  and history to specific sizes on creation.

  Since the row/cols/hist are specified directly, this prevents the widget from auto-calculating
  the initial text buffer size based on the widget's pixel width/height, bypassing calls to
  the font system before the widget is displayed.

  \note fluid uses this constructor internally to avoid font calculations that opens
  the display, useful for when running in a headless context. (issue 837)
*/
Fl_Terminal::Fl_Terminal(int X,int Y,int W,int H,const char*L,int rows,int cols,int hist)
  : Fl_Group(X,Y,W,H,L),
    select_(this)
{
  bool fontsize_defer = true;
  init_(X,Y,W,H,L,rows,cols,hist,fontsize_defer);
}

// Private constructor method
void Fl_Terminal::init_(int X,int Y,int W,int H,const char*L,int rows,int cols,int hist,bool fontsize_defer) {
  error_char_ = "¿";
  scrollbar = hscrollbar = 0;           // avoid problems w/update_screen_xywh()
  // currently unused params
  (void)X; (void)Y; (void)W; (void)H; (void)L;
  fontsize_defer_ = fontsize_defer;     // defer font calls until draw() (issue 837)
  current_style_  = new CharStyle(fontsize_defer);
  oflags_         = LF_TO_CRLF;         // default: "\n" handled as "\r\n"
  // scrollbar_size must be set before scrn_
  scrollbar_size_ = 0;                  // 0 uses Fl::scrollbar_size()
  Fl_Group::box(FL_DOWN_FRAME);         // set before update_screen_xywh()
  update_screen_xywh();
  // Tabs
  tabstops_       = 0;
  tabstops_size_  = 0;
  // Init ringbuffer. Also creates default tabstops
  if (rows == -1 || cols == -1) {
    int newrows = h_to_row(scrn_.h());  // rows based on height
    int newcols = w_to_col(scrn_.w());  // cols based on width
    // Sanity check
    newrows = (newrows >= 1) ? newrows : 1;
    newcols = (newcols >= 1) ? newcols : 1;
    create_ring(newrows, newcols, hist);
  } else {
    create_ring(rows, cols, 100);
  }
  // Misc
  redraw_style_    = RATE_LIMITED;      // NO_REDRAW, RATE_LIMITED, PER_WRITE
  redraw_rate_     = 0.10f;             // maximum rate in seconds (1/10=10fps)
  redraw_modified_ = false;             // display 'modified' flag
  redraw_timer_    = false;
  autoscroll_dir_  = 0;
  autoscroll_amt_  = 0;

  // Create scrollbars
  //    Final position/size/parameters are set by update_screen() **
  //
  scrollbar = new Fl_Scrollbar(x(), y(), scrollbar_actual_size(), h());  // tmp xywh (changed later) **
  scrollbar->type(FL_VERTICAL);
  scrollbar->value(0);
  scrollbar->callback(scrollbar_cb, (void*)this);

  hscrollbar = new Fl_Scrollbar(x(), y(), w(), scrollbar_actual_size());  // tmp xywh (changed later) **
  hscrollbar->type(FL_HORIZONTAL);
  hscrollbar->value(0);
  hscrollbar->callback(scrollbar_cb, (void *)this);

  hscrollbar_style_ = SCROLLBAR_AUTO;

  resizable(0);
  Fl_Group::color(FL_BLACK);  // black bg by default
  update_screen(true);        // update internal vars after setting screen size/font
  clear_screen_home();        // clear screen, home cursor
  clear_history();            // clear history buffer
  show_unknown_ = false;      // default "off"
  ansi_ = true;               // default "on"
  // End group
  end();
}

/**
  The destructor for Fl_Terminal.
  Destroys the terminal display, scroll history, and associated widgets.
*/
Fl_Terminal::~Fl_Terminal(void) {
  // Note: RingBuffer class handles destroying itself
  if (tabstops_)
    { free(tabstops_); tabstops_ = 0; }
  if (autoscroll_dir_)
    { Fl::remove_timeout(autoscroll_timer_cb, this); autoscroll_dir_ = 0; }
  if (redraw_timer_)
    { Fl::remove_timeout(redraw_timer_cb, this); redraw_timer_ = false; }
  delete current_style_;
}

/**
  Returns the scrollbar's actual "trough size", which is the width of FL_VERTICAL
  scrollbars, or height of FL_HORIZONTAL scrollbars.

  If scrollbar_size() is zero (default), then the value of the global Fl::scrollbar_size()
  is returned, which is the default global scrollbar size for the entire application.
*/
int Fl_Terminal::scrollbar_actual_size(void) const {
  return scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size();
}

/**
  Get current pixel size of all the scrollbar's troughs for this widget,
  or zero if the global Fl::scrollbar_size() is being used (default).

  If this value returns *zero*, this widget's scrollbars are using the
  global Fl::scrollbar_size(), in which case use scrollbar_actual_size()
  to get the actual (effective) pixel scrollbar size being used.

  \returns Scrollbar trough size in pixels, or 0 if the global Fl::scrollbar_size() is being used.
  \see Fl::scrollbar_size(int), scrollbar_actual_size()
*/
int Fl_Terminal::scrollbar_size(void) const {
  return scrollbar_size_;
}

/**
  Set the pixel size of both horizontal and vertical scrollbar's "trough" to \p val.

  Setting \p val to the special value 0 causes the widget to
  track the global Fl::scrollbar_size().

  Use non-zero values *only* if you need to override the global Fl::scrollbar_size() size.

  \see Fl::scrollbar_size(), scrollbar_actual_size()
*/
void Fl_Terminal::scrollbar_size(int val) {
  scrollbar_size_ = val;
  update_scrollbar();
  refit_disp_to_screen();
}

/**
  Get the horizontal scrollbar behavior style.

  This determines when the scrollbar is visible.

  Value will be one of the Fl_Terminal::ScrollbarStyle enum values.

  \see hscrollbar_style(Fl_Terminal::ScrollbarStyle)
*/
Fl_Terminal::ScrollbarStyle Fl_Terminal::hscrollbar_style() const {
  return hscrollbar_style_;
}

/**
  Set the horizontal scrollbar behavior style.

  This determines when the scrollbar is visible.

  \par
    |   ScrollbarStyle enum     | Description
    | :-----------------------: | :-----------------------------------------------------------
    |   \ref SCROLLBAR_ON       | Horizontal scrollbar always displayed.
    |   \ref SCROLLBAR_OFF      | Horizontal scrollbar never displayed.
    |   \ref SCROLLBAR_AUTO     | Horizontal scrollbar displayed whenever widget width hides columns.

  The default style is SCROLLBAR_AUTO.

  \see ScrollbarStyle
*/
void  Fl_Terminal::hscrollbar_style(ScrollbarStyle val) {
  hscrollbar_style_ = val;
  update_scrollbar();
  refit_disp_to_screen();
}

////////////////////////////
////// SCREEN DRAWING //////
////////////////////////////

/**
  Draw the background for the specified ring_chars[] global row \p grow
  starting at FLTK coords \p X and \p Y.

  Note we may be called to draw display, or even history if we're scrolled back.
  If there's any change in bg color, we draw the filled rects here.

  If the bg color for a character is the special "see through" color 0xffffffff,
  no pixels are drawn.

 \param[in] grow row number
 \param[in] X, Y top left corner of the row in FLTK coordinates
*/
void Fl_Terminal::draw_row_bg(int grow, int X, int Y) const {
  int bg_h = current_style_->fontheight();
  int bg_y = Y;
  Fl_Color bg_col;
  int pwidth    = 9;
  int start_col = hscrollbar->visible() ? hscrollbar->value() : 0;
  int end_col   = disp_cols();
  const Utf8Char *u8c = u8c_ring_row(grow) + start_col;   // start of spec'd row
  uchar lastattr      = u8c->attrib();
  for (int gcol=start_col; gcol<end_col; gcol++,u8c++) {  // walk columns
    // Attribute changed since last char?
    if (gcol==0 || u8c->attrib() != lastattr) {
      u8c->fl_font_set(*current_style_);                  // pwidth_int() needs fl_font set
      lastattr = u8c->attrib();
    }
    pwidth = u8c->pwidth_int();
    bg_col = is_inside_selection(grow, gcol)              // text in mouse select?
               ? select_.selectionbgcolor()               // ..use select bg color
               : (u8c->attrib() & Fl_Terminal::INVERSE)   // Inverse mode?
                 ? u8c->attr_fg_color(this)               // ..use fg color for bg
                 : u8c->attr_bg_color(this);              // ..use bg color for bg
    // Draw only if color != 0xffffffff ('see through' color) or widget's own color().
    if (bg_col != 0xffffffff && bg_col != Fl_Group::color()) {
      fl_color(bg_col);
      fl_rectf(X, bg_y, pwidth, bg_h);
    }
    X += pwidth;                                          // advance X to next char
  }
}

/**
  Draw the specified global row, which is the row in ring_chars[].
  The global row includes history + display buffers.

 \param[in] grow row number
 \param[in] Y top position of characters in the row in FLTK coordinates
*/
void Fl_Terminal::draw_row(int grow, int Y) const {
  // Draw background color spans, if any
  int X = scrn_.x();
  draw_row_bg(grow, X, Y);

  // Draw forground text
  int  baseline = Y + current_style_->fontheight() - current_style_->fontdescent();
  int  scrollval = scrollbar->value();
  int  disp_top = (disp_srow() - scrollval);              // top row we need to view
  int  drow = grow - disp_top;                            // disp row
  bool inside_display = is_disp_ring_row(grow);           // row inside 'display'?
// This looks better on macOS, but too low for X. Maybe we can get better results using fl_text_extents()?
//  int  strikeout_y = baseline - (current_style_->fontheight() / 4);
//  int  underline_y = baseline + (current_style_->fontheight() / 5);
  int  strikeout_y = baseline - (current_style_->fontheight() / 3);
  int  underline_y = baseline;
  uchar lastattr = -1;
  bool  is_cursor;
  Fl_Color fg;
  int start_col = hscrollbar->visible() ? hscrollbar->value() : 0;
  int end_col   = disp_cols();
  const Utf8Char *u8c = u8c_ring_row(grow) + start_col;
  for (int gcol=start_col; gcol<end_col; gcol++,u8c++) {  // walk the columns
    const int &dcol = gcol;                               // dcol and gcol are the same
    // Are we drawing the cursor? Only if inside display
    is_cursor = inside_display ? cursor_.is_rowcol(drow-scrollval, dcol) : 0;
    // Attribute changed since last char?
    if (u8c->attrib() != lastattr) {
      u8c->fl_font_set(*current_style_);                  // pwidth_int() needs fl_font set
      lastattr = u8c->attrib();
    }
    int pwidth = u8c->pwidth_int();
    // DRAW CURSOR BLOCK - TODO: support other cursor types?
    if (is_cursor) {
      int cx = X;
      int cy = Y + current_style_->fontheight() - cursor_.h();
      int cw = pwidth;
      int ch = cursor_.h();
      fl_color(cursorbgcolor());
      if (Fl::focus() == this) fl_rectf(cx, cy, cw, ch);
      else                     fl_rect(cx, cy, cw, ch);
    }
    // DRAW TEXT
    // 1) Color for text
    if (is_cursor) fg = cursorfgcolor();                     // color for text under cursor
    else fg = is_inside_selection(grow, gcol)                // text in mouse selection?
      ? select_.selectionfgcolor()                           // ..use selection FG color
      : (u8c->attrib() & Fl_Terminal::INVERSE)               // Inverse attrib?
        ? u8c->attr_bg_color(this)                           // ..use char's bg color for fg
        : u8c->attr_fg_color(this);                          // ..use char's fg color for fg
    fl_color(fg);
    // 2) Font for text - already set by u8c->fl_font_set() in the above
    if (is_cursor) {
      fl_font(fl_font()|FL_BOLD, fl_size());      // force text under cursor BOLD
      lastattr = -1;                              // (ensure font reset on next iter)
    }
    // 3) Draw text for UTF-8 char. No need to draw spaces
    if (!u8c->is_char(' ')) fl_draw(u8c->text_utf8(), u8c->length(), X, baseline);
    // 4) Strike or underline?
    if (u8c->attrib() & Fl_Terminal::UNDERLINE) fl_line(X, underline_y, X+pwidth, underline_y);
    if (u8c->attrib() & Fl_Terminal::STRIKEOUT) fl_line(X, strikeout_y, X+pwidth, strikeout_y);
    // Move to next char pixel position
    X += pwidth;
  }
}

/**
  Draws the buffer position we are scrolled to onto the FLTK screen
  starting at pixel position Y.

  This can be anywhere in the ring buffer, not just the 'active diplay';
  depends on what position the scrollbar is set to.

  Handles attributes, colors, text selections, cursor.

 \param[in] Y top position of top left character in the window in FLTK coordinates
*/
void Fl_Terminal::draw_buff(int Y) const {
  int srow = disp_srow() - scrollbar->value();
  int erow = srow + disp_rows();
  const int rowheight = current_style_->fontheight();
  for (int grow=srow; (grow<erow) && (Y<scrn_.b()); grow++) {
    draw_row(grow, Y);          // draw global row at Y
    Y += rowheight;             // advance Y to bottom left corner of row
  }
}

/**
  Draws the entire Fl_Terminal.
  Lets the group draw itself first (scrollbars should be only members),
  followed by the terminal's screen contents.
*/
void Fl_Terminal::draw(void) {
  // First time shown? Force deferred font size calculations here (issue 837)
  if (fontsize_defer_) {
    fontsize_defer_ = false;    // clear flag
    current_style_->update();   // do deferred update here
    update_screen(true);        // update fonts
  }
  // Detect if Fl::scrollbar_size() was changed in size, recalc if so
  if (scrollbar_size_ == 0 &&
      ((scrollbar->visible() && scrollbar->w() != Fl::scrollbar_size()) ||
       (hscrollbar->visible() && hscrollbar->h() != Fl::scrollbar_size()))) {
    update_scrollbar();
  }
  // Draw group first, terminal last
  Fl_Group::draw();
  // Draw that little square between the scrollbars:
  if (scrollbar->visible() && hscrollbar->visible()) {
    fl_color(parent()->color());
    fl_rectf(scrollbar->x(), hscrollbar->y(), scrollbar_actual_size(), scrollbar_actual_size());
  }
  if (is_frame(box())) {
    // Is box() a frame? Fill area inside frame with rectf().
    //    FL_XXX_FRAME types allow Fl_Terminal to have a /flat/ background.
    //    FL_XXX_BOX types inherit Fl::scheme() which can provide unwanted gradients.
    //
    fl_color(Fl_Group::color());
    // Draw flat field (inside border drawn by Fl_Group::draw() above)
    int X = x() + Fl::box_dx(box());
    int Y = y() + Fl::box_dy(box());
    int W = w() - Fl::box_dw(box());
    int H = h() - Fl::box_dh(box());
    if (scrollbar->visible())  W -= scrollbar_actual_size();
    if (hscrollbar->visible()) H -= scrollbar_actual_size();
    fl_rectf(X,Y,W,H);
  }
  //DEBUG  fl_color(0x80000000);     // dark red box inside margins
  //DEBUG  fl_rect(scrn_);
  fl_push_clip(scrn_.x(), scrn_.y(), scrn_.w(), scrn_.h());
    int Y = scrn_.y();
    draw_buff(Y);
  fl_pop_clip();
}

/**
  Given a width in pixels, return number of columns that "fits" into that area.
  This is used by the constructor to size the row/cols to fit the widget size.
*/
int Fl_Terminal::w_to_col(int W) const {
  return W / current_style_->charwidth();
}

/**
  Given a height in pixels, return number of rows that "fits" into that area.
  This is used by the constructor to size the row/cols to fit the widget size.
*/
int Fl_Terminal::h_to_row(int H) const {
  return H / current_style_->fontheight();
}

/**
  Handle widget resizing, such as if user resizes parent window.
  This may increase the column width of the widget if the width
  of the widget is made larger than it was.

  \note Resizing currently does not rewrap existing text.
  Currently enlarging makes room for longer lines, and shrinking
  the size lets long lines run off the right edge of the display,
  hidden from view. This behavior may change in the future to rewrap.
*/
void Fl_Terminal::resize(int X,int Y,int W,int H) {
  // Let group resize itself
  Fl_Group::resize(X,Y,W,H);
  // Update screen stuff; margins, etc
  update_screen(false);             // no change in font, just resizing
  // resize the display's rows+cols to match window size
  refit_disp_to_screen();
}

// Handle autoscrolling vals + timer
//   If mouse dragged beyond top/bottom, start/continue auto-scroll select
//
void Fl_Terminal::handle_selection_autoscroll(void) {
  int Y    = Fl::event_y();
  int top  = scrn_.y();
  int bot  = scrn_.b();
  int dist = (Y < top) ? Y - top :          // <0 if above top
             (Y > bot) ? Y - bot : 0;       // >0 if below bottom
  if (dist == 0) {
    // Not off edge? stop autoscrolling, done
    if (autoscroll_dir_) Fl::remove_timeout(autoscroll_timer_cb, this);
    autoscroll_dir_ = 0;
  } else {
    // Above top/below bot? Start/continue autoscroll select
    if (!autoscroll_dir_) Fl::add_timeout(.01, autoscroll_timer_cb, this);
    autoscroll_amt_ = dist;                 // <0 if above top, >0 if below bot
    autoscroll_dir_ = (dist < 0) ? 3 : 4;   // 3=scrolling up, 4=scrolling dn
  }
}

/**
  Handle mouse selection on LEFT-CLICK push/drag/release.
  Returns: 1 if 'handled', 0 if not.
*/
int Fl_Terminal::handle_selection(int e) {
  int grow=0, gcol=0;
  bool gcr = false;
  bool is_rowcol = (xy_to_glob_rowcol(Fl::event_x(), Fl::event_y(), grow, gcol, gcr) > 0)
                   ? true : false;
  switch (e) {
    case FL_PUSH: {
      // SHIFT-LEFT-CLICK? Extend or start new
      if (Fl::event_state(FL_SHIFT)) {
        if (is_selection()) {                           // extend if select in progress
          selection_extend(Fl::event_x(), Fl::event_y());
          redraw();
          return 1;                                     // express interest in FL_DRAG
        }
      } else {                                          // Start a new selection
        select_.push_rowcol(grow, gcol, gcr);
        if (select_.clear()) redraw();                  // clear prev selection
        if (is_rowcol) {
          switch (Fl::event_clicks()) {
            case 1: select_word(grow, gcol); break;
            case 2: select_line(grow); break;
          }
          return 1;                    // express interest in FL_DRAG
        }
      }
      // Left-Click outside terminal area?
      if (!Fl::event_state(FL_SHIFT)) {
        select_.push_clear();
        clear_mouse_selection();
        redraw();
      }
      return 0;                                         // event NOT handled
    }
    case FL_DRAG: {
      if (is_rowcol) {
        if (!is_selection()) {                          // no selection yet?
          if (select_.dragged_off(grow, gcol, gcr)) {   // dragged off FL_PUSH? enough to start
            select_.start_push();                       // ..start drag with FL_PUSH position
          }
        } else {
          if (select_.extend(grow, gcol, gcr)) redraw(); // redraw if selection changed
        }
      }
      // If we leave scrn area, start timer to auto-scroll+select
      handle_selection_autoscroll();
      return 1;
    }
    case FL_RELEASE: {
      select_.end();
      // middlemouse gets immediate copy of selection
      if (is_selection()) {
        const char *copy = selection_text();
        if (*copy) Fl::copy(copy, (int)strlen(copy), 0);
        free((void*)copy);
      }
      return 1;
    }
    default:
      break;
  }
  return 0;
}

/**
  Handle FLTK events.
*/
int Fl_Terminal::handle(int e) {
  int ret = Fl_Group::handle(e);
  if (Fl::event_inside(scrollbar)) return ret;             // early exit for scrollbar
  if (Fl::event_inside(hscrollbar)) return ret;             // early exit for hscrollbar
  switch (e) {
    case FL_ENTER:
    case FL_LEAVE:
      return 1;
    case FL_UNFOCUS:
    case FL_FOCUS:
      redraw();
      return Fl::visible_focus() ? 1 : 0;
    case FL_KEYBOARD:
      // ^C -- Copy?
      if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='c') {
        const char *copy = is_selection() ? selection_text() : fl_strdup(" ");
        if (*copy) Fl::copy(copy, (int)strlen(copy), 1);  // paste buffer
        free((void*)copy);
        return 1;
      }
      // ^A -- Select all?
      if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='a') {
        // Select entire screen and history buffer
        int srow = disp_srow() - hist_use();
        int erow = disp_srow() + disp_rows()-1;
        //DEBUG ::printf("CTRL-A: srow=%d erow=%d\n", srow, erow);
        select_.select(srow, 0, erow, disp_cols()-1);
        const char *copy = selection_text();
        if (*copy) Fl::copy(copy, (int) strlen(copy), 0); // middle mouse buffer
        free((void*)copy);
        redraw();
        return 1;
      }
      // Let scrollbar handle these when we have focus
      if (Fl::focus() == this) {
        switch (Fl::event_key()) {
          case FL_Page_Up: case FL_Page_Down:
          case FL_Up:      case FL_Down:
          case FL_Left:    case FL_Right:
            return scrollbar->handle(e);
        }
      }
      break;
    case FL_PUSH:
      if (handle(FL_FOCUS)) Fl::focus(this);              // Accepting focus? take it
      if (Fl::event_button() == FL_LEFT_MOUSE)            // LEFT-CLICK?
        { ret = handle_selection(FL_PUSH); }
      break;
    case FL_DRAG:
      // TODO: This logic can probably be improved to allow an FL_PUSH in margins
      //       to drag into terminal area to start a selection.
      if (Fl::event_button() == FL_LEFT_MOUSE)            // LEFT-DRAG?
        { ret = handle_selection(FL_DRAG); }
      break;
    case FL_RELEASE:
      // Selection mouse release?
      if (Fl::event_button() == FL_LEFT_MOUSE)            // LEFT-RELEASE?
        { ret = handle_selection(FL_RELEASE); }
      // Disable autoscroll timer, if any
      if (autoscroll_dir_)
        { Fl::remove_timeout(autoscroll_timer_cb, this); autoscroll_dir_ = 0; }
      break;
  } // switch
  return ret;
}

/**
  Return a string copy of all lines in the terminal (including history).
  The returned string is allocated with strdup(3), which the caller must free.

  If \p 'lines_below_cursor' is false (default), lines below the cursor on down
  to the bottom of the display are ignored, and not included in the returned string.

  If \p 'lines_below_cursor' is true, then all lines in the display are returned
  including any below the cursor, even if all are blank.

  Example use:
  \par
  \code
      Fl_Terminal *tty = new Fl_Terminal(..);
      :
      const char *s = tty->text();   // get a copy of the terminal's contents
      printf("Terminal's contents is:\n%s\n", s);
      free((void*)s);                // free() the copy when done!
  \endcode

  \param[in]  lines_below_cursor  include lines below cursor, default: false

  \return A string allocated with strdup(3) which must be free'd, text is UTF-8.
*/
const char* Fl_Terminal::text(bool lines_below_cursor) const {
  Fl_String lines;          // lines of text we'll return
  // See how many display rows we need to include
  int disprows = lines_below_cursor ? disp_rows() - 1    // all display lines
                                    : cursor_row();      // only lines up to cursor
  // Start at top of 'in use' history, and walk to end of display
  int srow = hist_use_srow();                            // start row of text to return
  int erow = srow + hist_use() + disprows;               // end row of text to return
  for (int row=srow; row<=erow; row++) {                 // walk rows
    const Utf8Char *u8c = u8c_ring_row(row);             // start of row
    int trim = 0;
    for (int col=0; col<ring_cols(); col++,u8c++) {      // walk cols in row
      const char *s = u8c->text_utf8();                  // first byte of char
      for (int i=0; i<u8c->length(); i++) lines += *s++; // append all bytes in multibyte char
      // Count any trailing whitespace to trim
      if (u8c->length()==1 && s[-1]==' ') trim++;        // trailing whitespace? trim
      else                                trim = 0;      // non-whitespace? don't trim
    }
    // trim trailing whitespace from each line, if any
    if (trim) lines.resize(lines.size() - trim);
    lines += "\n";
  }
  return fl_strdup(lines.c_str());
}

/**
  Get the redraw style.

  This determines when the terminal redraws itself while text
  is being added to it.

  Value will be one of the Fl_Terminal::RedrawStyle enum values.

  \see redraw_style(Fl_Terminal::RedrawStyle)
*/
Fl_Terminal::RedrawStyle Fl_Terminal::redraw_style() const {
  return redraw_style_;
}

/**
  Set how Fl_Terminal manages screen redrawing.

  This setting is relevant when Fl_Terminal is used for high bandwidth
  data; too many redraws will slow things down, too few cause redraws
  to be 'choppy' when realtime data comes in.

  Redrawing can be cpu intensive, depending on how many rows/cols are
  being displayed; worst case: large display + small font. Speed largely
  depends on the end user's graphics hardware and font drawing system.

  \par
    |   RedrawStyle enum    | Description
    | :-------------------: | :-----------------------------------------------------------
    |   \ref NO_REDRAW      | App must call redraw() as needed to update text to screen
    |   \ref RATE_LIMITED   | Rate limited, timer controlled redraws. (DEFAULT) See redraw_rate()
    |   \ref PER_WRITE      | Redraw triggered *every* call to append() / printf() / etc.

  The default style is RATE_LIMITED, which is the easiest
  to use, and automates redrawing to be capped at 10 redraws per second
  max. See redraw_rate(float) to control this automated redraw speed.

  \see redraw_rate(), RedrawStyle
*/
void  Fl_Terminal::redraw_style(RedrawStyle val) {
  redraw_style_ = val;
  // Disable rate limit timer if it's being turned off
  if (redraw_style_ != RATE_LIMITED && redraw_timer_)
    { Fl::remove_timeout(redraw_timer_cb, this); redraw_timer_ = false; }
}

/**
  Get max rate redraw speed in floating point seconds.
*/
float Fl_Terminal::redraw_rate(void) const {
  return redraw_rate_;
}

/**
  Set the maximum rate redraw speed in floating point seconds
  if redraw_style() is set to RATE_LIMITED.

  When output is sent to the terminal, rather than calling redraw()
  right away, a timer is started with this value indicating how long
  to wait before calling redraw(), causing the output to be shown.
  0.10 is recommended (1/10th of a second), to limit redraws to no
  more than 10 redraws per second.

  The value that works best depends on how fast data arrives, and
  how fast the font system can draw text at runtime.

  Values too small cause too many redraws to occur, causing the
  terminal to get backlogged if large bursts of data arrive quickly.
  Values too large cause realtime output to be too "choppy".
*/
void  Fl_Terminal::redraw_rate(float val) {
  redraw_rate_ = val;
}

/**
  Return the "show unknown" flag.
  \see show_unknown(bool), error_char(const char*).
*/
bool Fl_Terminal::show_unknown(void) const {
  return show_unknown_;
}

/**
  Set the "show unknown" flag.

  If true, invalid utf8 and invalid ANSI sequences will be shown
  with the error character "¿".

  If false, errors characters won't be shown.

  \see handle_unknown_char(), error_char(const char*).
*/
void Fl_Terminal::show_unknown(bool val) {
  show_unknown_ = val;
}

/**
  Return the state of the ANSI flag.
  \see ansi(bool)
*/
bool Fl_Terminal::ansi(void) const {
  return ansi_;
}

/**
  Enable/disable the ANSI mode flag.
  If true, ANSI and VT100/xterm codes will be processed.
  If false, these codes won't be processed and will either
  be ignored or print the error character "¿", depending on
  the value of show_unknown().
  \see show_unknown(), \ref Fl_Terminal_escape_codes
*/
void Fl_Terminal::ansi(bool val) {
  ansi_ = val;
  // If disabled, reset the class to clear old state information
  if (!ansi_) escseq.reset();
}

/**
  Return the number of lines of screen history.
*/
int Fl_Terminal::history_lines(void) const  { return history_rows(); }

/**
  Set the number of lines of screen history.
  Large values can be briefly heavy on cpu and memory usage.
*/
void Fl_Terminal::history_lines(int val) { history_rows(val); }

/**
  Appends printf formatted messages to the terminal.

  The string can contain UTF-8, crlf's, and ANSI sequences are
  also supported. Example:
  \par
  \code
  #include <FL/Fl_Terminal.H>
  int main(..) {
      :
      // Create a terminal, and append some messages to it
      Fl_Terminal *tty = new Fl_Terminal(..);
      :
      // Append three lines of formatted text to the buffer
      tty->printf("The current date is: %s.\nThe time is: %s\n", date_str, time_str);
      tty->printf("The current PID is %ld.\n", (long)getpid());
      :
  \endcode
  \note The expanded string is currently limited to 1024 characters (including NULL).
        For longer strings use append() which has no string limits.
*/
void Fl_Terminal::printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  Fl_Terminal::vprintf(fmt, ap);
  va_end(ap);
}

/**
 Appends printf formatted messages to the terminal.

 Subclasses can use this to implement their own printf()
 functionality.

 The string can contain UTF-8, crlf's, and ANSI sequences are
 also supported when ansi(bool) is set to 'true'.

 \note The expanded string is currently limited to 1024 characters (including NULL).
       For longer strings use append() which has no string limits.
 \param fmt is a printf format string for the message text.
 \param ap is a va_list created by va_start() and closed with va_end(),
           which the caller is responsible for handling.
*/
void Fl_Terminal::vprintf(const char *fmt, va_list ap) {
  char buffer[1024];    // XXX: should be user configurable..
  ::vsnprintf(buffer, 1024, fmt, ap);
  buffer[1024-1] = 0;   // XXX: MICROSOFT
  append(buffer);
}

