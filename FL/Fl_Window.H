//
// "$Id: Fl_Window.H,v 1.12.2.5 2001/01/22 15:13:38 easysw Exp $"
//
// Window header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Window_H
#define Fl_Window_H

#include "Fl_Group.H"

#define FL_WINDOW 0xF0	// all subclasses have type() >= this

class Fl_Window : public Fl_Group {

  friend class Fl_X; Fl_X *i; // points at the system-specific stuff

  const char* iconlabel_;
  const char* xclass_;
  const void* icon_;
  // size_range stuff:
  short minw, minh, maxw, maxh;
  uchar dw, dh, aspect, size_range_set;
  FL_EXPORT void size_range_();
  // values for flags():
  enum {
    FL_MODAL = 64,
    FL_NOBORDER = 8,
    FL_FORCE_POSITION = 16,
    FL_NON_MODAL = 32
  };
  FL_EXPORT void _Fl_Window(); // constructor innards

protected:

  static FL_EXPORT Fl_Window *current_;
  virtual FL_EXPORT void draw();
  virtual FL_EXPORT void flush();

public:

  FL_EXPORT Fl_Window(int,int,int,int, const char* = 0);
  FL_EXPORT Fl_Window(int,int, const char* = 0);
  virtual FL_EXPORT ~Fl_Window();

  virtual FL_EXPORT int handle(int);

  virtual FL_EXPORT void resize(int,int,int,int);
  FL_EXPORT void border(int b);
  void clear_border()	{set_flag(FL_NOBORDER);}
  int border() const	{return !(flags() & FL_NOBORDER);}
  void set_modal()	{set_flag(FL_MODAL);}
  uchar modal() const	{return flags() & FL_MODAL;}
  void set_non_modal()	{set_flag(FL_NON_MODAL);}
  uchar non_modal() const {return flags() & (FL_NON_MODAL|FL_MODAL);}

  FL_EXPORT void hotspot(int x, int y, int offscreen = 0);
  FL_EXPORT void hotspot(const Fl_Widget*, int offscreen = 0);
  void hotspot(const Fl_Widget& p, int offscreen = 0) {hotspot(&p,offscreen);}
  void free_position()	{clear_flag(FL_FORCE_POSITION);}
  void size_range(int a, int b, int c=0, int d=0, int e=0, int f=0, int g=0) {
    minw=a; minh=b; maxw=c; maxh=d; dw=e; dh=f; aspect=g; size_range_();}

  const char* label() const	{return Fl_Widget::label();}
  const char* iconlabel() const	{return iconlabel_;}
  FL_EXPORT void label(const char*);
  FL_EXPORT void iconlabel(const char*);
  FL_EXPORT void label(const char* label, const char* iconlabel);
  const char* xclass() const	{return xclass_;}
  void xclass(const char* c)	{xclass_ = c;}
  const void* icon() const	{return icon_;}
  void icon(const void * ic)	{icon_ = ic;}

  int shown() {return i != 0;}
  virtual FL_EXPORT void show();
  virtual FL_EXPORT void hide();
  FL_EXPORT void show(int, char**);
  FL_EXPORT void fullscreen();
  FL_EXPORT void fullscreen_off(int,int,int,int);
  FL_EXPORT void iconize();

  FL_EXPORT int x_root() const ;
  FL_EXPORT int y_root() const ;

  static Fl_Window *current() {return current_;}
  FL_EXPORT void make_current();

  // for back-compatability only:
  FL_EXPORT void cursor(Fl_Cursor, Fl_Color=FL_BLACK, Fl_Color=FL_WHITE);
  static FL_EXPORT void default_callback(Fl_Window*, void* v);

};

#endif

//
// End of "$Id: Fl_Window.H,v 1.12.2.5 2001/01/22 15:13:38 easysw Exp $".
//
