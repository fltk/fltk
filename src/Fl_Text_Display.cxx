//
// "$Id: Fl_Text_Display.cxx,v 1.12.2.1 2001/08/04 12:21:33 easysw Exp $"
//
// Copyright Mark Edel.  Permission to distribute under the LGPL for
// the FLTK library granted by Mark Edel.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <FL/Fl.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#undef min
#undef max

// Text area margins.  Left & right margins should be at least 3 so that
// there is some room for the overhanging parts of the cursor!
#define TOP_MARGIN 1
#define BOTTOM_MARGIN 1
#define LEFT_MARGIN 3
#define RIGHT_MARGIN 3

#define NO_HINT -1

/* Masks for text drawing methods.  These are or'd together to form an
   integer which describes what drawing calls to use to draw a string */
#define FILL_MASK 0x100
#define SECONDARY_MASK 0x200
#define PRIMARY_MASK 0x400
#define HIGHLIGHT_MASK 0x800
#define STYLE_LOOKUP_MASK 0xff

/* Maximum displayable line length (how many characters will fit across the
   widest window).  This amount of memory is temporarily allocated from the
   stack in the draw_vline() method for drawing strings */
#define MAX_DISP_LINE_LEN 1000

static int max( int i1, int i2 );
static int min( int i1, int i2 );
static int countlines( const char *string );

// CET - FIXME
#define TMPFONTWIDTH 6

Fl_Text_Display::Fl_Text_Display(int X, int Y, int W, int H,  const char* l)
    : Fl_Group(X, Y, W, H, l) {
  mMaxsize = 0;
  damage_range1_start = damage_range1_end = -1;
  damage_range2_start = damage_range2_end = -1;
  dragPos = dragType = dragging = 0;
  display_insert_position_hint = 0;

  box(FL_DOWN_FRAME);
  textsize(FL_NORMAL_SIZE);
  textcolor(FL_BLACK);
  textfont(FL_HELVETICA);

  Fl_Group* current = Fl_Group::current();
  Fl_Group::current(this);

  mVScrollBar = new Fl_Scrollbar(0,0,0,0);
  mVScrollBar->callback((Fl_Callback*)v_scrollbar_cb, this);
  mHScrollBar = new Fl_Scrollbar(0,0,0,0);
  mHScrollBar->callback((Fl_Callback*)h_scrollbar_cb, this);
  mHScrollBar->type(FL_HORIZONTAL);

  Fl_Group::current(current);

  scrollbar_width(16);
  scrollbar_align(FL_ALIGN_BOTTOM_RIGHT);

  mCursorOn = 0;
  mCursorPos = 0;
  mCursorOldY = -100;
  mCursorToHint = NO_HINT;
  mCursorStyle = NORMAL_CURSOR;
  mCursorPreferredCol = -1;
  mBuffer = 0;
  mFirstChar = 0;
  mLastChar = 0;
  mNBufferLines = 0;
  mTopLineNum = mTopLineNumHint = 1;
  mHorizOffset = mHorizOffsetHint = 0;

  mCursor_color = FL_BLACK;

  mFixedFontWidth = TMPFONTWIDTH;// CET - FIXME
  mStyleBuffer = 0;
  mStyleTable = 0;
  mNStyles = 0;
  mNVisibleLines = 1;
  mLineStarts = new int[mNVisibleLines];
  mLineStarts[0] = 0;
}

/*
** Free a text display and release its associated memory.  Note, the text
** BUFFER that the text display displays is a separate entity and is not
** freed, nor are the style buffer or style table.
*/
Fl_Text_Display::~Fl_Text_Display() {
  delete mVScrollBar;
  delete mHScrollBar;

  if (mBuffer) mBuffer->remove_modify_callback(buffer_modified_cb, this);
  delete[] mLineStarts;
}

/*
** Attach a text buffer to display, replacing the current buffer (if any)
*/
void Fl_Text_Display::buffer( Fl_Text_Buffer *buf ) {
  /* If the text display is already displaying a buffer, clear it off
     of the display and remove our callback from it */
  if ( mBuffer != 0 ) {
    buffer_modified_cb( 0, 0, mBuffer->length(), 0, 0, this );
    mBuffer->remove_modify_callback( buffer_modified_cb, this );
  }

  /* Add the buffer to the display, and attach a callback to the buffer for
     receiving modification information when the buffer contents change */
  mBuffer = buf;
  mBuffer->add_modify_callback( buffer_modified_cb, this );

  /* Update the display */
  buffer_modified_cb( 0, buf->length(), 0, 0, 0, this );
}

/*
** Attach (or remove) highlight information in text display and redisplay.
** Highlighting information consists of a style buffer which parallels the
** normal text buffer, but codes font and color information for the display;
** a style table which translates style buffer codes (indexed by buffer
** character - 'A') into fonts and colors; and a callback mechanism for
** as-needed highlighting, triggered by a style buffer entry of
** "unfinishedStyle".  Style buffer can trigger additional redisplay during
** a normal buffer modification if the buffer contains a primary Fl_Text_Selection
** (see extendRangeForStyleMods for more information on this protocol).
**
** Style buffers, tables and their associated memory are managed by the caller.
*/
void
Fl_Text_Display::highlight_data(Fl_Text_Buffer *styleBuffer,
                                Style_Table_Entry *styleTable,
                                int nStyles, char unfinishedStyle,
                                Unfinished_Style_Cb unfinishedHighlightCB,
                                void *cbArg ) {
  mStyleBuffer = styleBuffer;
  mStyleTable = styleTable;
  mNStyles = nStyles;
  mUnfinishedStyle = unfinishedStyle;
  mUnfinishedHighlightCB = unfinishedHighlightCB;
  mHighlightCBArg = cbArg;

  damage(FL_DAMAGE_EXPOSE);
}

int Fl_Text_Display::longest_vline() {
  int longest = 0;
  for (int i = 0; i < mNVisibleLines; i++)
    longest = max(longest, measure_vline(i));
  return longest;
}

/*
** Change the size of the displayed text area
*/
void Fl_Text_Display::resize(int X, int Y, int W, int H) {
  Fl_Widget::resize(X,Y,W,H);
  if (!buffer() || !visible_r()) return;
  X += Fl::box_dx(box());
  Y += Fl::box_dy(box());
  W -= Fl::box_dw(box());
  H -= Fl::box_dh(box());

  text_area.x = X+LEFT_MARGIN;
  text_area.y = Y+BOTTOM_MARGIN;
  text_area.w = W-LEFT_MARGIN-RIGHT_MARGIN;
  text_area.h = H-TOP_MARGIN-BOTTOM_MARGIN;
  int i;

  /* Find the new maximum font height for this text display */
  for (i = 0, mMaxsize = fl_height(textfont(), textsize()); i < mNStyles; i++)
    mMaxsize = max(mMaxsize, fl_height(mStyleTable[i].font, mStyleTable[i].size));

  // did we have scrollbars initially?
  bool hscrollbarvisible = mHScrollBar->visible();
  bool vscrollbarvisible = mVScrollBar->visible();

  // try without scrollbars first
  mVScrollBar->clear_visible();
  mHScrollBar->clear_visible();

  for (int again = 1; again;) {
     again = 0;
    /* reallocate and update the line starts array, which may have changed
       size and / or contents.  */
    int nvlines = (text_area.h + mMaxsize - 1) / mMaxsize;
    if (mNVisibleLines != nvlines) {
      mNVisibleLines = nvlines;
      delete[] mLineStarts;
      mLineStarts = new int [mNVisibleLines];
      calc_line_starts(0, mNVisibleLines);
      calc_last_char();
    }

    // figure the scrollbars
    if (scrollbar_width()) {
      /* Decide if the vertical scroll bar needs to be visible */
      if (scrollbar_align() & (FL_ALIGN_LEFT|FL_ALIGN_RIGHT) &&
          mNBufferLines >= mNVisibleLines - 1)
      {
        mVScrollBar->set_visible();
        if (scrollbar_align() & FL_ALIGN_LEFT) {
          text_area.x = X+scrollbar_width()+LEFT_MARGIN;
          text_area.w = W-scrollbar_width()-LEFT_MARGIN-RIGHT_MARGIN;
          mVScrollBar->resize(X, text_area.y-TOP_MARGIN, scrollbar_width(),
                              text_area.h+TOP_MARGIN+BOTTOM_MARGIN);
        } else {
          text_area.x = X+LEFT_MARGIN;
          text_area.w = W-scrollbar_width()-LEFT_MARGIN-RIGHT_MARGIN;
          mVScrollBar->resize(X+W-scrollbar_width(), text_area.y-TOP_MARGIN,
                              scrollbar_width(), text_area.h+TOP_MARGIN+BOTTOM_MARGIN);
        }
      }

      /*
         Decide if the horizontal scroll bar needs to be visible.  If there
         is a vertical scrollbar, a horizontal is always created too.  This
         is because the alternatives are unatractive:
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
      if (scrollbar_align() & (FL_ALIGN_TOP|FL_ALIGN_BOTTOM) &&
          (mVScrollBar->visible() || longest_vline() > text_area.w))
      {
        if (!mHScrollBar->visible()) {
          mHScrollBar->set_visible();
          again = 1; // loop again to see if we now need vert. & recalc sizes
        }
        if (scrollbar_align() & FL_ALIGN_TOP) {
          text_area.y = Y + scrollbar_width()+TOP_MARGIN;
          text_area.h = H - scrollbar_width()-TOP_MARGIN-BOTTOM_MARGIN;
          mHScrollBar->resize(text_area.x-LEFT_MARGIN, Y,
                              text_area.w+LEFT_MARGIN+RIGHT_MARGIN, scrollbar_width());
        } else {
          text_area.y = Y+TOP_MARGIN;
          text_area.h = H - scrollbar_width()-TOP_MARGIN-BOTTOM_MARGIN;
          mHScrollBar->resize(text_area.x-LEFT_MARGIN, Y+H-scrollbar_width(),
                              text_area.w+LEFT_MARGIN+RIGHT_MARGIN, scrollbar_width());
        }
      }
    }
  }

  // user request to change viewport
  if (mTopLineNumHint != mTopLineNum || mHorizOffsetHint != mHorizOffset)
    scroll_(mTopLineNumHint, mHorizOffsetHint);

  // everything will fit in the viewport
  if (mNBufferLines < mNVisibleLines)
    scroll_(1, mHorizOffset);
  /* if empty lines become visible, there may be an opportunity to
     display more text by scrolling down */
  else while (mLineStarts[mNVisibleLines-2] == -1)
    scroll_(mTopLineNum-1, mHorizOffset);

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

  if (hscrollbarvisible != mHScrollBar->visible() ||
      vscrollbarvisible != mVScrollBar->visible())
    redraw();

  update_v_scrollbar();
  update_h_scrollbar();
}

/*
** Refresh a rectangle of the text display.  left and top are in coordinates of
** the text drawing window
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

void Fl_Text_Display::redisplay_range(int start, int end) {
  if (damage_range1_start == -1 && damage_range1_end == -1) {
    damage_range1_start = start;
    damage_range1_end = end;
  } else if ((start >= damage_range1_start && start <= damage_range1_end) ||
             (end >= damage_range1_start && end <= damage_range1_end)) {
    damage_range1_start = min(damage_range1_start, start);
    damage_range1_end = max(damage_range1_end, end);
  } else if (damage_range2_start == -1 && damage_range2_end == -1) {
    damage_range2_start = start;
    damage_range2_end = end;
  } else {
    damage_range2_start = min(damage_range2_start, start);
    damage_range2_end = max(damage_range2_end, end);
  }
  damage(FL_DAMAGE_SCROLL);
}
/*
** Refresh all of the text between buffer positions "start" and "end"
** not including the character at the position "end".
** If end points beyond the end of the buffer, refresh the whole display
** after pos, including blank lines which are not technically part of
** any range of characters.
*/
void Fl_Text_Display::draw_range(int start, int end) {
  int i, startLine, lastLine, startIndex, endIndex;

  /* If the range is outside of the displayed text, just return */
  if ( end < mFirstChar || ( start > mLastChar &&
                             !empty_vlines() ) )
    return;

  /* Clean up the starting and ending values */
  if ( start < 0 ) start = 0;
  if ( start > mBuffer->length() ) start = mBuffer->length();
  if ( end < 0 ) end = 0;
  if ( end > mBuffer->length() ) end = mBuffer->length();

  /* Get the starting and ending lines */
  if ( start < mFirstChar )
    start = mFirstChar;
  if ( !position_to_line( start, &startLine ) )
    startLine = mNVisibleLines - 1;
  if ( end >= mLastChar ) {
    lastLine = mNVisibleLines - 1;
  } else {
    if ( !position_to_line( end, &lastLine ) ) {
      /* shouldn't happen */
      lastLine = mNVisibleLines - 1;
    }
  }

  /* Get the starting and ending positions within the lines */
  startIndex = mLineStarts[ startLine ] == -1 ? 0 :
               start - mLineStarts[ startLine ];
  if ( end >= mLastChar )
    endIndex = INT_MAX;
  else if ( mLineStarts[ lastLine ] == -1 )
    endIndex = 0;
  else
    endIndex = end - mLineStarts[ lastLine ];

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

/*
** Set the position of the text insertion cursor for text display
*/
void Fl_Text_Display::insert_position( int newPos ) {
  /* make sure new position is ok, do nothing if it hasn't changed */
  if ( newPos == mCursorPos )
    return;
  if ( newPos < 0 ) newPos = 0;
  if ( newPos > mBuffer->length() ) newPos = mBuffer->length();

  /* cursor movement cancels vertical cursor motion column */
  mCursorPreferredCol = -1;

  /* erase the cursor at it's previous position */
  redisplay_range(mCursorPos - 1, mCursorPos + 1);

  mCursorPos = newPos;

  /* draw cursor at its new position */
  redisplay_range(mCursorPos - 1, mCursorPos + 1);
}

void Fl_Text_Display::show_cursor(int b) {
  mCursorOn = b;
  redisplay_range(mCursorPos - 1, mCursorPos + 1);
}

void Fl_Text_Display::cursor_style(int style) {
  mCursorStyle = style;
  if (mCursorOn) show_cursor();
}

/*
** Insert "text" at the current cursor location.  This has the same
** effect as inserting the text into the buffer using BufInsert and
** then moving the insert position after the newly inserted text, except
** that it's optimized to do less redrawing.
*/
void Fl_Text_Display::insert(const char* text) {
  int pos = mCursorPos;

  mCursorToHint = pos + strlen( text );
  mBuffer->insert( pos, text );
  mCursorToHint = NO_HINT;
}

/*
** Insert "text" (which must not contain newlines), overstriking the current
** cursor location.
*/
void Fl_Text_Display::overstrike(const char* text) {
  int startPos = mCursorPos;
  Fl_Text_Buffer *buf = mBuffer;
  int lineStart = buf->line_start( startPos );
  int textLen = strlen( text );
  int i, p, endPos, indent, startIndent, endIndent;
  const char *c;
  char ch, *paddedText = NULL;

  /* determine how many displayed character positions are covered */
  startIndent = mBuffer->count_displayed_characters( lineStart, startPos );
  indent = startIndent;
  for ( c = text; *c != '\0'; c++ )
    indent += Fl_Text_Buffer::character_width( *c, indent, buf->tab_distance(), buf->null_substitution_character() );
  endIndent = indent;

  /* find which characters to remove, and if necessary generate additional
     padding to make up for removed control characters at the end */
  indent = startIndent;
  for ( p = startPos; ; p++ ) {
    if ( p == buf->length() )
      break;
    ch = buf->character( p );
    if ( ch == '\n' )
      break;
    indent += Fl_Text_Buffer::character_width( ch, indent, buf->tab_distance(), buf->null_substitution_character() );
    if ( indent == endIndent ) {
      p++;
      break;
    } else if ( indent > endIndent ) {
      if ( ch != '\t' ) {
        p++;
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

/*
** Translate a buffer text position to the XY location where the top left
** of the cursor would be positioned to point to that character.  Returns
** 0 if the position is not displayed because it is VERTICALLY out
** of view.  If the position is horizontally out of view, returns the
** X coordinate where the position would be if it were visible.
*/

int Fl_Text_Display::position_to_xy( int pos, int* X, int* Y ) {
  int charIndex, lineStartPos, fontHeight, lineLen;
  int visLineNum, charLen, outIndex, xStep, charStyle;
  char expandedChar[ FL_TEXT_MAX_EXP_CHAR_LEN ];
  const char *lineStr;

  /* If position is not displayed, return false */
  if (pos < mFirstChar || (pos > mLastChar && !empty_vlines()))
    return 0;

  /* Calculate Y coordinate */
  if (!position_to_line(pos, &visLineNum)) return 0;
  fontHeight = mMaxsize;
  *Y = text_area.y + visLineNum * fontHeight;

  /* Get the text, length, and  buffer position of the line. If the position
     is beyond the end of the buffer and should be at the first position on
     the first empty line, don't try to get or scan the text  */
  lineStartPos = mLineStarts[visLineNum];
  if ( lineStartPos == -1 ) {
    *X = text_area.x - mHorizOffset;
    return 1;
  }
  lineLen = vline_length( visLineNum );
  lineStr = mBuffer->text_range( lineStartPos, lineStartPos + lineLen );

  /* Step through character positions from the beginning of the line
     to "pos" to calculate the X coordinate */
  xStep = text_area.x - mHorizOffset;
  outIndex = 0;
  for ( charIndex = 0; charIndex < pos - lineStartPos; charIndex++ ) {
    charLen = Fl_Text_Buffer::expand_character( lineStr[ charIndex ], outIndex, expandedChar,
              mBuffer->tab_distance(), mBuffer->null_substitution_character() );
    charStyle = position_style( lineStartPos, lineLen, charIndex,
                                outIndex );
    xStep += string_width( expandedChar, charLen, charStyle );
    outIndex += charLen;
  }
  *X = xStep;
  delete [] (char *)lineStr;
  return 1;
}

/*
** Find the line number of position "pos".  Note: this only works for
** displayed lines.  If the line is not displayed, the function returns
** 0 (without the lineStarts array it could turn in to very long
** calculation involving scanning large amounts of text in the buffer).
*/
int Fl_Text_Display::position_to_linecol( int pos, int* lineNum, int* column ) {
  int retVal;

  retVal = position_to_line( pos, lineNum );
  if ( retVal ) {
    *column = mBuffer->count_displayed_characters(
                mLineStarts[ *lineNum ], pos );
    *lineNum += mTopLineNum;
  }
  return retVal;
}

/*
** Return 1 if position (X, Y) is inside of the primary Fl_Text_Selection
*/
int Fl_Text_Display::in_selection( int X, int Y ) {
  int row, column, pos = xy_to_position( X, Y, CHARACTER_POS );
  Fl_Text_Buffer *buf = mBuffer;

  xy_to_rowcol( X, Y, &row, &column, CHARACTER_POS );
  return buf->primary_selection()->includes(pos, buf->line_start( pos ), column);
}

/*
** Scroll the display to bring insertion cursor into view.
**
** Note: it would be nice to be able to do this without counting lines twice
** (scroll_() counts them too) and/or to count from the most efficient
** starting point, but the efficiency of this routine is not as important to
** the overall performance of the text display.
*/
void Fl_Text_Display::display_insert() {
  int hOffset, topLine, X, Y;
  hOffset = mHorizOffset;
  topLine = mTopLineNum;

  if (insert_position() < mFirstChar) {
    topLine -= buffer()->count_lines(insert_position(), mFirstChar);
  } else if (mLineStarts[mNVisibleLines-2] != -1) {
    int lastChar = buffer()->line_end(mLineStarts[mNVisibleLines-2]);
    if (insert_position() > lastChar)
      topLine += buffer()->count_lines(lastChar, insert_position());
  }
  /* Find the new setting for horizontal offset (this is a bit ungraceful).
     If the line is visible, just use PositionToXY to get the position
     to scroll to, otherwise, do the vertical scrolling first, then the
     horizontal */
  if (!position_to_xy( mCursorPos, &X, &Y )) {
    scroll_(topLine, hOffset);
    if (!position_to_xy( mCursorPos, &X, &Y ))
      return;   /* Give up, it's not worth it (but why does it fail?) */
  }
  if (X > text_area.x + text_area.w)
    hOffset += X-(text_area.x + text_area.w);
  else if (X < text_area.x)
    hOffset += X-text_area.x;

  /* Do the scroll */
  if (topLine != mTopLineNum || hOffset != mHorizOffset)
    scroll_(topLine, hOffset);
}

void Fl_Text_Display::show_insert_position() {
  display_insert_position_hint = 1;
  resize(x(), y(), w(), h());
}

/*
** Cursor movement functions
*/
int Fl_Text_Display::move_right() {
  if ( mCursorPos >= mBuffer->length() )
    return 0;
  insert_position( mCursorPos + 1 );
  return 1;
}

int Fl_Text_Display::move_left() {
  if ( mCursorPos <= 0 )
    return 0;
  insert_position( mCursorPos - 1 );
  return 1;
}

int Fl_Text_Display::move_up() {
  int lineStartPos, column, prevLineStartPos, newPos, visLineNum;

  /* Find the position of the start of the line.  Use the line starts array
     if possible */
  if ( position_to_line( mCursorPos, &visLineNum ) )
    lineStartPos = mLineStarts[ visLineNum ];
  else {
    lineStartPos = buffer()->line_start( mCursorPos );
    visLineNum = -1;
  }
  if ( lineStartPos == 0 )
    return 0;

  /* Decide what column to move to, if there's a preferred column use that */
  column = mCursorPreferredCol >= 0 ? mCursorPreferredCol :
           mBuffer->count_displayed_characters( lineStartPos, mCursorPos );

  /* count forward from the start of the previous line to reach the column */
  if ( visLineNum != -1 && visLineNum != 0 )
    prevLineStartPos = mLineStarts[ visLineNum - 1 ];
  else
    prevLineStartPos = buffer()->rewind_lines( lineStartPos, 1 );
  newPos = mBuffer->skip_displayed_characters( prevLineStartPos, column );

  /* move the cursor */
  insert_position( newPos );

  /* if a preferred column wasn't aleady established, establish it */
  mCursorPreferredCol = column;
  return 1;
}
int Fl_Text_Display::move_down() {
  int lineStartPos, column, nextLineStartPos, newPos, visLineNum;

  if ( mCursorPos == mBuffer->length() )
    return 0;
  if ( position_to_line( mCursorPos, &visLineNum ) )
    lineStartPos = mLineStarts[ visLineNum ];
  else {
    lineStartPos = buffer()->line_start( mCursorPos );
    visLineNum = -1;
  }
  column = mCursorPreferredCol >= 0 ? mCursorPreferredCol :
           mBuffer->count_displayed_characters( lineStartPos, mCursorPos );
  nextLineStartPos = buffer()->skip_lines( lineStartPos, 1 );
  newPos = mBuffer->skip_displayed_characters( nextLineStartPos, column );

  insert_position( newPos );
  mCursorPreferredCol = column;
  return 1;
}

void Fl_Text_Display::next_word() {
  int pos = insert_position();
  while ( pos < buffer()->length() && (
            isalnum( buffer()->character( pos ) ) || buffer()->character( pos ) == '_' ) ) {
    pos++;
  }
  while ( pos < buffer()->length() && !( isalnum( buffer()->character( pos ) ) || buffer()->character( pos ) == '_' ) ) {
    pos++;
  }

  insert_position( pos );
}

void Fl_Text_Display::previous_word() {
  int pos = insert_position();
  pos--;
  while ( pos && !( isalnum( buffer()->character( pos ) ) || buffer()->character( pos ) == '_' ) ) {
    pos--;
  }
  while ( pos && ( isalnum( buffer()->character( pos ) ) || buffer()->character( pos ) == '_' ) ) {
    pos--;
  }
  if ( !( isalnum( buffer()->character( pos ) ) || buffer()->character( pos ) == '_' ) ) pos++;

  insert_position( pos );
}

/*
** Callback attached to the text buffer to receive modification information
*/
void Fl_Text_Display::buffer_modified_cb( int pos, int nInserted, int nDeleted,
    int nRestyled, const char *deletedText, void *cbArg ) {
  int linesInserted, linesDeleted, startDispPos, endDispPos;
  Fl_Text_Display *textD = ( Fl_Text_Display * ) cbArg;
  Fl_Text_Buffer *buf = textD->mBuffer;
  int scrolled, origCursorPos = textD->mCursorPos;

  // refigure scrollbars & stuff
  textD->resize(textD->x(), textD->y(), textD->w(), textD->h());

  /* buffer modification cancels vertical cursor motion column */
  if ( nInserted != 0 || nDeleted != 0 )
    textD->mCursorPreferredCol = -1;

  /* Count the number of lines inserted and deleted */
  linesInserted = nInserted == 0 ? 0 :
                  textD->buffer()->count_lines( pos, pos + nInserted );
  linesDeleted = nDeleted == 0 ? 0 : countlines( deletedText );

  /* Update the line starts and topLineNum */
  if ( nInserted != 0 || nDeleted != 0 ) {
    textD->update_line_starts( pos, nInserted, nDeleted, linesInserted,
                               linesDeleted, &scrolled );
  } else
    scrolled = 0;

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

  // don't need to do anything else if not visible?
  if (!textD->visible_r()) return;

  /* If the changes caused scrolling, re-paint everything and we're done. */
  if ( scrolled ) {
    textD->damage(FL_DAMAGE_EXPOSE);
    if ( textD->mStyleBuffer )   /* See comments in extendRangeForStyleMods */
      textD->mStyleBuffer->primary_selection()->selected(0);
    return;
  }

  /* If the changes didn't cause scrolling, decide the range of characters
     that need to be re-painted.  Also if the cursor position moved, be
     sure that the redisplay range covers the old cursor position so the
     old cursor gets erased, and erase the bits of the cursor which extend
     beyond the left and right edges of the text. */
  startDispPos = pos;
  if ( origCursorPos == startDispPos && textD->mCursorPos != startDispPos )
    startDispPos = min( startDispPos, origCursorPos - 1 );
  if ( linesInserted == linesDeleted ) {
    if ( nInserted == 0 && nDeleted == 0 )
      endDispPos = pos + nRestyled;
    else {
      endDispPos = buf->line_end( pos + nInserted ) + 1;
      // CET - FIXME      if ( origCursorPos >= startDispPos &&
      //                ( origCursorPos <= endDispPos || endDispPos == buf->length() ) )
    }

  } else {
    endDispPos = textD->mLastChar + 1;
    // CET - FIXME   if ( origCursorPos >= pos )
  }

  /* If there is a style buffer, check if the modification caused additional
     changes that need to be redisplayed.  (Redisplaying separately would
     cause double-redraw on almost every modification involving styled
     text).  Extend the redraw range to incorporate style changes */
  if ( textD->mStyleBuffer )
    textD->extend_range_for_styles( &startDispPos, &endDispPos );

  /* Redisplay computed range */
  textD->redisplay_range( startDispPos, endDispPos );
}

/*
** Find the line number of position "pos" relative to the first line of
** displayed text. Returns 0 if the line is not displayed.
*/
int Fl_Text_Display::position_to_line( int pos, int *lineNum ) {
  int i;

  if ( pos < mFirstChar )
    return 0;
  if ( pos > mLastChar ) {
    if ( empty_vlines() ) {
      if ( mLastChar < mBuffer->length() ) {
        if ( !position_to_line( mLastChar, lineNum ) ) {
          fprintf( stderr, "Consistency check ptvl failed\n" );
          return 0;
        }
        return ++( *lineNum ) <= mNVisibleLines - 1;
      } else {
        position_to_line( mLastChar - 1, lineNum );
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

/*
** Draw the text on a single line represented by "visLineNum" (the
** number of lines down from the top of the display), limited by
** "leftClip" and "rightClip" window coordinates and "leftCharIndex" and
** "rightCharIndex" character positions (not including the character at
** position "rightCharIndex").
*/
void Fl_Text_Display::draw_vline(int visLineNum, int leftClip, int rightClip,
                                 int leftCharIndex, int rightCharIndex) {
  Fl_Text_Buffer * buf = mBuffer;
  int i, X, Y, startX, charIndex, lineStartPos, lineLen, fontHeight;
  int stdCharWidth, charWidth, startIndex, charStyle, style;
  int charLen, outStartIndex, outIndex, cursorX, hasCursor = 0;
  int dispIndexOffset, cursorPos = mCursorPos;
  char expandedChar[ FL_TEXT_MAX_EXP_CHAR_LEN ], outStr[ MAX_DISP_LINE_LEN ];
  char *outPtr;
  const char *lineStr;

  /* If line is not displayed, skip it */
  if ( visLineNum < 0 || visLineNum > mNVisibleLines )
    return;

  /* Calculate Y coordinate of the string to draw */
  fontHeight = mMaxsize;
  Y = text_area.y + visLineNum * fontHeight;

  /* Get the text, length, and  buffer position of the line to display */
  lineStartPos = mLineStarts[ visLineNum ];
  if ( lineStartPos == -1 ) {
    lineLen = 0;
    lineStr = NULL;
  } else {
    lineLen = vline_length( visLineNum );
    lineStr = buf->text_range( lineStartPos, lineStartPos + lineLen );
  }

  /* Space beyond the end of the line is still counted in units of characters
     of a standardized character width (this is done mostly because style
     changes based on character position can still occur in this region due
     to rectangular Fl_Text_Selections).  stdCharWidth must be non-zero to prevent a
     potential infinite loop if X does not advance */
  stdCharWidth = TMPFONTWIDTH;   //mFontStruct->max_bounds.width;
  if ( stdCharWidth <= 0 ) {
    fprintf( stderr, "Internal Error, bad font measurement\n" );
    delete [] (char *)lineStr;
    return;
  }

  /* Shrink the clipping range to the active display area */
  leftClip = max( text_area.x, leftClip );
  rightClip = min( rightClip, text_area.x + text_area.w );

  /* Rectangular Fl_Text_Selections are based on "real" line starts (after a newline
     or start of buffer).  Calculate the difference between the last newline
     position and the line start we're using.  Since scanning back to find a
     newline is expensive, only do so if there's actually a rectangular
     Fl_Text_Selection which needs it */
  dispIndexOffset = 0;

  /* Step through character positions from the beginning of the line (even if
     that's off the left edge of the displayed area) to find the first
     character position that's not clipped, and the X coordinate for drawing
     that character */
  X = text_area.x - mHorizOffset;
  outIndex = 0;
  for ( charIndex = 0; ; charIndex++ ) {
    charLen = charIndex >= lineLen ? 1 :
              Fl_Text_Buffer::expand_character( lineStr[ charIndex ], outIndex,
                                                expandedChar, buf->tab_distance(), buf->null_substitution_character() );
    style = position_style( lineStartPos, lineLen, charIndex,
                            outIndex + dispIndexOffset );
    charWidth = charIndex >= lineLen ? stdCharWidth :
                string_width( expandedChar, charLen, style );
    if ( X + charWidth >= leftClip && charIndex >= leftCharIndex ) {
      startIndex = charIndex;
      outStartIndex = outIndex;
      startX = X;
      break;
    }
    X += charWidth;
    outIndex += charLen;
  }

  /* Scan character positions from the beginning of the clipping range, and
     draw parts whenever the style changes (also note if the cursor is on
     this line, and where it should be drawn to take advantage of the x
     position which we've gone to so much trouble to calculate) */
  outPtr = outStr;
  outIndex = outStartIndex;
  X = startX;
  for ( charIndex = startIndex; charIndex < rightCharIndex; charIndex++ ) {
    if ( lineStartPos + charIndex == cursorPos ) {
      if ( charIndex < lineLen || ( charIndex == lineLen &&
                                    cursorPos >= buf->length() ) ) {
        hasCursor = 1;  // CET - FIXME
        cursorX = X - 1;
      } else if ( charIndex == lineLen ) {
        hasCursor = 1;
        cursorX = X - 1;
      }
    }
    charLen = charIndex >= lineLen ? 1 :
              Fl_Text_Buffer::expand_character( lineStr[ charIndex ], outIndex, expandedChar,
                                                buf->tab_distance(), buf->null_substitution_character() );
    charStyle = position_style( lineStartPos, lineLen, charIndex,
                                outIndex + dispIndexOffset );
    for ( i = 0; i < charLen; i++ ) {
      if ( i != 0 && charIndex < lineLen && lineStr[ charIndex ] == '\t' )
        charStyle = position_style( lineStartPos, lineLen,
                                    charIndex, outIndex + dispIndexOffset );
      if ( charStyle != style ) {
        draw_string( style, startX, Y, X, outStr, outPtr - outStr );
        outPtr = outStr;
        startX = X;
        style = charStyle;
      }
      if ( charIndex < lineLen ) {
        *outPtr = expandedChar[ i ];
        charWidth = string_width( &expandedChar[ i ], 1, charStyle );
      } else
        charWidth = stdCharWidth;
      outPtr++;
      X += charWidth;
      outIndex++;
    }
    if ( outPtr - outStr + FL_TEXT_MAX_EXP_CHAR_LEN >= MAX_DISP_LINE_LEN || X >= rightClip )
      break;
  }

  /* Draw the remaining style segment */
  draw_string( style, startX, Y, X, outStr, outPtr - outStr );

  /* Draw the cursor if part of it appeared on the redisplayed part of
     this line.  Also check for the cases which are not caught as the
     line is scanned above: when the cursor appears at the very end
     of the redisplayed section. */
  /*  CET - FIXME
    if ( mCursorOn )
    {
      if ( hasCursor )
        draw_cursor( cursorX, Y );
      else if ( charIndex < lineLen && ( lineStartPos + charIndex + 1 == cursorPos )
                && X == rightClip )
      {
        if ( cursorPos >= buf->length() )
          draw_cursor( X - 1, Y );
        else
        {
          draw_cursor( X - 1, Y );
        }
      }
    }
  */
  if ( lineStr != NULL )
    delete [] (char *)lineStr;
}

/*
** Draw a string or blank area according to parameter "style", using the
** appropriate colors and drawing method for that style, with top left
** corner at X, y.  If style says to draw text, use "string" as source of
** characters, and draw "nChars", if style is FILL, erase
** rectangle where text would have drawn from X to toX and from Y to
** the maximum Y extent of the current font(s).
*/
void Fl_Text_Display::draw_string( int style, int X, int Y, int toX,
                                   const char *string, int nChars ) {
  Style_Table_Entry * styleRec;

  /* Draw blank area rather than text, if that was the request */
  if ( style & FILL_MASK ) {
    clear_rect( style, X, Y, toX - X, mMaxsize );
    return;
  }

  /* Set font, color, and gc depending on style.  For normal text, GCs
     for normal drawing, or drawing within a Fl_Text_Selection or highlight are
     pre-allocated and pre-configured.  For syntax highlighting, GCs are
     configured here, on the fly. */

  Fl_Font font = textfont();
  int size = textsize();
  Fl_Color foreground;
  Fl_Color background;

  if ( style & STYLE_LOOKUP_MASK ) {
    styleRec = &mStyleTable[ ( style & STYLE_LOOKUP_MASK ) - 'A' ];
    font = styleRec->font;
    size = styleRec->size;
    foreground = styleRec->color;
    background = style & PRIMARY_MASK ? FL_SELECTION_COLOR :
                 style & HIGHLIGHT_MASK ? fl_contrast(textcolor(),color()) : color();
    if ( foreground == background )   /* B&W kludge */
      foreground = textcolor();
  } else if ( style & HIGHLIGHT_MASK ) {
    foreground = textcolor();
    background = fl_contrast(textcolor(),color());
  } else if ( style & PRIMARY_MASK ) {
    foreground = textcolor();
    background = FL_SELECTION_COLOR;
  } else {
    foreground = textcolor();
    background = color();
  }

  fl_color( background );
  fl_rectf( X, Y, toX - X, mMaxsize );
  fl_color( foreground );
  fl_font( font, size );
  fl_draw( string, nChars, X, Y + mMaxsize - fl_descent());

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

/*
** Clear a rectangle with the appropriate background color for "style"
*/
void Fl_Text_Display::clear_rect( int style, int X, int Y,
                                  int width, int height ) {
  /* A width of zero means "clear to end of window" to XClearArea */
  if ( width == 0 )
    return;

  if ( style & HIGHLIGHT_MASK ) {
    fl_color( fl_contrast(textcolor(), color()) );
    fl_rectf( X, Y, width, height );
  } else if ( style & PRIMARY_MASK ) {
    fl_color( FL_SELECTION_COLOR );
    fl_rectf( X, Y, width, height );
  } else {
    fl_color( color() );
    fl_rectf( X, Y, width, height );
  }
}



/*
** Draw a cursor with top center at X, y.
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
  }
  fl_color( mCursor_color );

  for ( int k = 0; k < nSegs; k++ ) {
    fl_line( segs[ k ].x1, segs[ k ].y1, segs[ k ].x2, segs[ k ].y2 );
  }
}

/*
** Determine the drawing method to use to draw a specific character from "buf".
** "lineStartPos" gives the character index where the line begins, "lineIndex",
** the number of characters past the beginning of the line, and "dispIndex",
** the number of displayed characters past the beginning of the line.  Passing
** lineStartPos of -1 returns the drawing style for "no text".
**
** Why not just: position_style(pos)?  Because style applies to blank areas
** of the window beyond the text boundaries, and because this routine must also
** decide whether a position is inside of a rectangular Fl_Text_Selection, and do
** so efficiently, without re-counting character positions from the start of the
** line.
**
** Note that style is a somewhat incorrect name, drawing method would
** be more appropriate.
*/
int Fl_Text_Display::position_style( int lineStartPos,
                                     int lineLen, int lineIndex, int dispIndex ) {
  Fl_Text_Buffer * buf = mBuffer;
  Fl_Text_Buffer *styleBuf = mStyleBuffer;
  int pos, style = 0;

  if ( lineStartPos == -1 || buf == NULL )
    return FILL_MASK;

  pos = lineStartPos + min( lineIndex, lineLen );

  if ( lineIndex >= lineLen )
    style = FILL_MASK;
  else if ( styleBuf != NULL ) {
    style = ( unsigned char ) styleBuf->character( pos );
    /*!!!       if (style == mUnfinishedStyle) {
                    // encountered "unfinished" style, trigger parsing
                    (mUnfinishedHighlightCB)( pos, mHighlightCBArg);
                    style = (unsigned char) styleBuf->character( pos);
                }
          */
  }
  if (buf->primary_selection()->includes(pos, lineStartPos, dispIndex))
    style |= PRIMARY_MASK;
  if (buf->highlight_selection()->includes(pos, lineStartPos, dispIndex))
    style |= HIGHLIGHT_MASK;
  if (buf->secondary_selection()->includes(pos, lineStartPos, dispIndex))
    style |= SECONDARY_MASK;
  return style;
}

/*
** Find the width of a string in the font of a particular style
*/
int Fl_Text_Display::string_width( const char *string, int length, int style ) {
  Fl_Font font;
  int size;

  if ( style & STYLE_LOOKUP_MASK ) {
    font = mStyleTable[ ( style & STYLE_LOOKUP_MASK ) - 'A' ].font;
    size = mStyleTable[ ( style & STYLE_LOOKUP_MASK ) - 'A' ].size;
  } else {
    font = textfont();
    size = textsize();
  }
  fl_font( font, size );

  return ( int ) ( fl_width( string, length ) );
}

/*
** Translate window coordinates to the nearest (insert cursor or character
** cell) text position.  The parameter posType specifies how to interpret the
** position: CURSOR_POS means translate the coordinates to the nearest cursor
** position, and CHARACTER_POS means return the position of the character
** closest to (X, Y).
*/
int Fl_Text_Display::xy_to_position( int X, int Y, int posType ) {
  int charIndex, lineStart, lineLen, fontHeight;
  int charWidth, charLen, charStyle, visLineNum, xStep, outIndex;
  char expandedChar[ FL_TEXT_MAX_EXP_CHAR_LEN ];
  const char *lineStr;

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
  lineStr = mBuffer->text_range( lineStart, lineStart + lineLen );

  /* Step through character positions from the beginning of the line
     to find the character position corresponding to the X coordinate */
  xStep = text_area.x - mHorizOffset;
  outIndex = 0;
  for ( charIndex = 0; charIndex < lineLen; charIndex++ ) {
    charLen = Fl_Text_Buffer::expand_character( lineStr[ charIndex ], outIndex, expandedChar,
              mBuffer->tab_distance(), mBuffer->null_substitution_character() );
    charStyle = position_style( lineStart, lineLen, charIndex, outIndex );
    charWidth = string_width( expandedChar, charLen, charStyle );
    if ( X < xStep + ( posType == CURSOR_POS ? charWidth / 2 : charWidth ) ) {
      delete [] (char *)lineStr;
      return lineStart + charIndex;
    }
    xStep += charWidth;
    outIndex += charLen;
  }

  /* If the X position was beyond the end of the line, return the position
     of the newline at the end of the line */
  delete [] (char *)lineStr;
  return lineStart + lineLen;
}

/*
** Translate window coordinates to the nearest row and column number for
** positioning the cursor.  This, of course, makes no sense when the font is
** proportional, since there are no absolute columns.  The parameter posType
** specifies how to interpret the position: CURSOR_POS means translate the
** coordinates to the nearest position between characters, and CHARACTER_POS
** means translate the position to the nearest character cell.
*/
void Fl_Text_Display::xy_to_rowcol( int X, int Y, int *row,
                                    int *column, int posType ) {
  int fontHeight = mMaxsize;
  int fontWidth = TMPFONTWIDTH;   //mFontStruct->max_bounds.width;

  /* Find the visible line number corresponding to the Y coordinate */
  *row = ( Y - text_area.y ) / fontHeight;
  if ( *row < 0 ) * row = 0;
  if ( *row >= mNVisibleLines ) * row = mNVisibleLines - 1;
  *column = ( ( X - text_area.x ) + mHorizOffset +
              ( posType == CURSOR_POS ? fontWidth / 2 : 0 ) ) / fontWidth;
  if ( *column < 0 ) * column = 0;
}

/*
** Offset the line starts array, topLineNum, firstChar and lastChar, for a new
** vertical scroll position given by newTopLineNum.  If any currently displayed
** lines will still be visible, salvage the line starts values, otherwise,
** count lines from the nearest known line start (start or end of buffer, or
** the closest value in the lineStarts array)
*/
void Fl_Text_Display::offset_line_starts( int newTopLineNum ) {
  int oldTopLineNum = mTopLineNum;
  int lineDelta = newTopLineNum - oldTopLineNum;
  int nVisLines = mNVisibleLines;
  int *lineStarts = mLineStarts;
  int i, lastLineNum;
  Fl_Text_Buffer *buf = mBuffer;

  /* If there was no offset, nothing needs to be changed */
  if ( lineDelta == 0 )
    return;

  /* Find the new value for firstChar by counting lines from the nearest
     known line start (start or end of buffer, or the closest value in the
     lineStarts array) */
  lastLineNum = oldTopLineNum + nVisLines - 1;
  if ( newTopLineNum < oldTopLineNum && newTopLineNum < -lineDelta ) {
    mFirstChar = buffer()->skip_lines( 0, newTopLineNum - 1 );
  } else if ( newTopLineNum < oldTopLineNum ) {
    mFirstChar = buffer()->rewind_lines( mFirstChar, -lineDelta );
  } else if ( newTopLineNum < lastLineNum ) {
    mFirstChar = lineStarts[ newTopLineNum - oldTopLineNum ];
  } else if ( newTopLineNum - lastLineNum < mNBufferLines - newTopLineNum ) {
    mFirstChar = buffer()->skip_lines( lineStarts[ nVisLines - 1 ],
                                       newTopLineNum - lastLineNum );
  } else {
    mFirstChar = buffer()->rewind_lines( buf->length(), mNBufferLines - newTopLineNum + 1 );
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

  /* Set lastChar and topLineNum */
  calc_last_char();
  mTopLineNum = newTopLineNum;
}

/*
** Update the line starts array, topLineNum, firstChar and lastChar for text
** display "textD" after a modification to the text buffer, given by the
** position where the change began "pos", and the nmubers of characters
** and lines inserted and deleted.
*/
void Fl_Text_Display::update_line_starts( int pos, int charsInserted,
    int charsDeleted, int linesInserted, int linesDeleted, int *scrolled ) {
  int * lineStarts = mLineStarts;
  int i, lineOfPos, lineOfEnd, nVisLines = mNVisibleLines;
  int charDelta = charsInserted - charsDeleted;
  int lineDelta = linesInserted - linesDeleted;

  /* If all of the changes were before the displayed text, the display
     doesn't change, just update the top line num and offset the line
     start entries and first and last characters */
  if ( pos + charsDeleted < mFirstChar ) {
    mTopLineNum += lineDelta;
    for ( i = 0; i < nVisLines; i++ )
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
      mFirstChar = buffer()->rewind_lines( lineStarts[ lineOfEnd ] + charDelta, lineOfEnd );
      /* Otherwise anchor on original line number and recount everything */
    } else {
      if ( mTopLineNum > mNBufferLines + lineDelta ) {
        mTopLineNum = 1;
        mFirstChar = 0;
      } else
        mFirstChar = buffer()->skip_lines( 0, mTopLineNum - 1 );
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

/*
** Scan through the text in the "textD"'s buffer and recalculate the line
** starts array values beginning at index "startLine" and continuing through
** (including) "endLine".  It assumes that the line starts entry preceding
** "startLine" (or mFirstChar if startLine is 0) is good, and re-counts
** newlines to fill in the requested entries.  Out of range values for
** "startLine" and "endLine" are acceptable.
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
    lineEnd = buffer()->line_end(startPos);
    nextLineStart = min(buffer()->length(), lineEnd + 1);
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

/*
** Given a Fl_Text_Display with a complete, up-to-date lineStarts array, update
** the lastChar entry to point to the last buffer position displayed.
*/
void Fl_Text_Display::calc_last_char() {
  int i;
  for (i = mNVisibleLines - 1; i >= 0 && mLineStarts[i] == -1; i--) ;
  mLastChar = i < 0 ? 0 : buffer()->line_end(mLineStarts[i]);
}

void Fl_Text_Display::scroll(int topLineNum, int horizOffset) {
  mTopLineNumHint = topLineNum;
  mHorizOffsetHint = horizOffset;
  resize(x(), y(), w(), h());
}

void Fl_Text_Display::scroll_(int topLineNum, int horizOffset) {
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
    return;

  /* If the vertical scroll position has changed, update the line
     starts array and related counters in the text display */
  offset_line_starts(topLineNum);

  /* Just setting mHorizOffset is enough information for redisplay */
  mHorizOffset = horizOffset;

  // redraw all text
  damage(FL_DAMAGE_EXPOSE);
}

/*
** Update the minimum, maximum, slider size, page increment, and value
** for vertical scroll bar.
*/
void Fl_Text_Display::update_v_scrollbar() {
  /* The Vert. scroll bar value and slider size directly represent the top
     line number, and the number of visible lines respectively.  The scroll
     bar maximum value is chosen to generally represent the size of the whole
     buffer, with minor adjustments to keep the scroll bar widget happy */
  mVScrollBar->value(mTopLineNum, mNVisibleLines, 1, mNBufferLines+2);
}

/*
** Update the minimum, maximum, slider size, page increment, and value
** for the horizontal scroll bar.
*/
void Fl_Text_Display::update_h_scrollbar() {
  int sliderMax = max(longest_vline(), text_area.w + mHorizOffset);
  mHScrollBar->value( mHorizOffset, text_area.w, 0, sliderMax );
}

/*
** Callbacks for drag or valueChanged on scroll bars
*/
void Fl_Text_Display::v_scrollbar_cb(Fl_Scrollbar* b, Fl_Text_Display* textD) {
  if (b->value() == textD->mTopLineNum) return;
  textD->scroll(b->value(), textD->mHorizOffset);
}

void Fl_Text_Display::h_scrollbar_cb(Fl_Scrollbar* b, Fl_Text_Display* textD) {
  if (b->value() == textD->mHorizOffset) return;
  textD->scroll(textD->mTopLineNum, b->value());
}

static int max( int i1, int i2 ) {
  return i1 >= i2 ? i1 : i2;
}

static int min( int i1, int i2 ) {
  return i1 <= i2 ? i1 : i2;
}

/*
** Count the number of newlines in a null-terminated text string;
*/
static int countlines( const char *string ) {
  const char * c;
  int lineCount = 0;

  for ( c = string; *c != '\0'; c++ )
    if ( *c == '\n' ) lineCount++;
  return lineCount;
}

/*
** Return the width in pixels of the displayed line pointed to by "visLineNum"
*/
int Fl_Text_Display::measure_vline( int visLineNum ) {
  int i, width = 0, len, style, lineLen = vline_length( visLineNum );
  int charCount = 0, lineStartPos = mLineStarts[ visLineNum ];
  char expandedChar[ FL_TEXT_MAX_EXP_CHAR_LEN ];

  if ( mStyleBuffer == NULL ) {
    for ( i = 0; i < lineLen; i++ ) {
      len = mBuffer->expand_character( lineStartPos + i,
                                       charCount, expandedChar );

      fl_font( textfont(), textsize() );

      width += ( int ) fl_width( expandedChar, len );

      charCount += len;
    }
  } else {
    for ( i = 0; i < lineLen; i++ ) {
      len = mBuffer->expand_character( lineStartPos + i,
                                       charCount, expandedChar );
      style = ( unsigned char ) mStyleBuffer->character(
                lineStartPos + i ) - 'A';

      fl_font( mStyleTable[ style ].font, mStyleTable[ style ].size );

      width += ( int ) fl_width( expandedChar, len );

      charCount += len;
    }
  }
  return width;
}

/*
** Return true if there are lines visible with no corresponding buffer text
*/
int Fl_Text_Display::empty_vlines() {
  return mNVisibleLines > 0 &&
         mLineStarts[ mNVisibleLines - 1 ] == -1;
}

/*
** Return the length of a line (number of displayable characters) by examining
** entries in the line starts array rather than by scanning for newlines
*/
int Fl_Text_Display::vline_length( int visLineNum ) {
  int nextLineStart, lineStartPos = mLineStarts[ visLineNum ];

  if ( lineStartPos == -1 )
    return 0;
  if ( visLineNum + 1 >= mNVisibleLines )
    return mLastChar - lineStartPos;
  nextLineStart = mLineStarts[ visLineNum + 1 ];
  if ( nextLineStart == -1 )
    return mLastChar - lineStartPos;
  return nextLineStart - 1 - lineStartPos;
}

/*
** Extend the range of a redraw request (from *start to *end) with additional
** redraw requests resulting from changes to the attached style buffer (which
** contains auxiliary information for coloring or styling text).
*/
void Fl_Text_Display::extend_range_for_styles( int *start, int *end ) {
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
     tells the text display's buffer modify callback to extend it's redraw
     range to show the text color/and font changes as well. */
  if ( sel->selected() ) {
    if ( sel->start() < *start ) {
      *start = sel->start();
      extended = 1;
    }
    if ( sel->end() > *end ) {
      *end = sel->end();
      extended = 1;
    }
  }

  /* If the Fl_Text_Selection was extended due to a style change, and some of the
     fonts don't match in spacing, extend redraw area to end of line to
     redraw characters exposed by possible font size changes */
  if ( mFixedFontWidth == -1 && extended )
    * end = mBuffer->line_end( *end ) + 1;
}

// The draw() method.  It tries to minimize what is draw as much as possible.
void Fl_Text_Display::draw(void) {
  // don't even try if there is no associated text buffer!
  if (!buffer()) { draw_box(); return; }

  // draw the non-text, non-scrollbar areas.
  if (damage() & FL_DAMAGE_ALL) {
    //printf("drawing all\n");
    // draw the box()
    draw_box(box(), text_area.x, text_area.y, text_area.w, text_area.h,
             color());

    // left margin
    fl_rectf(text_area.x-LEFT_MARGIN, text_area.y-TOP_MARGIN,
             LEFT_MARGIN, text_area.h+TOP_MARGIN+BOTTOM_MARGIN,
             color());

    // right margin
    fl_rectf(text_area.x+text_area.w, text_area.y-TOP_MARGIN,
             RIGHT_MARGIN, text_area.h+TOP_MARGIN+BOTTOM_MARGIN,
             color());

    // top margin
    fl_rectf(text_area.x, text_area.y-TOP_MARGIN,
             text_area.w, TOP_MARGIN, color());

    // bottom margin
    fl_rectf(text_area.x, text_area.y+text_area.h,
             text_area.w, BOTTOM_MARGIN, color());

    // draw that little box in the corner of the scrollbars
    if (mVScrollBar->visible() && mHScrollBar->visible())
      fl_rectf(mVScrollBar->x(), mHScrollBar->y(),
               mVScrollBar->w(), mHScrollBar->h(),
               color());

    // blank the previous cursor protrusions
  }
  else if (damage() & (FL_DAMAGE_SCROLL | FL_DAMAGE_EXPOSE)) {
    //printf("blanking previous cursor extrusions at Y: %d\n", mCursorOldY);
    // CET - FIXME - save old cursor position instead and just draw side needed?
    fl_push_clip(text_area.x-LEFT_MARGIN,
                 text_area.y,
                 text_area.w+LEFT_MARGIN+RIGHT_MARGIN,
                 text_area.h);
    fl_rectf(text_area.x-LEFT_MARGIN, mCursorOldY,
             LEFT_MARGIN, mMaxsize, color());
    fl_rectf(text_area.x+text_area.w, mCursorOldY,
             RIGHT_MARGIN, mMaxsize, color());
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
    int X, Y, W, H;
    fl_clip_box(text_area.x, text_area.y,
                text_area.w, text_area.h,
                X, Y, W, H);
    draw_text(X, Y, W, H); // this sets the clipping internally

    // draw some lines of text
  }
  else if (damage() & FL_DAMAGE_SCROLL) {
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
  if (damage() & (FL_DAMAGE_ALL | FL_DAMAGE_SCROLL | FL_DAMAGE_EXPOSE)
      && !buffer()->primary_selection()->selected() &&
      mCursorOn && Fl::focus() == this ) {
    fl_push_clip(text_area.x-LEFT_MARGIN,
                 text_area.y,
                 text_area.w+LEFT_MARGIN+RIGHT_MARGIN,
                 text_area.h);

    int X, Y;
    if (position_to_xy(mCursorPos, &X, &Y)) draw_cursor(X, Y);
    //printf("drew cursor at pos: %d (%d,%d)\n", mCursorPos, X, Y);
    mCursorOldY = Y;
    fl_pop_clip();
  }
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

int Fl_Text_Display::handle(int event) {
  if (!buffer()) return 0;
  // This isn't very elegant!
  if (!Fl::event_inside(text_area.x, text_area.y, text_area.w, text_area.h)
      && !dragging) {
    return Fl_Group::handle(event);
  }

  switch (event) {
    case FL_PUSH: {
        if (Fl::event_state()&FL_SHIFT) return handle(FL_DRAG);
        dragging = 1;
        int pos = xy_to_position(Fl::event_x(), Fl::event_y(), CURSOR_POS);
        dragType = Fl::event_clicks();
        dragPos = pos;
        if (dragType == DRAG_CHAR)
          buffer()->unselect();
        else if (dragType == DRAG_WORD)
          buffer()->select(word_start(pos), word_end(pos));
        else if (dragType == DRAG_LINE)
          buffer()->select(buffer()->line_start(pos), buffer()->line_end(pos)+1);

        if (buffer()->primary_selection()->selected())
          insert_position(buffer()->primary_selection()->end());
        else
          insert_position(pos);
        show_insert_position();
        return 1;
      }

    case FL_DRAG: {
        if (dragType < 0) return 1;
        int X = Fl::event_x(), Y = Fl::event_y(), pos;
        if (Y < text_area.y) {
          move_up();
          pos = insert_position();
        } else if (Y >= text_area.y+text_area.h) {
          move_down();
          pos = insert_position();
        } else pos = xy_to_position(X, Y, CURSOR_POS);
        fl_text_drag_me(pos, this);
        return 1;
      }

    case FL_RELEASE: {
        dragging = 0;

        // convert from WORD or LINE selection to CHAR
        if (insert_position() >= dragPos)
          dragPos = buffer()->primary_selection()->start();
        else
          dragPos = buffer()->primary_selection()->end();
        dragType = DRAG_CHAR;

        const char* copy = buffer()->selection_text();
        if (*copy) Fl::selection(*this, copy, strlen(copy));
        free((void*)copy);
        return 1;
      }

    case FL_MOUSEWHEEL:
      return mVScrollBar->handle(event);
#if 0
        // I shouldn't be using mNVisibleLines or mTopLineNum here in handle()
        // because the values for these might change between now and layout(),
        // but it's OK because I really want the result based on how things
        // were last displayed rather than where they should be displayed next
        // time layout()/draw() happens.
        int lines, sign = (Fl::event_dy() < 0) ? -1 : 1;
        if (abs(Fl::event_dy()) > mNVisibleLines-2) lines = mNVisibleLines-2;
        else lines = abs(Fl::event_dy());
        scroll(mTopLineNum - lines*sign, mHorizOffset);
        return 1;
#endif
  }

  return 0;
}


//
// End of "$Id: Fl_Text_Display.cxx,v 1.12.2.1 2001/08/04 12:21:33 easysw Exp $".
//
