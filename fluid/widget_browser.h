//
// Widget Browser code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#ifndef _FLUID_WIDGET_BROWSER_H
#define _FLUID_WIDGET_BROWSER_H

#include <FL/Fl_Browser_.H>

class Fl_Type;
class Widget_Browser;

extern Widget_Browser *widget_browser;

extern void redraw_browser();
extern Fl_Widget *make_widget_browser(int x,int y,int w,int h);
extern void redraw_widget_browser(Fl_Type *caller);
extern void select(Fl_Type *o, int v);
extern void select_only(Fl_Type *o);
extern void deselect();
extern void reveal_in_browser(Fl_Type *t);

class Widget_Browser : public Fl_Browser_
{
  friend class Fl_Type;

  static void callback_stub(Fl_Widget *o, void *) {
    ((Widget_Browser *)o)->callback();
  }

  Fl_Type* pushedtitle;
  int saved_h_scroll_;
  int saved_v_scroll_;

  // required routines for Fl_Browser_ subclass:
  void *item_first() const FL_OVERRIDE;
  void *item_next(void *) const FL_OVERRIDE;
  void *item_prev(void *) const FL_OVERRIDE;
  int item_selected(void *) const FL_OVERRIDE;
  void item_select(void *,int) FL_OVERRIDE;
  int item_width(void *) const FL_OVERRIDE;
  int item_height(void *) const FL_OVERRIDE;
  void item_draw(void *,int,int,int,int) const FL_OVERRIDE;
  int incr_height() const FL_OVERRIDE;

public:
  Widget_Browser(int,int,int,int,const char * =NULL);
  int handle(int) FL_OVERRIDE;
  void callback();
  void save_scroll_position();
  void restore_scroll_position();
  void rebuild();
  void display(Fl_Type *);
};

#endif // _FLUID_WIDGET_BROWSER_H
