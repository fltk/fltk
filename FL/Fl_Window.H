// Fl_Window.H

// fltk (Fast Light Tool Kit) version 0.99
// Copyright (C) 1998 Bill Spitzak

#ifndef Fl_Window_H
#define Fl_Window_H

#include "Fl_Group.H"

#define FL_WINDOW 0xF0	// all subclasses have type() >= this

class Fl_Window : public Fl_Group {

  friend class Fl_X; Fl_X *i; // points at the system-specific stuff

  const char* iconlabel_;
  const char* xclass_;
  // size_range stuff:
  short minw, minh, maxw, maxh;
  uchar dw, dh, aspect, size_range_set;
  void size_range_();
  // values for flags():
  enum {
    FL_MODAL = 64,
    FL_NOBORDER = 8,
    FL_FORCE_POSITION = 16,
    FL_NON_MODAL = 32
  };
  static Fl_Window *current_;
  void _Fl_Window(); // constructor innards

protected:

  virtual void draw();
  virtual void flush();

public:

  Fl_Window(int,int,int,int, const char* = 0);
  Fl_Window(int,int, const char* = 0);
  virtual ~Fl_Window();

  virtual int handle(int);

  virtual void resize(int,int,int,int);
  void border(int b);
  void clear_border()	{set_flag(FL_NOBORDER);}
  int border() const	{return !(flags() & FL_NOBORDER);}
  void set_modal()	{set_flag(FL_MODAL);}
  uchar modal() const	{return flags() & FL_MODAL;}
  void set_non_modal()	{set_flag(FL_NON_MODAL);}
  uchar non_modal() const {return flags() & (FL_NON_MODAL|FL_MODAL);}

  void hotspot(int x, int y, int offscreen = 0);
  void hotspot(const Fl_Widget*, int offscreen = 0);
  void hotspot(const Fl_Widget& p, int offscreen = 0) {hotspot(&p,offscreen);}
  void free_position()	{clear_flag(FL_FORCE_POSITION);}
  void size_range(int a, int b, int c=0, int d=0, int e=0, int f=0, int g=0) {
    minw=a; minh=b; maxw=c; maxh=d; dw=e; dh=f; aspect=g; size_range_();}

  const char* label() const	{return Fl_Widget::label();}
  const char* iconlabel() const	{return iconlabel_;}
  void label(const char*);
  void iconlabel(const char*);
  void label(const char* label, const char* iconlabel);
  const char* xclass() const	{return xclass_;}
  void xclass(const char* c)	{xclass_ = c;}

  int shown() {return i != 0;}
  virtual void show();
  virtual void hide();
  void show(int, char**);
  void fullscreen();
  void fullscreen_off(int,int,int,int);
  void iconize();
  void expose(uchar flags,int X,int Y,int W,int H);

  int x_root() const ;
  int y_root() const ;

  static Fl_Window *current() {return current_;}
  void make_current();

  // for back-compatability only:
  void cursor(Fl_Cursor, Fl_Color=FL_BLACK, Fl_Color=FL_WHITE);
  static void default_callback(Fl_Window*, void* v);

};

#endif
