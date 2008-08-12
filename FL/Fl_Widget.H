//
// "$Id$"
//
// Widget header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2005 by Bill Spitzak and others.
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
 * This file describes the Fl_Widget and Fl_Label classes.
 */

#ifndef Fl_Widget_H
#define Fl_Widget_H

#include "Enumerations.H"

class Fl_Widget;
class Fl_Window;
class Fl_Group;
class Fl_Image;

typedef void (Fl_Callback )(Fl_Widget*, void*);
typedef Fl_Callback* Fl_Callback_p; // needed for BORLAND
typedef void (Fl_Callback0)(Fl_Widget*);
typedef void (Fl_Callback1)(Fl_Widget*, long);

/** This struct stores all information for a text or mixed graphics label.
 *
 *  \todo For FLTK1.3, the Fl_Label type eill become a widget by itself. That way
 *        we will be avoiding a lot of code duplication by handling labels in 
 *        a similar fashion to widgets containing text. We also provide an easy
 *        interface for very complex labels, containing html or vector graphics.
 */
struct FL_EXPORT Fl_Label {
  const char* value;
  Fl_Image* image;
  Fl_Image* deimage;
  uchar type;
  Fl_Font font;
  Fl_Font_Size size;
  unsigned color;
  void draw(int,int,int,int, Fl_Align) const ;
  void measure(int&, int&) const ;
};


/** Fl_Widget is the base class for all widgets in FLTK.  
 *
 *  You can't create one of these because the constructor is not public.  However 
 *  you can subclass it.  
 *
 *  All "property" accessing methods, such as color(), parent(), or argument() 
 *  are implemented as trivial inline functions and thus are as fast and small 
 *  as accessing fields in a structure. Unless otherwise noted, the property 
 *  setting methods such as color(n) or label(s) are also trivial inline functions, 
 *  even if they change the widget's appearance.  It is up to the user code to call 
 *  redraw() after these.
 */
class FL_EXPORT Fl_Widget {
  friend class Fl_Group;

  Fl_Group* parent_;
  Fl_Callback* callback_;
  void* user_data_;
  int x_,y_,w_,h_;
  Fl_Label label_;
  int flags_;
  unsigned color_;
  unsigned color2_;
  uchar type_;
  uchar damage_;
  uchar box_;
  uchar align_;
  uchar when_;

  const char *tooltip_;

  // unimplemented copy ctor and assignment operator
  Fl_Widget(const Fl_Widget &);
  Fl_Widget& operator=(const Fl_Widget &);

protected:

  /** Creates a widget at the given position and size. 
   *  The Fl_Widget is a protected constructor, but all derived widgets have a 
   *  matching public constructor. It takes a value for x(), y(), w(), h(), and 
   *  an optional value for label().
   *
   *  \param[in] x, y the position of the widget relative to the enclosing window
   *  \param[in] w, h size of the widget in pixels
   *  \param[in] label optional text for the widget label
   */
  Fl_Widget(int x, int y, int w, int h, Fl_CString label=0L);

  void x(int v) {x_ = v;}
  void y(int v) {y_ = v;}
  void w(int v) {w_ = v;}
  void h(int v) {h_ = v;}

  int flags() const {return flags_;}
  void set_flag(int c) {flags_ |= c;}
  void clear_flag(int c) {flags_ &= ~c;}
  enum {INACTIVE=1, INVISIBLE=2, OUTPUT=4, SHORTCUT_LABEL=64,
        CHANGED=128, VISIBLE_FOCUS=512, COPIED_LABEL = 1024};

  void draw_box() const;
  void draw_box(Fl_Boxtype, Fl_Color) const;
  void draw_box(Fl_Boxtype, int,int,int,int, Fl_Color) const;
  void draw_focus() {draw_focus(box(),x(),y(),w(),h());}
  void draw_focus(Fl_Boxtype, int,int,int,int) const;
  void draw_label() const;
  void draw_label(int, int, int, int) const;

public:

  /** Destroys the widget. 
   *  Destroying single widgets is not very common, and it is your responsibility 
   *  to either remove() them from any enclosing group or destroy that group 
   *  \em immediately after destroying the children. You almost always want to 
   *  destroy the parent group instead which will destroy all of the child widgets 
   *  and groups in that group.
   */
  virtual ~Fl_Widget();

  virtual void draw() = 0;

  /** Handles the specified event. 
   *  You normally don't call this method directly, but instead let FLTK do 
   *  it when the user interacts with the widget.
   * 
   *  When implemented in a new widget, this function must return 0 if the 
   *  widget does not use the event or 1 if it uses the event.
   *
   *  \param[in] event the kind of event received
   *  \retval 0 if the event was not used or understood
   *  \retval 1 if the event was used and can be deleted
   *  \see Fl_Event
   */
  virtual int handle(int event);

  Fl_Group* parent() const {return parent_;}
  void parent(Fl_Group* p) {parent_ = p;} // for hacks only, Fl_Group::add()

  uchar type() const {return type_;}
  void type(uchar t) {type_ = t;}

  int x() const {return x_;}
  int y() const {return y_;}
  int w() const {return w_;}
  int h() const {return h_;}
  virtual void resize(int,int,int,int);
  int damage_resize(int,int,int,int);
  void position(int X,int Y) {resize(X,Y,w_,h_);}
  void size(int W,int H) {resize(x_,y_,W,H);}

  /** Gets the label alignment.
   *  \return label alignment
   *  \see label(), align(uchar), Fl_Align
   *  \todo This function should not take ucahr as an argument. Apart from the fact that ucahr is too short 
   *        with only 8 bits, it does not provide type safety (in which case we don't need to declare Fl_Type
   *        an enum to begin with).
   */
  Fl_Align align() const {return (Fl_Align)align_;}

  /** Sets the label alignment.
   * This controls how the label is displayed next to or inside the widget. 
   * The default value is FL_ALIGN_CENTER, which centers the label inside the widget.
   * \param[in] alignment new label alignment
   * \see align(), Fl_Align
   */
  void align(uchar alignment) {align_ = alignment;}

  /** Gets the box type for the widget.
   *  \return the current box type
   *  \see box(Fl_Boxtype), Fl_Boxtype
   */
  Fl_Boxtype box() const {return (Fl_Boxtype)box_;}
  
  /** Sets the box type for the widget. 
   *  This identifies a routine that draws the background of the widget. 
   *  See Fl_Boxtype for the available types. The default depends on the widget, 
   *  but is usually FL_NO_BOX or FL_UP_BOX.
   *  \param[in] new_box the new box type
   *  \see box(), Fl_Boxtype
   */
  void box(Fl_Boxtype new_box) {box_ = new_box;}

  /** Gets the background color of the widget.
   *  \return current background color
   *  \see color(unsigned), color(unsigned, unsigned)
   */
  Fl_Color color() const {return (Fl_Color)color_;}

  /** Sets the background color of the widget. 
   *  The color is passed to the box routine. The color is either an index into an 
   *  internal table of RGB colors or an RGB color value generated using fl_rgb_color().
   *  The default for most widgets is FL_BACKGROUND_COLOR. Use Fl::set_color() to 
   *  redefine colors in the color map.
   *  \param[in] bg background color
   *  \see color(), color(unsigned, unsigned), selection_color(unsigned)
   */
  void color(unsigned bg) {color_ = bg;}

  Fl_Color selection_color() const {return (Fl_Color)color2_;}
  void selection_color(unsigned a) {color2_ = a;}

  /** Sets the background and selection color of the widget. 
   *  The two color form sets both the background and selection colors. 
   *  \param[in] bg background color
   *  \param[in] sel selection color
   *  \see color(unsigned), selection_color(unsigned)
   */
  void color(unsigned bg, unsigned sel) {color_=bg; color2_=sel;}

  /** Get the current label text.
   *  \return a pointer to the current label text
   *  \see label(Fl_CString), copy_label(Fl_CString)
   */
  const char* label() const {return label_.value;}

  /** Get or set the current label pointer. 
   *  The label is shown somewhere on or next to the widget. The passed pointer 
   *  is stored unchanged in the widget (the string is \em not copied), so if 
   *  you need to set the label to a formatted value, make sure the buffer is 
   *  \code static, global, or allocated. The copy_label() method can be used 
   *  to make a copy of the label string automatically.
   *  \param[in] text pointer to new label text
   *  \see copy_label()
   */
  void label(const char* text);

  /** Sets the current label. 
   *  Unlike label(), this method allocates a copy of the label 
   *  string instead of using the original string pointer.
   *  \param[in] new_label the new label text
   *  \see label()
   */
  void copy_label(Fl_CString new_label);

  void label(Fl_Labeltype a,const char* b) {label_.type = a; label_.value = b;}

  /** Gets the label type.
   *  \return the current label type.
   *  \see Fl_Labeltype
   */
  Fl_Labeltype labeltype() const {return (Fl_Labeltype)label_.type;}

  /** Sets the label type. 
   *  The label type identifies the function that draws the label of the widget. 
   *  This is generally used for special effects such as embossing or for using 
   *  the label() pointer as another form of data such as an icon. The value 
   *  FL_NORMAL_LABEL prints the label as plain text.
   *  \param a new label type
   *  \see Fl_Labeltype
   */
  void labeltype(Fl_Labeltype a) {label_.type = a;}

  /** Gets the label color. 
   *  The default color is FL_FOREGROUND_COLOR. 
   *  \return the current label color
   */
  Fl_Color labelcolor() const {return (Fl_Color)label_.color;}

  /** Sets the label color. 
   *  The default color is FL_FOREGROUND_COLOR. 
   *  \param[in] c the new label color
   */
  void labelcolor(unsigned c) {label_.color=c;}

  /** Gets the font to use. 
   *  Fonts are identified by indexes into a table. The default value uses a 
   *  Helvetica typeface (Arial for Microsoft&reg; Windows&reg;). The function 
   *  Fl::set_font() can define new typefaces.
   *  \return current font used by the label
   *  \see Fl_Font
   */
  Fl_Font labelfont() const {return label_.font;}

  /** Sets the font to use. 
   *  Fonts are identified by indexes into a table. The default value uses a 
   *  Helvetica typeface (Arial for Microsoft&reg; Windows&reg;). The function 
   *  Fl::set_font() can define new typefaces.
   *  \param[in] f the new font for the label
   *  \see Fl_Font
   */
  void labelfont(Fl_Font f) {label_.font=f;}

  /** Gets the font size in pixels. 
   *  The default size is 14 pixels.
   *  \return the current font size
   */
  Fl_Font_Size labelsize() const {return label_.size;}

  /** Gets or sets the font size in pixels. 
   *  The default size is 14 pixels.
   *  \param[in] pix the new font size
   */
  void labelsize(Fl_Font_Size pix) {label_.size=pix;}

  /** Gets or sets the image to use as part of the widget label.
   *  This image is used when drawing the widget in the active state.
   *  \return the current image
   */
  Fl_Image* image() {return label_.image;}

  /** Gets or sets the image to use as part of the widget label.
   *  This image is used when drawing the widget in the active state.
   *  \param[in] img the new image for the label 
   */
  void image(Fl_Image* img) {label_.image=img;}

  /** Gets or sets the image to use as part of the widget label.
   *  This image is used when drawing the widget in the active state.
   *  \param[in] img the new image for the label 
   */
  void image(Fl_Image& img) {label_.image=&img;}

  /** Gets the image to use as part of the widget label.  
   *  This image is used when drawing the widget in the inactive state.
   *  \return the current deactivated image for this widget
   */
  Fl_Image* deimage() {return label_.deimage;}

  /** Sets the image to use as part of the widget label.  
   *  This image is used when drawing the widget in the inactive state.
   *  \param[in] img the new image for the deactivated widget
   */
  void deimage(Fl_Image* img) {label_.deimage=img;}

  /** Sets the image to use as part of the widget label.  
   *  This image is used when drawing the widget in the inactive state.
   *  \param[in] img the new image for the deactivated widget
   */
  void deimage(Fl_Image& img) {label_.deimage=&img;}

  const char *tooltip() const {return tooltip_;}
  void tooltip(const char *t);

  /** Gets the current callback function for the widget.
   *  Each widget has a single callback.
   *  \return current callback
   */
  Fl_Callback_p callback() const {return callback_;}

  /** Sets the current callback function for the widget.
   *  Each widget has a single callback.
   *  \param[in] cb new callback
   *  \param[in] p user data
   */
  void callback(Fl_Callback* cb, void* p) {callback_=cb; user_data_=p;}

  /** Sets the current callback function for the widget.
   *  Each widget has a single callback.
   *  \param[in] cb new callback
   */
  void callback(Fl_Callback* cb) {callback_=cb;}

  /** Sets the current callback function for the widget.
   *  Each widget has a single callback.
   *  \param[in] cb new callback
   */
  void callback(Fl_Callback0*cb) {callback_=(Fl_Callback*)cb;}

  /** Sets the current callback function for the widget.
   *  Each widget has a single callback.
   *  \param[in] cb new callback
   *  \param[in] p user data
   */
  void callback(Fl_Callback1*cb, long p=0) {callback_=(Fl_Callback*)cb; user_data_=(void*)p;}

  void* user_data() const {return user_data_;}
  void user_data(void* v) {user_data_ = v;}

  /** Gets the current user data (long) argument that is passed to the callback function.
   */
  long argument() const {return (long)user_data_;}

  /** Sets the current user data (long) argument that is passed to the callback function.
   *  \todo The user data value must be implemented using a \em union to avoid 64 bit machine incompatibilities.
   */
  void argument(long v) {user_data_ = (void*)v;}

  Fl_When when() const {return (Fl_When)when_;}
  void when(uchar i) {when_ = i;}

  int visible() const {return !(flags_&INVISIBLE);}
  int visible_r() const;
  void show();
  void hide();
  void set_visible() {flags_ &= ~INVISIBLE;}

  /** Hides the widget. 
   *  You must still redraw the parent to see a change in the window. 
   *  Normally you want to use the hide() method instead.
   */
  void clear_visible() {flags_ |= INVISIBLE;}

  /** Returns whether the widget is active.
   *  \retval 0 if the widget is inactive
   *  \see active_r(), activate(), deactivate()
   */
  int active() const {return !(flags_&INACTIVE);}

  /** active_r() returns whether the widget and all of its 
   *  parents are active. 
   *  \retval 0 if this or any of the parent widgets are inactive
   *  \see active(), activate(), deactivate()
   */
  int active_r() const;

  /** Activate a widget.
   *  Changing this value will send FL_ACTIVATE to the widget if 
   *  active_r() is true.
   *  \see active(), active_r(), deactivate()
   */
  void activate();

  /** Deactivate a widget.
   *  Inactive widgets will be drawn "grayed out", e.g. with less contrast 
   *  than the active widget. Inactive widgets will not receive any keyboard 
   *  or mouse button events. Other events (including FL_ENTER, FL_MOVE, 
   *  FL_LEAVE, FL_SHORTCUT, and others) will still be sent. A widget is 
   *  only active if active() is true on it <I>and all of its parents</I>.  
   *
   *  Changing this value will send FL_DEACTIVATE to the widget if 
   *  active_r() is true.
   *
   *  Currently you cannot deactivate Fl_Window widgets.
   *
   *  \see activate(), active(), active_r()
   */
  void deactivate();

  int output() const {return (flags_&OUTPUT);}
  void set_output() {flags_ |= OUTPUT;}
  void clear_output() {flags_ &= ~OUTPUT;}
  int takesevents() const {return !(flags_&(INACTIVE|INVISIBLE|OUTPUT));}

  /** Check if the widget value changed since the last callback.
   *  "Changed" is a flag that is turned on when the user changes the value stored 
   *  in the widget. This is only used by subclasses of Fl_Widget that store values, 
   *  but is in the base class so it is easier to scan all the widgets in a panel 
   *  and do_callback() on the changed ones in response to an "OK" button.
   *
   *  Most widgets turn this flag off when they do the callback, and when the program 
   *  sets the stored value.
   *
   * \retval 0 if the value did not change
   * \see set_changed(), clear_changed()`
   */
  int changed() const {return flags_&CHANGED;}

  /** Mark the value of the widget as changed.
   *  \see changed(), clear_changed()
   */
  void set_changed() {flags_ |= CHANGED;}

  /** Mark the value of the widget as unchanged.
   *  \see changed(), set_changed()
   */
  void clear_changed() {flags_ &= ~CHANGED;}

  int take_focus();
  void set_visible_focus() { flags_ |= VISIBLE_FOCUS; }

  /** Disables keyboard focus navigation with this widget. 
   *  Normally, all widgets participate in keyboard focus navigation.
   */
  void clear_visible_focus() { flags_ &= ~VISIBLE_FOCUS; }
  void visible_focus(int v) { if (v) set_visible_focus(); else clear_visible_focus(); }
  int  visible_focus() { return flags_ & VISIBLE_FOCUS; }

  /** Sets the default callback for all widgets..
   *  Sets the default callback, which puts a pointer to the widget on the queue 
   *  returned by Fl::readqueue(). You may want to call this from your own callback.
   *  \param cb the new callback
   *  \param d user data associated with that callback
   *  \see callback(), do_callback(), Fl::readqueu()
   */
  static void default_callback(Fl_Widget *cb, void *d);

  /** Call the widget callback.
   *  Causes a widget to invoke its callback function, optionally
   *  with arbitrary arguments.
   *  \see callback()
   */
  void do_callback() {callback_(this,user_data_); if (callback_ != default_callback) clear_changed();}

  /** Call the widget callback.
   *  Causes a widget to invoke its callback function, optionally
   *  with arbitrary arguments.
   *  \param o call the callback with \em o as the widget argument
   *  \param arg call the callback with \em arg as the user data argument
   *  \see callback()
   */
  void do_callback(Fl_Widget* o,void* arg=0) {callback_(o,arg); if (callback_ != default_callback) clear_changed();}

  /** Call the widget callback.
   *  Causes a widget to invoke its callback function, optionally
   *  with arbitrary arguments.
   *  \param o call the callback with \em o as the widget argument
   *  \param arg call the callback with \em arg as the user data argument
   *  \see callback()
   */
  void do_callback(Fl_Widget* o,long arg) {callback_(o,(void*)arg); if (callback_ != default_callback) clear_changed();}

  int test_shortcut();
  static char label_shortcut(const char *t);
  static int test_shortcut(const char*);

  /** Checks if w is a child of this widget.
   *  \param[in] w potential child widget
   *  \return Returns 1 if \em w is a child of this widget, or is
   *  equal to this widget. Returns 0 if \en w is NULL.
   */
  int contains(const Fl_Widget *w) const ;

  /** Check if this widget is a child of w.
   *  Returns 1 if this widget is a child of \em w, or is
   *  equal to \em w. Returns 0 if \em w is NULL.
   *  \param[in] w the possible parent widget.
   *  \see contains()
   */
  int inside(const Fl_Widget* w) const {return w ? w->contains(this) : 0;}

  void redraw();
  void redraw_label();

  /** Returns non-zero if draw() needs to be called. 
   *  The damage value is actually a bit field that the widget 
   *  subclass can use to figure out what parts to draw.
   *  \return a bitmap of flags describing the kind of damage to the widget
   *  \see damage(uchar), clear_damage(uchar)
   */
  uchar damage() const {return damage_;}

  /** Clear damage flags.
   *  Damage flags are cleared when parts of the widget drawing is repaired.
   *  \param[in] c bitmask of flags to clear
   *  \see damage(uchar), damage()
   */
  void clear_damage(uchar c = 0) {damage_ = c;}

  /** Set the damage bits for the widget.
   *  Setting damage bits will schedule the widget for the next redraw.
   *  \param[in] c bitmask of flags to set
   *  \see damage(), clear_damage(uchar)
   */
  void damage(uchar c);

  /** Set the damage bits for an area inside the widget.
   *  Setting damage bits will schedule the widget for the next redraw.
   *  \param[in] c bitmask of flags to set
   *  \param x, y, w, h size of damaged area
   *  \see damage(), clear_damage(uchar)
   */
  void damage(uchar c, int x, int y, int w, int h);

  void draw_label(int, int, int, int, Fl_Align) const;
  void measure_label(int& xx, int& yy) {label_.measure(xx,yy);}

  Fl_Window* window() const ;

  // back compatability only:
  Fl_Color color2() const {return (Fl_Color)color2_;}
  void color2(unsigned a) {color2_ = a;}
};

// reserved type numbers (necessary for my cheapo RTTI) start here.
// grep the header files for "RESERVED_TYPE" to find the next available
// number.
#define FL_RESERVED_TYPE 100

#endif

//
// End of "$Id$".
//
