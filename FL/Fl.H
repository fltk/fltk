//
// "$Id: Fl.H,v 1.4 1999/01/07 19:16:50 mike Exp $"
//
// Main header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#ifndef Fl_H
#define Fl_H

#include "Enumerations.H"
#ifndef Fl_Object
#define Fl_Object Fl_Widget
#endif

class Fl_Widget;
class Fl_Window;
struct Fl_Label;
typedef void (Fl_Label_Draw_F)(const Fl_Label*, int,int,int,int, Fl_Align);
typedef void (Fl_Label_Measure_F)(const Fl_Label*, int&, int&);
typedef void (Fl_Box_Draw_F)(int,int,int,int, Fl_Color);

class Fl {
  Fl() {}; // no constructor!

public: // should be private!

  static int e_x,e_y,e_x_root,e_y_root;
  static int e_state;
  static int e_clicks;
  static int e_is_click;
  static int e_keysym;
  static char* e_text;
  static int e_length;
  static Fl_Widget* belowmouse_;
  static Fl_Widget* pushed_;
  static Fl_Widget* focus_;
  static int damage_;
  static Fl_Widget* selection_owner_;
  static Fl_Window* modal_;
  static Fl_Window* grab_;

  static void damage(int x) {damage_ = x;}

  static void (*idle)();

public:

  // argument parsers:
  static int arg(int, char**, int&);
  static int args(int, char**, int&, int (*)(int,char**,int&) = 0);
  static const char* const help;
  static void args(int, char**);

  // things called by initialization:
  static void display(const char*);
  static int visual(int);
  static int gl_visual(int, int *alist=0);
  static void own_colormap();
  static void get_system_colors();
  static void foreground(uchar, uchar, uchar);
  static void background(uchar, uchar, uchar);
  static void background2(uchar, uchar, uchar);

  // execution:
  static int wait();
  static double wait(double time);
  static int check();
  static int ready();
  static int run();
  static Fl_Widget* readqueue();
  static void add_timeout(double t,void (*cb)(void*),void* = 0);
  static void remove_timeout(void (*cb)(void*), void* = 0);
  static void add_fd(int fd, int when, void (*cb)(int, void*), void* = 0);
  static void add_fd(int fd, void (*cb)(int, void*), void* = 0);
  static void remove_fd(int);
  static void add_idle(void (*cb)(void*), void* = 0);
  static void remove_idle(void (*cb)(void*), void* = 0);
  static int damage() {return damage_;}
  static void redraw();
  static void flush();
  static void (*warning)(const char*, ...);
  static void (*error)(const char*, ...);
  static void (*fatal)(const char*, ...);
  static Fl_Window* first_window();
  static Fl_Window* next_window(const Fl_Window*);
  static Fl_Window* modal() {return modal_;}
  static Fl_Window* grab() {return grab_;}
  static void grab(Fl_Window&);
  static void release();

  // event information:
  static int event_x()		{return e_x;}
  static int event_y()		{return e_y;}
  static int event_x_root()	{return e_x_root;}
  static int event_y_root()	{return e_y_root;}
  static void get_mouse(int &,int &);
  static int event_clicks()	{return e_clicks;}
  static void event_clicks(int i) {e_clicks = i;}
  static int event_is_click()	{return e_is_click;}
  static void event_is_click(int i) {e_is_click = i;} // only 0 works!
  static int event_button()	{return e_keysym-FL_Button;}
  static int event_state()	{return e_state;}
  static int event_state(int i) {return e_state&i;}
  static int event_key()	{return e_keysym;}
  static int event_key(int);
  static int get_key(int);
  static const char* event_text() {return e_text;}
  static int event_length() {return e_length;}
  static int event_inside(int,int,int,int);
  static int event_inside(const Fl_Widget*);
  static int test_shortcut(int);

  // event destinations:
  static int handle(int, Fl_Window*);
  static Fl_Widget* belowmouse() {return belowmouse_;}
  static void belowmouse(Fl_Widget*);
  static Fl_Widget* pushed()	{return pushed_;}
  static void pushed(Fl_Widget*);
  static Fl_Widget* focus()	{return focus_;}
  static void focus(Fl_Widget*);
  static void add_handler(int (*h)(int));

  // cut/paste:
  static Fl_Widget* selection_owner() {return selection_owner_;}
  static void selection_owner(Fl_Widget*);
  static void selection(Fl_Widget &owner, const char* stuff, int len);
  static void paste(Fl_Widget &receiver);

  // screen size:
  static int x() {return 0;}
  static int y() {return 0;}
  static int w();
  static int h();

  // color map:
  static void	set_color(Fl_Color, uchar, uchar, uchar);
  static void	set_color(Fl_Color, unsigned);
  static unsigned get_color(Fl_Color);
  static void	get_color(Fl_Color, uchar&, uchar&, uchar&);
  static void	free_color(Fl_Color, int overlay = 0);

  // fonts:
  static const char* get_font(Fl_Font);
  static const char* get_font_name(Fl_Font, int* attributes = 0);
  static int get_font_sizes(Fl_Font, int*& sizep);
  static void set_font(Fl_Font, const char*);
  static void set_font(Fl_Font, Fl_Font);
  static Fl_Font set_fonts(const char* = 0);

  // labeltypes:
  static void set_labeltype(Fl_Labeltype,Fl_Label_Draw_F*,Fl_Label_Measure_F*);
  static void set_labeltype(Fl_Labeltype, Fl_Labeltype from);
  static void enable_symbols();

  // boxtypes:
  static void set_boxtype(Fl_Boxtype, Fl_Box_Draw_F*,uchar,uchar,uchar,uchar);
  static void set_boxtype(Fl_Boxtype, Fl_Boxtype from);
  static int box_dx(Fl_Boxtype);
  static int box_dy(Fl_Boxtype);
  static int box_dw(Fl_Boxtype);
  static int box_dh(Fl_Boxtype);

  // back compatability:
  static void set_abort(void (*f)(const char*,...)) {fatal = f;}
  static void (*atclose)(Fl_Window*,void*);
  static void default_atclose(Fl_Window*,void*);
  static void set_atclose(void (*f)(Fl_Window*,void*)) {atclose = f;}
  static int event_shift() {return e_state&FL_SHIFT;}
  static int event_ctrl() {return e_state&FL_CTRL;}
  static int event_alt() {return e_state&FL_ALT;}
  static int event_buttons() {return e_state&0x7f000000;}
  static int event_button1() {return e_state&FL_BUTTON1;}
  static int event_button2() {return e_state&FL_BUTTON2;}
  static int event_button3() {return e_state&FL_BUTTON3;}
  static void set_idle(void (*cb)()) {idle = cb;}

};

#endif

//
// End of "$Id: Fl.H,v 1.4 1999/01/07 19:16:50 mike Exp $".
//
