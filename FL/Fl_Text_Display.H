//
// "$Id: Fl_Text_Display.H,v 1.4.2.9 2002/08/05 18:03:58 easysw Exp $"
//
// Header file for Fl_Text_Display class.
//
// Copyright 2001-2002 by Bill Spitzak and others.
// Original code Copyright Mark Edel.  Permission to distribute under
// the LGPL for the FLTK library granted by Mark Edel.
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

#include "fl_draw.H"
#include "Fl_Group.H"
#include "Fl_Widget.H"
#include "Fl_Scrollbar.H"
#include "Fl_Text_Buffer.H"

class Fl_Text_Display: public Fl_Group {
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

    // style attributes - currently not implemented!
    enum {
      ATTR_NONE = 0,
      ATTR_UNDERLINE = 1,
      ATTR_HIDDEN = 2
    };

    struct FL_EXPORT Style_Table_Entry {
      Fl_Color	color;
      Fl_Font	font;
      int	size;
      unsigned	attr;
    };

    FL_EXPORT Fl_Text_Display(int X, int Y, int W, int H, const char *l = 0);
    FL_EXPORT ~Fl_Text_Display();

    FL_EXPORT virtual int handle(int e);
    FL_EXPORT void buffer(Fl_Text_Buffer* buf);
    void buffer(Fl_Text_Buffer& buf) { buffer(&buf); }
    Fl_Text_Buffer* buffer() { return mBuffer; }
    FL_EXPORT void redisplay_range(int start, int end);
    FL_EXPORT void scroll(int topLineNum, int horizOffset);
    FL_EXPORT void insert(const char* text);
    FL_EXPORT void overstrike(const char* text);
    FL_EXPORT void insert_position(int newPos);
    int insert_position() { return mCursorPos; }
    FL_EXPORT int in_selection(int x, int y);
    FL_EXPORT void show_insert_position();
    FL_EXPORT int move_right();
    FL_EXPORT int move_left();
    FL_EXPORT int move_up();
    FL_EXPORT int move_down();
    FL_EXPORT void next_word(void);
    FL_EXPORT void previous_word(void);
    FL_EXPORT void show_cursor(int b = 1);
    void hide_cursor() { show_cursor(0); }
    FL_EXPORT void cursor_style(int style);
    int scrollbar_width() { return scrollbar_width_; }
    Fl_Align scrollbar_align() { return scrollbar_align_; }
    void scrollbar_width(int w) { scrollbar_width_ = w; }
    void scrollbar_align(Fl_Align a) { scrollbar_align_ = a; }
    int word_start(int pos) { return buffer()->word_start(pos); }
    int word_end(int pos) { return buffer()->word_end(pos); }

    
    FL_EXPORT void highlight_data(Fl_Text_Buffer *styleBuffer,
                        const Style_Table_Entry *styleTable,
                        int nStyles, char unfinishedStyle,
                        Unfinished_Style_Cb unfinishedHighlightCB,
                        void *cbArg);

    FL_EXPORT int position_style(int lineStartPos, int lineLen, int lineIndex,
                       int dispIndex);

    Fl_Font textfont() const {return (Fl_Font)textfont_;}
    void textfont(uchar s) {textfont_ = s;}
    uchar textsize() const {return textsize_;}
    void textsize(uchar s) {textsize_ = s;}
    Fl_Color textcolor() const {return (Fl_Color)textcolor_;}
    void textcolor(unsigned n) {textcolor_ = n;}

    FL_EXPORT virtual void resize(int X, int Y, int W, int H);

  protected:
    // Most (all?) of this stuff should only be called from resize() or
    // draw().
    // Anything with "vline" indicates thats it deals with currently
    // visible lines.

    FL_EXPORT virtual void draw();
    FL_EXPORT void draw_text(int X, int Y, int W, int H);
    FL_EXPORT void draw_range(int start, int end);
    FL_EXPORT void draw_cursor(int, int);

    FL_EXPORT void draw_string(int style, int x, int y, int toX, const char *string,
                     int nChars);

    FL_EXPORT void draw_vline(int visLineNum, int leftClip, int rightClip,
                    int leftCharIndex, int rightCharIndex);

    FL_EXPORT void clear_rect(int style, int x, int y, int width, int height);
    FL_EXPORT void display_insert();

    FL_EXPORT void offset_line_starts(int newTopLineNum);

    FL_EXPORT void calc_line_starts(int startLine, int endLine);

    FL_EXPORT void update_line_starts(int pos, int charsInserted, int charsDeleted,
                            int linesInserted, int linesDeleted, int *scrolled);

    FL_EXPORT void calc_last_char();

    FL_EXPORT int position_to_line( int pos, int* lineNum );
    FL_EXPORT int string_width(const char* string, int length, int style);

    FL_EXPORT static void buffer_modified_cb(int pos, int nInserted, int nDeleted,
                                   int nRestyled, const char* deletedText,
                                   void* cbArg);

    FL_EXPORT static void h_scrollbar_cb(Fl_Scrollbar* w, Fl_Text_Display* d);
    FL_EXPORT static void v_scrollbar_cb( Fl_Scrollbar* w, Fl_Text_Display* d);
    FL_EXPORT void update_v_scrollbar();
    FL_EXPORT void update_h_scrollbar();
    FL_EXPORT int measure_vline(int visLineNum);
    FL_EXPORT int longest_vline();
    FL_EXPORT int empty_vlines();
    FL_EXPORT int vline_length(int visLineNum);
    FL_EXPORT int xy_to_position(int x, int y, int PosType = CHARACTER_POS);

    FL_EXPORT void xy_to_rowcol(int x, int y, int* row, int* column,
                      int PosType = CHARACTER_POS);

    FL_EXPORT int position_to_xy(int pos, int* x, int* y);
    FL_EXPORT int position_to_linecol(int pos, int* lineNum, int* column);
    FL_EXPORT void scroll_(int topLineNum, int horizOffset);

    FL_EXPORT void extend_range_for_styles(int* start, int* end);


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
    int mNStyles;               /* Number of entries in styleTable */
    const Style_Table_Entry *mStyleTable; /* Table of fonts and colors for
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
    Fl_Align scrollbar_align_;
    int dragPos, dragType, dragging;
    int display_insert_position_hint;
    struct { int x, y, w, h; } text_area;

    uchar textfont_;
    uchar textsize_;
    unsigned textcolor_;

    // The following are not presently used from the original NEdit
    // code, but are being put here so that future versions of
    // Fl_Text_Display can implement wrapping and line numbers
    // without breaking binary compatibility.
    int mLineNumLeft, mLineNumWidth;
				/* Line number margin and width */
    int mContinuousWrap;       	/* Wrap long lines when displaying */
    int mWrapMargin; 	     	/* Margin in # of char positions for
    	    	    	       	   wrapping in continuousWrap mode */
};

#endif

//
// End of "$Id: Fl_Text_Display.H,v 1.4.2.9 2002/08/05 18:03:58 easysw Exp $".
//
