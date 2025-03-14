//
// Widget Tree Browser code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

#ifndef FLUID_WIDGETS_NODE_BROWSER_H
#define FLUID_WIDGETS_NODE_BROWSER_H

#include <FL/Fl_Browser_.H>

class Node;

namespace fld {
namespace widget {

class Node_Browser : public Fl_Browser_
{
  friend class Node;

  static void callback_stub(Fl_Widget *o, void *) {
    ((Node_Browser *)o)->callback();
  }

  Node* pushedtitle { nullptr };
  int saved_h_scroll_ { 0 };
  int saved_v_scroll_ { 0 };

  // required routines for Fl_Browser_ subclass:
  void *item_first() const override;
  void *item_next(void *) const override;
  void *item_prev(void *) const override;
  int item_selected(void *) const override;
  void item_select(void *,int) override;
  int item_width(void *) const override;
  int item_height(void *) const override;
  void item_draw(void *,int,int,int,int) const override;
  int incr_height() const override;

public:
  Node_Browser(int,int,int,int,const char * = nullptr);
  int handle(int) override;
  void callback();
  void save_scroll_position();
  void restore_scroll_position();
  void rebuild();
  void new_list() { Fl_Browser_::new_list(); }
  void display(Node *);
  void load_prefs();
  void save_prefs();

  static Fl_Color label_color;
  static Fl_Font label_font;
  static Fl_Color class_color;
  static Fl_Font class_font;
  static Fl_Color func_color;
  static Fl_Font func_font;
  static Fl_Color name_color;
  static Fl_Font name_font;
  static Fl_Color code_color;
  static Fl_Font code_font;
  static Fl_Color comment_color;
  static Fl_Font comment_font;
};

} // namespace widget
} // namespace fld

extern void redraw_browser();
extern Fl_Widget *make_widget_browser(int x,int y,int w,int h);
extern void redraw_widget_browser(Node *caller);
extern void select(Node *o, int v);
extern void select_only(Node *o);
extern void deselect();
extern void reveal_in_browser(Node *t);

extern fld::widget::Node_Browser *widget_browser;

#endif // FLUID_WIDGETS_NODE_BROWSER_H
