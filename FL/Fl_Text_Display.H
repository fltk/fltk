//
// "$Id: Fl_Text_Display.H,v 1.4 2000/08/20 04:31:36 spitzak Exp $"
//
// Header file for Fl_Text_Display class.
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

#ifndef FL_TEXT_DISPLAY_H
#define FL_TEXT_DISPLAY_H

#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Text_Buffer.H>

class FL_API Fl_Text_Display: public Fl_Group {
  public:
    enum {
      NORMAL_CURSOR, CARET_CURSOR, DIM_CURSOR,
      BLOCK_CURSOR, HEAVY_CURSOR
    };

    enum {
      CURSOR_POS, CHARACTER_POS
    };

    // drag types- they match Fl::event_clicks() so that single clicking to
    // start a collection selects by character, double clicking selects by
    // word and triple clicking selects by line.
    enum {
      DRAG_CHAR = 0, DRAG_WORD = 1, DRAG_LINE = 2
    };
    friend void fl_text_drag_me(int pos, Fl_Text_Display* d);

    typedef void (*Unfinished_Style_Cb)();

    struct FL_API Style_Table_Entry {
      Fl_Color color;
      Fl_Font  font;
      int      size;
    };

    Fl_Text_Display(int X, int Y, int W, int H, const char *l = 0);
    ~Fl_Text_Display();

    virtual int handle(int e);
    void buffer(Fl_Text_Buffer* buf);
    void buffer(Fl_Text_Buffer& buf) { buffer(&buf); }
    Fl_Text_Buffer* buffer() { return mBuffer; }
    void redisplay_range(int start, int end);
    void scroll(int topLineNum, int horizOffset);
    void insert(const char* text);
    void overstrike(const char* text);
    void insert_position(int newPos);
    int insert_position() { return mCursorPos; }
    int in_selection(int x, int y);
    void show_insert_position();
    int move_right();
    int move_left();
    int move_up();
    int move_down();
    void next_word(void);
    void previous_word(void);
    void show_cursor(int b = 1);
    void hide_cursor() { show_cursor(0); }
    void cursor_style(int style);
    int scrollbar_width() { return scrollbar_width_; }
    Fl_Flags scrollbar_align() { return scrollbar_align_; }
    void scrollbar_width(int w) { scrollbar_width_ = w; }
    void scrollbar_align(Fl_Flags a) { scrollbar_align_ = a; }
    int word_start(int pos) { return buffer()->word_start(pos); }
    int word_end(int pos) { return buffer()->word_end(pos); }

    void highlight_data(Fl_Text_Buffer *styleBuffer,
                        Style_Table_Entry *styleTable,
                        int nStyles, char unfinishedStyle,
                        Unfinished_Style_Cb unfinishedHighlightCB,
                        void *cbArg);

    int position_style(int lineStartPos, int lineLen, int lineIndex,
                       int dispIndex);
    
  protected:
    // Most (all?) of this stuff should only be called from layout() or
    // draw().
    // Anything with "vline" indicates thats it deals with currently
    // visible lines.
    virtual void layout();

    virtual void draw();
    void draw_text(int X, int Y, int W, int H);
    void draw_range(int start, int end);
    void draw_cursor(int, int);

    void draw_string(int style, int x, int y, int toX, const char *string,
                     int nChars);

    void draw_vline(int visLineNum, int leftClip, int rightClip,
                    int leftCharIndex, int rightCharIndex);

    void clear_rect(int style, int x, int y, int width, int height);
    void display_insert();

    void offset_line_starts(int newTopLineNum);

    void calc_line_starts(int startLine, int endLine);

    void update_line_starts(int pos, int charsInserted, int charsDeleted,
                            int linesInserted, int linesDeleted, int *scrolled);

    void calc_last_char();

    int position_to_line( int pos, int* lineNum );
    int string_width(const char* string, int length, int style);

    static void buffer_modified_cb(int pos, int nInserted, int nDeleted,
                                   int nRestyled, const char* deletedText,
                                   void* cbArg);

    static void h_scrollbar_cb(Fl_Scrollbar* w, Fl_Text_Display* d);
    static void v_scrollbar_cb( Fl_Scrollbar* w, Fl_Text_Display* d);
    void update_v_scrollbar();
    void update_h_scrollbar();
    int measure_vline(int visLineNum);
    int longest_vline();
    int empty_vlines();
    int vline_length(int visLineNum);
    int xy_to_position(int x, int y, int PosType = CHARACTER_POS);

    void xy_to_rowcol(int x, int y, int* row, int* column,
                      int PosType = CHARACTER_POS);

    int position_to_xy(int pos, int* x, int* y);
    int position_to_linecol(int pos, int* lineNum, int* column);
    void scroll_(int topLineNum, int horizOffset);

    void extend_range_for_styles(int* start, int* end);


    int damage_range1_start, damage_range1_end;
    int damage_range2_start, damage_range2_end;
    int mCursorPos;
    int mCursorOn;
    int mCursorOldY;		/* Y pos. of cursor for blanking */
    int mCursorToHint;		/* Tells the buffer modified callback
                                   where to move the cursor, to reduce
                                   the number of redraw calls */
    int mCursorStyle;           /* One of enum cursorStyles above */
    int mCursorPreferredCol;    /* Column for vert. cursor movement */
    int mNVisibleLines;         /* # of visible (displayed) lines */
    int mNBufferLines;          /* # of newlines in the buffer */
    Fl_Text_Buffer* mBuffer;    /* Contains text to be displayed */
    Fl_Text_Buffer* mStyleBuffer; /* Optional parallel buffer containing
                                     color and font information */
    int mFirstChar, mLastChar;  /* Buffer positions of first and last
                                   displayed character (lastChar points
                                   either to a newline or one character
                                   beyond the end of the buffer) */
    int* mLineStarts;
    int mTopLineNum;            /* Line number of top displayed line
                                   of file (first line of file is 1) */
    int mHorizOffset;           /* Horizontal scroll pos. in pixels */
    int mTopLineNumHint;        /* Line number of top displayed line
                                   of file (first line of file is 1) */
    int mHorizOffsetHint;       /* Horizontal scroll pos. in pixels */
    int mVisibility;            /* Window visibility (see XVisibility
                                   event) */
    int mNStyles;               /* Number of entries in styleTable */
    Style_Table_Entry *mStyleTable; /* Table of fonts and colors for
                                   coloring/syntax-highlighting */
    char mUnfinishedStyle;      /* Style buffer entry which triggers
                                   on-the-fly reparsing of region */
    Unfinished_Style_Cb mUnfinishedHighlightCB; /* Callback to parse "unfinished" */
                                /* regions */
    void* mHighlightCBArg;      /* Arg to unfinishedHighlightCB */

    int mMaxsize;

    int mFixedFontWidth;        /* Font width if all current fonts are
                                   fixed and match in width, else -1 */

    Fl_Color mCursor_color;

    Fl_Scrollbar* mHScrollBar;
    Fl_Scrollbar* mVScrollBar;
    int scrollbar_width_;
    Fl_Flags scrollbar_align_;
    int dragPos, dragType, dragging;
    int display_insert_position_hint;
    struct { int x, y, w, h; } text_area;
};

#endif

//
// End of "$Id: Fl_Text_Display.H,v 1.4 2000/08/20 04:31:36 spitzak Exp $".
//
