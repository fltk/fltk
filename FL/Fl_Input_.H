//
// "$Id$"
//
// Input base class header file for the Fast Light Tool Kit (FLTK).
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

/* \file
   Fl_Input_ widget . */

#ifndef Fl_Input__H
#define Fl_Input__H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

#define FL_NORMAL_INPUT		0
#define FL_FLOAT_INPUT		1
#define FL_INT_INPUT		2
#define FL_HIDDEN_INPUT		3
#define FL_MULTILINE_INPUT	4
#define FL_SECRET_INPUT		5
#define FL_INPUT_TYPE		7
#define FL_INPUT_READONLY	8
#define FL_NORMAL_OUTPUT	(FL_NORMAL_INPUT | FL_INPUT_READONLY)
#define FL_MULTILINE_OUTPUT	(FL_MULTILINE_INPUT | FL_INPUT_READONLY)
#define FL_INPUT_WRAP		16
#define FL_MULTILINE_INPUT_WRAP	(FL_MULTILINE_INPUT | FL_INPUT_WRAP)
#define FL_MULTILINE_OUTPUT_WRAP (FL_MULTILINE_INPUT | FL_INPUT_READONLY | FL_INPUT_WRAP)

/**
  This is a virtual base class below Fl_Input. It has all
  the same interfaces, but lacks the handle() and
  draw() method. You may want to subclass it if you are
  one of those people who likes to change how the editing keys
  work.
  
  <P>This can act like any of the subclasses of Fl_Input, by
  setting type() to one of the following values:</P>
  
  \code
  #define FL_NORMAL_INPUT	   0
  #define FL_FLOAT_INPUT	   1
  #define FL_INT_INPUT		   2
  #define FL_MULTILINE_INPUT	   4
  #define FL_SECRET_INPUT	   5
  #define FL_INPUT_TYPE		   7
  #define FL_INPUT_READONLY	   8
  #define FL_NORMAL_OUTPUT	   (FL_NORMAL_INPUT | FL_INPUT_READONLY)
  #define FL_MULTILINE_OUTPUT	   (FL_MULTILINE_INPUT | FL_INPUT_READONLY)
  #define FL_INPUT_WRAP		   16
  #define FL_MULTILINE_INPUT_WRAP  (FL_MULTILINE_INPUT | FL_INPUT_WRAP)
  #define FL_MULTILINE_OUTPUT_WRAP (FL_MULTILINE_INPUT | FL_INPUT_READONLY | FL_INPUT_WRAP)
  \endcode
*/
class FL_EXPORT Fl_Input_ : public Fl_Widget {

  const char* value_;
  char* buffer;

  int size_;
  int bufsize;
  int position_;
  int mark_;
  int xscroll_, yscroll_;
  int mu_p;
  int maximum_size_;
  int shortcut_;

  uchar erase_cursor_only;
  Fl_Font textfont_;
  Fl_Fontsize textsize_;
  unsigned textcolor_;
  unsigned cursor_color_;

  const char* expand(const char*, char*) const;
  double expandpos(const char*, const char*, const char*, int*) const;
  void minimal_update(int, int);
  void minimal_update(int p);
  void put_in_buffer(int newsize);

  void setfont() const;

protected:

  int word_start(int i) const;
  int word_end(int i) const;
  int line_start(int i) const;
  int line_end(int i) const;
  void drawtext(int, int, int, int);
  int up_down_position(int, int keepmark=0);
  void handle_mouse(int, int, int, int, int keepmark=0);
  int handletext(int e, int, int, int, int);
  void maybe_do_callback();
  int xscroll() const {return xscroll_;}
  int yscroll() const {return yscroll_;}

public:

  void resize(int, int, int, int);

  Fl_Input_(int, int, int, int, const char* = 0);
  ~Fl_Input_();

  int value(const char*);
  int value(const char*, int);
  int static_value(const char*);
  int static_value(const char*, int);
  /**
      The first form returns the current value, which is a pointer
      to the internal buffer and is valid only until the next event is
      handled.
      
      <P>The second two forms change the text and set the mark and the
      point to the end of it. The string is copied to the internal
      buffer. Passing NULL is the same as "". 
      This returns non-zero if the new value is different than the
      current one.  You can use the second version to directly set the
      length if you know it already or want to put nul's in the
      text.
  */
  const char* value() const {return value_;}
  /**
      Same as value()[n], but may be faster in plausible
      implementations. No bounds checking is done. 
  */
char index(int i) const {return value_[i];}
  /**
      Returns the number of characters in value(). This
      may be greater than strlen(value()) if there are nul
      characters in it.
  */
  int size() const {return size_;}
  void size(int W, int H) { Fl_Widget::size(W, H); }
  /** Gets the maximum length of the input field.  */
  int maximum_size() const {return maximum_size_;}
  /** Sets the maximum length of the input field.  */
  void maximum_size(int m) {maximum_size_ = m;}
  /**
    The input widget maintains two pointers into the string. The
    "position" is where the cursor is. The
    "mark" is the other end of the selected text. If they
    are equal then there is no selection. Changing this does not
    affect the clipboard (use copy() to do that).
    
    <P>Changing these values causes a redraw(). The new
    values are bounds checked. The return value is non-zero if the
    new position is different than the old one. position(n)
    is the same as position(n,n). mark(n) is the
    same as position(position(),n).
  */
  int position() const {return position_;}
  /** Gets the current selection mark. mark(n) is  the same as position(position(),n).*/
  int mark() const {return mark_;}
  int position(int p, int m);
   /**  See int Fl_Input_::position() const */
  int position(int p) {return position(p, p);}
  /** Sets the current selection mark. mark(n) is  the same as position(position(),n).*/
  int mark(int m) {return position(position(), m);}
  int replace(int, int, const char*, int=0);
  /**
    Deletes the current selection.
    cut(n) deletes n characters after the
    position(). cut(-n) deletes n
    characters before the position(). cut(a,b)
    deletes the characters between offsets a and
    b. A, b, and n are all
    clamped to the size of the string. The mark and point are left
    where the deleted text was.
    
    <P>If you want the data to go into the clipboard, do
    Fl_Input_::copy() before calling
    Fl_Input_::cut(), or do Fl_Input_::copy_cuts()
    afterwards.
  */
  int cut() {return replace(position(), mark(), 0);}
  /**    See int Fl_Input_::cut()   */
  int cut(int n) {return replace(position(), position()+n, 0);}
  /**    See int Fl_Input_::cut()   */
  int cut(int a, int b) {return replace(a, b, 0);}
  /**
    Insert the string t at the current position, and
    leave the mark and position after it. If l is not zero
    then it is assumed to be strlen(t).
  */
  int insert(const char* t, int l=0){return replace(position_, mark_, t, l);}
  int copy(int clipboard);
  int undo();
  int copy_cuts();

  /**
    The first form returns the current shortcut key for the Input.
    <P>The second form sets the shortcut key to key. Setting this
    overrides the use of '&' in the label().  The value is a bitwise
    OR of a key and a set of shift flags, for example FL_ALT | 'a'
    , FL_ALT | (FL_F + 10), or just 'a'.  A value
    of 0 disables the shortcut. </P>
    <P>The key can be any value returned by 
    Fl::event_key(), but will usually be an ASCII letter.  Use
    a lower-case letter unless you require the shift key to be held down. </P>
    <P>The shift flags can be any set of values accepted by 
    Fl::event_state().  If the bit is on that shift key must
    be pushed.  Meta, Alt, Ctrl, and Shift must be off if they are not in
    the shift flags (zero for the other bits indicates a "don't care"
    setting).
  */
  int shortcut() const {return shortcut_;}
  /** See int shortcut() const */
  void shortcut(int s) {shortcut_ = s;}

  /** Gets the font of the text in the input field.*/
  Fl_Font textfont() const {return textfont_;}
  /** Sets the font of the text in the input field.*/
  void textfont(Fl_Font s) {textfont_ = s;}
  /** Gets the size of the text in the input field.*/
  Fl_Fontsize textsize() const {return textsize_;}
  /** Sets the size of the text in the input field.*/
  void textsize(Fl_Fontsize s) {textsize_ = s;}
  /** Gets the color of the text in the input field.*/
  Fl_Color textcolor() const {return (Fl_Color)textcolor_;}
  /** Sets the color of the text in the input field.*/
  void textcolor(unsigned n) {textcolor_ = n;}
  /** Gets the color of the cursor.  This is black by default.*/
  Fl_Color cursor_color() const {return (Fl_Color)cursor_color_;}
  /** Sets the color of the cursor.  This is black by default.*/
  void cursor_color(unsigned n) {cursor_color_ = n;}
  /** Gets the input field type. */
  int input_type() const {return type() & FL_INPUT_TYPE; }
  /** Sets the input field type. */
  void input_type(int t) { type((uchar)(t | readonly())); }
  /** Gets the read-only state of the input field.  */
  int readonly() const { return type() & FL_INPUT_READONLY; }
  /** Sets the read-only state of the input field.  */
  void readonly(int b) { if (b) type((uchar)(type() | FL_INPUT_READONLY));
                         else type((uchar)(type() & ~FL_INPUT_READONLY)); }
  /**
    Gets  the word wrapping state of the input field. Word
    wrap is only functional with multi-line input fields.
  */
  int wrap() const { return type() & FL_INPUT_WRAP; }
  /**
    Sets the word wrapping state of the input field. Word
    wrap is only functional with multi-line input fields.
  */
  void wrap(int b) { if (b) type((uchar)(type() | FL_INPUT_WRAP));
                         else type((uchar)(type() & ~FL_INPUT_WRAP)); }
};

#endif 

//
// End of "$Id$".
//
