//
// "$Id: Fl_Browser_.H,v 1.6.2.4 2001/01/22 15:13:37 easysw Exp $"
//
// Common browser header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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

// This is the base class for browsers.  To be useful it must
// be subclassed and several virtual functions defined.  The
// Forms-compatable browser and the file chooser's browser are
// subclassed off of this.

// Yes, I know this should be a template...

#ifndef Fl_Browser__H
#define Fl_Browser__H

#ifndef Fl_Group_H
#include "Fl_Group.H"
#endif
#include "Fl_Scrollbar.H"

#define FL_NORMAL_BROWSER	0
#define FL_SELECT_BROWSER	1
#define FL_HOLD_BROWSER		2
#define FL_MULTI_BROWSER	3

class Fl_Browser_ : public Fl_Group {
  int position_;	// where user wants it scrolled to
  int real_position_;	// the current vertical scrolling position
  int hposition_;	// where user wants it panned to
  int real_hposition_;	// the current horizontal scrolling position
  int offset_;		// how far down top_ item the real_position is
  int max_width;	// widest object seen so far
  uchar textfont_, textsize_, textcolor_;
  uchar has_scrollbar_;	// which scrollbars are enabled
  void* top_;		// which item scrolling position is in
  void* selection_;	// which is selected (except for FL_MULTI_BROWSER)
  void *redraw1,*redraw2; // minimal update pointers
  void* max_width_item;	// which item has max_width_

  static int scrollbar_width_;

  FL_EXPORT void update_top();

protected:

  // All of the following must be supplied by the subclass:
  virtual FL_EXPORT void *item_first() const = 0;
  virtual FL_EXPORT void *item_next(void *) const = 0;
  virtual FL_EXPORT void *item_prev(void *) const = 0;
  virtual FL_EXPORT int item_height(void *) const = 0;
  virtual FL_EXPORT int item_width(void *) const = 0;
  virtual FL_EXPORT int item_quick_height(void *) const ;
  virtual FL_EXPORT void item_draw(void *,int,int,int,int) const = 0;
  // you don't have to provide these but it may help speed it up:
  virtual FL_EXPORT int full_width() const ;	// current width of all items
  virtual FL_EXPORT int full_height() const ;	// current height of all items
  virtual FL_EXPORT int incr_height() const ;	// average height of an item
  // These only need to be done by subclass if you want a multi-browser:
  virtual FL_EXPORT void item_select(void *,int=1);
  virtual FL_EXPORT int item_selected(void *) const ;

  // things the subclass may want to call:
  FL_EXPORT void *top() const {return top_;}
  FL_EXPORT void *selection() const {return selection_;}
  FL_EXPORT void new_list(); // completely clobber all data, as though list replaced
  FL_EXPORT void deleting(void *a); // get rid of any pointers to a
  FL_EXPORT void replacing(void *a,void *b); // change a pointers to b
  FL_EXPORT void inserting(void *a,void *b); // insert a before b
  FL_EXPORT int displayed(void *) const ; // true if this line is visible
  FL_EXPORT void redraw_line(void *); // minimal update, no change in size
  FL_EXPORT void redraw_lines() {damage(FL_DAMAGE_SCROLL);} // redraw all of them
  FL_EXPORT void bbox(int&,int&,int&,int&) const;
  FL_EXPORT int leftedge() const;	// x position after scrollbar & border
  FL_EXPORT void *find_item(int my); // item under mouse
  FL_EXPORT void draw(int,int,int,int);
  FL_EXPORT int handle(int,int,int,int,int);

  FL_EXPORT void draw();
  FL_EXPORT Fl_Browser_(int,int,int,int,const char * = 0);

public:

  Fl_Scrollbar scrollbar;		// Vertical scrollbar
  Fl_Scrollbar hscrollbar;		// Horizontal scrollbar

  FL_EXPORT int handle(int);
  FL_EXPORT void resize(int,int,int,int);

  FL_EXPORT int select(void *,int=1,int docallbacks=0);
  FL_EXPORT int select_only(void *,int docallbacks=0);
  FL_EXPORT int deselect(int docallbacks=0);
  int position() const {return position_;}
  int hposition() const {return hposition_;}
  FL_EXPORT void position(int); // scroll to here
  FL_EXPORT void hposition(int); // pan to here
  FL_EXPORT void display(void*); // scroll so this item is shown

  uchar has_scrollbar() const {return has_scrollbar_;}
  void has_scrollbar(uchar i) {has_scrollbar_ = i;}
  enum { // values for has_scrollbar()
    HORIZONTAL = 1,
    VERTICAL = 2,
    BOTH = 3,
    ALWAYS_ON = 4,
    HORIZONTAL_ALWAYS = 5,
    VERTICAL_ALWAYS = 6,
    BOTH_ALWAYS = 7
  };

  Fl_Font textfont() const {return (Fl_Font)textfont_;}
  void textfont(uchar s) {textfont_ = s;}
  uchar textsize() const {return textsize_;}
  void textsize(uchar s) {textsize_ = s;}
  Fl_Color textcolor() const {return (Fl_Color)textcolor_;}
  void textcolor(uchar n) {textcolor_ = n;}

  static void scrollbar_width(int b) {scrollbar_width_ = b;}
  static int scrollbar_width() {return scrollbar_width_;}

  // for back compatability:
  void scrollbar_right() {scrollbar.align(FL_ALIGN_RIGHT);}
  void scrollbar_left() {scrollbar.align(FL_ALIGN_LEFT);}

};

#endif

//
// End of "$Id: Fl_Browser_.H,v 1.6.2.4 2001/01/22 15:13:37 easysw Exp $".
//
