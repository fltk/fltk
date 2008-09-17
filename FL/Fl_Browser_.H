//
// "$Id$"
//
// Common browser header file for the Fast Light Tool Kit (FLTK).
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
   Fl_Browser_ widget . */

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

/**
  This is the base for browsers.  To be useful it must be
  subclassed and several virtual functions defined.  The Forms-compatible
  browser and the file chooser's browser are subclassed off of this.
  <P>This has been designed so that the subhas complete control
  over the storage of the data, although because next() and 
  prev() functions are used to index, it works best as a linked list
  or as a large block of characters in which the line breaks must be
  searched for. </P>
  <P>A great deal of work has been done so that the "height" of a data
  object does not need to be determined until it is drawn.  This is
  useful if actually figuring out the size of an object requires
  accessing image data or doing stat() on a file or doing some
  other slow operation.
*/
class FL_EXPORT Fl_Browser_ : public Fl_Group {
  int position_;	// where user wants it scrolled to
  int real_position_;	// the current vertical scrolling position
  int hposition_;	// where user wants it panned to
  int real_hposition_;	// the current horizontal scrolling position
  int offset_;		// how far down top_ item the real_position is
  int max_width;	// widest object seen so far
  uchar has_scrollbar_;	// which scrollbars are enabled
  Fl_Font textfont_;
  Fl_Fontsize textsize_;
  unsigned textcolor_;
  void* top_;		// which item scrolling position is in
  void* selection_;	// which is selected (except for FL_MULTI_BROWSER)
  void *redraw1,*redraw2; // minimal update pointers
  void* max_width_item;	// which item has max_width_

  static int scrollbar_width_;

  void update_top();

protected:

  // All of the following must be supplied by the subclass:
  /** This method must be provided by the subclass to return the first item in  the list. */
    virtual void *item_first() const = 0;
  /** This method must be provided by the subclass to return the item in the list after p. */
    virtual void *item_next(void *) const = 0;
  /** This method must be provided by the subclass to return the item in the list before p. */
    virtual void *item_prev(void *) const = 0;
  /** 
    This method must be provided by the subclass to return the height of the
    item p in pixels.  Allow for two additional pixels for the list
    selection box.
  */
  virtual int item_height(void *) const = 0;
  /**
    This method must be provided by the subclass to return the width of the
    item p in pixels.  Allow for two additional pixels for the list
    selection box.
  */
  virtual int item_width(void *) const = 0;
  virtual int item_quick_height(void *) const ;
  /**
    This method must be provided by the subclass to draw the item
    p in the area indicated by x, y, w,
    and h.
  */
  virtual void item_draw(void *,int,int,int,int) const = 0;
  // you don't have to provide these but it may help speed it up:
  virtual int full_width() const ;	// current width of all items
  virtual int full_height() const ;	// current height of all items
  virtual int incr_height() const ;	// average height of an item
  // These only need to be done by subclass if you want a multi-browser:
  virtual void item_select(void *,int=1);
  virtual int item_selected(void *) const ;

  // things the subclass may want to call:
  /**  Returns the item the appears at the top of the list.  */
  void *top() const {return top_;}
  /**
    Returns the item currently selected, or NULL if there is no selection.
    
    <P>For multiple selection browsers this call returns the currently focused item,
    even if it is not selected. To find all selected items, call 
    
    Fl_Multi_Browser::selected() for every item in question.
  */
  void *selection() const {return selection_;}
  void new_list(); // completely clobber all data, as though list replaced
  void deleting(void *a); // get rid of any pointers to a
  void replacing(void *a,void *b); // change a pointers to b
  void swapping(void *a,void *b); // exchange pointers a and b
  void inserting(void *a,void *b); // insert b near a
  int displayed(void *) const ; // true if this line is visible
  void redraw_line(void *); // minimal update, no change in size
  /**    This method will cause the entire list to be redrawn.  */
  void redraw_lines() {damage(FL_DAMAGE_SCROLL);} // redraw all of them
  void bbox(int&,int&,int&,int&) const;
  int leftedge() const;	// x position after scrollbar & border
  void *find_item(int my); // item under mouse
  /**
    The first form draws the list within the normal widget bounding box.
    
    <P>The second form draws the contents of the browser within the
    specified bounding box.
  */
  void draw(int,int,int,int);
  int handle(int,int,int,int,int);

  void draw();
  Fl_Browser_(int,int,int,int,const char * = 0);

public:

  Fl_Scrollbar scrollbar;		// Vertical scrollbar
  Fl_Scrollbar hscrollbar;		// Horizontal scrollbar

  int handle(int);
  void resize(int,int,int,int);

  int select(void *,int=1,int docallbacks=0);
  int select_only(void *,int docallbacks=0);
  int deselect(int docallbacks=0);
  /**
    Gets or sets the vertical scrolling position of the list,
    which is the pixel offset of the list items within the list
    area.
  */
  int position() const {return position_;}
  /**
    Gets or sets the horizontal scrolling position of the list,
    which is the pixel offset of the list items within the list
    area.
  */
  int hposition() const {return hposition_;}
  void position(int); // scroll to here
  void hposition(int); // pan to here
  void display(void*); // scroll so this item is shown
  /** See Fl_Browser_::has_scrollbar(uchar) */
  uchar has_scrollbar() const {return has_scrollbar_;}
  /**
    By default you can scroll in both directions, and the scrollbars
    disappear if the data will fit in the widget.  has_scrollbar() changes
    this based on the value of h:
  
    <UL>
  
  	<LI>0 - No scrollbars.
  
  	<LI>Fl_Browser_::HORIZONTAL - Only a horizontal
  	scrollbar.
  
  	<LI>Fl_Browser_::VERTICAL - Only a vertical
  	scrollbar.
  
  	<LI>Fl_Browser_::BOTH - The default is both
  	scrollbars.
  
  	<LI>Fl_Browser_::HORIZONTAL_ALWAYS - Horizontal
  	scrollbar always on, vertical always off.
  
  	<LI>Fl_Browser_::VERTICAL_ALWAYS - Vertical
  	scrollbar always on, horizontal always off.
  
  	<LI>Fl_Browser_::BOTH_ALWAYS - Both always on.
  
    </UL>
  */
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

  /**
    The first form gets the default text font for the lines in the
    browser.
    
    <P>The second form sets the default text font to font
  */
  Fl_Font textfont() const {return textfont_;}
  /**
    The first form gets the default text font for the lines in the
    browser.
    
    <P>The second form sets the default text font to font
  */
  void textfont(Fl_Font s) {textfont_ = s;}
  /**
    The first form gets the default text size for the lines in the
    browser.
    
    <P>The second form sets the default text size to size
  */
  Fl_Fontsize textsize() const {return textsize_;}
  void textsize(Fl_Fontsize s) {textsize_ = s;}
  /**
    The first form gets the default text color for the lines in the
    browser.
    
    <P>The second form sets the default text color to color
  */
  Fl_Color textcolor() const {return (Fl_Color)textcolor_;}
  void textcolor(unsigned n) {textcolor_ = n;}

  /**    Sets or gets the width of any scrollbars that are used.  */
  static void scrollbar_width(int b) {scrollbar_width_ = b;}
  /**    Sets or gets the width of any scrollbars that are used.  */
  static int scrollbar_width() {return scrollbar_width_;}

  // for back compatibility:
  /**    This method moves the vertical scrollbar to the righthand side of the list.  */
  void scrollbar_right() {scrollbar.align(FL_ALIGN_RIGHT);}
  /**    This method moves the vertical scrollbar to the lefthand side of the list.  */
  void scrollbar_left() {scrollbar.align(FL_ALIGN_LEFT);}

};

#endif

//
// End of "$Id$".
//
