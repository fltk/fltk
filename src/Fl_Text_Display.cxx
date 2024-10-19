//
// Copyright 2001-2022 by Bill Spitzak and others.
// Original code Copyright Mark Edel.  Permission to distribute under
// the LGPL for the FLTK library granted by Mark Edel.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file. If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

// TODO: rendering of the "optional hyphen"
// TODO: font background color control via style buffer

#include <stdio.h>
#include <stdlib.h>
#include <FL/fl_utf8.h>
#include <FL/fl_string_functions.h>     // fl_strdup()
#include "flstring.h"
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Input.H>
#include "Fl_Screen_Driver.H"

#undef min
#undef max

// #define DEBUG
// #define DEBUG2

#define LINENUM_LEFT_OF_VSCROLL         // uncomment this line ...
// ... if you want the line numbers to be drawn left of the vertical
// scrollbar (only if the vertical scrollbar is aligned left).
// This is the default.
// If not defined and the vertical scrollbar is aligned left, then the
// scrollbar is positioned at the left border and the line numbers are
// drawn between the scrollbar (left) and the text area (right).
// If the vertical scrollbar is aligned right, then the line number
// position is not affected by this definition.

// Text area margins.  Left & right margins should be at least 3 so that
// there is some room for the overhanging parts of the cursor!
#define TOP_MARGIN 1
#define BOTTOM_MARGIN 1
#define LEFT_MARGIN 3
#define RIGHT_MARGIN 3

#define NO_HINT -1

/* Masks for text drawing methods.  These are or'd together to form an
 integer which describes what drawing calls to use to draw a string */
#define FILL_MASK         0x0100
#define SECONDARY_MASK    0x0200
#define PRIMARY_MASK      0x0400
#define HIGHLIGHT_MASK    0x0800
#define BG_ONLY_MASK      0x1000
#define TEXT_ONLY_MASK    0x2000
#define STYLE_LOOKUP_MASK   0xff

/* Maximum displayable line length (how many characters will fit across the
 widest window).  This amount of memory is temporarily allocated from the
 stack in the draw_vline() method for drawing strings */
#define MAX_DISP_LINE_LEN 1000

static int max( int i1, int i2 );
static int min( int i1, int i2 );
static int countlines( const char *string );

/* The variables below are used in a timer event to allow smooth
 scrolling of the text area when the pointer has left the area. */
static int scroll_direction = 0;
static int scroll_amount = 0;
static int scroll_y = 0;
static int scroll_x = 0;

static Fl_Menu_Item rmb_menu[] = {
  { NULL, 0, NULL, (void*)1 },
  { NULL, 0, NULL, (void*)2 },
  { NULL, 0, NULL, (void*)3 },
  { NULL }
};

// CET - FIXME
#define TMPFONTWIDTH 6



/**
 \brief Creates a new text display widget.

 \param X, Y, W, H position and size of widget
 \param l label text, defaults to none
 */
Fl_Text_Display::Fl_Text_Display(int X, int Y, int W, int H, const char* l)
: Fl_Group(X, Y, W, H, l) {

#define VISIBLE_LINES_INIT 1 // allow compiler to remove unused code (PR #582)

  // Member initialization: same order as declared in .H file
  //    Any Fl_Text_Display methods should only be called /after/ all
  //    members initialized; avoids methods referencing uninitialized values.
  //
  damage_range1_start = damage_range1_end = -1;
  damage_range2_start = damage_range2_end = -1;
  mCursorPos = 0;
  mCursorOn = 0;
  mCursorOldY = -100;
  mCursorToHint = NO_HINT;
  mCursorStyle = NORMAL_CURSOR;
  mCursorPreferredXPos = -1;
  mNVisibleLines = VISIBLE_LINES_INIT;
  mNBufferLines = 0;
  mBuffer = NULL;
  mStyleBuffer = NULL;
  mFirstChar = 0;
  mLastChar = 0;
  mContinuousWrap = 0;
  mWrapMarginPix = 0;
  mLineStarts = new int[mNVisibleLines];
#if VISIBLE_LINES_INIT > 1
  { // Note: this code is unused unless mNVisibleLines is ever initialized > 1
    for (int i=1; i<mNVisibleLines; i++) mLineStarts[i] = -1;
  }
#endif
  mLineStarts[0] = 0;
  mTopLineNum = 1;
  mAbsTopLineNum = 1;
  mNeedAbsTopLineNum = 0;
  mHorizOffset = 0;
  mTopLineNumHint = 1;
  mHorizOffsetHint = 0;
  mNStyles = 0;
  mStyleTable = NULL;
  mUnfinishedStyle = 0;
  mUnfinishedHighlightCB = 0;
  mHighlightCBArg = 0;
  mMaxsize = 0;
  mSuppressResync = 0;
  mNLinesDeleted = 0;
  mModifyingTabDistance = 0;    // XXX: UNUSED
  mColumnScale = 0;
  mCursor_color = FL_FOREGROUND_COLOR;

  mHScrollBar = new Fl_Scrollbar(0,0,1,1);
  mHScrollBar->callback((Fl_Callback*)h_scrollbar_cb, this);
  mHScrollBar->type(FL_HORIZONTAL);

  mVScrollBar = new Fl_Scrollbar(0,0,1,1);
  mVScrollBar->callback((Fl_Callback*)v_scrollbar_cb, this);

  display_needs_recalc_ = false;

  scrollbar_width_ = 0;         // 0: default from Fl::scrollbar_size()
  scrollbar_align_ = FL_ALIGN_BOTTOM_RIGHT;

  dragPos = 0;
  dragType = DRAG_CHAR;
  dragging = 0;
  display_insert_position_hint = 0;

  text_area.x = 0;
  text_area.y = 0;
  text_area.w = 0;
  text_area.h = 0;

  shortcut_ = 0;
  textfont_ = FL_HELVETICA;             // textfont()
  textsize_ = FL_NORMAL_SIZE;           // textsize()
  textcolor_ = FL_FOREGROUND_COLOR;     // textcolor()
  grammar_underline_color_ = FL_BLUE;
  spelling_underline_color_ = FL_RED;
  secondary_selection_color_ = FL_GRAY;
  mLineNumLeft = 0;             // XXX: UNUSED
  mLineNumWidth = 0;

  linenumber_font_    = FL_HELVETICA;
  linenumber_size_    = FL_NORMAL_SIZE;
  linenumber_fgcolor_ = FL_INACTIVE_COLOR;
  linenumber_bgcolor_ = 53;     // ~90% gray
  linenumber_align_   = FL_ALIGN_RIGHT;
  linenumber_format_  = fl_strdup("%d");

  // Method calls -- only AFTER all members initialized
  color(FL_BACKGROUND2_COLOR, FL_SELECTION_COLOR);
  box(FL_DOWN_FRAME);
  set_flag(SHORTCUT_LABEL);
  clear_flag(NEEDS_KEYBOARD);

  end();
}


/**
  Free a text display and release its associated memory.

  \note The text buffer that the text display displays is a separate entity
        and is not freed, nor are the style buffer or style table.

  \see Fl_Text_Display::buffer(Fl_Text_Buffer* buf)
*/

Fl_Text_Display::~Fl_Text_Display() {
  if (scroll_direction) {
    Fl::remove_timeout(scroll_timer_cb, this);
    scroll_direction = 0;
  }
  if (mBuffer) {
    mBuffer->remove_modify_callback(buffer_modified_cb, this);
    mBuffer->remove_predelete_callback(buffer_predelete_cb, this);
  }
  if (mLineStarts) delete[] mLineStarts;
  if (linenumber_format_) {
    free((void*)linenumber_format_);
    linenumber_format_ = 0;
  }
}


/**
  Set width of screen area for line numbers.
  Use to also enable/disable line numbers.
  A value of 0 disables line numbering, values >0 enable the line number display.
  \param width The new width of the area for line numbers to appear, in pixels.
              0 disables line numbers (default)
*/
void Fl_Text_Display::linenumber_width(int width) {
  if (width < 0) return;
  mLineNumWidth = width;
  display_needs_recalc(); // recalc line#s        // resize(x(), y(), w(), h());
  if (width > 0) reset_absolute_top_line_number();
}

/**
 Return the screen area width provided for line numbers.
*/
int Fl_Text_Display::linenumber_width() const {
  return mLineNumWidth;
}

/**
 Set the font used for line numbers (if enabled).
 \version 1.3.3
*/
void Fl_Text_Display::linenumber_font(Fl_Font val) {
  linenumber_font_ = val;
}

/**
 Return the font used for line numbers (if enabled).
*/
Fl_Font Fl_Text_Display::linenumber_font() const {
  return linenumber_font_;
}

/**
 Set the font size used for line numbers (if enabled).
 \version 1.3.3
*/
void Fl_Text_Display::linenumber_size(Fl_Fontsize val) {
  linenumber_size_ = val;
}

/**
 Return the font size used for line numbers (if enabled).
*/
Fl_Fontsize Fl_Text_Display::linenumber_size() const {
  return linenumber_size_;
}

/**
 Set the foreground color used for line numbers (if enabled).
 \version 1.3.3
*/
void Fl_Text_Display::linenumber_fgcolor(Fl_Color val) {
  linenumber_fgcolor_ = val;
}

/**
 Return the foreground color used for line numbers (if enabled).
*/
Fl_Color Fl_Text_Display::linenumber_fgcolor() const {
  return linenumber_fgcolor_;
}

/**
 Set the background color used for line numbers (if enabled).
 \version 1.3.3
*/
void Fl_Text_Display::linenumber_bgcolor(Fl_Color val) {
  linenumber_bgcolor_ = val;
}

/**
 Returns the background color used for line numbers (if enabled).
*/
Fl_Color Fl_Text_Display::linenumber_bgcolor() const {
  return linenumber_bgcolor_;
}

/**
 Set alignment for line numbers (if enabled).
 Valid values are FL_ALIGN_LEFT, FL_ALIGN_CENTER or FL_ALIGN_RIGHT.
 \version 1.3.3
*/
void Fl_Text_Display::linenumber_align(Fl_Align val) {
  linenumber_align_ = val;
}

/**
 Returns the alignment used for line numbers (if enabled).
*/
Fl_Align Fl_Text_Display::linenumber_align() const {
  return linenumber_align_;
}

/**
 Sets the printf() style format string used for line numbers.
 Default is "%d" for normal unpadded decimal integers.

 An internal copy of \p val is allocated and managed;
 it is automatically freed whenever a new value is assigned,
 or when the widget is destroyed.

 The value of \p val must \a not be NULL.

 Example values:

     - "%d"   -- For normal line numbers without padding (Default)
     - "%03d" -- For 000 padding
     - "%x"   -- For hexadecimal line numbers
     - "%o"   -- For octal line numbers

 \version 1.3.3
*/
void Fl_Text_Display::linenumber_format(const char* val) {
  if ( linenumber_format_ ) free((void*)linenumber_format_);
  linenumber_format_ = val ? fl_strdup(val) : 0;
}

/**
 Returns the line number printf() format string.
*/
const char* Fl_Text_Display::linenumber_format() const {
  return linenumber_format_;
}

/**
  Attach a text buffer to display, replacing the current buffer (if any).

  Multiple text widgets can be associated with the same text buffer.

  \note The caller is responsible for the old (replaced) buffer (if any).
        This method does not delete the old buffer.

  \param buf attach this text buffer
*/
void Fl_Text_Display::buffer( Fl_Text_Buffer *buf ) {
  /* If the text display is already displaying a buffer, clear it off
   of the display and remove our callback from it */
  if ( buf == mBuffer) return;
  if ( mBuffer != 0 ) {
    // we must provide a copy of the buffer that we are deleting!
    char *deletedText = mBuffer->text();
    buffer_modified_cb( 0, 0, mBuffer->length(), 0, deletedText, this );
    free(deletedText);
    mNBufferLines = 0;
    mBuffer->remove_modify_callback( buffer_modified_cb, this );
    mBuffer->remove_predelete_callback( buffer_predelete_cb, this );
  }

  /* Add the buffer to the display, and attach a callback to the buffer for
   receiving modification information when the buffer contents change */
  mBuffer = buf;
  if (mBuffer) {
    mBuffer->add_modify_callback( buffer_modified_cb, this );
    mBuffer->add_predelete_callback( buffer_predelete_cb, this );

    /* Update the display */
    buffer_modified_cb( 0, buf->length(), 0, 0, 0, this );
  }

  /* Resize the widget to update the screen... */
  display_needs_recalc(); // resize(x(), y(), w(), h());
}



/**
 \brief Attach (or remove) highlight information in text display and redisplay.

 Highlighting information consists of a style buffer which parallels the
 normal text buffer, but codes font and color information for the display;
 a style table which translates style buffer codes (indexed by buffer
 character - 'A') into fonts and colors; and a callback mechanism for
 as-needed highlighting, triggered by a style buffer entry of
 "unfinishedStyle".  Style buffer can trigger additional redisplay during
 a normal buffer modification if the buffer contains a primary Fl_Text_Selection
 (see extend_range_for_styles() for more information on this protocol).

 Style buffers, tables and their associated memory are managed by the caller.

 Styles are ranged from 65 ('A') to 126.

 \note Style information in the style buffer must have the same byte offset as
 the corresponding character in the text buffer. UTF-8 characters can have a
 maximum length of four bytes. Style information must take
 this into account and fill the unused bytes with 0. See `fl_utf8len()`.

 Text: "*g* r &uuml; *n*" , where normal style is 'A', and bold is 'B'
 \code
 Text Buffer(hex):  67 72 c3 bc 6e : gr..n
 Style Buffer(hex): 42 41 41 00 42 : BAA.B
 \endcode

 \param styleBuffer this buffer works in parallel to the text buffer. For every
   character in the text buffer, the style buffer has a byte at the same offset
   that contains an index into an array of possible styles.
 \param styleTable a list of styles indexed by the style buffer
 \param nStyles number of styles in the style table
 \param unfinishedStyle if this style is found, the callback below is called
 \param unfinishedHighlightCB if a character with an unfinished style is found,
   this callback will be called
 \param cbArg an optional argument for the callback above, usually a pointer
   to the Text Display.

 \see Fl_Text_Display::style_buffer()
 */
void Fl_Text_Display::highlight_data(Fl_Text_Buffer *styleBuffer,
                                     const Style_Table_Entry *styleTable,
                                     int nStyles, char unfinishedStyle,
                                     Unfinished_Style_Cb unfinishedHighlightCB,
                                     void *cbArg ) {
  mStyleBuffer = styleBuffer;
  mStyleTable = styleTable;
  mNStyles = nStyles;
  mUnfinishedStyle = unfinishedStyle;
  mUnfinishedHighlightCB = unfinishedHighlightCB;
  mHighlightCBArg = cbArg;
  mColumnScale = 0;

  if (mStyleBuffer)
    mStyleBuffer->canUndo(0);
  damage(FL_DAMAGE_EXPOSE);
}

/**
 \brief Find the longest line of all visible lines.

 \return the width of the longest visible line in pixels
 */
int Fl_Text_Display::longest_vline() const {
  int longest = 0;
  for (int i = 0; i < mNVisibleLines; i++)
    longest = max(longest, measure_vline(i));
  return longest;
}

/**
 \brief Change the size of the displayed text area.

 Calling this function will trigger a recalculation of all visible lines
 and of all scrollbar sizes.
 \param X, Y, W, H new position and size of this widget
 */
void Fl_Text_Display::resize(int X, int Y, int W, int H) {

#ifdef DEBUG2
  printf("\n");
  printf("Fl_Text_Display::resize(X=%d, Y=%d, W=%d, H=%d)\n", X, Y, W, H);
  printf("           current size(x=%d, y=%d, w=%d, h=%d)\n", x(), y(), w(), h());
  printf("            box_d* size(x=%d, y=%d, w=%d, h=%d)\n",
         Fl::box_dx(box()),Fl::box_dy(box()),Fl::box_dw(box()),Fl::box_dh(box()));
  printf("         text_area size(x=%d, y=%d, w=%d, h=%d)\n",
         text_area.x, text_area.y, text_area.w, text_area.h);
  printf("    mContinuousWrap=%d, mWrapMarginPix=%d\n",
              mContinuousWrap, mWrapMarginPix);
  fflush(stdout);
#endif // DEBUG2

  Fl_Widget::resize(X,Y,W,H);
  mColumnScale = 0; // force recomputation of the width of a column when display is rescaled
  display_needs_recalc();
}

/**
  Schedule a recalc_display() to be done on next draw().
  Call this from methods that might be called repeatedly, to defers potentially
  CPU intensive recalc_display() until it's actually needed just before draw().
*/
void Fl_Text_Display::display_needs_recalc() {
  display_needs_recalc_ = true;
  redraw(); // ensure draw() gets called
}

/**
  Recalculate the display's visible lines and scrollbar sizes.
  Beware calling this directly may cause a lot of CPU if called repeatedly (issue 300).
  Better to call display_needs_recalc() to flag a recalc to be done during next draw().
 */
void Fl_Text_Display::recalc_display() {
  if (!buffer()) return;
  // Make sure the display is opened.
  Fl_Display_Device::display_device();
  // did we have scrollbars initially?
  unsigned int hscrollbarvisible = mHScrollBar->visible();
  unsigned int vscrollbarvisible = mVScrollBar->visible();
  int scrollsize = scrollbar_width_ ? scrollbar_width_ : Fl::scrollbar_size();

  int X = x() + Fl::box_dx(box());
  int Y = y() + Fl::box_dy(box());
  int W = w() - Fl::box_dw(box());
  int H = h() - Fl::box_dh(box());

  text_area.x = X + LEFT_MARGIN + mLineNumWidth;
  text_area.y = Y + TOP_MARGIN;
  text_area.w = W - LEFT_MARGIN - RIGHT_MARGIN - mLineNumWidth;
  text_area.h = H - TOP_MARGIN - BOTTOM_MARGIN;

  // Find the new maximum font height for this text display
  int i;
  for (i = 0, mMaxsize = fl_height(textfont(), textsize()); i < mNStyles; i++)
    mMaxsize = max(mMaxsize, fl_height(mStyleTable[i].font, mStyleTable[i].size));

  // try without scrollbars first
  mVScrollBar->clear_visible();
  mHScrollBar->clear_visible();

  // Optimization: if the number of lines in the buffer does not fit in
  // the display area, then we need a vertical scrollbar regardless of
  // word wrapping. If we switch it on here, this saves one line counting
  // run in wrap mode in the loop below ("... again ..."). This is important
  // for large buffers that suffer from slow calculations of character width
  // to determine line wrapping.
  // Note: active since Oct 25, 2017: commit eb772d027d (svn r12526)

  // force _first_ calculation in loop (STR #3412)
  int oldTAWidth = -1; // was: text_area.w (before STR #3412)

  if (mContinuousWrap && !mWrapMarginPix) {

    int nvlines = (text_area.h + mMaxsize - 1) / mMaxsize;
    int nlines = buffer()->count_lines(0,buffer()->length());
    if (nvlines < 1) nvlines = 1;
    if (nlines >= nvlines-1) {
      mVScrollBar->set_visible(); // we need a vertical scrollbar
      text_area.w -= scrollsize;
    }
  }
  // End of optimization, see comment above.

  for (int again = 1; again;) {
    again = 0;
    /* In continuous wrap mode, a change in width affects the total number of
     lines in the buffer, and can leave the top line number incorrect, and
     the top character no longer pointing at a valid line start */

#ifdef DEBUG2
     printf("*** again ... text_area.w = %d, oldTAWidth = %d, diff = %d\n",
              text_area.w, oldTAWidth, text_area.w - oldTAWidth);
#endif // DEBUG2

    if (mContinuousWrap && !mWrapMarginPix && text_area.w != oldTAWidth) {

      int oldFirstChar = mFirstChar;
      mFirstChar = line_start(mFirstChar);
      mTopLineNum = count_lines(0, mFirstChar, true)+1;
      mNBufferLines = mTopLineNum-1 + count_lines(mFirstChar, buffer()->length(), true);
      absolute_top_line_number(oldFirstChar);
#ifdef DEBUG2
      printf("    mNBufferLines=%d\n", mNBufferLines);
#endif // DEBUG2

    }

    oldTAWidth = text_area.w;

    /* reallocate and update the line starts array, which may have changed
     size and / or contents.  */
    int nvlines = (text_area.h + mMaxsize - 1) / mMaxsize;
    if (nvlines < 1) nvlines = 1;
    if (mNVisibleLines != nvlines) {
      mNVisibleLines = nvlines;
      if (mLineStarts) delete[] mLineStarts;
      mLineStarts = new int [mNVisibleLines];
    }

    calc_line_starts(0, mNVisibleLines);
    calc_last_char();

    // figure the scrollbars
    if (scrollsize) {

      /* Decide if the vertical scrollbar needs to be visible */
      if (!mVScrollBar->visible() &&
          scrollbar_align() & (FL_ALIGN_LEFT|FL_ALIGN_RIGHT) &&
          mNBufferLines >= mNVisibleLines-(mContinuousWrap?0:1))
      {
        mVScrollBar->set_visible();
        text_area.w -= scrollsize;
        again = 1;
      }

      /*
       Decide if the horizontal scrollbar needs to be visible. If the text
       wraps at the right edge, do not draw a horizontal scrollbar. Otherwise, if there
       is a vertical scrollbar, a horizontal is always created too.  This
       is because the alternatives are unattractive:
       * Dynamically creating a horizontal scrollbar based on the currently
       visible lines is what the original nedit does, but it always wastes
       space for the scrollbar even when it's not used.  Since the FLTK
       widget dynamically allocates the space for the scrollbar and
       rearranges the widget to make room for it, this would create a very
       visually displeasing "bounce" effect when the vertical scrollbar is
       dragged.  Trust me, I tried it and it looks really bad.
       * The other alternative would be to keep track of what the longest
       line in the entire buffer is and base the scrollbar on that.  I
       didn't do this because I didn't see any easy way to do that using
       the nedit code and this could involve a lengthy calculation for
       large buffers.  If an efficient and non-costly way of doing this
       can be found, this might be a way to go.
       */
      /* WAS: Suggestion: Try turning the horizontal scrollbar on when
       you first see a line that is too wide in the window, but then
       don't turn it off (ie mix both of your solutions). */

      if (!mHScrollBar->visible() &&
          scrollbar_align() & (FL_ALIGN_TOP|FL_ALIGN_BOTTOM) &&
          (mVScrollBar->visible() || longest_vline() > text_area.w))
      {
        char wrap_at_bounds = mContinuousWrap && (mWrapMarginPix<text_area.w);
        if (!wrap_at_bounds) {
          mHScrollBar->set_visible();
          text_area.h -= scrollsize;
          again = 1; // loop again to see if we now need vert. & recalc sizes
        }
      }
    }
  } // (... again ...)

  // Calculate text area position, dependent on scrollbars and line numbers.
  // Note: width and height have been calculated above.
  text_area.x = X + mLineNumWidth + LEFT_MARGIN;
  if (mVScrollBar->visible() && scrollbar_align() & FL_ALIGN_LEFT)
    text_area.x += scrollsize;

  text_area.y = Y + TOP_MARGIN;
  if (mHScrollBar->visible() &&
      scrollbar_align() & FL_ALIGN_TOP)
    text_area.y += scrollsize;

  // position and resize scrollbars
  if (mVScrollBar->visible()) {
    if (scrollbar_align() & FL_ALIGN_LEFT) {
#ifdef LINENUM_LEFT_OF_VSCROLL
      mVScrollBar->resize(text_area.x - LEFT_MARGIN - scrollsize,
                          text_area.y - TOP_MARGIN,
                          scrollsize,
                          text_area.h + TOP_MARGIN + BOTTOM_MARGIN);
#else
      mVScrollBar->resize(X,
                          text_area.y - TOP_MARGIN,
                          scrollsize,
                          text_area.h + TOP_MARGIN + BOTTOM_MARGIN);
#endif
    } else {
      mVScrollBar->resize(X+W-scrollsize,
                          text_area.y - TOP_MARGIN,
                          scrollsize,
                          text_area.h + TOP_MARGIN + BOTTOM_MARGIN);
    }
  }

  if (mHScrollBar->visible()) {
    if (scrollbar_align() & FL_ALIGN_TOP) {
      mHScrollBar->resize(text_area.x - LEFT_MARGIN,
                          Y,
                          text_area.w + LEFT_MARGIN + RIGHT_MARGIN,
                          scrollsize);
    } else {
      mHScrollBar->resize(text_area.x - LEFT_MARGIN,
                          Y + H - scrollsize,
                          text_area.w + LEFT_MARGIN + RIGHT_MARGIN,
                          scrollsize);
    }
  }


  // user request to change viewport
  if (mTopLineNumHint != mTopLineNum || mHorizOffsetHint != mHorizOffset)
    scroll_(mTopLineNumHint, mHorizOffsetHint);

  // everything will fit in the viewport
  if ((mNBufferLines+1 < mNVisibleLines) || (mBuffer == NULL) || (mBuffer->length() == 0)) {
    scroll_(1, mHorizOffset);
  /* if empty lines become visible, there may be an opportunity to
   display more text by scrolling down */
  } else {
    while (   mNVisibleLines>=2
           && (mLineStarts[mNVisibleLines-2]==-1)
           && scroll_(mTopLineNum-1, mHorizOffset))
    { }
  }

  // user request to display insert position
  if (display_insert_position_hint)
    display_insert();

  // in case horizontal offset is now greater than longest line
  int maxhoffset = max(0, longest_vline()-text_area.w);
  if (mHorizOffset > maxhoffset)
    scroll_(mTopLineNumHint, maxhoffset);

  mTopLineNumHint = mTopLineNum;
  mHorizOffsetHint = mHorizOffset;
  display_insert_position_hint = 0;

  if (mContinuousWrap ||
      hscrollbarvisible != mHScrollBar->visible() ||
      vscrollbarvisible != mVScrollBar->visible())
    redraw();

  update_v_scrollbar();
  update_h_scrollbar();
}


/**
 \brief Refresh a rectangle of the text display.
 \param left, top are in coordinates of the text drawing window.
 \param width, height size in pixels
 */
void Fl_Text_Display::draw_text( int left, int top, int width, int height ) {
  int fontHeight, firstLine, lastLine, line;

  /* find the line number range of the display */
  fontHeight = mMaxsize ? mMaxsize : textsize_;
  firstLine = ( top - text_area.y - fontHeight + 1 ) / fontHeight;
  lastLine = ( top + height - text_area.y ) / fontHeight + 1;

  fl_push_clip( left, top, width, height );

  /* draw the lines */
  for ( line = firstLine; line <= lastLine; line++ )
    draw_vline( line, left, left + width, 0, INT_MAX );

  fl_pop_clip();
}



/**
 \brief Marks text from start to end as needing a redraw.

 This function will trigger a damage event and later a redraw of parts of
 the widget.
 \param startpos index of first character needing redraw
 \param endpos index after last character needing redraw
 */
void Fl_Text_Display::redisplay_range(int startpos, int endpos) {
  IS_UTF8_ALIGNED2(buffer(), startpos)
  IS_UTF8_ALIGNED2(buffer(), endpos)

  if (damage_range1_start == -1 && damage_range1_end == -1) {
    damage_range1_start = startpos;
    damage_range1_end = endpos;
  } else if ((startpos >= damage_range1_start && startpos <= damage_range1_end) ||
             (endpos >= damage_range1_start && endpos <= damage_range1_end)) {
    damage_range1_start = min(damage_range1_start, startpos);
    damage_range1_end = max(damage_range1_end, endpos);
  } else if (damage_range2_start == -1 && damage_range2_end == -1) {
    damage_range2_start = startpos;
    damage_range2_end = endpos;
  } else {
    damage_range2_start = min(damage_range2_start, startpos);
    damage_range2_end = max(damage_range2_end, endpos);
  }
  damage(FL_DAMAGE_SCROLL);
}



/**
 \brief Draw a range of text.

 Refresh all of the text between buffer positions \p startpos and
 \p endpos not including the character at the position \p endpos.

 If \p endpos points beyond the end of the buffer, refresh the whole display
 after \p startpos, including blank lines which are not technically part of
 any range of characters.

 \param startpos index of first character to draw
 \param endpos index after last character to draw
 */
void Fl_Text_Display::draw_range(int startpos, int endpos) {
  startpos = buffer()->utf8_align(startpos);
  endpos = buffer()->utf8_align(endpos);

  int i, startLine, lastLine, startIndex, endIndex;

  /* If the range is outside of the displayed text, just return */
  if ( endpos < mFirstChar || ( startpos > mLastChar && !empty_vlines() ) )
    return;

  /* Clean up the starting and ending values */
  if ( startpos < 0 ) startpos = 0;
  if ( startpos > mBuffer->length() ) startpos = mBuffer->length();
  if ( endpos < 0 ) endpos = 0;
  if ( endpos > mBuffer->length() ) endpos = mBuffer->length();

  /* Get the starting and ending lines */
  if ( startpos < mFirstChar )
    startpos = mFirstChar;
  if ( !position_to_line( startpos, &startLine ) )
    startLine = mNVisibleLines - 1;
  if ( endpos >= mLastChar ) {
    lastLine = mNVisibleLines - 1;
  } else {
    if ( !position_to_line( endpos, &lastLine ) ) {
      /* shouldn't happen */
      lastLine = mNVisibleLines - 1;
    }
  }

  /* Get the starting and ending positions within the lines */
  startIndex = mLineStarts[ startLine ] == -1 ? 0 : startpos - mLineStarts[ startLine ];
  if ( endpos >= mLastChar )
    endIndex = INT_MAX;
  else if ( mLineStarts[ lastLine ] == -1 )
    endIndex = 0;
  else
    endIndex = endpos - mLineStarts[ lastLine ];

  /* If the starting and ending lines are the same, redisplay the single
   line between "start" and "end" */
  if ( startLine == lastLine ) {
    draw_vline( startLine, 0, INT_MAX, startIndex, endIndex );
    return;
  }

  /* Redisplay the first line from "start" */
  draw_vline( startLine, 0, INT_MAX, startIndex, INT_MAX );

  /* Redisplay the lines in between at their full width */
  for ( i = startLine + 1; i < lastLine; i++ )
    draw_vline( i, 0, INT_MAX, 0, INT_MAX );

  /* Redisplay the last line to "end" */
  draw_vline( lastLine, 0, INT_MAX, 0, endIndex );
}



/**
 \brief Sets the position of the text insertion cursor for text display.

 Moves the insertion cursor in front of the character at \p newPos.
 This function may trigger a redraw.
 \param newPos new caret position
 */
void Fl_Text_Display::insert_position( int newPos ) {
  IS_UTF8_ALIGNED2(buffer(), newPos)

  /* make sure new position is ok, do nothing if it hasn't changed */
  if ( newPos == mCursorPos ) return;
  if ( newPos < 0 ) newPos = 0;
  if ( newPos > mBuffer->length() ) newPos = mBuffer->length();

  /* cursor movement cancels vertical cursor motion column */
  mCursorPreferredXPos = -1;

  /* erase the cursor at its previous position */
  redisplay_range(buffer()->prev_char_clipped(mCursorPos), buffer()->next_char(mCursorPos));

  mCursorPos = newPos;

  /* draw cursor at its new position */
  redisplay_range(buffer()->prev_char_clipped(mCursorPos), buffer()->next_char(mCursorPos));
}



/**
 \brief Shows the text cursor.

 This function may trigger a redraw.
 \param b show(1) or hide(0) the text cursor (caret).
 */
void Fl_Text_Display::show_cursor(int b) {
  mCursorOn = b;
  if (!buffer()) return;
  redisplay_range(buffer()->prev_char_clipped(mCursorPos), buffer()->next_char(mCursorPos));
}



/**
 \brief Sets the text cursor style.

 Sets the text cursor style to one of the following:

 \li Fl_Text_Display::NORMAL_CURSOR - Shows an I beam.
 \li Fl_Text_Display::CARET_CURSOR - Shows a caret under the text.
 \li Fl_Text_Display::DIM_CURSOR - Shows a dimmed I beam.
 \li Fl_Text_Display::BLOCK_CURSOR - Shows an unfilled box around the current
      character.
 \li Fl_Text_Display::HEAVY_CURSOR - Shows a thick I beam.

 This call also switches the cursor on and may trigger a redraw.

 \param style new cursor style
 */
void Fl_Text_Display::cursor_style(int style) {
  mCursorStyle = style;
  if (mCursorOn) show_cursor();
}



/**
 \brief Set the new text wrap mode.

 If \p wrap mode is not zero, this call enables automatic word wrapping at column
 \p wrapMargin. Word-wrapping does not change the text buffer itself, only the way
 the text is displayed. Different Text Displays can have different wrap modes,
 even if they share the same Text Buffer.

 Valid wrap modes are:

  - WRAP_NONE :         don't wrap text at all
  - WRAP_AT_COLUMN :    wrap text at the given text column
  - WRAP_AT_PIXEL :     wrap text at a pixel position
  - WRAP_AT_BOUNDS :    wrap text so that it fits into the widget width

 \param wrap new wrap mode (see above)

 \param wrapMargin in WRAP_AT_COLUMN mode, text will wrap at the n'th character.
      For variable width fonts, an average character width is calculated. The
      column width is calculated using the current textfont or the first style
      when this function is called. If the font size changes, this function
      must be called again. In WRAP_AT_PIXEL mode, this is the pixel position.
 */
void Fl_Text_Display::wrap_mode(int wrap, int wrapMargin) {
  switch (wrap) {
    case WRAP_NONE:
      mWrapMarginPix = 0;
      mContinuousWrap = 0;
      break;
    case WRAP_AT_COLUMN:
    default:
      mWrapMarginPix = int(col_to_x(wrapMargin));
      mContinuousWrap = 1;
      break;
    case WRAP_AT_PIXEL:
      mWrapMarginPix = wrapMargin;
      mContinuousWrap = 1;
      break;
    case WRAP_AT_BOUNDS:
      mWrapMarginPix = 0;
      mContinuousWrap = 1;
      break;
  }

  if (buffer()) {
    /* wrapping can change the total number of lines, re-count */
    mNBufferLines = count_lines(0, buffer()->length(), true);

    /* changing wrap margins or changing from wrapped mode to non-wrapped
     can leave the character at the top no longer at a line start, and/or
     change the line number */
    mFirstChar = line_start(mFirstChar);
    mTopLineNum = count_lines(0, mFirstChar, true) + 1;

    reset_absolute_top_line_number();

    /* update the line starts array */
    calc_line_starts(0, mNVisibleLines);
    calc_last_char();
  } else {
    // No buffer, so just clear the state info for later...
    mNBufferLines  = 0;
    mFirstChar     = 0;
    mTopLineNum    = 1;
    mAbsTopLineNum = 1;         // changed from 0 to 1 -- LZA / STR#2621
  }

  display_needs_recalc();             // resize(x(), y(), w(), h());
}


/**
 \brief Inserts "text" at the current cursor location.

 This has the same effect as inserting the text into the buffer using
 insert(insert_position(),text) and then moving the insert position after
 the newly inserted text, except that it's optimized to do less redrawing.

 \param text new text in UTF-8 encoding.
 */
void Fl_Text_Display::insert(const char* text) {
  IS_UTF8_ALIGNED2(buffer(), mCursorPos)
  IS_UTF8_ALIGNED(text)

  int pos = mCursorPos;

  mCursorToHint = (int) (pos + strlen( text ));
  mBuffer->insert( pos, text );
  mCursorToHint = NO_HINT;
}



/**
 \brief Replaces text at the current insert position.
 \param text new text in UTF-8 encoding

 \todo Unicode? Find out exactly what we do here and simplify.
 */
void Fl_Text_Display::overstrike(const char* text) {
  IS_UTF8_ALIGNED2(buffer(), mCursorPos)
  IS_UTF8_ALIGNED(text)

  int startPos = mCursorPos;
  Fl_Text_Buffer *buf = mBuffer;
  int lineStart = buf->line_start( startPos );
  int textLen = (int) strlen( text );
  int i, p, endPos, indent, startIndent, endIndent;
  const char *c;
  unsigned int ch;
  char *paddedText = NULL;

  /* determine how many displayed character positions are covered */
  startIndent = mBuffer->count_displayed_characters( lineStart, startPos );
  indent = startIndent;
  for ( c = text; *c != '\0'; c += fl_utf8len1(*c) )
    indent++;
  endIndent = indent;

  /* find which characters to remove, and if necessary generate additional
   padding to make up for removed control characters at the end */
  indent = startIndent;
  for ( p = startPos; ; p = buf->next_char(p) ) {
    if ( p == buf->length() )
      break;
    ch = buf->char_at( p );
    if ( ch == '\n' )
      break;
    indent++;
    if ( indent == endIndent ) {
      p = buf->next_char(p);
      break;
    } else if ( indent > endIndent ) {
      if ( ch != '\t' ) {
        p = buf->next_char(p);
        paddedText = new char [ textLen + FL_TEXT_MAX_EXP_CHAR_LEN + 1 ];
        strcpy( paddedText, text );
        for ( i = 0; i < indent - endIndent; i++ )
          paddedText[ textLen + i ] = ' ';
        paddedText[ textLen + i ] = '\0';
      }
      break;
    }
  }
  endPos = p;

  mCursorToHint = startPos + textLen;
  buf->replace( startPos, endPos, paddedText == NULL ? text : paddedText );
  mCursorToHint = NO_HINT;
  if ( paddedText != NULL )
    delete [] paddedText;
}



/**
 \brief Convert a character index into a pixel position.

 Translate a buffer text position to the XY location where the top left of the
 cursor would be positioned to point to that character. Returns 0 if the
 position is not displayed because it is \e \b vertically out of view.
 If the position is horizontally out of view, returns the X coordinate where
 the position would be if it were visible.

 \param pos character index
 \param[out] X, Y pixel position of character on screen
 \return 0 if character vertically out of view, X & Y positions otherwise
 */
int Fl_Text_Display::position_to_xy( int pos, int* X, int* Y ) const {
  IS_UTF8_ALIGNED2(buffer(), pos)

  int lineStartPos, fontHeight;
  int visLineNum;
  /* If position is not displayed, return false */
  if ((pos < mFirstChar) ||
      (pos > mLastChar && !empty_vlines()) ||
      (pos > buffer()->length()) ) {            // STR #3231
    return (*X=*Y=0); // make sure X & Y are set when it is out of view
  }

  /* Calculate Y coordinate */
  if (!position_to_line(pos, &visLineNum) || visLineNum < 0 || visLineNum > mNBufferLines) {
    return (*X=*Y=0); // make sure X & Y are set when it is out of view
  }

  fontHeight = mMaxsize;
  *Y = text_area.y + visLineNum * fontHeight;

  /* Get the text, length, and buffer position of the line. If the position
   is beyond the end of the buffer and should be at the first position on
   the first empty line, don't try to get or scan the text  */
  lineStartPos = mLineStarts[visLineNum];
  if ( lineStartPos == -1 ) {
    *X = text_area.x - mHorizOffset;
    return 1;
  }
  *X = text_area.x + handle_vline(GET_WIDTH, lineStartPos, pos-lineStartPos, 0, 0, 0, 0, 0, 0) - mHorizOffset;
  return 1;
}



/**
 \brief Find the line and column number of position \p pos.

 This only works for displayed lines. If the line is not displayed, the
 function returns 0 (without the mLineStarts array it could turn in to very long
 calculation involving scanning large amounts of text in the buffer).
 If continuous wrap mode is on, returns the absolute line number (as opposed
 to the wrapped line number which is used for scrolling).

 \param pos character index
 \param[out] lineNum absolute (unwrapped) line number
 \param[out] column character offset to the beginning of the line
 \return 0 if \p pos is off screen, line number otherwise
 \todo a column number makes little sense in the UTF-8/variable font width
    environment. We will have to further define what exactly we want to return.
    Please check the functions that call this particular function.
 */
int Fl_Text_Display::position_to_linecol( int pos, int* lineNum, int* column ) const {
  IS_UTF8_ALIGNED2(buffer(), pos)

  int retVal;

  /* In continuous wrap mode, the absolute (non-wrapped) line count is
   maintained separately, as needed.  Only return it if we're actually
   keeping track of it and pos is in the displayed text */
  if (mContinuousWrap) {
    if (!maintaining_absolute_top_line_number() || pos < mFirstChar || pos > mLastChar)
      return 0;
    *lineNum = mAbsTopLineNum + buffer()->count_lines(mFirstChar, pos);
    *column = buffer()->count_displayed_characters(buffer()->line_start(pos), pos);
    return 1;
  }

  retVal = position_to_line( pos, lineNum );
  if ( retVal ) {
    *column = mBuffer->count_displayed_characters( mLineStarts[ *lineNum ], pos );
    *lineNum += mTopLineNum;
  }
  return retVal;
}



/**
 \brief Check if a pixel position is within the primary selection.

 \param X, Y pixel position to test
 \return 1 if position (X, Y) is inside of the primary Fl_Text_Selection
 */
int Fl_Text_Display::in_selection( int X, int Y ) const {
  int pos = xy_to_position( X, Y, CHARACTER_POS );
  IS_UTF8_ALIGNED2(buffer(), pos)
  Fl_Text_Buffer *buf = mBuffer;
  return buf->primary_selection()->includes(pos);
}



/**
 \brief Nobody knows what this function does.

 Correct a column number based on an unconstrained position (as returned by
 TextDXYToUnconstrainedPosition) to be relative to the last actual newline
 in the buffer before the row and column position given, rather than the
 last line start created by line wrapping.  This is an adapter
 for rectangular selections and code written before continuous wrap mode,
 which thinks that the unconstrained column is the number of characters
 from the last newline.  Obviously this is time consuming, because it
 invloves character re-counting.

 \param row
 \param column
 \return something unknown

 \todo What does this do and how is it useful? Column numbers mean little in
    this context. Which functions depend on this one?
    Function TextDXYToUnconstrainedPosition does not exist (nedit port?)

 \todo Unicode?
 */
int Fl_Text_Display::wrapped_column(int row, int column) const {
  int lineStart, dispLineStart;

  if (!mContinuousWrap || row < 0 || row > mNVisibleLines)
    return column;
  dispLineStart = mLineStarts[row];
  if (dispLineStart == -1)
    return column;
  lineStart = buffer()->line_start(dispLineStart);
  return column + buffer()->count_displayed_characters(lineStart, dispLineStart);
}



/**
 \brief Nobody knows what this function does.

 Correct a row number from an unconstrained position (as returned by
 TextDXYToUnconstrainedPosition) to a straight number of newlines from the
 top line of the display.  Because rectangular selections are based on
 newlines, rather than display wrapping, and anywhere a rectangular selection
 needs a row, it needs it in terms of un-wrapped lines.

 \param row
 \return something unknown

 \todo  What does this do and how is it useful? Column numbers mean little in
        this context. Which functions depend on this one?
        Function TextDXYToUnconstrainedPosition does not exist (nedit port?)
 */
int Fl_Text_Display::wrapped_row(int row) const {
  if (!mContinuousWrap || row < 0 || row > mNVisibleLines)
    return row;
  return buffer()->count_lines(mFirstChar, mLineStarts[row]);
}



/**
 \brief Scroll the display to bring insertion cursor into view.

 Note: it would be nice to be able to do this without counting lines twice
 (scroll_() counts them too) and/or to count from the most efficient
 starting point, but the efficiency of this routine is not as important to
 the overall performance of the text display.
 */
void Fl_Text_Display::display_insert() {
  int hOffset, topLine, X, Y;
  hOffset = mHorizOffset;
  topLine = mTopLineNum;

  if (insert_position() < mFirstChar) {
    topLine -= count_lines(insert_position(), mFirstChar, false);
  } else if (mNVisibleLines>=2 && mLineStarts[mNVisibleLines-2] != -1) {
    int lastChar = line_end(mLineStarts[mNVisibleLines-2],true);
    if (insert_position() >= lastChar)
      topLine += count_lines(lastChar - (wrap_uses_character(mLastChar) ? 0 : 1),
                             insert_position(), false);
  }

  /* Find the new setting for horizontal offset (this is a bit ungraceful).
   If the line is visible, just use PositionToXY to get the position
   to scroll to, otherwise, do the vertical scrolling first, then the
   horizontal */
  if (!position_to_xy( mCursorPos, &X, &Y )) {
    scroll_(topLine, hOffset);
    if (!position_to_xy( mCursorPos, &X, &Y )) {
#ifdef DEBUG
      printf ("*** display_insert/position_to_xy # GIVE UP !\n"); fflush(stdout);
#endif // DEBUG
      return;   /* Give up, it's not worth it (but why does it fail?) */
    }
  }
  if (X > text_area.x + text_area.w)
    hOffset += X-(text_area.x + text_area.w);
  else if (X < text_area.x)
    hOffset += X-text_area.x;

  /* Do the scroll */
  if (topLine != mTopLineNum || hOffset != mHorizOffset)
    scroll_(topLine, hOffset);
}


/**
 \brief Scrolls the text buffer to show the current insert position.

 This function triggers a complete recalculation, ending in a call to
 Fl_Text_Display::display_insert()
 */
void Fl_Text_Display::show_insert_position() {
  display_insert_position_hint = 1;
  display_needs_recalc();             // resize(x(), y(), w(), h());
}


/*
 Cursor movement functions
 */

/**
 \brief Moves the current insert position right one character.
 \return 1 if the cursor moved, 0 if the end of the text was reached
 */
int Fl_Text_Display::move_right() {
  if ( mCursorPos >= mBuffer->length() )
    return 0;
  int p = insert_position();
  int q = buffer()->next_char(p);
  insert_position(q);
  return 1;
}



/**
 \brief Moves the current insert position left one character.
 \return 1 if the cursor moved, 0 if the beginning of the text was reached
 */
int Fl_Text_Display::move_left() {
  if ( mCursorPos <= 0 )
    return 0;
  int p = insert_position();
  int q = buffer()->prev_char_clipped(p);
  insert_position(q);
  return 1;
}



/**
 \brief Moves the current insert position up one line.
 \return 1 if the cursor moved, 0 if the beginning of the text was reached
 */
int Fl_Text_Display::move_up() {
  int lineStartPos, xPos, prevLineStartPos, newPos, visLineNum;

  /* Find the position of the start of the line.  Use the line starts array
   if possible */
  if ( position_to_line( mCursorPos, &visLineNum ) )
    lineStartPos = mLineStarts[ visLineNum ];
  else {
    lineStartPos = line_start( mCursorPos );
    visLineNum = -1;
  }
  if ( lineStartPos == 0 )
    return 0;

  /* Decide what column to move to, if there's a preferred column use that */
  if (mCursorPreferredXPos >= 0)
    xPos = mCursorPreferredXPos;
  else
    xPos = handle_vline(GET_WIDTH, lineStartPos, mCursorPos-lineStartPos,
                        0, 0, 0, 0, 0, INT_MAX);

  /* count forward from the start of the previous line to reach the column */
  if ( visLineNum != -1 && visLineNum != 0 )
    prevLineStartPos = mLineStarts[ visLineNum - 1 ];
  else
    prevLineStartPos = rewind_lines( lineStartPos, 1 );

  int lineEnd = line_end(prevLineStartPos, true);
  newPos = handle_vline(FIND_INDEX_FROM_ZERO, prevLineStartPos, lineEnd-prevLineStartPos,
                        0, 0, 0, 0, 0, xPos);

  /* move the cursor */
  insert_position( newPos );

  /* if a preferred column wasn't aleady established, establish it */
  mCursorPreferredXPos = xPos;
  return 1;
}



/**
 \brief Moves the current insert position down one line.
 \return 1 if the cursor moved, 0 if the beginning of the text was reached
 */
int Fl_Text_Display::move_down() {
  int lineStartPos, xPos, newPos, visLineNum;

  if ( mCursorPos == mBuffer->length() )
    return 0;

  if ( position_to_line( mCursorPos, &visLineNum ) )
    lineStartPos = mLineStarts[ visLineNum ];
  else {
    lineStartPos = line_start( mCursorPos );
    visLineNum = -1;
  }
  if (mCursorPreferredXPos >= 0) {
    xPos = mCursorPreferredXPos;
  } else {
    xPos = handle_vline(GET_WIDTH, lineStartPos, mCursorPos-lineStartPos,
                        0, 0, 0, 0, 0, INT_MAX);
  }

  int nextLineStartPos = skip_lines( lineStartPos, 1, true );
  int lineEnd = line_end(nextLineStartPos, true);
  newPos = handle_vline(FIND_INDEX_FROM_ZERO, nextLineStartPos, lineEnd-nextLineStartPos,
                        0, 0, 0, 0, 0, xPos);

  insert_position( newPos );
  mCursorPreferredXPos = xPos;
  return 1;
}



/**
 \brief Count the number of lines between two positions.

 Same as Fl_Text_Buffer::count_lines(), but takes into account wrapping if
 wrapping is turned on. If the caller knows that \p startPos is at a line
 start, it can pass \p startPosIsLineStart as True to make the call more
 efficient by avoiding the additional step of scanning back to the last newline.

 \param startPos index to first character
 \param endPos index after last character
 \param startPosIsLineStart avoid scanning back to the line start
 \return number of lines
 */
int Fl_Text_Display::count_lines(int startPos, int endPos,
                                 bool startPosIsLineStart) const {
  IS_UTF8_ALIGNED2(buffer(), startPos)
  IS_UTF8_ALIGNED2(buffer(), endPos)

  int retLines, retPos, retLineStart, retLineEnd;

#ifdef DEBUG
  printf("Fl_Text_Display::count_lines(startPos=%d, endPos=%d, startPosIsLineStart=%d\n",
         startPos, endPos, startPosIsLineStart);
#endif // DEBUG

  /* If we're not wrapping use simple (and more efficient) Fl_Text_Buffer::count_lines() */
  if (!mContinuousWrap)
    return buffer()->count_lines(startPos, endPos);

  /*
   Correctly counting wrapped lines is very slow. We have to query the length
   of every segment of text for every line change and style change and find
   potential soft line breaks.

   Most of the resulting information is needed for calculating the vertical
   scroll bar size. After a certain text length, the scroll bar size is no
   longer very precise anyway, so we optimize line count for all lines but
   the visible ones (plus minus a few lines for rounding).

   The optimized code is several magnitudes faster and makes scrolling and
   window resizing of long texts quite responsive. There is a slight but IMHO
   tollerable drawback: when walking huge files using arrow up and down, the
   text display sometimes jumps 2 or 3 lines instead of 1, but the overall
   buffer stays intact as well as the scroll position.
   */
  if (buffer()->length() > 16384) {
    // Optimized line counting
    int nLines = 0;
    int firstVisibleChar = buffer()->rewind_lines(mFirstChar, 3);
    int lastVisibleChar = buffer()->skip_lines(mLastChar, 3);
    // Calculate the averga number of characters up to a soft line break
    if (mColumnScale==0.0) x_to_col(1.0);
    int avgCharsPerLine = mWrapMarginPix;
    if (!avgCharsPerLine) avgCharsPerLine = text_area.w;
    avgCharsPerLine = (int)(avgCharsPerLine / mColumnScale) + 1;

    // first segment, lines up to display, count fast
    if (startPos < firstVisibleChar) {
      int tmpEnd = endPos<firstVisibleChar ? endPos : firstVisibleChar;
      nLines += buffer()->estimate_lines(startPos, tmpEnd, avgCharsPerLine);
      startPos = tmpEnd;
    }
    // second segement, count displayed liens
    if (startPos < endPos && startPos < mLastChar) {
      // Precisse line counting only for visible text:
      int tmpEnd = endPos<lastVisibleChar ? endPos : lastVisibleChar;
      wrapped_line_counter(buffer(), startPos, tmpEnd, INT_MAX,
                           startPosIsLineStart, 0, &retPos, &retLines, &retLineStart,
                           &retLineEnd);
      nLines += retLines;
      startPos = tmpEnd;
    }
    // third segement is everything after displayed lines
    if (startPos < endPos && startPos >= lastVisibleChar) {
      nLines += buffer()->estimate_lines(startPos, endPos, avgCharsPerLine);
    }
    return nLines;
  } else {
    // Precise line counting only for small text buffer sizes:
    wrapped_line_counter(buffer(), startPos, endPos, INT_MAX,
                         startPosIsLineStart, 0, &retPos, &retLines, &retLineStart,
                         &retLineEnd);

#ifdef DEBUG
    printf("   # after WLC: retPos=%d, retLines=%d, retLineStart=%d, retLineEnd=%d\n",
           retPos, retLines, retLineStart, retLineEnd);
#endif // DEBUG
    return retLines;
  }
}



/**
 \brief Skip a number of lines forward.

 Same as Fl_Text_Buffer::skip_lines(startPos, nLines), but takes into
 account line breaks when wrapping is turned on.
 If the caller knows that \p startPos is at a line start, it can pass
 \p startPosIsLineStart as True to make the call more efficient
 by avoiding the additional step of scanning back to the last newline.

 \param startPos index to starting character
 \param nLines number of lines to skip ahead
 \param startPosIsLineStart avoid scanning back to the line start
 \return new position as index
 */
int Fl_Text_Display::skip_lines(int startPos, int nLines,
                                bool startPosIsLineStart) {
  IS_UTF8_ALIGNED2(buffer(), startPos)

  int retLines, retPos, retLineStart, retLineEnd;

  /* if we're not wrapping use more efficient skip_lines(startPos, nLines) */
  if (!mContinuousWrap)
    return buffer()->skip_lines(startPos, nLines);

  /* wrappedLineCounter can't handle the 0 lines case */
  if (nLines == 0)
    return startPos;

  /* use the common line counting routine to count forward */
  wrapped_line_counter(buffer(), startPos, buffer()->length(),
                       nLines, startPosIsLineStart, 0,
                       &retPos, &retLines, &retLineStart, &retLineEnd);
  IS_UTF8_ALIGNED2(buffer(), retPos)
  return retPos;
}



/**
 \brief Returns the end of a line.

 Same as buffer()->line_end(startPos), but takes into account line breaks
 when wrapping is turned on.
 If the caller knows that \p startPos is at a line start, it can
 pass \p startPosIsLineStart as True to make the call more efficient
 by avoiding the additional step of scanning back to the last newline.

 Note that the definition of the end of a line is less clear when continuous
 wrap is on.  With continuous wrap off, it's just a pointer to the newline
 that ends the line.  When it's on, it's the character beyond the last
 \b displayable character on the line, where a whitespace character which has
 been "converted" to a newline for wrapping is not considered displayable.
 Also note that a line can be wrapped at a non-whitespace character if the
 line had no whitespace.  In this case, this routine returns a pointer to
 the start of the next line.  This is also consistent with the model used by
 visLineLength.

 \param startPos index to starting character
 \param startPosIsLineStart avoid scanning back to the line start
 \return new position as index
 */
int Fl_Text_Display::line_end(int startPos, bool startPosIsLineStart) const {
  IS_UTF8_ALIGNED2(buffer(), startPos)

  int retLines, retPos, retLineStart, retLineEnd;

  /* If we're not wrapping use more efficient buffer()->line_end(startPos) */
  if (!mContinuousWrap)
    return buffer()->line_end(startPos);

  if (startPos == buffer()->length())
    return startPos;

  wrapped_line_counter(buffer(), startPos, buffer()->length(), 1,
                       startPosIsLineStart, 0, &retPos, &retLines, &retLineStart,
                       &retLineEnd);

  IS_UTF8_ALIGNED2(buffer(), retLineEnd)
  return retLineEnd;
}



/**
 \brief Return the beginning of a line.

 Same as buffer()->line_start(pos), but returns the character after last
 wrap point rather than the last newline.

 \param pos index to starting character
 \return new position as index
 */
int Fl_Text_Display::line_start(int pos) const {
  IS_UTF8_ALIGNED2(buffer(), pos)

  int retLines, retPos, retLineStart, retLineEnd;

  /* If we're not wrapping, use the more efficient buffer()->line_start(pos) */
  if (!mContinuousWrap)
    return buffer()->line_start(pos);

  wrapped_line_counter(buffer(), buffer()->line_start(pos), pos, INT_MAX, true, 0,
                       &retPos, &retLines, &retLineStart, &retLineEnd);

  IS_UTF8_ALIGNED2(buffer(), retLineStart)
  return retLineStart;
}



/**
 \brief Skip a number of lines back.

 Same as buffer()->rewind_lines(startPos, nLines), but takes into account
 line breaks when wrapping is turned on.

 \param startPos index to starting character
 \param nLines number of lines to skip back
 \return new position as index
 */
int Fl_Text_Display::rewind_lines(int startPos, int nLines) {
  IS_UTF8_ALIGNED2(buffer(), startPos)

  Fl_Text_Buffer *buf = buffer();
  int pos, lineStart, retLines, retPos, retLineStart, retLineEnd;

  /* If we're not wrapping, use the more efficient
     Fl_Text_Buffer::rewind_lines(startPos, nLines) */
  if (!mContinuousWrap)
    return buf->rewind_lines(startPos, nLines);

  pos = startPos;
  for (;;) {
    lineStart = buf->line_start(pos);
    wrapped_line_counter(buf, lineStart, pos, INT_MAX, true, 0,
                         &retPos, &retLines, &retLineStart, &retLineEnd, false);
    if (retLines > nLines)
      return skip_lines(lineStart, retLines-nLines, true);
    nLines -= retLines;
    pos = lineStart - 1;
    if (pos < 0)
      return 0;
    nLines -= 1;
  }
}



/**
 \brief Moves the current insert position right one word.
 */
void Fl_Text_Display::next_word() {
  int pos = insert_position();

  while (pos < buffer()->length() && !buffer()->is_word_separator(pos)) {
    pos = buffer()->next_char(pos);
  }

  while (pos < buffer()->length() && buffer()->is_word_separator(pos)) {
    pos = buffer()->next_char(pos);
  }

  insert_position( pos );
}



/**
 \brief Moves the current insert position left one word.
 */
void Fl_Text_Display::previous_word() {
  int pos = insert_position();
  if (pos==0) return;
  pos = buffer()->prev_char(pos);

  while (pos && buffer()->is_word_separator(pos)) {
    pos = buffer()->prev_char(pos);
  }

  while (pos && !buffer()->is_word_separator(pos)) {
    pos = buffer()->prev_char(pos);
  }

  if (buffer()->is_word_separator(pos)) {
    pos = buffer()->next_char(pos);
  }

  insert_position( pos );
}



/**
 \brief This is called before any characters are deleted.

 Callback attached to the text buffer to receive delete information before
 the modifications are actually made.

 This callback can be used to adjust the display or update other setting.
 It is not advisable to change any buffers or text in this callback, or
 line counting may get out of sync.

 \param pos starting index of deletion
 \param nDeleted number of bytes we will delete (must be UTF-8 aligned!)
 \param cbArg "this" pointer for static callback function
 */
void Fl_Text_Display::buffer_predelete_cb(int pos, int nDeleted, void *cbArg) {
  Fl_Text_Display *textD = (Fl_Text_Display *)cbArg;
  if (textD->mContinuousWrap) {
  /* Note: we must perform this measurement, even if there is not a
   single character deleted; the number of "deleted" lines is the
   number of visual lines spanned by the real line in which the
   modification takes place.
   Also, a modification of the tab distance requires the same
   kind of calculations in advance, even if the font width is "fixed",
   because when the width of the tab characters changes, the layout
   of the text may be completely different. */
    IS_UTF8_ALIGNED2(textD->buffer(), pos)
    textD->measure_deleted_lines(pos, nDeleted);
  } else {
    textD->mSuppressResync = 0; /* Probably not needed, but just in case */
  }
}



/**
 \brief This is called whenever the buffer is modified.

 Callback attached to the text buffer to receive modification information.

 This callback can be used to adjust the display or update other setting.
 It is not advisable to change any buffers or text in this callback, or
 line counting may get out of sync.

 \param pos starting index of modification
 \param nInserted number of bytes we inserted (must be UTF-8 aligned!)
 \param nDeleted number of bytes deleted (must be UTF-8 aligned!)
 \param nRestyled ??
 \param deletedText this is what was removed, must not be NULL if nDeleted is set
 \param cbArg "this" pointer for static callback function
 */
void Fl_Text_Display::buffer_modified_cb( int pos, int nInserted, int nDeleted,
                                         int nRestyled, const char *deletedText, void *cbArg ) {
  int linesInserted, linesDeleted, startDispPos, endDispPos;
  Fl_Text_Display *textD = ( Fl_Text_Display * ) cbArg;
  Fl_Text_Buffer *buf = textD->mBuffer;
  int oldFirstChar = textD->mFirstChar;
  int scrolled, origCursorPos = textD->mCursorPos;
  int wrapModStart = 0, wrapModEnd = 0;

  IS_UTF8_ALIGNED2(buf, pos)
  IS_UTF8_ALIGNED2(buf, oldFirstChar)

  /* buffer modification cancels vertical cursor motion column */
  if ( nInserted != 0 || nDeleted != 0 )
    textD->mCursorPreferredXPos = -1;

  /* Count the number of lines inserted and deleted, and in the case
   of continuous wrap mode, how much has changed */
  if (textD->mContinuousWrap) {
    textD->find_wrap_range(deletedText, pos, nInserted, nDeleted,
                           &wrapModStart, &wrapModEnd, &linesInserted, &linesDeleted);
  } else {
    linesInserted = nInserted == 0 ? 0 : buf->count_lines( pos, pos + nInserted );
    linesDeleted = nDeleted == 0 ? 0 : countlines( deletedText );
  }

  /* Update the line starts and mTopLineNum */
  if ( nInserted != 0 || nDeleted != 0 ) {
    if (textD->mContinuousWrap) {
      textD->update_line_starts( wrapModStart, wrapModEnd-wrapModStart,
                                nDeleted + pos-wrapModStart + (wrapModEnd-(pos+nInserted)),
                                linesInserted, linesDeleted, &scrolled );
    } else {
      textD->update_line_starts( pos, nInserted, nDeleted, linesInserted,
                                linesDeleted, &scrolled );
    }
  } else
    scrolled = 0;

  /* If we're counting non-wrapped lines as well, maintain the absolute
   (non-wrapped) line number of the text displayed */
  if (textD->maintaining_absolute_top_line_number() &&
      (nInserted != 0 || nDeleted != 0)) {
    if (deletedText && (pos + nDeleted < oldFirstChar))
      textD->mAbsTopLineNum += buf->count_lines(pos, pos + nInserted) -
                               countlines(deletedText);
    else if (pos < oldFirstChar)
      textD->reset_absolute_top_line_number();
  }

  /* Update the line count for the whole buffer */
  textD->mNBufferLines += linesInserted - linesDeleted;

  /* Update the cursor position */
  if ( textD->mCursorToHint != NO_HINT ) {
    textD->mCursorPos = textD->mCursorToHint;
    textD->mCursorToHint = NO_HINT;
  } else if ( textD->mCursorPos > pos ) {
    if ( textD->mCursorPos < pos + nDeleted )
      textD->mCursorPos = pos;
    else
      textD->mCursorPos += nInserted - nDeleted;
  }

  // refigure scrollbars & stuff
  textD->display_needs_recalc(); // textD->resize(textD->x(), textD->y(), textD->w(), textD->h());

  // don't need to do anything else if not visible?
  if (!textD->visible_r()) return;

  /* If the changes caused scrolling, re-paint everything and we're done. */
  if ( scrolled ) {
    textD->damage(FL_DAMAGE_EXPOSE);
    if ( textD->mStyleBuffer )   /* See comments in extend_range_for_styles() */
      textD->mStyleBuffer->primary_selection()->selected(0);
    return;
  }

  /* If the changes didn't cause scrolling, decide the range of characters
   that need to be re-painted.  Also if the cursor position moved, be
   sure that the redisplay range covers the old cursor position so the
   old cursor gets erased, and erase the bits of the cursor which extend
   beyond the left and right edges of the text. */
  startDispPos = textD->mContinuousWrap ? wrapModStart : pos;
  IS_UTF8_ALIGNED2(buf, startDispPos)

  if ( origCursorPos == startDispPos && textD->mCursorPos != startDispPos )
    startDispPos = min( startDispPos, buf->prev_char_clipped(origCursorPos) );
  IS_UTF8_ALIGNED2(buf, startDispPos)

  if ( linesInserted == linesDeleted ) {
    if ( nInserted == 0 && nDeleted == 0 )
      endDispPos = pos + nRestyled;
    else {
      if (textD->mContinuousWrap)
        endDispPos = wrapModEnd;
      else
        endDispPos = buf->next_char(buf->line_end( pos + nInserted ));

      // CET - FIXME      if ( origCursorPos >= startDispPos &&
      //                ( origCursorPos <= endDispPos || endDispPos == buf->length() ) )
    }
    if (linesInserted > 1) {
      // textD->draw_line_numbers(false); // can't do this b/c not called from virtual draw();
      textD->damage(FL_DAMAGE_EXPOSE);
    }
  } else {
    endDispPos = buf->next_char(textD->mLastChar);
    // CET - FIXME   if ( origCursorPos >= pos )
    /* If more than one line is inserted/deleted, a line break may have
     been inserted or removed in between, and the line numbers may
     have changed. If only one line is altered, line numbers cannot
     be affected (the insertion or removal of a line break always
     results in at least two lines being redrawn). */

    // Call draw_line_numbers() here to ensure line# is drawn
    // when hitting enter for new line -- LZA / STR #2621
    //textD->draw_line_numbers(true);    // no, can't call this here, not in draw() context -- ERCO / STR#2621
    //textD->damage(::FL_DAMAGE_EXPOSE);
  }
  IS_UTF8_ALIGNED2(buf, startDispPos)
  IS_UTF8_ALIGNED2(buf, endDispPos)

  /* If there is a style buffer, check if the modification caused additional
   changes that need to be redisplayed.  (Redisplaying separately would
   cause double-redraw on almost every modification involving styled
   text).  Extend the redraw range to incorporate style changes */
  if ( textD->mStyleBuffer )
    textD->extend_range_for_styles( &startDispPos, &endDispPos );
  IS_UTF8_ALIGNED2(buf, startDispPos)
  IS_UTF8_ALIGNED2(buf, endDispPos)

  /* Redisplay computed range */
  textD->redisplay_range( startDispPos, endDispPos );
}


/* Line Numbering Methods */

/**
 \brief Line numbering stuff, currently unused.

 In continuous wrap mode, internal line numbers are calculated after
 wrapping.  A separate non-wrapped line count is maintained when line
 numbering is turned on.  There is some performance cost to maintaining this
 line count, so normally absolute line numbers are not tracked if line
 numbering is off.  This routine allows callers to specify that they still
 want this line count maintained (for use via Fl_Text_Display::position_to_linecol()).
 More specifically, this allows the line number reported in the statistics
 line to be calibrated in absolute lines, rather than post-wrapped lines.
 */
void Fl_Text_Display::maintain_absolute_top_line_number(int state) {
  mNeedAbsTopLineNum = state;
  reset_absolute_top_line_number();
}



/**
  Returns the absolute (non-wrapped) line number of the first line displayed.

  Returns 0 if the absolute top line number is not being maintained.
*/
int Fl_Text_Display::get_absolute_top_line_number() const {
  if (!mContinuousWrap)
    return mTopLineNum;
  if (maintaining_absolute_top_line_number())
    return mAbsTopLineNum;
  return 0;
}



/**
  Re-calculate absolute top line number for a change in scroll position.

  Does nothing if the absolute top line number is not being maintained.
*/
void Fl_Text_Display::absolute_top_line_number(int oldFirstChar) {
  if (maintaining_absolute_top_line_number() && buffer()) {
    if (mFirstChar < oldFirstChar)
      mAbsTopLineNum -= buffer()->count_lines(mFirstChar, oldFirstChar);
    else
      mAbsTopLineNum += buffer()->count_lines(oldFirstChar, mFirstChar);
  }
}



/**
  Returns true if a separate absolute top line number is being maintained.

  The absolute top line number is used for displaying line numbers in
  continuous wrap mode or showing in the statistics line (the latter is
  currently not available in FLTK).
*/
int Fl_Text_Display::maintaining_absolute_top_line_number() const {
  return mContinuousWrap &&
  (mLineNumWidth != 0 || mNeedAbsTopLineNum);
}



/**
 Reestablish the absolute (non-wrapped) top line number.

 Count lines from the beginning of the buffer to reestablish the absolute
 (non-wrapped) top line number. If mode is not continuous wrap, or the
 number is not being maintained, does nothing.
 */
void Fl_Text_Display::reset_absolute_top_line_number() {
  mAbsTopLineNum = 1;
  absolute_top_line_number(0);
}



/**
  Convert a position index into a line number offset.

  Find the line number of position \p pos relative to the first line of
  displayed text, counting from 0 to <i>visible lines - 1</i>.
  The line number is returned in \p lineNum.

  Returns 0 if the line is not displayed. In this case \p lineNum is 0 as well.

  Returns 1 if the line is displayed. In this case \p lineNum is the relative
  line number.

  \param[in]  pos       byte position in buffer
  \param[out] lineNum   relative line number of byte \p pos in buffer

  \returns whether the character at byte position \p pos is currently displayed
  \retval 0 \p pos is not displayed; \p lineNum is invalid (zero)
  \retval 1 \p pos is displayed; \p lineNum is valid
*/
int Fl_Text_Display::position_to_line( int pos, int *lineNum ) const {
  IS_UTF8_ALIGNED2(buffer(), pos)

  int i;

  *lineNum = 0;
  if ( pos < mFirstChar ) return 0;
  if ( pos > mLastChar ) {
    if ( empty_vlines() ) {
      if ( mLastChar < mBuffer->length() ) {
        if ( !position_to_line( mLastChar, lineNum ) ) {
          Fl::error("Fl_Text_Display::position_to_line(): Consistency check ptvl failed");
          return 0;
        }
        return ++( *lineNum ) <= mNVisibleLines - 1;
      } else {
        position_to_line( buffer()->prev_char_clipped(mLastChar), lineNum );
        return 1;
      }
    }
    return 0;
  }

  for ( i = mNVisibleLines - 1; i >= 0; i-- ) {
    if ( mLineStarts[ i ] != -1 && pos >= mLineStarts[ i ] ) {
      *lineNum = i;
      return 1;
    }
  }
  return 0;   /* probably never be reached */
}


/**
 Universal pixel machine.

 We use a single function that handles all line layout, measuring, and drawing
  \li draw a text range
  \li return the width of a text range in pixels
  \li return the index of a character that is at a pixel position

 \param[in] mode DRAW_LINE, GET_WIDTH, FIND_INDEX, FIND_INDEX_FROM_ZERO, or FIND_CURSOR_INDEX
 \param[in] lineStartPos index of first character
 \param[in] lineLen size of string in bytes
 \param[in] leftChar, rightChar
 \param[in] Y drawing position
 \param[in] bottomClip, leftClip, rightClip stop work when we reach the clipped
            area. rightClip is the X position that we search in FIND_INDEX.
 \retval DRAW_LINE index of last drawn character
 \retval GET_WIDTH width in pixels of text segment if we would draw it
 \retval FIND_INDEX index of character at given x position in window coordinates
 \retval FIND_INDEX_FROM_ZERO index of character at given x position without scrolling and widget offsets
 \todo we need to handle hidden hyphens and tabs here!
 \todo we handle all styles and selections
 \todo we must provide code to get pixel positions of the middle of a character as well
 */
int Fl_Text_Display::handle_vline(
                                  int mode,
                                  int lineStartPos, int lineLen, int leftChar, int rightChar,
                                  int Y, int bottomClip,
                                  int leftClip, int rightClip) const
{
  IS_UTF8_ALIGNED2(buffer(), lineStartPos)

  /* STR #2531

   The variables startStyle and styleX seem to introduce some additional
   complexity. They were required to fix STR #2531 in which a horizontal
   character wiggle could be observed when drag-selecting text. This was caused
   by native drawing an measuring routines that support kerning (inter-character
   spacing, the width of 'T' plus the width of 'e' is greater than the width of
   'Te', because advanced typesetting moves the 'e' slightly to the left below
   the 'T').

   To acommodate this, FLTK uses slightly different routines for a true style
   change vs. a change in highlighting only.
   */
  int i, X, startIndex, startStyle, style, charStyle;
  char *lineStr;
  double startX, styleX;

  if ( lineStartPos == -1 ) {
    lineStr = NULL;
  } else {
    lineStr = mBuffer->text_range( lineStartPos, lineStartPos + lineLen );
  }

  // STR #2788
  int cursor_pos = 0;
  if (mode==FIND_CURSOR_INDEX) {
    mode = FIND_INDEX;
    cursor_pos = 1;
  }

  if (mode==GET_WIDTH) {
    X = 0;
  } else if (mode==FIND_INDEX_FROM_ZERO) {
    X = 0;
    mode = FIND_INDEX;
  } else {
    X = text_area.x - mHorizOffset;
  }

  // In DRAW_LINE mode, the first iteration of the loop will draw all
  // backgrounds. The second iteration will draw the text, so that text
  // overlapping background color changes will not be clipped.
  for (int loop=1; loop<=2; loop++) {
    int mask = (loop==1) ? BG_ONLY_MASK : TEXT_ONLY_MASK;
    startX = X;
    startIndex = 0;
    if (!lineStr) {
      // just clear the background
      if (mode==DRAW_LINE) {
        style = position_style(lineStartPos, lineLen, -1);
        if (loop==1)
          draw_string( style|BG_ONLY_MASK, text_area.x, Y, text_area.x+text_area.w, lineStr, lineLen );
      }
      if (mode==FIND_INDEX) {
        IS_UTF8_ALIGNED2(buffer(), lineStartPos)
        return lineStartPos;
      }
      return 0;
    }
    char currChar = 0, prevChar = 0;
    styleX = startX; startStyle = startIndex;
    // draw the line
    style = position_style(lineStartPos, lineLen, 0);
    for (i=0; i<lineLen; ) {
      currChar = lineStr[i]; // one byte is enough to handele tabs and other cases
      int len = fl_utf8len1(currChar);
      if (len<=0) len = 1; // OUCH!
      charStyle = position_style(lineStartPos, lineLen, i);
      if (charStyle!=style || currChar=='\t' || prevChar=='\t') {
        // draw a segment whenever the style changes or a Tab is found
        double w = 0;
        if (prevChar=='\t') {
          // draw a single Tab space
          double tab = col_to_x(mBuffer->tab_distance());
          double xAbs = (mode==GET_WIDTH) ? startX : startX+mHorizOffset-text_area.x;
          w = ((int(xAbs/tab)+1)*tab) - xAbs;
          styleX = startX+w; startStyle = i;
          if (mode==DRAW_LINE && loop==1)
            draw_string( style|BG_ONLY_MASK, int(startX), Y, int(startX+w), 0, 0 );
          if (mode==FIND_INDEX && startX+w>rightClip) {
            // find x pos inside block
            free(lineStr);
            if (cursor_pos && (startX+w/2<rightClip))  // STR #2788
              return lineStartPos + startIndex + 1;  // STR #2788
            return lineStartPos + startIndex;
          }
        } else {
          // draw the text segment from the previous style change up to this point
          if ( (style&0xff)==(charStyle&0xff)) {
            w = string_width( lineStr+startStyle, i-startStyle, style ) - startX + styleX;
          } else {
            w = string_width( lineStr+startIndex, i-startIndex, style );
          }
          if (mode==DRAW_LINE) {
            if (startIndex!=startStyle) {
              fl_push_clip(int(startX), Y, int(w)+1, mMaxsize);
              draw_string( style|mask, int(styleX), Y, int(startX+w), lineStr+startStyle, i-startStyle );
              fl_pop_clip();
            } else {
              draw_string( style|mask, int(startX), Y, int(startX+w), lineStr+startIndex, i-startIndex );
            }
          }
          if (mode==FIND_INDEX && startX+w>rightClip) {
            // find x pos inside block
            int di;
            if (startIndex!=startStyle) {
              di = find_x(lineStr+startStyle, i-startStyle, style, -int(rightClip-styleX)); // STR #2788
              di = lineStartPos + startStyle + di;
            } else {
              di = find_x(lineStr+startIndex, i-startIndex, style, -int(rightClip-startX)); // STR #2788
              di = lineStartPos + startIndex + di;
            }
            free(lineStr);
            IS_UTF8_ALIGNED2(buffer(), (lineStartPos+startIndex+di))
            return di;
          }
          if ( (style&0xff)!=(charStyle&0xff)) {
            startStyle = i;
            styleX = startX+w;
          }
        }
        style = charStyle;
        startX += w;
        startIndex = i;
      }
      i += len;
      prevChar = currChar;
    }
    double w = 0;
    if (currChar=='\t') {
      // draw a single Tab space
      double tab = col_to_x(mBuffer->tab_distance());
      double xAbs = (mode==GET_WIDTH) ? startX : startX+mHorizOffset-text_area.x;
      w = ((int(xAbs/tab)+1)*tab) - xAbs;
      if (mode==DRAW_LINE && loop==1)
        draw_string( style|BG_ONLY_MASK, int(startX), Y, int(startX+w), 0, 0 );
      if (mode==FIND_INDEX) {
        // find x pos inside block
        free(lineStr);
        if (cursor_pos) // STR #2788
          return lineStartPos + startIndex + ( rightClip-startX>w/2 ? 1 : 0 ); // STR #2788
        return lineStartPos + startIndex + ( rightClip-startX>w ? 1 : 0 );
      }
    } else {
      w = string_width( lineStr+startIndex, i-startIndex, style );
      if (mode==DRAW_LINE) {
        // STR 2531
        if (startIndex!=startStyle) {
          fl_push_clip(int(startX), Y, int(w)+1, mMaxsize);
          draw_string( style|mask, int(styleX), Y, int(startX+w), lineStr+startStyle, i-startStyle );
          fl_pop_clip();
        } else {
          draw_string( style|mask, int(startX), Y, int(startX+w), lineStr+startIndex, i-startIndex );
        }
      }
      if (mode==FIND_INDEX) {
        // find x pos inside block
        int di;
        if (startIndex!=startStyle) {
          di = find_x(lineStr+startStyle, i-startStyle, style, -int(rightClip-styleX)); // STR #2788
          di = lineStartPos + startStyle + di;
        } else {
          di = find_x(lineStr+startIndex, i-startIndex, style, -int(rightClip-startX)); // STR #2788
          di = lineStartPos + startIndex + di;
        }
        free(lineStr);
        IS_UTF8_ALIGNED2(buffer(), (lineStartPos+startIndex+di))
        return di;
      }
    }
    if (mode==GET_WIDTH) {
      free(lineStr);
      return int(startX+w);
    }

    // clear the rest of the line
    startX += w;
    style = position_style(lineStartPos, lineLen, i);
    if (mode==DRAW_LINE && loop==1)
      draw_string( style|BG_ONLY_MASK, int(startX), Y, text_area.x+text_area.w, lineStr, lineLen );
  }

  free(lineStr);
  IS_UTF8_ALIGNED2(buffer(), (lineStartPos+lineLen))
  return lineStartPos + lineLen;
}


/**
 \brief Find the index of the character that lies at the given x position / closest cursor position.

 \param s UTF-8 text string
 \param len length of string
 \param style index into style lookup table
 \param x position in pixels - negative returns closest cursor position
 \return index into buffer
 */
int Fl_Text_Display::find_x(const char *s, int len, int style, int x) const {
  IS_UTF8_ALIGNED(s)

  int cursor_pos = x<0; // STR #2788
  x = x<0 ? -x : x;     // STR #2788

  // TODO: use binary search which may be quicker.
  int i = 0;
  int last_w = 0;       // STR #2788
  while (i<len) {
    int cl = fl_utf8len1(s[i]);
    int w = int( string_width(s, i+cl, style) );
    if (w>x) {
      if (cursor_pos && (w-x < x-last_w)) return i+cl; // STR #2788
      return i;
    }
    last_w = w;        // STR #2788
    i += cl;
  }
  return len;
}



/**
 \brief Draw a single line of text.

 Draw the text on a single line represented by \p visLineNum (the
 number of lines down from the top of the display), limited by
 \p leftClip and \p rightClip window coordinates and \p leftCharIndex and
 \p rightCharIndex character positions (not including the character at
 position \p rightCharIndex).

 \param visLineNum index of line in the visible line number lookup
 \param leftClip, rightClip pixel position of clipped area
 \param leftCharIndex, rightCharIndex index into line of segment that we want to draw
 */
void Fl_Text_Display::draw_vline(int visLineNum, int leftClip, int rightClip,
                                 int leftCharIndex, int rightCharIndex) {
  int Y, lineStartPos, lineLen, fontHeight;

  //  printf("draw_vline(visLineNum=%d, leftClip=%d, rightClip=%d, leftCharIndex=%d, rightCharIndex=%d)\n",
  //         visLineNum, leftClip, rightClip, leftCharIndex, rightCharIndex);
  //  printf("nNVisibleLines=%d\n", mNVisibleLines);

  /* If line is not displayed, skip it */
  if ( visLineNum < 0 || visLineNum >= mNVisibleLines )
    return;

  /* Calculate Y coordinate of the string to draw */
  fontHeight = mMaxsize;
  Y = text_area.y + visLineNum * fontHeight;

  /* Get the text, length, and  buffer position of the line to display */
  lineStartPos = mLineStarts[ visLineNum ];
  if ( lineStartPos == -1 ) {
    lineLen = 0;
  } else {
    lineLen = vline_length( visLineNum );
  }

  /* Shrink the clipping range to the active display area */
  leftClip = max( text_area.x, leftClip );
  rightClip = min( rightClip, text_area.x + text_area.w );

  handle_vline(DRAW_LINE,
               lineStartPos, lineLen, leftCharIndex, rightCharIndex,
               Y, Y+fontHeight, leftClip, rightClip);
  return;
}



/**
 \brief Draw a text segment in a single style.

 Draw a string or blank area according to parameter \p style, using the
 appropriate colors and drawing method for that style, with top left
 corner at \p X, \p Y.  If style says to draw text, use \p string as
 source of characters, and draw \p nChars, if style is FILL, erase
 rectangle where text would have drawn from \p X to \p toX and from
 \p Y to the maximum y extent of the current font(s).

 \param style index into style lookup table
 \param X, Y drawing origin
 \param toX rightmost position if this is a fill operation
 \param string text if this is a drawing operation
 \param nChars number of characters to draw
 */
void Fl_Text_Display::draw_string(int style,
                                  int X, int Y, int toX,
                                  const char *string, int nChars) const {
  IS_UTF8_ALIGNED(string)

  const Style_Table_Entry *styleRec = NULL;

  /* Draw blank area rather than text, if that was the request */
  if ( style & FILL_MASK ) {
    if (style & TEXT_ONLY_MASK) return;
    clear_rect( style, X, Y, toX - X, mMaxsize );
    return;
  }
  /* Set font, color, and gc depending on style.  For normal text, GCs
   for normal drawing, or drawing within a Fl_Text_Selection or highlight are
   pre-allocated and pre-configured.  For syntax highlighting, GCs are
   configured here, on the fly. */

  Fl_Font font = textfont();
  int fsize = textsize();
  Fl_Color foreground;
  Fl_Color background;
  Fl_Color bgbasecolor;

  if ( style & STYLE_LOOKUP_MASK ) {
    int si = (style & STYLE_LOOKUP_MASK) - 'A';
    if (si < 0) si = 0;
    else if (si >= mNStyles) si = mNStyles - 1;

    styleRec = mStyleTable + si;
    font  = styleRec->font;
    fsize = styleRec->size;
    bgbasecolor = (styleRec->attr&ATTR_BGCOLOR) ? styleRec->bgcolor : color();

    if (style & PRIMARY_MASK) {
      if (Fl::focus() == (Fl_Widget*)this) {
        if (Fl::screen_driver()->has_marked_text() && Fl::compose_state) {
          background = bgbasecolor; // Mac OS: underline marked text
        } else {
          background = selection_color();
        }
      } else {
        background = fl_color_average(bgbasecolor, selection_color(), 0.4f);
      }
    } else if (style & HIGHLIGHT_MASK) {
      if (Fl::focus() == (Fl_Widget*)this) {
        background = fl_color_average(bgbasecolor, selection_color(), 0.5f);
      } else {
        background = fl_color_average(bgbasecolor, selection_color(), 0.6f);
      }
    } else if (style & SECONDARY_MASK) {
      if (Fl::focus() == (Fl_Widget*)this) {
        background = fl_color_average(bgbasecolor, secondary_selection_color(), 0.5f);
      } else {
        background = fl_color_average(bgbasecolor, secondary_selection_color(), 0.6f);
      }
    } else {
      background = bgbasecolor;
    }
    foreground = (style & PRIMARY_MASK) ? fl_contrast(styleRec->color, background) : styleRec->color;
  } else if (style & PRIMARY_MASK) {
    if (Fl::focus() == (Fl_Widget*)this) {
      background = selection_color();
    } else {
      background = fl_color_average(color(), selection_color(), 0.4f);
    }
    foreground = fl_contrast(textcolor(), background);
  } else if (style & HIGHLIGHT_MASK) {
    if (Fl::focus() == (Fl_Widget*)this) {
      background = fl_color_average(color(), selection_color(), 0.5f);
    } else {
      background = fl_color_average(color(), selection_color(), 0.6f);
    }
    foreground = fl_contrast(textcolor(), background);
  } else if (style & SECONDARY_MASK) {
    if (Fl::focus() == (Fl_Widget*)this) {
      background = secondary_selection_color();
    } else {
      background = fl_color_average(color(), secondary_selection_color(), 0.4f);
    }
    foreground = fl_contrast(textcolor(), background);
  } else {
    foreground = textcolor();
    background = color();
  }

  if ( !active_r() ) {
    foreground = fl_inactive(foreground);
    background = fl_inactive(background);
  }

  if (!(style & TEXT_ONLY_MASK)) {
    fl_color( background );
    fl_rectf( X, Y, toX - X, mMaxsize );
  }
  if (!(style & BG_ONLY_MASK)) {
    fl_color( foreground );
    fl_font( font, fsize );
    int baseline = Y + mMaxsize - fl_descent();
    // Make sure antialiased ÄÖÜ do not leak on line above:
    // on X11+Xft the antialiased part of characters such as ÄÖÜ leak on the bottom pixel of the line above
    static int can_leak = Fl::screen_driver()->text_display_can_leak();
    // Clip top and bottom only. Add margin to avoid clipping horizontally
    if (can_leak) fl_push_clip(x(), Y, w(), mMaxsize);
    fl_draw( string, nChars, X, baseline);
    if (styleRec) {
      if (styleRec->attr & ATTR_LINES_MASK) {
        int pitch = fsize/7;
        int prevAA = fl_antialias();
        fl_antialias(1);
        switch (styleRec->attr & ATTR_LINES_MASK) {
          case ATTR_UNDERLINE:
            fl_color(foreground);
            fl_line_style(FL_SOLID, pitch);
            goto DRAW_UNDERLINE;
            break;
          case ATTR_GRAMMAR:
            fl_color(grammar_underline_color());
            goto DRAW_DOTTED_UNDERLINE;
          case ATTR_SPELLING:
            fl_color(spelling_underline_color());
          DRAW_DOTTED_UNDERLINE:
            fl_line_style(FL_DOT, pitch);
          DRAW_UNDERLINE:
            fl_xyline(X, baseline + fl_descent()/2, toX);
            break;
          case ATTR_STRIKE_THROUGH:
            fl_color(foreground);
            fl_line_style(FL_SOLID, pitch);
            fl_xyline(X, baseline - (fl_height()-fl_descent())/3, toX);
            break;
        }
        fl_line_style(FL_SOLID, 1);
        fl_antialias(prevAA);
      }
    }
    if (Fl::screen_driver()->has_marked_text() && Fl::compose_state && (style & PRIMARY_MASK)) {
      fl_color( fl_color_average(foreground, background, 0.6f) );
      fl_line(X, Y + mMaxsize - 1, X + (int)fl_width(string, nChars), Y + mMaxsize - 1);
    }
    if (can_leak) fl_pop_clip();
  }

  // CET - FIXME
  /* If any space around the character remains unfilled (due to use of
   different sized fonts for highlighting), fill in above or below
   to erase previously drawn characters */
  /*
   if (fs->ascent < mAscent)
   clear_rect( style, X, Y, toX - X, mAscent - fs->ascent);
   if (fs->descent < mDescent)
   clear_rect( style, X, Y + mAscent + fs->descent, toX - x,
   mDescent - fs->descent);
   */
  /* Underline if style is secondary Fl_Text_Selection */

  /*
   if (style & SECONDARY_MASK)
   XDrawLine(XtDisplay(mW), XtWindow(mW), gc, x,
   y + mAscent, toX - 1, Y + fs->ascent);
   */
}



/**
 \brief Clear a rectangle with the appropriate background color for \p style.

 \param style index into style table
 \param X, Y, width, height size and position of background area
 */
void Fl_Text_Display::clear_rect(int style,
                                 int X, int Y,
                                 int width, int height) const {
  /* A width of zero means "clear to end of window" to XClearArea */
  if ( width == 0 )
    return;

  Fl_Color bgbasecolor = color();
  if ( style & STYLE_LOOKUP_MASK ) {
    int si = (style & STYLE_LOOKUP_MASK) - 'A';
    if (si < 0) si = 0;
    else if (si >= mNStyles) si = mNStyles - 1;
    const Style_Table_Entry *styleRec = mStyleTable + si;
    if (styleRec->attr&ATTR_BGCOLOR_EXT_)
      bgbasecolor = styleRec->bgcolor;
  }

  Fl_Color c;
  if (style & PRIMARY_MASK) {
    if (Fl::focus()==(Fl_Widget*)this) {
      c = selection_color();
    } else {
      c = fl_color_average(bgbasecolor, selection_color(), 0.4f);
    }
  } else if (style & HIGHLIGHT_MASK) {
    if (Fl::focus()==(Fl_Widget*)this) {
      c = fl_color_average(bgbasecolor, selection_color(), 0.5f);
    } else {
      c = fl_color_average(bgbasecolor, selection_color(), 0.6f);
    }
  } else {
    c = bgbasecolor;
  }
  fl_color(active_r() ? c : fl_inactive(c));
  fl_rectf( X, Y, width, height );
}



/**
 \brief Draw a cursor with top center at \p X, \p Y.

 \param X, Y cursor position in pixels
 */
void Fl_Text_Display::draw_cursor( int X, int Y ) {

  typedef struct {
    int x1, y1, x2, y2;
  }
  Segment;

  Segment segs[ 5 ];
  int left, right, cursorWidth, midY;
  //    int fontWidth = mFontStruct->min_bounds.width, nSegs = 0;
  int fontWidth = TMPFONTWIDTH; // CET - FIXME
  int nSegs = 0;
  int fontHeight = mMaxsize;
  int bot = Y + fontHeight - 1;

  if ( X < text_area.x - 1 || X > text_area.x + text_area.w )
    return;

  /* For cursors other than the block, make them around 2/3 of a character
   width, rounded to an even number of pixels so that X will draw an
   odd number centered on the stem at x. */
  cursorWidth = 4;   //(fontWidth/3) * 2;
  left = X - cursorWidth / 2;
  right = left + cursorWidth;

  /* Create segments and draw cursor */
  if ( mCursorStyle == CARET_CURSOR ) {
    midY = bot - fontHeight / 5;
    segs[ 0 ].x1 = left; segs[ 0 ].y1 = bot; segs[ 0 ].x2 = X; segs[ 0 ].y2 = midY;
    segs[ 1 ].x1 = X; segs[ 1 ].y1 = midY; segs[ 1 ].x2 = right; segs[ 1 ].y2 = bot;
    segs[ 2 ].x1 = left; segs[ 2 ].y1 = bot; segs[ 2 ].x2 = X; segs[ 2 ].y2 = midY - 1;
    segs[ 3 ].x1 = X; segs[ 3 ].y1 = midY - 1; segs[ 3 ].x2 = right; segs[ 3 ].y2 = bot;
    nSegs = 4;
  } else if ( mCursorStyle == NORMAL_CURSOR ) {
    segs[ 0 ].x1 = left; segs[ 0 ].y1 = Y; segs[ 0 ].x2 = right; segs[ 0 ].y2 = Y;
    segs[ 1 ].x1 = X; segs[ 1 ].y1 = Y; segs[ 1 ].x2 = X; segs[ 1 ].y2 = bot;
    segs[ 2 ].x1 = left; segs[ 2 ].y1 = bot; segs[ 2 ].x2 = right; segs[ 2 ].y2 = bot;
    nSegs = 3;
  } else if ( mCursorStyle == HEAVY_CURSOR ) {
    segs[ 0 ].x1 = X - 1; segs[ 0 ].y1 = Y; segs[ 0 ].x2 = X - 1; segs[ 0 ].y2 = bot;
    segs[ 1 ].x1 = X; segs[ 1 ].y1 = Y; segs[ 1 ].x2 = X; segs[ 1 ].y2 = bot;
    segs[ 2 ].x1 = X + 1; segs[ 2 ].y1 = Y; segs[ 2 ].x2 = X + 1; segs[ 2 ].y2 = bot;
    segs[ 3 ].x1 = left; segs[ 3 ].y1 = Y; segs[ 3 ].x2 = right; segs[ 3 ].y2 = Y;
    segs[ 4 ].x1 = left; segs[ 4 ].y1 = bot; segs[ 4 ].x2 = right; segs[ 4 ].y2 = bot;
    nSegs = 5;
  } else if ( mCursorStyle == DIM_CURSOR ) {
    midY = Y + fontHeight / 2;
    segs[ 0 ].x1 = X; segs[ 0 ].y1 = Y; segs[ 0 ].x2 = X; segs[ 0 ].y2 = Y;
    segs[ 1 ].x1 = X; segs[ 1 ].y1 = midY; segs[ 1 ].x2 = X; segs[ 1 ].y2 = midY;
    segs[ 2 ].x1 = X; segs[ 2 ].y1 = bot; segs[ 2 ].x2 = X; segs[ 2 ].y2 = bot;
    nSegs = 3;
  } else if ( mCursorStyle == BLOCK_CURSOR ) {
    right = X + fontWidth;
    segs[ 0 ].x1 = X; segs[ 0 ].y1 = Y; segs[ 0 ].x2 = right; segs[ 0 ].y2 = Y;
    segs[ 1 ].x1 = right; segs[ 1 ].y1 = Y; segs[ 1 ].x2 = right; segs[ 1 ].y2 = bot;
    segs[ 2 ].x1 = right; segs[ 2 ].y1 = bot; segs[ 2 ].x2 = X; segs[ 2 ].y2 = bot;
    segs[ 3 ].x1 = X; segs[ 3 ].y1 = bot; segs[ 3 ].x2 = X; segs[ 3 ].y2 = Y;
    nSegs = 4;
  } else if ( mCursorStyle == SIMPLE_CURSOR ){
    segs[ 0 ].x1 = X; segs[ 0 ].y1 = Y; segs[ 0 ].x2 = X; segs[ 0 ].y2 = bot;
    segs[ 1 ].x1 = X+1; segs[ 1 ].y1 = Y; segs[ 1 ].x2 = X+1; segs[ 1 ].y2 = bot;
    nSegs = 2;
  }
  fl_color( mCursor_color );

  for ( int k = 0; k < nSegs; k++ ) {
    fl_line( segs[ k ].x1, segs[ k ].y1, segs[ k ].x2, segs[ k ].y2 );
  }

  //fix issue #270
  if (Fl::focus() == this) {
       fl_set_spot(textfont(), textsize(), X, bot, text_area.w, text_area.h, window());
  }
}



/**
 \brief Find the correct style for a character.

 Determine the drawing method to use to draw a specific character from "buf".

 \p lineStartPos gives the character index where the line begins, \p lineIndex,
 the number of characters past the beginning of the line, and \p lineLen
 the number of displayed characters past the beginning of the line.  Passing
 \p lineStartPos of -1 returns the drawing style for "no text".

 Why not just: position_style(pos)?  Because style applies to blank areas
 of the window beyond the text boundaries, and because this routine must also
 decide whether a position is inside of a rectangular Fl_Text_Selection, and do
 so efficiently, without re-counting character positions from the start of the
 line.

 Note that style is a somewhat incorrect name, drawing method would
 be more appropriate.

 If lineIndex is pointing to the last character in a line, and the second
 to last character has the ATTR_BGCOLOR_EXT set, the background color will
 extend into the remaining line.

 \param lineStartPos beginning of this line
 \param lineLen number of bytes in line
 \param lineIndex position of character within line
 \return style for the given character
 */
int Fl_Text_Display::position_style( int lineStartPos, int lineLen, int lineIndex) const
{
  IS_UTF8_ALIGNED2(buffer(), lineStartPos)

  Fl_Text_Buffer * buf = mBuffer;
  Fl_Text_Buffer *styleBuf = mStyleBuffer;
  int pos, style = 0;

  if ( lineStartPos == -1 || buf == NULL )
    return FILL_MASK;

  pos = lineStartPos + min( lineIndex, lineLen );

  if ( styleBuf && lineIndex==lineLen && lineLen>0) {
    style = ( unsigned char ) styleBuf->byte_at( pos-1 );
    if (style == mUnfinishedStyle && mUnfinishedHighlightCB) {
      (mUnfinishedHighlightCB)( pos, mHighlightCBArg);
      style = (unsigned char) styleBuf->byte_at( pos);
    }
    int si = (style & STYLE_LOOKUP_MASK) - 'A';
    if (si < 0) si = 0;
    else if (si >= mNStyles) si = mNStyles - 1;
    const Style_Table_Entry *styleRec = mStyleTable + si;
    if ((styleRec->attr&ATTR_BGCOLOR_EXT_)==0)
      style = FILL_MASK;
  } else if ( lineIndex >= lineLen ) {
    style = FILL_MASK;
  } else if ( styleBuf != NULL ) {
    style = ( unsigned char ) styleBuf->byte_at( pos );
    if (style == mUnfinishedStyle && mUnfinishedHighlightCB) {
      /* encountered "unfinished" style, trigger parsing */
      (mUnfinishedHighlightCB)( pos, mHighlightCBArg);
      style = (unsigned char) styleBuf->byte_at( pos);
    }
  }
  if (buf->primary_selection()->includes(pos))
    style |= PRIMARY_MASK;
  if (buf->highlight_selection()->includes(pos))
    style |= HIGHLIGHT_MASK;
  if (buf->secondary_selection()->includes(pos))
    style |= SECONDARY_MASK;
  return style;
}


/**
 \brief Find the width of a string in the font of a particular style.

 \param string the text
 \param length number of bytes in string
 \param style index into style table
 \return width of text segment in pixels
 */
double Fl_Text_Display::string_width( const char *string, int length, int style ) const {
  IS_UTF8_ALIGNED(string)

  Fl_Font font;
  Fl_Fontsize fsize;

  if ( mNStyles && (style & STYLE_LOOKUP_MASK) ) {
    int si = (style & STYLE_LOOKUP_MASK) - 'A';
    if (si < 0) si = 0;
    else if (si >= mNStyles) si = mNStyles - 1;

    font  = mStyleTable[si].font;
    fsize = mStyleTable[si].size;
  } else {
    font  = textfont();
    fsize = textsize();
  }
  fl_font( font, fsize );
  return fl_width( string, length );
}



/**
 \brief Translate a pixel position into a character index.

 Translate window coordinates to the nearest (insert cursor or character
 cell) text position.  The parameter \p posType specifies how to interpret the
 position: CURSOR_POS means translate the coordinates to the nearest cursor
 position, and CHARACTER_POS means return the position of the character
 closest to (\p X, \p Y).

 \param X, Y pixel position
 \param posType CURSOR_POS or CHARACTER_POS
 \return index into text buffer
 */
int Fl_Text_Display::xy_to_position( int X, int Y, int posType ) const {
  int lineStart, lineLen, fontHeight;
  int visLineNum;

  /* Find the visible line number corresponding to the Y coordinate */
  fontHeight = mMaxsize;
  visLineNum = ( Y - text_area.y ) / fontHeight;
  if ( visLineNum < 0 )
    return mFirstChar;
  if ( visLineNum >= mNVisibleLines )
    visLineNum = mNVisibleLines - 1;

  /* Find the position at the start of the line */
  lineStart = mLineStarts[ visLineNum ];

  /* If the line start was empty, return the last position in the buffer */
  if ( lineStart == -1 )
    return mBuffer->length();

  /* Get the line text and its length */
  lineLen = vline_length( visLineNum );

  int mode = (posType == CURSOR_POS) ? FIND_CURSOR_INDEX : FIND_INDEX; // STR #2788
  return handle_vline(mode,
                      lineStart, lineLen, 0, 0,
                      0, 0,
                      text_area.x, X);
}



/**
 \brief Translate pixel coordinates into row and column.

 Translate window coordinates to the nearest row and column number for
 positioning the cursor.  This, of course, makes no sense when the font is
 proportional, since there are no absolute columns.  The parameter posType
 specifies how to interpret the position: CURSOR_POS means translate the
 coordinates to the nearest position between characters, and CHARACTER_POS
 means translate the position to the nearest character cell.

 \param X, Y pixel coordinates
 \param[out] row, column neares row and column
 \param posType CURSOR_POS or CHARACTER_POS
 */
void Fl_Text_Display::xy_to_rowcol( int X, int Y, int *row,
                                   int *column, int posType ) const {
  int fontHeight = mMaxsize;
  int fontWidth = TMPFONTWIDTH;   //mFontStruct->max_bounds.width;

  /* Find the visible line number corresponding to the Y coordinate */
  *row = ( Y - text_area.y ) / fontHeight;
  if ( *row < 0 ) *row = 0;
  if ( *row >= mNVisibleLines ) *row = mNVisibleLines - 1;

  *column = ( ( X - text_area.x ) + mHorizOffset +
             ( posType == CURSOR_POS ? fontWidth / 2 : 0 ) ) / fontWidth;
  if ( *column < 0 ) * column = 0;
}



/**
 \brief Offset line start counters for a new vertical scroll position.

 Offset the line starts array, mTopLineNum, mFirstChar and lastChar, for a new
 vertical scroll position given by newTopLineNum.  If any currently displayed
 lines will still be visible, salvage the line starts values, otherwise,
 count lines from the nearest known line start (start or end of buffer, or
 the closest value in the mLineStarts array)

 \param newTopLineNum index into buffer
 */
void Fl_Text_Display::offset_line_starts( int newTopLineNum ) {
  int oldTopLineNum = mTopLineNum;
  int oldFirstChar = mFirstChar;
  int lineDelta = newTopLineNum - oldTopLineNum;
  int nVisLines = mNVisibleLines;
  int *lineStarts = mLineStarts;
  int i, lastLineNum;
  Fl_Text_Buffer *buf = mBuffer;

  /* If there was no offset, nothing needs to be changed */
  if ( lineDelta == 0 )
    return;

  /* Find the new value for mFirstChar by counting lines from the nearest
   known line start (start or end of buffer, or the closest value in the
   lineStarts array) */
  lastLineNum = oldTopLineNum + nVisLines - 1;
  if ( newTopLineNum < oldTopLineNum && newTopLineNum < -lineDelta ) {
    mFirstChar = skip_lines( 0, newTopLineNum - 1, true );
  } else if ( newTopLineNum < oldTopLineNum ) {
    mFirstChar = rewind_lines( mFirstChar, -lineDelta );
  } else if ( newTopLineNum < lastLineNum ) {
    mFirstChar = lineStarts[ newTopLineNum - oldTopLineNum ];
  } else if ( newTopLineNum - lastLineNum < mNBufferLines - newTopLineNum ) {
    mFirstChar = skip_lines( lineStarts[ nVisLines - 1 ],
                            newTopLineNum - lastLineNum, true );
  } else {
    mFirstChar = rewind_lines( buf->length(), mNBufferLines - newTopLineNum + 1 );
  }

  /* Fill in the line starts array */
  if ( lineDelta < 0 && -lineDelta < nVisLines ) {
    for ( i = nVisLines - 1; i >= -lineDelta; i-- )
      lineStarts[ i ] = lineStarts[ i + lineDelta ];
    calc_line_starts( 0, -lineDelta );
  } else if ( lineDelta > 0 && lineDelta < nVisLines ) {
    for ( i = 0; i < nVisLines - lineDelta; i++ )
      lineStarts[ i ] = lineStarts[ i + lineDelta ];
    calc_line_starts( nVisLines - lineDelta, nVisLines - 1 );
  } else
    calc_line_starts( 0, nVisLines );

  /* Set lastChar and mTopLineNum */
  calc_last_char();
  mTopLineNum = newTopLineNum;

  /* If we're numbering lines or being asked to maintain an absolute line
   number, re-calculate the absolute line number */
  absolute_top_line_number(oldFirstChar);
}



/**
 \brief Update line start arrays and variables.

 Update the line starts array, mTopLineNum, mFirstChar and lastChar for this
 text display after a modification to the text buffer, given by the
 position \p pos where the change began, and the numbers of characters
 and lines inserted and deleted.

 \param pos index into buffer of recent changes
 \param charsInserted number of bytes(!) inserted
 \param charsDeleted number of bytes(!) deleted
 \param linesInserted number of lines
 \param linesDeleted number of lines
 \param[out] scrolled set to 1 if the text display needs to be scrolled
 */
void Fl_Text_Display::update_line_starts(int pos, int charsInserted,
                                         int charsDeleted, int linesInserted,
                                         int linesDeleted, int *scrolled ) {
  IS_UTF8_ALIGNED2(buffer(), pos)

  int *lineStarts = mLineStarts;
  int i, lineOfPos, lineOfEnd, nVisLines = mNVisibleLines;
  int charDelta = charsInserted - charsDeleted;
  int lineDelta = linesInserted - linesDeleted;

  /* If all of the changes were before the displayed text, the display
   doesn't change, just update the top line num and offset the line
   start entries and first and last characters */
  if ( pos + charsDeleted < mFirstChar ) {
    mTopLineNum += lineDelta;
    for ( i = 0; i < nVisLines && lineStarts[i] != -1; i++ )
      lineStarts[ i ] += charDelta;
    mFirstChar += charDelta;
    mLastChar += charDelta;
    *scrolled = 0;
    return;
  }

  /* The change began before the beginning of the displayed text, but
   part or all of the displayed text was deleted */
  if ( pos < mFirstChar ) {
    /* If some text remains in the window, anchor on that  */
    if ( position_to_line( pos + charsDeleted, &lineOfEnd ) &&
        ++lineOfEnd < nVisLines && lineStarts[ lineOfEnd ] != -1 ) {
      mTopLineNum = max( 1, mTopLineNum + lineDelta );
      mFirstChar = rewind_lines(lineStarts[ lineOfEnd ] + charDelta, lineOfEnd );
      /* Otherwise anchor on original line number and recount everything */
    } else {
      if ( mTopLineNum > mNBufferLines + lineDelta ) {
        mTopLineNum = 1;
        mFirstChar = 0;
      } else
        mFirstChar = skip_lines( 0, mTopLineNum - 1, true );
    }
    calc_line_starts( 0, nVisLines - 1 );
    /* calculate lastChar by finding the end of the last displayed line */
    calc_last_char();
    *scrolled = 1;
    return;
  }

  /* If the change was in the middle of the displayed text (it usually is),
   salvage as much of the line starts array as possible by moving and
   offsetting the entries after the changed area, and re-counting the
   added lines or the lines beyond the salvaged part of the line starts
   array */
  if ( pos <= mLastChar ) {
    /* find line on which the change began */
    position_to_line( pos, &lineOfPos );
    /* salvage line starts after the changed area */
    if ( lineDelta == 0 ) {
      for ( i = lineOfPos + 1; i < nVisLines && lineStarts[ i ] != -1; i++ )
        lineStarts[ i ] += charDelta;
    } else if ( lineDelta > 0 ) {
      for ( i = nVisLines - 1; i >= lineOfPos + lineDelta + 1; i-- )
        lineStarts[ i ] = lineStarts[ i - lineDelta ] +
        ( lineStarts[ i - lineDelta ] == -1 ? 0 : charDelta );
    } else /* (lineDelta < 0) */ {
      for ( i = max( 0, lineOfPos + 1 ); i < nVisLines + lineDelta; i++ )
        lineStarts[ i ] = lineStarts[ i - lineDelta ] +
        ( lineStarts[ i - lineDelta ] == -1 ? 0 : charDelta );
    }
    /* fill in the missing line starts */
    if ( linesInserted >= 0 )
      calc_line_starts( lineOfPos + 1, lineOfPos + linesInserted );
    if ( lineDelta < 0 )
      calc_line_starts( nVisLines + lineDelta, nVisLines );
    /* calculate lastChar by finding the end of the last displayed line */
    calc_last_char();
    *scrolled = 0;
    return;
  }

  /* Change was past the end of the displayed text, but displayable by virtue
   of being an insert at the end of the buffer into visible blank lines */
  if ( empty_vlines() ) {
    position_to_line( pos, &lineOfPos );
    calc_line_starts( lineOfPos, lineOfPos + linesInserted );
    calc_last_char();
    *scrolled = 0;
    return;
  }

  /* Change was beyond the end of the buffer and not visible, do nothing */
  *scrolled = 0;
}


/**
 \brief Update the line starts array.

 Scan through the text in the Text Display's buffer and recalculate the line
 starts array values beginning at index "startLine" and continuing through
 (including) "endLine".  It assumes that the line starts entry preceding
 "startLine" (or mFirstChar if startLine is 0) is good, and re-counts
 newlines to fill in the requested entries.  Out of range values for
 "startLine" and "endLine" are acceptable.

 \param startLine, endLine range of lines to scan as line numbers
 */
void Fl_Text_Display::calc_line_starts( int startLine, int endLine ) {
  int startPos, bufLen = mBuffer->length();
  int line, lineEnd, nextLineStart, nVis = mNVisibleLines;
  int *lineStarts = mLineStarts;

  /* Clean up (possibly) messy input parameters */
  if ( endLine < 0 ) endLine = 0;
  if ( endLine >= nVis ) endLine = nVis - 1;
  if ( startLine < 0 ) startLine = 0;
  if ( startLine >= nVis ) startLine = nVis - 1;
  if ( startLine > endLine )
    return;

  /* Find the last known good line number -> position mapping */
  if ( startLine == 0 ) {
    lineStarts[ 0 ] = mFirstChar;
    startLine = 1;
  }
  startPos = lineStarts[ startLine - 1 ];

  /* If the starting position is already past the end of the text,
   fill in -1's (means no text on line) and return */
  if ( startPos == -1 ) {
    for ( line = startLine; line <= endLine; line++ )
      lineStarts[ line ] = -1;
    return;
  }

  /* Loop searching for ends of lines and storing the positions of the
   start of the next line in lineStarts */
  for ( line = startLine; line <= endLine; line++ ) {
    find_line_end(startPos, true, &lineEnd, &nextLineStart);
    startPos = nextLineStart;
    if ( startPos >= bufLen ) {
      /* If the buffer ends with a newline or line break, put
       buf->length() in the next line start position (instead of
       a -1 which is the normal marker for an empty line) to
       indicate that the cursor may safely be displayed there */
      if ( line == 0 || ( lineStarts[ line - 1 ] != bufLen &&
                         lineEnd != nextLineStart ) ) {
        lineStarts[ line ] = bufLen;
        line++;
      }
      break;
    }
    lineStarts[ line ] = startPos;
  }

  /* Set any entries beyond the end of the text to -1 */
  for ( ; line <= endLine; line++ )
    lineStarts[ line ] = -1;
}


/**
 \brief Update last display character index.

 Given a Fl_Text_Display with a complete, up-to-date lineStarts array, update
 the lastChar entry to point to the last buffer position displayed.
 */
void Fl_Text_Display::calc_last_char() {
  int i;
  for (i = mNVisibleLines - 1; i >= 0 && mLineStarts[i] == -1; i--) ;
  mLastChar = i < 0 ? 0 : line_end(mLineStarts[i], true);
}


/**
 \brief Scrolls the current buffer to start at the specified line and column.

 \param topLineNum top line number
 \param horizOffset column number
 \todo Column numbers make little sense here.
 */
void Fl_Text_Display::scroll(int topLineNum, int horizOffset) {
  mTopLineNumHint = topLineNum;
  mHorizOffsetHint = horizOffset;
  display_needs_recalc();     // resize(x(), y(), w(), h());
}


/**
 \brief Scrolls the current buffer to start at the specified line and column.

 \param topLineNum top line number
 \param horizOffset in pixels
 \return 0 if nothing changed, 1 if we scrolled
 */
int Fl_Text_Display::scroll_(int topLineNum, int horizOffset) {
  /* Limit the requested scroll position to allowable values */
  if (topLineNum > mNBufferLines + 3 - mNVisibleLines)
    topLineNum = mNBufferLines + 3 - mNVisibleLines;
  if (topLineNum < 1) topLineNum = 1;

  if (horizOffset > longest_vline() - text_area.w)
    horizOffset = longest_vline() - text_area.w;
  if (horizOffset < 0) horizOffset = 0;

  /* Do nothing if scroll position hasn't actually changed or there's no
   window to draw in yet */
  if (mHorizOffset == horizOffset && mTopLineNum == topLineNum)
    return 0;

  /* If the vertical scroll position has changed, update the line
   starts array and related counters in the text display */
  offset_line_starts(topLineNum);

  /* Just setting mHorizOffset is enough information for redisplay */
  mHorizOffset = horizOffset;

  // redraw all text
  damage(FL_DAMAGE_EXPOSE);
  return 1;
}


/**
 \brief Update vertical scrollbar.

 Update the minimum, maximum, slider size, page increment, and value
 for the vertical scrollbar.
 */
void Fl_Text_Display::update_v_scrollbar() {
  /* The vertical scrollbar value and slider size directly represent the top
   line number, and the number of visible lines respectively.  The scroll
   bar maximum value is chosen to generally represent the size of the whole
   buffer, with minor adjustments to keep the scrollbar widget happy */
#ifdef DEBUG
  printf("Fl_Text_Display::update_v_scrollbar():\n"
         "    mTopLineNum=%d, mNVisibleLines=%d, mNBufferLines=%d\n",
         mTopLineNum, mNVisibleLines, mNBufferLines);
#endif // DEBUG

  mVScrollBar->value(mTopLineNum, mNVisibleLines, 1, mNBufferLines+1+(mContinuousWrap?0:1));
  mVScrollBar->linesize(3);
}


/**
 \brief Update horizontal scrollbar.

 Update the minimum, maximum, slider size, page increment, and value
 for the horizontal scrollbar.
 */
void Fl_Text_Display::update_h_scrollbar() {
  int sliderMax = max(longest_vline(), text_area.w + mHorizOffset);
  mHScrollBar->value( mHorizOffset, text_area.w, 0, sliderMax );
}


/**
 \brief Callback for drag or valueChanged on vertical scrollbar.
 */
void Fl_Text_Display::v_scrollbar_cb(Fl_Scrollbar* b, Fl_Text_Display* textD) {
  if (b->value() == textD->mTopLineNum) return;
  textD->scroll(b->value(), textD->mHorizOffset);
}


/**
 \brief Callback for drag or valueChanged on horizontal scrollbar.
 */
void Fl_Text_Display::h_scrollbar_cb(Fl_Scrollbar* b, Fl_Text_Display* textD) {
  if (b->value() == textD->mHorizOffset) return;
  textD->scroll(textD->mTopLineNum, b->value());
}


/**
 \brief Refresh the line number area.
 \param clearAll -- (currently unused) If False, only draws the line number text,
                    does not clear the area behind it. If True, clears the area
                    and redraws the text. Use False to avoid a 'flash' for
                    single buffered windows.
 */

// This draw_line_numbers() method based on patch from
// http://www.mail-archive.com/fltk-dev@easysw.com/msg06376.html
// altered to support line numbers right alignment. -LZA / STR #2621
//
void Fl_Text_Display::draw_line_numbers(bool /*clearAll*/) {
  int Y, line, visLine, lineStart;
  char lineNumString[16];
  int lineHeight = mMaxsize;
  int isactive = active_r() ? 1 : 0;

  // Don't draw if lineNumWidth == 0 (line numbers are hidden),
  // or widget is not yet realized
  if (mLineNumWidth <= 0 || !visible_r())
    return;

  // Make sure we set the correct clipping range for line numbers.
  // Take scrollbars and positions into account.
  int hscroll_h = mHScrollBar->visible() ? mHScrollBar->h() : 0;
  int xoff = Fl::box_dx(box());
  int yoff = text_area.y - y();

#ifndef LINENUM_LEFT_OF_VSCROLL
  int vscroll_w = mVScrollBar->visible() ? mVScrollBar->w() : 0;
  if (scrollbar_align() & FL_ALIGN_LEFT)
    xoff += vscroll_w;
#endif

  Fl_Color fgcolor = isactive ? linenumber_fgcolor() : fl_inactive(linenumber_fgcolor());
  Fl_Color bgcolor = isactive ? linenumber_bgcolor() : fl_inactive(linenumber_bgcolor());
  fl_push_clip(x() + xoff,
               y() + Fl::box_dy(box()),
               mLineNumWidth,
               h() - Fl::box_dh(box()));
  {
    // Set background color for line number area -- LZA / STR# 2621
    // Erase background
    fl_color(bgcolor);
    fl_rectf(x()+xoff, y(), mLineNumWidth, h());

    // Draw separator line
    //fl_color(180,180,180);
    //fl_line(x()+mLineNumWidth-1, y(), x()+mLineNumWidth-1, y()+h());

    // Draw line number text
    fl_font(linenumber_font(), linenumber_size());

    Y = y() + yoff;
    line = get_absolute_top_line_number();

    // set font color for line numbers
    fl_color(fgcolor);
    for (visLine=0; visLine < mNVisibleLines; visLine++) {
      lineStart = mLineStarts[visLine];
      if (lineStart != -1 && (lineStart==0 || buffer()->char_at(lineStart-1)=='\n')) {
        snprintf(lineNumString, sizeof(lineNumString),
                 linenumber_format(), line);
        int xx = x() + xoff + 3,
            yy = Y,
            ww = mLineNumWidth - (3*2),
            hh = lineHeight;
        fl_draw(lineNumString, xx, yy, ww, hh, linenumber_align(), 0, 0);
        //DEBUG fl_rect(xx, yy, ww, hh);
        line++;
      } else {
        if (visLine == 0) line++;
      }
      Y += lineHeight;
    }
  }
  // fill the void area to the left of the horizontal scrollbar that exists
  // above or beneath the line number display (when on) with background color
  fl_color(FL_BACKGROUND_COLOR);
  if (scrollbar_align() & FL_ALIGN_TOP)
    fl_rectf(x() + xoff, y() + Fl::box_dy(box()), mLineNumWidth, hscroll_h);
  else
    fl_rectf(x() + xoff, y() + h() - hscroll_h - Fl::box_dy(box()), mLineNumWidth, hscroll_h + Fl::box_dy(box()));
  fl_pop_clip();
}

static int max( int i1, int i2 ) {
  return i1 >= i2 ? i1 : i2;
}

static int min( int i1, int i2 ) {
  return i1 <= i2 ? i1 : i2;
}


/**
 Count the number of newlines in a null-terminated text string;
 */
static int countlines( const char *string ) {
  IS_UTF8_ALIGNED(string)

  const char * c;
  int lineCount = 0;

  if (!string) return 0;

  for ( c = string; *c != '\0'; c++ )
    if ( *c == '\n' ) lineCount++;
  return lineCount;
}


/**
 \brief Returns the width in pixels of the displayed line pointed to by "visLineNum".
 \param visLineNum index into visible lines array
 \return width of line in pixels
 */
int Fl_Text_Display::measure_vline( int visLineNum ) const {
  int lineLen = vline_length( visLineNum );
  int lineStartPos = mLineStarts[ visLineNum ];
  if (lineStartPos < 0 || lineLen == 0) return 0;
  return handle_vline(GET_WIDTH, lineStartPos, lineLen, 0, 0, 0, 0, 0, 0);
}


/**
 \brief Return true if there are lines visible with no corresponding buffer text.
 \return 1 if there are empty lines
 */
int Fl_Text_Display::empty_vlines() const {
  return (mNVisibleLines > 0) && (mLineStarts[ mNVisibleLines - 1 ] == -1);
}


/**
 \brief Count number of bytes in a visible line.

 Return the length of a line (number of bytes) by examining
 entries in the line starts array rather than by scanning for newlines.

 \param visLineNum index of line in visible line array
 \return number of bytes in this line
 */
int Fl_Text_Display::vline_length( int visLineNum ) const {
  int nextLineStart, lineStartPos;

  if (visLineNum < 0 || visLineNum >= mNVisibleLines)
    return (0);

  lineStartPos = mLineStarts[ visLineNum ];

  if ( lineStartPos == -1 )
    return 0;

  if ( visLineNum + 1 >= mNVisibleLines )
    return mLastChar - lineStartPos;

  nextLineStart = mLineStarts[ visLineNum + 1 ];
  if ( nextLineStart == -1 )
    return mLastChar - lineStartPos;

  int nextLineStartMinus1 = buffer()->prev_char(nextLineStart);
  if (wrap_uses_character(nextLineStartMinus1))
    return nextLineStartMinus1 - lineStartPos;

  return nextLineStart - lineStartPos;
}


/**
 \brief Wrapping calculations.

 When continuous wrap is on, and the user inserts or deletes characters,
 wrapping can happen before and beyond the changed position.  This routine
 finds the extent of the changes, and counts the deleted and inserted lines
 over that range.  It also attempts to minimize the size of the range to
 what has to be counted and re-displayed, so the results can be useful
 both for delimiting where the line starts need to be recalculated, and
 for deciding what part of the text to redisplay.

 \param deletedText
 \param pos
 \param nInserted
 \param nDeleted
 \param modRangeStart
 \param modRangeEnd
 \param linesInserted
 \param linesDeleted
 */
void Fl_Text_Display::find_wrap_range(const char *deletedText, int pos,
                                      int nInserted, int nDeleted,
                                      int *modRangeStart, int *modRangeEnd,
                                      int *linesInserted, int *linesDeleted) {
  IS_UTF8_ALIGNED(deletedText)
  IS_UTF8_ALIGNED2(buffer(), pos)

  int length, retPos, retLines, retLineStart, retLineEnd;
  Fl_Text_Buffer *deletedTextBuf, *buf = buffer();
  int nVisLines = mNVisibleLines;
  int *lineStarts = mLineStarts;
  int countFrom, countTo, lineStart, adjLineStart, i;
  int visLineNum = 0, nLines = 0;

  /*
   ** Determine where to begin searching: either the previous newline, or
   ** if possible, limit to the start of the (original) previous displayed
   ** line, using information from the existing line starts array
   */
  if (pos >= mFirstChar && pos <= mLastChar) {
    for (i=nVisLines-1; i>0; i--) {
      if (lineStarts[i] != -1 && pos >= lineStarts[i]) {
        break;
      }
    }
    if (i > 0) {
      countFrom = lineStarts[i-1];
      visLineNum = i-1;
    } else {
      countFrom = buf->line_start(pos);
    }
  } else {
    countFrom = buf->line_start(pos);
  }

  IS_UTF8_ALIGNED2(buf, countFrom)

  /*
   ** Move forward through the (new) text one line at a time, counting
   ** displayed lines, and looking for either a real newline, or for the
   ** line starts to re-sync with the original line starts array
   */
  lineStart = countFrom;
  *modRangeStart = countFrom;
  for (;;) {

    /* advance to the next line.  If the line ended in a real newline
     or the end of the buffer, that's far enough */
    wrapped_line_counter(buf, lineStart, buf->length(), 1, true, 0,
                         &retPos, &retLines, &retLineStart, &retLineEnd);
    if (retPos >= buf->length()) {
      countTo = buf->length();
      *modRangeEnd = countTo;
      if (retPos != retLineEnd)
        nLines++;
      break;
    } else {
      lineStart = retPos;
    }
    nLines++;
    if (lineStart > pos + nInserted && buf->char_at(buf->prev_char(lineStart)) == '\n') {
      countTo = lineStart;
      *modRangeEnd = lineStart;
      break;
    }

    /* Don't try to resync in continuous wrap mode with non-fixed font
     sizes; it would result in a chicken-and-egg dependency between
     the calculations for the inserted and the deleted lines.
     If we're in that mode, the number of deleted lines is calculated in
     advance, without resynchronization, so we shouldn't resynchronize
     for the inserted lines either. */
    if (mSuppressResync)
      continue;

    /* check for synchronization with the original line starts array
     before pos, if so, the modified range can begin later */
    if (lineStart <= pos) {
      while (visLineNum<nVisLines && lineStarts[visLineNum] < lineStart)
        visLineNum++;
      if (visLineNum < nVisLines && lineStarts[visLineNum] == lineStart) {
        countFrom = lineStart;
        nLines = 0;
        if (visLineNum+1 < nVisLines && lineStarts[visLineNum+1] != -1)
          *modRangeStart = min(pos, buf->prev_char(lineStarts[visLineNum+1]));
        else
          *modRangeStart = countFrom;
      } else
        *modRangeStart = min(*modRangeStart, buf->prev_char(lineStart));
    }

    /* check for synchronization with the original line starts array
     after pos, if so, the modified range can end early */
    else if (lineStart > pos + nInserted) {
      adjLineStart = lineStart - nInserted + nDeleted;
      while (visLineNum<nVisLines && lineStarts[visLineNum]<adjLineStart)
        visLineNum++;
      if (visLineNum < nVisLines && lineStarts[visLineNum] != -1 &&
          lineStarts[visLineNum] == adjLineStart) {
        countTo = line_end(lineStart, true);
        *modRangeEnd = lineStart;
        break;
      }
    }
  }
  *linesInserted = nLines;


  /* Count deleted lines between countFrom and countTo as the text existed
   before the modification (that is, as if the text between pos and
   pos+nInserted were replaced by "deletedText").  This extra context is
   necessary because wrapping can occur outside of the modified region
   as a result of adding or deleting text in the region. This is done by
   creating a textBuffer containing the deleted text and the necessary
   additional context, and calling the wrappedLineCounter on it.

   NOTE: This must not be done in continuous wrap mode when the font
   width is not fixed. In that case, the calculation would try
   to access style information that is no longer available (deleted
   text), or out of date (updated highlighting), possibly leading
   to completely wrong calculations and/or even crashes eventually.
   (This is not theoretical; it really happened.)

   In that case, the calculation of the number of deleted lines
   has happened before the buffer was modified (only in that case,
   because resynchronization of the line starts is impossible
   in that case, which makes the whole calculation less efficient).
   */
  if (mSuppressResync) {
    *linesDeleted = mNLinesDeleted;
    mSuppressResync = 0;
    return;
  }

  length = (pos-countFrom) + nDeleted +(countTo-(pos+nInserted));
  deletedTextBuf = new Fl_Text_Buffer(length);
  deletedTextBuf->copy(buffer(), countFrom, pos, 0);
  if (nDeleted != 0)
    deletedTextBuf->insert(pos-countFrom, deletedText);
  deletedTextBuf->copy(buffer(), pos+nInserted, countTo, pos-countFrom+nDeleted);
  /* Note that we need to take into account an offset for the style buffer:
   the deletedTextBuf can be out of sync with the style buffer. */
  wrapped_line_counter(deletedTextBuf, 0, length, INT_MAX, true, countFrom,
                       &retPos, &retLines, &retLineStart, &retLineEnd, false);
  delete deletedTextBuf;
  *linesDeleted = retLines;
  mSuppressResync = 0;
}


/**
 \brief Wrapping calculations.

 This is a stripped-down version of the findWrapRange() function above,
 intended to be used to calculate the number of "deleted" lines during
 a buffer modification. It is called _before_ the modification takes place.

 This function should only be called in continuous wrap mode with a
 non-fixed font width. In that case, it is impossible to calculate
 the number of deleted lines, because the necessary style information
 is no longer available _after_ the modification. In other cases, we
 can still perform the calculation afterwards (possibly even more
 efficiently).

 \param pos
 \param nDeleted
 */
void Fl_Text_Display::measure_deleted_lines(int pos, int nDeleted) {
  IS_UTF8_ALIGNED2(buffer(), pos)

  int retPos, retLines, retLineStart, retLineEnd;
  Fl_Text_Buffer *buf = buffer();
  int nVisLines = mNVisibleLines;
  int *lineStarts = mLineStarts;
  int countFrom, lineStart;
  int nLines = 0, i;
  /*
   ** Determine where to begin searching: either the previous newline, or
   ** if possible, limit to the start of the (original) previous displayed
   ** line, using information from the existing line starts array
   */
  if (pos >= mFirstChar && pos <= mLastChar) {
    for (i=nVisLines-1; i>0; i--)
      if (lineStarts[i] != -1 && pos >= lineStarts[i])
        break;
    if (i > 0) {
      countFrom = lineStarts[i-1];
    } else
      countFrom = buf->line_start(pos);
  } else
    countFrom = buf->line_start(pos);

  /*
   ** Move forward through the (new) text one line at a time, counting
   ** displayed lines, and looking for either a real newline, or for the
   ** line starts to re-sync with the original line starts array
   */
  lineStart = countFrom;
  for (;;) {
    /* advance to the next line.  If the line ended in a real newline
     or the end of the buffer, that's far enough */
    wrapped_line_counter(buf, lineStart, buf->length(), 1, true, 0,
                         &retPos, &retLines, &retLineStart, &retLineEnd);
    if (retPos >= buf->length()) {
      if (retPos != retLineEnd)
        nLines++;
      break;
    } else
      lineStart = retPos;
    nLines++;
    if (lineStart > pos + nDeleted && buf->char_at(lineStart-1) == '\n') {
      break;
    }

    /* Unlike in the findWrapRange() function above, we don't try to
     resync with the line starts, because we don't know the length
     of the inserted text yet, nor the updated style information.

     Because of that, we also shouldn't resync with the line starts
     after the modification either, because we must perform the
     calculations for the deleted and inserted lines in the same way.

     This can result in some unnecessary recalculation and redrawing
     overhead, and therefore we should only use this two-phase mode
     of calculation when it's really needed (continuous wrap + variable
     font width). */
  }
  mNLinesDeleted = nLines;
  mSuppressResync = 1;
}


/**
 \brief Wrapping calculations.

 Count forward from startPos to either maxPos or maxLines (whichever is
 reached first), and return all relevant positions and line count.
 The provided textBuffer may differ from the actual text buffer of the
 widget. In that case it must be a (partial) copy of the actual text buffer
 and the styleBufOffset argument must indicate the starting position of the
 copy, to take into account the correct style information.

 \param[in] buf      The text buffer to operate on
 \param[in] startPos Starting index position into the buffer
 \param[in] maxPos   Maximum index position into the buffer we'll reach
 \param[in] maxLines Maximum number of lines we'll reach
 \param[in] startPosIsLineStart  Flag indicating if startPos is start of line.
                                 (If set, prevents our having to find the line start)
 \param[in] styleBufOffset Offset index position into style buffer.

 \param[out] retPos Position where counting ended.  When counting lines, the
    position returned is the start of the line "maxLines" lines
    beyond "startPos".
 \param[out] retLines Number of line breaks counted
 \param[out] retLineStart Start of the line where counting ended
 \param[out] retLineEnd End position of the last line traversed
 \param[out] countLastLineMissingNewLine
 */
void Fl_Text_Display::wrapped_line_counter(Fl_Text_Buffer *buf, int startPos,
                                           int maxPos, int maxLines, bool startPosIsLineStart, int styleBufOffset,
                                           int *retPos, int *retLines, int *retLineStart, int *retLineEnd,
                                           bool countLastLineMissingNewLine) const {
  IS_UTF8_ALIGNED2(buf, startPos)
  IS_UTF8_ALIGNED2(buf, maxPos)

  int lineStart, newLineStart = 0, b, p, colNum, wrapMarginPix;
  int i, foundBreak;
  double width;
  int nLines = 0;
  unsigned int c;

  /* Set the wrap margin to the wrap column or the view width */
  if (mWrapMarginPix != 0) {
    wrapMarginPix = mWrapMarginPix;
  } else {
    wrapMarginPix = text_area.w;
  }

  /* Find the start of the line if the start pos is not marked as a
   line start. */
  if (startPosIsLineStart)
    lineStart = startPos;
  else
    lineStart = line_start(startPos);

  /*
   ** Loop until position exceeds maxPos or line count exceeds maxLines.
   ** (actually, continues beyond maxPos to end of line containing maxPos,
   ** in case later characters cause a word wrap back before maxPos)
   */
  colNum = 0;
  width = 0;
  for (p=lineStart; p<buf->length(); p=buf->next_char(p)) {
    c = buf->char_at(p);  // UCS-4

    /* If the character was a newline, count the line and start over,
     otherwise, add it to the width and column counts */
    if (c == '\n') {
      if (p >= maxPos) {
        *retPos = maxPos;
        *retLines = nLines;
        *retLineStart = lineStart;
        *retLineEnd = maxPos;
        return;
      }
      nLines++;
      int p1 = buf->next_char(p);
      if (nLines >= maxLines) {
        *retPos = p1;
        *retLines = nLines;
        *retLineStart = p1;
        *retLineEnd = p;
        return;
      }
      lineStart = p1;
      colNum = 0;
      width = 0;
    } else {
      const char *s = buf->address(p);
      colNum++;
      // FIXME: it is not a good idea to simply add character widths because on
      // some platforms, the width is a floating point value and depends on the
      // previous character as well.
      width += measure_proportional_character(s, (int)width, p+styleBufOffset);
    }

    /* If character exceeded wrap margin, find the break point and wrap there */
    if (width > wrapMarginPix) {
      foundBreak = false;
      for (b=p; b>=lineStart; b=buf->prev_char(b)) {
        c = buf->char_at(b);
        if (c == '\t' || c == ' ') {
          newLineStart = buf->next_char(b);
          colNum = 0;
          width = 0;
          int iMax = buf->next_char(p);
          for (i=buf->next_char(b); i<iMax; i = buf->next_char(i)) {
            width += measure_proportional_character(buf->address(i), (int)width,
                                                    i+styleBufOffset);
            colNum++;
          }
          foundBreak = true;
          break;
        }
      }
      if (b<lineStart) b = lineStart;
      if (!foundBreak) { /* no whitespace, just break at margin */
        newLineStart = max(p, buf->next_char(lineStart));
        colNum++;
        if (b >= buf->length()) { // STR #2730
          width = 0;
        } else {
          const char *s = buf->address(b);
          width = measure_proportional_character(s, 0, p+styleBufOffset);
        }
      }
      if (p >= maxPos) {
        *retPos = maxPos;
        *retLines = maxPos < newLineStart ? nLines : nLines + 1;
        *retLineStart = maxPos < newLineStart ? lineStart : newLineStart;
        *retLineEnd = maxPos;
        return;
      }
      nLines++;
      if (nLines >= maxLines) {
        *retPos = foundBreak ? buf->next_char(b) : max(p, buf->next_char(lineStart));
        *retLines = nLines;
        *retLineStart = lineStart;
        *retLineEnd = foundBreak ? b : p;
        return;
      }
      lineStart = newLineStart;
    }
  }

  /* reached end of buffer before reaching pos or line target */
  *retPos = buf->length();
  *retLines = nLines;
  if (countLastLineMissingNewLine && colNum > 0)
    *retLines = buf->next_char(*retLines);
  *retLineStart = lineStart;
  *retLineEnd = buf->length();
}


/**
 \brief Wrapping calculations.

 Measure the width in pixels of the first character of string "s" at a
 particular column "colNum" and buffer position "pos".  This is for measuring
 characters in proportional or mixed-width highlighting fonts.

 A note about proportional and mixed-width fonts: the mixed width and
 proportional font code in nedit does not get much use in general editing,
 because nedit doesn't allow per-language-mode fonts, and editing programs
 in a proportional font is usually a bad idea, so very few users would
 choose a proportional font as a default.  There are still probably mixed-
 width syntax highlighting cases where things don't redraw properly for
 insertion/deletion, though static display and wrapping and resizing
 should now be solid because they are now used for online help display.

 \param s text string
 \param xPix x pixel position needed for calculating tab widths
 \param pos offset within string
 \return width of character in pixels
 */
double Fl_Text_Display::measure_proportional_character(const char *s, int xPix, int pos) const {
  IS_UTF8_ALIGNED(s)

  if (*s=='\t') {
    int tab = (int)col_to_x(mBuffer->tab_distance());
    return (((xPix/tab)+1)*tab) - xPix;
  }

  int charLen = fl_utf8len1(*s), style = 0;
  if (mStyleBuffer) {
    style = mStyleBuffer->byte_at(pos);
  }
  return string_width(s, charLen, style);
}


/**
 \brief Finds both the end of the current line and the start of the next line.

 Why?
 In continuous wrap mode, if you need to know both, figuring out one from the
 other can be expensive or error prone.  The problem comes when there's a
 trailing space or tab just before the end of the buffer.  To translate an
 end of line value to or from the next lines start value, you need to know
 whether the trailing space or tab is being used as a line break or just a
 normal character, and to find that out would otherwise require counting all
 the way back to the beginning of the line.

 \param startPos
 \param startPosIsLineStart
 \param[out] lineEnd
 \param[out] nextLineStart
 */
void Fl_Text_Display::find_line_end(int startPos, bool startPosIsLineStart,
                                    int *lineEnd, int *nextLineStart) const {
  IS_UTF8_ALIGNED2(buffer(), startPos)

  int retLines, retLineStart;

  /* if we're not wrapping use more efficient Fl_Text_Buffer::line_end() */
  if (!mContinuousWrap) {
    int le = buffer()->line_end(startPos);
    int ls = buffer()->next_char(le);
    *lineEnd = le;
    *nextLineStart = min(buffer()->length(), ls);
    return;
  }

  /* use the wrapped line counter routine to count forward one line */
  wrapped_line_counter(buffer(), startPos, buffer()->length(),
                       1, startPosIsLineStart, 0, nextLineStart, &retLines,
                       &retLineStart, lineEnd);
}


/**
 \brief Check if the line break is caused by a newline or by line wrapping.

 Line breaks in continuous wrap mode usually happen at newlines (\\n) or
 whitespace.  This line-terminating character is not included in line
 width measurements and has a special status as a non-visible character.
 However, lines with no whitespace are wrapped without the benefit of a
 line terminating character, and this distinction causes endless trouble
 with all of the text display code which was originally written without
 continuous wrap mode and always expects to wrap at a newline character.

 Given the position of the end of the line, as returned by Fl_Text_Display::line_end()
 or Fl_Text_Buffer::line_end(), this returns true if there is a line terminating
 character, and false if there's not.  On the last character in the
 buffer, this function can't tell for certain whether a trailing space was
 used as a wrap point, and just guesses that it wasn't.  So if an exact
 accounting is necessary, don't use this function.

 \param lineEndPos index of character where the line wraps
 \return 1 if a \\n character causes the line wrap
 */
int Fl_Text_Display::wrap_uses_character(int lineEndPos) const {
  IS_UTF8_ALIGNED2(buffer(), lineEndPos)

  unsigned int c;

  if (!mContinuousWrap || lineEndPos == buffer()->length())
    return 1;

  c = buffer()->char_at(lineEndPos);
  return c == '\n' || ((c == '\t' || c == ' ') &&
                       lineEndPos + 1 < buffer()->length());
}


/**
 \brief I don't know what this does!

 Extend the range of a redraw request (from *start to *end) with additional
 redraw requests resulting from changes to the attached style buffer (which
 contains auxiliary information for coloring or styling text).

 \param startpos ??
 \param endpos ??

 \todo Unicode?
 */
void Fl_Text_Display::extend_range_for_styles( int *startpos, int *endpos ) {
  IS_UTF8_ALIGNED2(buffer(), (*startpos))
  IS_UTF8_ALIGNED2(buffer(), (*endpos))

  Fl_Text_Selection * sel = mStyleBuffer->primary_selection();
  int extended = 0;

  /* The peculiar protocol used here is that modifications to the style
   buffer are marked by selecting them with the buffer's primary Fl_Text_Selection.
   The style buffer is usually modified in response to a modify callback on
   the text buffer BEFORE Fl_Text_Display.c's modify callback, so that it can keep
   the style buffer in step with the text buffer.  The style-update
   callback can't just call for a redraw, because Fl_Text_Display hasn't processed
   the original text changes yet.  Anyhow, to minimize redrawing and to
   avoid the complexity of scheduling redraws later, this simple protocol
   tells the text display's buffer modify callback to extend its redraw
   range to show the text color/and font changes as well. */
  if ( sel->selected() ) {
    if ( sel->start() < *startpos ) {
      *startpos = sel->start();
      // somewhere while deleting, alignment is lost. We do this just to be sure.
      *startpos = buffer()->utf8_align(*startpos);
      IS_UTF8_ALIGNED2(buffer(), (*startpos))
      extended = 1;
    }
    if ( sel->end() > *endpos ) {
      *endpos = sel->end();
      *endpos = buffer()->utf8_align(*endpos);
      IS_UTF8_ALIGNED2(buffer(), (*endpos))
      extended = 1;
    }
  }

  /* If the Fl_Text_Selection was extended due to a style change, and some of the
   fonts don't match in spacing, extend redraw area to end of line to
   redraw characters exposed by possible font size changes */
  if ( extended )
    *endpos = mBuffer->line_end( *endpos ) + 1;

  IS_UTF8_ALIGNED2(buffer(), (*endpos))
}


/**
 \brief Draw the widget.

 This function tries to limit drawing to smaller areas if possible.
 */
void Fl_Text_Display::draw(void) {
  // don't even try if there is no associated text buffer!
  if (!buffer()) { draw_box(); return; }

  // recalc if needed -- issue #300
  if (display_needs_recalc_ || (damage() & FL_DAMAGE_ALL)) {
    display_needs_recalc_ = false;
    recalc_display();
  }

  fl_push_clip(x(),y(),w(),h());        // prevent drawing outside widget area

  // background color -- change if inactive
  Fl_Color bgcolor = active_r() ? color() : fl_inactive(color());

  // draw the non-text, non-scrollbar areas.
  if (damage() & FL_DAMAGE_ALL) {
    recalc_display();
    //    printf("drawing all (box = %d)\n", box());
    if (Fl_Surface_Device::surface() != Fl_Display_Device::display_device()) {
      // if to printer, draw the background
      fl_rectf(text_area.x, text_area.y, text_area.w, text_area.h, bgcolor);
    }

    // draw the box()
    draw_box(box(), x(), y(), w(), h(), bgcolor);

    // left margin
    fl_rectf(text_area.x-LEFT_MARGIN, text_area.y-TOP_MARGIN,
             LEFT_MARGIN, text_area.h+TOP_MARGIN+BOTTOM_MARGIN,
             bgcolor);

    // right margin
    fl_rectf(text_area.x+text_area.w, text_area.y-TOP_MARGIN,
             RIGHT_MARGIN, text_area.h+TOP_MARGIN+BOTTOM_MARGIN,
             bgcolor);

    // top margin
    fl_rectf(text_area.x, text_area.y-TOP_MARGIN,
             text_area.w, TOP_MARGIN, bgcolor);

    // bottom margin
    fl_rectf(text_area.x, text_area.y+text_area.h,
             text_area.w, BOTTOM_MARGIN, bgcolor);

    // draw that little box in the corner of the scrollbars
    if (mVScrollBar->visible() && mHScrollBar->visible())
      fl_rectf(mVScrollBar->x(), mHScrollBar->y(),
               mVScrollBar->w(), mHScrollBar->h(),
               FL_BACKGROUND_COLOR);
  }
  else if (damage() & (FL_DAMAGE_SCROLL | FL_DAMAGE_EXPOSE)) {
    //    printf("blanking previous cursor extrusions at Y: %d\n", mCursorOldY);
    // CET - FIXME - save old cursor position instead and just draw side needed?
    fl_push_clip(text_area.x-LEFT_MARGIN,
                 text_area.y,
                 text_area.w+LEFT_MARGIN+RIGHT_MARGIN,
                 text_area.h);
    fl_rectf(text_area.x-LEFT_MARGIN, mCursorOldY,
             LEFT_MARGIN, mMaxsize, bgcolor);
    fl_rectf(text_area.x+text_area.w, mCursorOldY,
             RIGHT_MARGIN, mMaxsize, bgcolor);
    fl_pop_clip();
  }

  // draw the scrollbars
  if (damage() & (FL_DAMAGE_ALL | FL_DAMAGE_CHILD)) {
    mVScrollBar->damage(FL_DAMAGE_ALL);
    mHScrollBar->damage(FL_DAMAGE_ALL);
  }
  update_child(*mVScrollBar);
  update_child(*mHScrollBar);

  // draw all of the text
  if (damage() & (FL_DAMAGE_ALL | FL_DAMAGE_EXPOSE)) {
    //printf("drawing all text\n");
    int X = 0, Y = 0, W = 0, H = 0;
    if (fl_clip_box(text_area.x, text_area.y, text_area.w, text_area.h,
                    X, Y, W, H)) {
      // Draw text using the intersected clipping box...
      // (this sets the clipping internally)
      draw_text(X, Y, W, H);
    } else {
      // Draw the whole area...
      draw_text(text_area.x, text_area.y, text_area.w, text_area.h);
    }
  }
  else if (damage() & FL_DAMAGE_SCROLL) {
    // draw some lines of text
    fl_push_clip(text_area.x, text_area.y,
                 text_area.w, text_area.h);
    //printf("drawing text from %d to %d\n", damage_range1_start, damage_range1_end);
    draw_range(damage_range1_start, damage_range1_end);
    if (damage_range2_end != -1) {
      //printf("drawing text from %d to %d\n", damage_range2_start, damage_range2_end);
      draw_range(damage_range2_start, damage_range2_end);
    }
    damage_range1_start = damage_range1_end = -1;
    damage_range2_start = damage_range2_end = -1;
    fl_pop_clip();
  }

  // draw the text cursor
  int start, end;
  int has_selection = buffer()->selection_position(&start, &end);
  if (damage() & (FL_DAMAGE_ALL | FL_DAMAGE_SCROLL | FL_DAMAGE_EXPOSE)
      && (
          (Fl::screen_driver()->has_marked_text() && Fl::compose_state) ||
          (!has_selection) || mCursorPos < start || mCursorPos > end) &&
      mCursorOn && Fl::focus() == (Fl_Widget*)this ) {
    fl_push_clip(text_area.x-LEFT_MARGIN,
                 text_area.y,
                 text_area.w+LEFT_MARGIN+RIGHT_MARGIN,
                 text_area.h);

    int X = 0, Y = 0;
    if (position_to_xy(mCursorPos, &X, &Y)) {
      draw_cursor(X, Y);
      mCursorOldY = Y;
    }
    //    else puts("position_to_xy() failed - unable to draw cursor!");
    //printf("drew cursor at pos: %d (%d,%d)\n", mCursorPos, X, Y);
    fl_pop_clip();
  }

  // Important to do this at end of this method, otherwise line numbers
  // will not scroll with the text edit area
  draw_line_numbers(true);

  fl_pop_clip();
}

// GitHub Issue #196: internal selection and visible selection can run out of
// sync, giving the user unexpected keyboard selection. The code block below
// captures that and fixes it.
// - set pos to the drag target postion or -1 if we don't know
// - if pos is -1, and key is not -1, key can be set to indicate a direction
//   (e.g. FL_Left)
// return 0 if nothing changed, return 1 if dragPos or mCursorPos were modified
int fl_text_drag_prepare(int pos, int key, Fl_Text_Display* d) {
  if (d->buffer()->selected()) {
    int start, end;
    d->buffer()->selection_position(&start, &end);
    if ( (d->dragPos!=start || d->mCursorPos!=end) && (d->dragPos!=end || d->mCursorPos!=start) ) {
      if (pos!=-1) {
        if (pos<start) {
          d->mCursorPos = start;
          d->dragPos = end;
        } else {
          d->mCursorPos = end;
          d->dragPos = start;
        }
      } else if (key!=-1) {
        switch (key) {
          case FL_Home: case FL_Left: case FL_Up: case FL_Page_Up:
            d->dragPos = end; d->mCursorPos = start; break;
          default:
            d->dragPos = start; d->mCursorPos = end; break;
        }
      } else {
        d->dragPos = start;
        d->mCursorPos = end;
      }
      return 1;
    }
  }
  return 0;
}

// this processes drag events due to mouse for Fl_Text_Display and
// also drags due to cursor movement with shift held down for
// Fl_Text_Editor
void fl_text_drag_me(int pos, Fl_Text_Display* d) {
  if (d->dragType == Fl_Text_Display::DRAG_CHAR) {
    if (pos >= d->dragPos) {
      d->buffer()->select(d->dragPos, pos);
    } else {
      d->buffer()->select(pos, d->dragPos);
    }
    d->insert_position(pos);
  } else if (d->dragType == Fl_Text_Display::DRAG_WORD) {
    if (pos >= d->dragPos) {
      d->insert_position(d->word_end(pos));
      d->buffer()->select(d->word_start(d->dragPos), d->word_end(pos));
    } else {
      d->insert_position(d->word_start(pos));
      d->buffer()->select(d->word_start(pos), d->word_end(d->dragPos));
    }
  } else if (d->dragType == Fl_Text_Display::DRAG_LINE) {
    if (pos >= d->dragPos) {
      d->insert_position(d->buffer()->line_end(pos)+1);
      d->buffer()->select(d->buffer()->line_start(d->dragPos),
                          d->buffer()->line_end(pos)+1);
    } else {
      d->insert_position(d->buffer()->line_start(pos));
      d->buffer()->select(d->buffer()->line_start(pos),
                          d->buffer()->line_end(d->dragPos)+1);
    }
  }
}


/**
 \brief Timer callback for scroll events.

 This timer event scrolls the text view proportionally to
 how far the mouse pointer has left the text area. This
 allows for smooth scrolling without "wiggeling" the mouse.
 */
void Fl_Text_Display::scroll_timer_cb(void *user_data) {
  Fl_Text_Display *w = (Fl_Text_Display*)user_data;
  int pos;
  switch (scroll_direction) {
    case 1: // mouse is to the right, scroll left
      w->scroll(w->mTopLineNum, w->mHorizOffset + scroll_amount);
      pos = w->xy_to_position(w->text_area.x + w->text_area.w, scroll_y, CURSOR_POS);
      break;
    case 2: // mouse is to the left, scroll right
      w->scroll(w->mTopLineNum, w->mHorizOffset + scroll_amount);
      pos = w->xy_to_position(w->text_area.x, scroll_y, CURSOR_POS);
      break;
    case 3: // mouse is above, scroll down
      w->scroll(w->mTopLineNum + scroll_amount, w->mHorizOffset);
      pos = w->xy_to_position(scroll_x, w->text_area.y, CURSOR_POS);
      break;
    case 4: // mouse is below, scroll up
      w->scroll(w->mTopLineNum + scroll_amount, w->mHorizOffset);
      pos = w->xy_to_position(scroll_x, w->text_area.y + w->text_area.h, CURSOR_POS);
      break;
    default:
      return;
  }
  fl_text_drag_me(pos, w);
  Fl::repeat_timeout(.1, scroll_timer_cb, user_data);
}


/** Handle right mouse button down events.
 \return 0 for no op, 1 to cut, 2 to copy, 3 to paste
 */
int Fl_Text_Display::handle_rmb(int readonly) {
  Fl_Text_Buffer *txtbuf = buffer();
  int newpos = xy_to_position(Fl::event_x(), Fl::event_y(), CURSOR_POS);
  int oldpos = txtbuf->primary_selection()->start();
  int oldmark = txtbuf->primary_selection()->end();
  if (   ((oldpos < newpos) && (oldmark > newpos))
      || ((oldmark < newpos) && (oldpos > newpos))
      || (type() == FL_SECRET_INPUT)) {
    // if the user clicked inside an existing selection, keep
    // the selection
  } else {
    if ((txtbuf->char_at(newpos) == 0) || (txtbuf->char_at(newpos) == '\n')) {
      // if clicked to the right of the line or text end, clear the
      // selection and set the cursor at the end of the line
      txtbuf->select(newpos, newpos);
    } else {
      // if clicked on a word, select the entire word
      txtbuf->select(txtbuf->word_start(newpos), txtbuf->word_end(newpos));
    }
  }
  // keep the menu labels current
  rmb_menu[0].label(Fl_Input::cut_menu_text);
  rmb_menu[1].label(Fl_Input::copy_menu_text);
  rmb_menu[2].label(Fl_Input::paste_menu_text);
  // give only the menu options that make sense
  if (readonly) {
    rmb_menu[0].deactivate(); // cut
    rmb_menu[2].deactivate(); // paste
  } else {
    rmb_menu[0].activate(); // cut
    rmb_menu[2].activate(); // paste
  }
  // pop up the menu
  fl_cursor(FL_CURSOR_DEFAULT);
  const Fl_Menu_Item *mi = rmb_menu->popup(Fl::event_x(), Fl::event_y());
  if (mi) return (int)mi->argument();
  return 0;
}

/**
 \brief Event handling.
 */
int Fl_Text_Display::handle(int event) {
  if (!buffer()) return 0;
  // This isn't very elegant!
  if (!Fl::event_inside(text_area.x, text_area.y, text_area.w, text_area.h) &&
      !dragging && event != FL_LEAVE && event != FL_ENTER &&
      event != FL_MOVE && event != FL_FOCUS && event != FL_UNFOCUS &&
      event != FL_KEYBOARD && event != FL_KEYUP && event != FL_MOUSEWHEEL) {
    return Fl_Group::handle(event);
  }

  switch (event) {
    case FL_ENTER:
    case FL_MOVE:
      if (active_r()) {
        if (Fl::event_inside(text_area.x, text_area.y, text_area.w,
                             text_area.h)) window()->cursor(FL_CURSOR_INSERT);
        else window()->cursor(FL_CURSOR_DEFAULT);
        return 1;
      } else {
        return 0;
      }

    case FL_LEAVE:
    case FL_HIDE:
      if (active_r() && window()) {
        window()->cursor(FL_CURSOR_DEFAULT);

        return 1;
      } else {
        return 0;
      }

    case FL_PUSH: {
      if (active_r() && window()) {
        if (Fl::event_inside(text_area.x, text_area.y, text_area.w,
                             text_area.h)) window()->cursor(FL_CURSOR_INSERT);
        else window()->cursor(FL_CURSOR_DEFAULT);
      }

      if (Fl::focus() != this) {
        Fl::focus(this);
        handle(FL_FOCUS);
      }
      if (Fl_Group::handle(event)) return 1;

      if (Fl::event_button() == FL_RIGHT_MOUSE) {
        switch (handle_rmb(1)) {
          case 2: {
            if (!buffer()->selected()) break;
            const char *copy = buffer()->selection_text();
            if (*copy) Fl::copy(copy, (int) strlen(copy), 1);
            free((void*)copy);
            show_insert_position();
            break;
          }
        }
        return 1;
      }

      if (Fl::event_state()&FL_SHIFT) {
        if (buffer()->primary_selection()->selected()) {
          int pos = xy_to_position(Fl::event_x(), Fl::event_y(), CURSOR_POS);
          fl_text_drag_prepare(pos, -1, this);
        } else {
          dragPos = insert_position();
        }
        return handle(FL_DRAG);
      }
      dragging = 1;
      int pos = xy_to_position(Fl::event_x(), Fl::event_y(), CURSOR_POS);
      dragPos = pos;
      if (buffer()->primary_selection()->includes(pos)) {
        dragType = DRAG_START_DND;
        return 1;
      }
      dragType = Fl::event_clicks();
      if (dragType == DRAG_CHAR) {
        buffer()->unselect();
//      Fl::copy("", 0, 0); /* removed for STR 2668 */
      }
      else if (dragType == DRAG_WORD) {
        buffer()->select(word_start(pos), word_end(pos));
        dragPos = word_start(pos);
        }

      if (buffer()->primary_selection()->selected())
        insert_position(buffer()->primary_selection()->end());
      else
        insert_position(pos);
      show_insert_position();
      return 1;
    }

    case FL_DRAG: {
      if (dragType==DRAG_NONE)
        return 1;
      if (dragType==DRAG_START_DND) {
        if (!Fl::event_is_click() && Fl::dnd_text_ops()) {
          const char* copy = buffer()->selection_text();
          Fl::copy(copy, (int)strlen(copy));
          Fl::screen_driver()->dnd(1);
          free((void*)copy);
        }
        return 1;
      }
      int X = Fl::event_x(), Y = Fl::event_y(), pos = insert_position();
      // if we leave the text_area, we start a timer event
      // that will take care of scrolling and selecting
      if (Y < text_area.y) {
        scroll_x = X;
        scroll_amount = (Y - text_area.y) / 5 - 1;
        if (!scroll_direction) {
          Fl::add_timeout(.01, scroll_timer_cb, this);
        }
        scroll_direction = 3;
      } else if (Y >= text_area.y+text_area.h) {
        scroll_x = X;
        scroll_amount = (Y - text_area.y - text_area.h) / 5 + 1;
        if (!scroll_direction) {
          Fl::add_timeout(.01, scroll_timer_cb, this);
        }
        scroll_direction = 4;
      } else if (X < text_area.x) {
        scroll_y = Y;
        scroll_amount = (X - text_area.x) / 2 - 1;
        if (!scroll_direction) {
          Fl::add_timeout(.01, scroll_timer_cb, this);
        }
        scroll_direction = 2;
      } else if (X >= text_area.x+text_area.w) {
        scroll_y = Y;
        scroll_amount = (X - text_area.x - text_area.w) / 2 + 1;
        if (!scroll_direction) {
          Fl::add_timeout(.01, scroll_timer_cb, this);
        }
        scroll_direction = 1;
      } else {
        if (scroll_direction) {
          Fl::remove_timeout(scroll_timer_cb, this);
          scroll_direction = 0;
        }
        pos = xy_to_position(X, Y, CURSOR_POS);
      }
      fl_text_drag_me(pos, this);
      return 1;
    }

    case FL_RELEASE: {
      if (Fl::event_is_click() && (! Fl::event_clicks()) &&
          buffer()->primary_selection()->includes(dragPos) && !(Fl::event_state()&FL_SHIFT) ) {
        buffer()->unselect(); // clicking in the selection: unselect and move cursor
        insert_position(dragPos);
        dragType = DRAG_CHAR;
        return 1;
      } else if (Fl::event_clicks() == DRAG_LINE && Fl::event_button() == FL_LEFT_MOUSE) {
        buffer()->select(buffer()->line_start(dragPos), buffer()->next_char(buffer()->line_end(dragPos)));
        dragPos = line_start(dragPos);
        dragType = DRAG_CHAR;
      } else {
        dragging = 0;
        if (scroll_direction) {
          Fl::remove_timeout(scroll_timer_cb, this);
          scroll_direction = 0;
        }

        // convert from WORD or LINE selection to CHAR
        /*if (insert_position() >= dragPos)
          dragPos = buffer()->primary_selection()->start();
        else
          dragPos = buffer()->primary_selection()->end();*/
        dragType = DRAG_CHAR;
      }

      const char* copy = buffer()->selection_text();
      if (*copy) Fl::copy(copy, (int) strlen(copy), 0);
      free((void*)copy);
      return 1;
    }

    case FL_MOUSEWHEEL:
      if (Fl::e_dy && mVScrollBar->visible()) {
        // Issue #879
        Fl_Scrollbar *vs = mVScrollBar;
        if ((Fl::e_dy < 0) && (vs->value() == int(vs->minimum()))) return 0;  // hit top? ignore
        if ((Fl::e_dy > 0) && (vs->value() == int(vs->maximum()))) return 0;  // hit bot? ignore
        return vs->handle(event);
      } else if (Fl::e_dx && mHScrollBar->visible()) {
        // Issue #879
        Fl_Scrollbar *hs = mHScrollBar;
        if ((Fl::e_dx < 0) && (hs->value() == int(hs->minimum()))) return 0;  // hit left? ignore
        if ((Fl::e_dx > 0) && (hs->value() == int(hs->maximum()))) return 0;  // hit right? ignore
        return hs->handle(event);
      }
      return 0;

    case FL_UNFOCUS:
      if (active_r() && window()) window()->cursor(FL_CURSOR_DEFAULT);
    case FL_FOCUS:
      if (buffer()->selected()) {
        int start, end;
        if (buffer()->selection_position(&start, &end))
          redisplay_range(start, end);
      }
      if (buffer()->secondary_selected()) {
        int start, end;
        if (buffer()->secondary_selection_position(&start, &end))
          redisplay_range(start, end);
      }
      if (buffer()->highlight()) {
        int start, end;
        if (buffer()->highlight_position(&start, &end))
          redisplay_range(start, end);
      }
      return 1;

    case FL_KEYBOARD:
      // Copy?
      if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='c') {
        if (!buffer()->selected()) return 1;
        const char *copy = buffer()->selection_text();
        if (*copy) Fl::copy(copy, (int) strlen(copy), 1);
        free((void*)copy);
        return 1;
      }

      // Select all ?
      if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='a') {
        buffer()->select(0,buffer()->length());
        const char *copy = buffer()->selection_text();
        if (*copy) Fl::copy(copy, (int) strlen(copy), 0);
        free((void*)copy);
        return 1;
      }

      if (mVScrollBar->handle(event)) return 1;
      if (mHScrollBar->handle(event)) return 1;

      break;

    case FL_SHORTCUT:
      if (!(shortcut() ? Fl::test_shortcut(shortcut()) : test_shortcut()))
        return 0;
      if (Fl::visible_focus() && handle(FL_FOCUS)) {
        Fl::focus(this);
        return 1;
      }
      break;

  }

  return 0;
}


/*
 Convert an x pixel position into a column number.
 The width of a column is calculated as the average width of a few
 representative characters, giving a good estimate for proportional fonts.
 This method does not take the possition of the scroll bars into account.
 \param[in] x offset to the left edge of the text in FLTK units.
 \return approximation to the corresponding text column
 \see col_to_x()
 */
double Fl_Text_Display::x_to_col(double x) const
{
  if (!mColumnScale) {
    mColumnScale = string_width("Mitg", 4, 'A') / 4.0;
  }
  return (x/mColumnScale)+0.5;
}


/**
 Convert a column number into an x pixel position.
 \see x_to_col()
 */
double Fl_Text_Display::col_to_x(double col) const
{
  if (!mColumnScale) {
    // recalculate column scale value
    x_to_col(0);
  }
  return col*mColumnScale;
}
