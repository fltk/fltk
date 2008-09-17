//
// "$Id$"
//
// Browser header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2008 by Bill Spitzak and others.
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
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

/** \file
   Fl_Browser widget . */

// Forms-compatable browser.  Probably useful for other
// lists of textual data.  Notice that the line numbers
// start from 1, and 0 means "no line".

#ifndef Fl_Browser_H
#define Fl_Browser_H

#include "Fl_Browser_.H"

struct FL_BLINE;

/**
  The Fl_Browser widget displays a scrolling list of text
  lines, and manages all the storage for the text.  This is not a text
  editor or spreadsheet!  But it is useful for showing a vertical list of
  named objects to the user.
  <P>Each line in the browser is identified by number. <I>The numbers
  start at one</I> (this is so that zero can be reserved for &quot;no line&quot; in
  the selective browsers). <I>Unless otherwise noted, the methods do not
  check to see if the passed line number is in range and legal.  It must
  always be greater than zero and &lt;= size().</I></P>
  <P>Each line contains a null-terminated string of text and a void *
  data pointer.  The text string is displayed, the void *
  pointer can be used by the callbacks to reference the object the text
  describes. </P>
  <P>The base does nothing when the user clicks on it.  The
  subclasses 
  Fl_Select_Browser, 
  Fl_Hold_Browser, and 
  Fl_Multi_Browser react to user clicks to select lines in
  the browser and do callbacks. </P>
  <P>The base called 
  Fl_Browser_ provides the scrolling and selection mechanisms of
  this and all the subclasses, but the dimensions and appearance of each
  item are determined by the subclass. You can use Fl_Browser_
  to display information other than text, or text that is dynamically
  produced from your own data structures. If you find that loading the
  browser is a lot of work or is inefficient, you may want to make a
  subof Fl_Browser_.
*/
class FL_EXPORT Fl_Browser : public Fl_Browser_ {

  FL_BLINE *first;		// the array of lines
  FL_BLINE *last;
  FL_BLINE *cache;
  int cacheline;		// line number of cache
  int lines;                	// Number of lines
  int full_height_;
  const int* column_widths_;
  char format_char_;		// alternative to @-sign
  char column_char_;		// alternative to tab

protected:

  // required routines for Fl_Browser_ subclass:
  void* item_first() const ;
  void* item_next(void*) const ;
  void* item_prev(void*) const ;
  int item_selected(void*) const ;
  void item_select(void*, int);
  int item_height(void*) const ;
  int item_width(void*) const ;
  void item_draw(void*, int, int, int, int) const ;
  int full_height() const ;
  int incr_height() const ;

  FL_BLINE* find_line(int) const ;
  FL_BLINE* _remove(int) ;
  void insert(int, FL_BLINE*);
  int lineno(void*) const ;
  void swap(FL_BLINE *a, FL_BLINE *b);

public:

  void remove(int);
  void add(const char*, void* = 0);
  void insert(int, const char*, void* = 0);
  void move(int to, int from);
  int  load(const char* filename);
  void swap(int a, int b);
  void clear();

  /**
    Returns how many lines are in the browser.  The last line number is
    equal to this.
  */
  int size() const {return lines;}
  void size(int W, int H) { Fl_Widget::size(W, H); }

  int topline() const ;
  enum Fl_Line_Position { TOP, BOTTOM, MIDDLE };
  void lineposition(int, Fl_Line_Position);
  /**
    The first form returns the current top line in the browser. If there
    is no vertical scrollbar then this will always return 1.
    <P>The second form scrolls the browser so the top line in the browser is n.
  */
  void topline(int l) { lineposition(l, TOP); }
  /**    Scrolls the browser so the bottom line in the browser is n.  */
  void bottomline(int l) { lineposition(l, BOTTOM); }
  /**    Scrolls the browser so the middle line in the browser is n.  */
  void middleline(int l) { lineposition(l, MIDDLE); }

  int select(int, int=1);
  int selected(int) const ;
  void show(int n);
  void show() {Fl_Widget::show();}
  void hide(int n);
  void hide() {Fl_Widget::hide();}
  int visible(int n) const ;

  int value() const ;
  void value(int v) {select(v);}
  const char* text(int) const ;
  void text(int, const char*);
  void* data(int) const ;
  void data(int, void* v);

  Fl_Browser(int, int, int, int, const char* = 0);
  /** The destructor deletes all list items and destroys the browser.*/
  ~Fl_Browser() { clear(); }

  /**
    The first form gets the current format code prefix character, which by
    default is '\@'. A string of formatting codes at the start of
    each column are stripped off and used to modify how the rest of the
    line is printed:
    <UL>
    <LI><tt>'\@.'</tt> Print rest of line, don't look for more '\@' signs </LI>
    <LI><tt>'\@\@'</tt> Print rest of line starting with '\@' </LI>
    <LI><tt>'\@l'</tt> Use a <BIG>large</BIG> (24 point) font </LI>
    <LI><tt>'\@m'</tt> Use a <BIG>medium large</BIG> (18 point) font </LI>
    <LI><tt>'\@s'</tt> Use a <SMALL>small</SMALL> (11 point) font </LI>
    <LI><tt>'\@b'</tt> Use a <B>bold</B> font (adds FL_BOLD to font) </LI>
    <LI><tt>'\@i'</tt> Use an <I>italic</I> font (adds FL_ITALIC to font) </LI>
    <LI><tt>'\@f' or \@t</tt> Use a fixed-pitch
    font (sets font to FL_COURIER) </LI>
    <LI><tt>'\@c'</tt> Center the line horizontally </LI>
    <LI><tt>'\@r'</tt> Right-justify the text </LI>
    <LI><tt>'\@B0', '\@B1', ... '\@B255'</tt> Fill the backgound with
    fl_color(n) </LI>
    <LI><tt>'\@C0', '\@C1', ... '\@C255'</tt> Use fl_color(n) to draw the text </LI>
    <LI><tt>'\@F0', '\@F1', ...</tt> Use fl_font(n) to draw the text </LI>
    <LI><tt>'\@S1', '\@S2', ...</tt> Use point size n to draw the text </LI>
    <LI><tt>'\@u' or '\@_'</tt> Underline the text. </LI>
    <LI><tt>'\@-'</tt> draw an engraved line through the middle. </LI>
    </UL>
    Notice that the '\@.' command can be used to reliably
    terminate the parsing.  To print a random string in a random color, use 
    sprintf("\@C%d\@.%s", color, string) and it will work even if the
    string starts with a digit or has the format character in it.
    <P>The second form sets the current prefix to c.  Set the
    prefix to 0 to disable formatting.
  */
  char format_char() const {return format_char_;}
  /**    See uchar Fl_Browser::format_char() const   */
  void format_char(char c) {format_char_ = c;}
  /**
    The first form gets the current column separator character. By default
    this is '\\t' (tab).
    <P>The second form sets the column separator to c. This will
    only have an effect if you also set column_widths().
  */
  char column_char() const {return column_char_;}
  /**
    The first form gets the current column separator character. By default
    this is '\\t' (tab).
    <P>The second form sets the column separator to c. This will
    only have an effect if you also set column_widths().
  */
  void column_char(char c) {column_char_ = c;}
  /**
    The first form gets the current column width array.  This array is
    zero-terminated and specifies the widths in pixels of each column. The
    text is split at each column_char() and each part is formatted
    into it's own column.  After the last column any remaining text is
    formatted into the space between the last column and the right edge of
    the browser, even if the text contains instances of column_char()
    .  The default value is a one-element array of just a zero, which makes
    there are no columns.
    <P>The second form sets the current array to w.  Make sure the
    last entry is zero.
  */
  const int* column_widths() const {return column_widths_;}
  /**    See const int *Fl_Browser::column_widths() const   */
  void column_widths(const int* l) {column_widths_ = l;}

  int displayed(int n) const {return Fl_Browser_::displayed(find_line(n));}
  void make_visible(int n) {
  	if (n < 1) Fl_Browser_::display(find_line(1));
	else if (n > lines) Fl_Browser_::display(find_line(lines));
	else Fl_Browser_::display(find_line(n));
  }

  // for back compatibility only:
  void replace(int a, const char* b) {text(a, b);}
  void display(int, int=1);
};

#endif

//
// End of "$Id$".
//
