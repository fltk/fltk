//
// "$Id: Fl_Widget.H,v 1.6.2.4 2001/01/22 15:13:38 easysw Exp $"
//
// Widget header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Widget_H
#define Fl_Widget_H

#include "Enumerations.H"

class Fl_Widget;
class Fl_Window;

typedef void (Fl_Callback )(Fl_Widget*, void*);
typedef Fl_Callback* Fl_Callback_p; // needed for BORLAND
typedef void (Fl_Callback0)(Fl_Widget*);
typedef void (Fl_Callback1)(Fl_Widget*, long);

struct Fl_Label {
  const char* value;
  uchar type;
  uchar font;
  uchar size;
  uchar color;
  FL_EXPORT void draw(int,int,int,int, Fl_Align) const ;
  FL_EXPORT void measure(int&, int&) const ;
};

class Fl_Widget {
  friend class Fl_Group;

  Fl_Widget* parent_;
  Fl_Callback* callback_;
  void* user_data_;
  short x_,y_,w_,h_;
  Fl_Label label_;
  uchar type_;
  uchar flags_;
  uchar damage_;
  uchar box_;
  uchar color_;
  uchar color2_;
  uchar align_;
  uchar when_;

  // "de-implement" the copy constructors:
  Fl_Widget & operator=(const Fl_Widget &);
  FL_EXPORT Fl_Widget(const Fl_Widget &);

protected:

  FL_EXPORT Fl_Widget(int,int,int,int,const char* =0);

  void x(int v) {x_ = v;}
  void y(int v) {y_ = v;}
  void w(int v) {w_ = v;}
  void h(int v) {h_ = v;}

  uchar flags() const {return flags_;}
  void set_flag(int c) {flags_ |= c;}
  void clear_flag(int c) {flags_ &= ~c;}
  enum {INACTIVE=1, INVISIBLE=2, OUTPUT=4, SHORTCUT_LABEL=64, CHANGED=128};

  FL_EXPORT void draw_box() const;
  FL_EXPORT void draw_box(Fl_Boxtype, Fl_Color) const;
  FL_EXPORT void draw_box(Fl_Boxtype, int,int,int,int, Fl_Color) const;
  FL_EXPORT void draw_label() const;
  FL_EXPORT void draw_label(int, int, int, int) const;

public:

  virtual FL_EXPORT ~Fl_Widget();

  virtual FL_EXPORT void draw() = 0;
  virtual FL_EXPORT int handle(int);
  FL_EXPORT Fl_Widget* parent() const {return parent_;}
  FL_EXPORT void parent(Fl_Widget* w) {parent_ = w;} // for hacks only

  FL_EXPORT uchar type() const {return type_;}
  FL_EXPORT void type(uchar t) {type_ = t;}

  FL_EXPORT int x() const {return x_;}
  FL_EXPORT int y() const {return y_;}
  FL_EXPORT int w() const {return w_;}
  FL_EXPORT int h() const {return h_;}
  virtual FL_EXPORT void resize(int,int,int,int);
  FL_EXPORT int damage_resize(int,int,int,int);
  FL_EXPORT void position(int X,int Y) {resize(X,Y,w_,h_);}
  FL_EXPORT void size(int W,int H) {resize(x_,y_,W,H);}

  FL_EXPORT Fl_Align align() const {return (Fl_Align)align_;}
  FL_EXPORT void align(uchar a) {align_ = a;}
  FL_EXPORT Fl_Boxtype box() const {return (Fl_Boxtype)box_;}
  FL_EXPORT void box(Fl_Boxtype a) {box_ = a;}
  FL_EXPORT Fl_Color color() const {return (Fl_Color)color_;}
  FL_EXPORT void color(uchar a) {color_ = a;}
  FL_EXPORT Fl_Color selection_color() const {return (Fl_Color)color2_;}
  FL_EXPORT void selection_color(uchar a) {color2_ = a;}
  FL_EXPORT void color(uchar a, uchar b) {color_=a; color2_=b;}
  FL_EXPORT const char* label() const {return label_.value;}
  FL_EXPORT void label(const char* a) {label_.value=a;}
  FL_EXPORT void label(Fl_Labeltype a,const char* b) {label_.type = a; label_.value = b;}
  FL_EXPORT Fl_Labeltype labeltype() const {return (Fl_Labeltype)label_.type;}
  FL_EXPORT void labeltype(Fl_Labeltype a) {label_.type = a;}
  FL_EXPORT Fl_Color labelcolor() const {return (Fl_Color)label_.color;}
  FL_EXPORT void labelcolor(uchar a) {label_.color=a;}
  FL_EXPORT Fl_Font labelfont() const {return (Fl_Font)label_.font;}
  FL_EXPORT void labelfont(uchar a) {label_.font=a;}
  FL_EXPORT uchar labelsize() const {return label_.size;}
  FL_EXPORT void labelsize(uchar a) {label_.size=a;}
  FL_EXPORT Fl_Callback_p callback() const {return callback_;}
  FL_EXPORT void callback(Fl_Callback* c, void* p) {callback_=c; user_data_=p;}
  FL_EXPORT void callback(Fl_Callback* c) {callback_=c;}
  FL_EXPORT void callback(Fl_Callback0*c) {callback_=(Fl_Callback*)c;}
  FL_EXPORT void callback(Fl_Callback1*c, long p=0) {callback_=(Fl_Callback*)c; user_data_=(void*)p;}
  FL_EXPORT void* user_data() const {return user_data_;}
  FL_EXPORT void user_data(void* v) {user_data_ = v;}
  FL_EXPORT long argument() const {return (long)user_data_;}
  FL_EXPORT void argument(long v) {user_data_ = (void*)v;}
  FL_EXPORT Fl_When when() const {return (Fl_When)when_;}
  FL_EXPORT void when(uchar i) {when_ = i;}

  FL_EXPORT int visible() const {return !(flags_&INVISIBLE);}
  FL_EXPORT int visible_r() const;
  FL_EXPORT void show();
  FL_EXPORT void hide();
  FL_EXPORT void set_visible() {flags_ &= ~INVISIBLE;}
  FL_EXPORT void clear_visible() {flags_ |= INVISIBLE;}
  FL_EXPORT int active() const {return !(flags_&INACTIVE);}
  FL_EXPORT int active_r() const;
  FL_EXPORT void activate();
  FL_EXPORT void deactivate();
  FL_EXPORT int output() const {return (flags_&OUTPUT);}
  FL_EXPORT void set_output() {flags_ |= OUTPUT;}
  FL_EXPORT void clear_output() {flags_ &= ~OUTPUT;}
  FL_EXPORT int takesevents() const {return !(flags_&(INACTIVE|INVISIBLE|OUTPUT));}
  FL_EXPORT int changed() const {return flags_&CHANGED;}
  FL_EXPORT void set_changed() {flags_ |= CHANGED;}
  FL_EXPORT void clear_changed() {flags_ &= ~CHANGED;}
  FL_EXPORT int take_focus();

  static FL_EXPORT void default_callback(Fl_Widget*, void*);
  FL_EXPORT void do_callback() {callback_(this,user_data_);}
  FL_EXPORT void do_callback(Fl_Widget* o,void* arg=0) {callback_(o,arg);}
  FL_EXPORT void do_callback(Fl_Widget* o,long arg) {callback_(o,(void*)arg);}
  FL_EXPORT int test_shortcut();
  static FL_EXPORT int test_shortcut(const char*);
  FL_EXPORT int contains(const Fl_Widget*) const ;
  FL_EXPORT int inside(const Fl_Widget* o) const {return o ? o->contains(this) : 0;}

  FL_EXPORT void redraw();
  FL_EXPORT uchar damage() const {return damage_;}
  FL_EXPORT void clear_damage(uchar c = 0) {damage_ = c;}
  FL_EXPORT void damage(uchar c);
  FL_EXPORT void damage(uchar c,int,int,int,int);
  FL_EXPORT void draw_label(int, int, int, int, Fl_Align) const;
  FL_EXPORT void measure_label(int& x, int& y) {label_.measure(x,y);}

  FL_EXPORT Fl_Window* window() const ;

  // back compatability only:
  FL_EXPORT Fl_Color color2() const {return (Fl_Color)color2_;}
  FL_EXPORT void color2(uchar a) {color2_ = a;}
};

// reserved type numbers (necessary for my cheapo RTTI) start here.
// grep the header files for "RESERVED_TYPE" to find the next available
// number.
#define FL_RESERVED_TYPE 100

#endif

//
// End of "$Id: Fl_Widget.H,v 1.6.2.4 2001/01/22 15:13:38 easysw Exp $".
//
