//
// Window type code for the Fast Light Tool Kit (FLTK).
//
// The widget describing an Fl_Window.  This is also all the code
// for interacting with the overlay, which allows the user to
// select, move, and resize the children widgets.
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

#include "Fl_Window_Type.h"

#include "Fl_Group_Type.h"
#include "fluid.h"
#include "widget_browser.h"
#include "undo.h"
#include "alignment_panel.h"
#include "file.h"
#include "code.h"
#include "widget_panel.h"
#include "factory.h"

#include <FL/Fl.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/fl_message.H>
#include <FL/fl_draw.H>
#include <FL/platform.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Round_Button.H>
#include "../src/flstring.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

extern Fl_Preferences   fluid_prefs;

inline int fl_min(int a, int b) { return (a < b ? a : b); }

// Update the XYWH values in the widget panel...
static void update_xywh() {
  if (current_widget && current_widget->is_widget()) {
    Fl_Widget *o = ((Fl_Widget_Type *)current_widget)->o;
    widget_x_input->value(o->x());
    widget_y_input->value(o->y());
    widget_w_input->value(o->w());
    widget_h_input->value(o->h());
    if (Fl_Flex_Type::parent_is_flex(current_widget)) {
      widget_flex_size->value(Fl_Flex_Type::size(current_widget));
      widget_flex_fixed->value(Fl_Flex_Type::is_fixed(current_widget));
    }
  }
}

void guides_cb(Fl_Check_Button *i, long) {
  show_guides = i->value();
  fluid_prefs.set("show_guides", show_guides);

  for (Fl_Type *p = Fl_Type::first; p; p = p->next) {
    if (p->is_window()) {
      Fl_Window_Type *w = (Fl_Window_Type *)p;
      ((Fl_Overlay_Window *)(w->o))->redraw_overlay();
    }
  }
}

void grid_cb(Fl_Int_Input *i, long v) {
  int n = atoi(i->value());
  if (n < 0) n = 0;
  switch (v) {
    case 1:
      gridx = n;
      fluid_prefs.set("gridx", n);
      break;
    case 2:
      gridy = n;
      fluid_prefs.set("gridy", n);
      break;
    case 3:
      snap = n;
      fluid_prefs.set("snap", n);
      break;
  }

  // Next go through all of the windows in the project and set the
  // stepping for resizes...
  Fl_Type *p;
  Fl_Window_Type *w;

  for (p = Fl_Type::first; p; p = p->next) {
    if (p->is_window()) {
      w = (Fl_Window_Type *)p;
      ((Fl_Window *)(w->o))->size_range(gridx, gridy, Fl::w(), Fl::h());
    }
  }
}

// Set default widget sizes...
void default_widget_size_cb(Fl_Round_Button *b, long size) {
  // Update the "normal" text size of new widgets...
  b->setonly();
  Fl_Widget_Type::default_size = size;
  fluid_prefs.set("widget_size", Fl_Widget_Type::default_size);
}


void i18n_type_cb(Fl_Choice *c, void *) {
  undo_checkpoint();

  switch (P.i18n_type = c->value()) {
  case 0 : /* None */
      i18n_include_input->hide();
      i18n_conditional_input->hide();
      i18n_file_input->hide();
      i18n_set_input->hide();
      i18n_function_input->hide();
      i18n_static_function_input->hide();
      break;
  case 1 : /* GNU gettext */
      i18n_include_input->value("<libintl.h>");
      P.i18n_include = i18n_include_input->value();
      i18n_conditional_input->value("");
      P.i18n_conditional = i18n_conditional_input->value();
      i18n_function_input->value("gettext");
      P.i18n_function = i18n_function_input->value();
      i18n_static_function_input->value("gettext_noop");
      P.i18n_static_function = i18n_static_function_input->value();
      i18n_include_input->show();
      i18n_conditional_input->show();
      i18n_file_input->hide();
      i18n_set_input->hide();
      i18n_function_input->show();
      i18n_static_function_input->show();
      break;
  case 2 : /* POSIX cat */
      i18n_include_input->value("<nl_types.h>");
      P.i18n_include = i18n_include_input->value();
      i18n_conditional_input->value("");
      P.i18n_conditional = i18n_conditional_input->value();
      i18n_file_input->value("");
      P.i18n_file = i18n_file_input->value();
      i18n_set_input->value("1");
      P.i18n_set = i18n_set_input->value();
      i18n_include_input->show();
      i18n_conditional_input->show();
      i18n_file_input->show();
      i18n_set_input->show();
      i18n_function_input->hide();
      i18n_static_function_input->hide();
      break;
  }

  set_modflag(1);
}

void i18n_text_cb(Fl_Input *i, void *) {
  undo_checkpoint();

  if (i == i18n_function_input)
    P.i18n_function = i->value();
  else if (i == i18n_static_function_input)
    P.i18n_static_function = i->value();
  else if (i == i18n_file_input)
    P.i18n_file = i->value();
  else if (i == i18n_include_input)
    P.i18n_include = i->value();
  else if (i == i18n_conditional_input)
    P.i18n_conditional = i->value();

  set_modflag(1);
}

void i18n_int_cb(Fl_Int_Input *i, void *) {
  undo_checkpoint();

  if (i == i18n_set_input)
    P.i18n_set = i->value();

  set_modflag(1);
}

void show_project_cb(Fl_Widget *, void *) {
  if(project_window==0) make_project_window();
  include_H_from_C_button->value(P.include_H_from_C);
  use_FL_COMMAND_button->value(P.use_FL_COMMAND);
  utf8_in_src_button->value(P.utf8_in_src);
  avoid_early_includes_button->value(P.avoid_early_includes);
  header_file_input->value(P.header_file_name);
  code_file_input->value(P.code_file_name);
  i18n_type_chooser->value(P.i18n_type);
  i18n_function_input->value(P.i18n_function);
  i18n_static_function_input->value(P.i18n_static_function);
  i18n_file_input->value(P.i18n_file);
  i18n_set_input->value(P.i18n_set);
  i18n_include_input->value(P.i18n_include);
  i18n_conditional_input->value(P.i18n_conditional);
  switch (P.i18n_type) {
  case 0 : /* None */
      i18n_include_input->hide();
      i18n_conditional_input->hide();
      i18n_file_input->hide();
      i18n_set_input->hide();
      i18n_function_input->hide();
      i18n_static_function_input->hide();
      break;
  case 1 : /* GNU gettext */
      i18n_include_input->show();
      i18n_conditional_input->show();
      i18n_file_input->hide();
      i18n_set_input->hide();
      i18n_function_input->show();
      i18n_static_function_input->show();
      break;
  case 2 : /* POSIX cat */
      i18n_include_input->show();
      i18n_conditional_input->show();
      i18n_file_input->show();
      i18n_set_input->show();
      i18n_function_input->hide();
      i18n_static_function_input->hide();
      break;
  }
  project_window->hotspot(project_window);
  project_window->show();
}

void show_grid_cb(Fl_Widget *, void *) {
  char buf[128];
  sprintf(buf,"%d",gridx); horizontal_input->value(buf);
  sprintf(buf,"%d",gridy); vertical_input->value(buf);
  sprintf(buf,"%d",snap); snap_input->value(buf);
  guides_toggle->value(show_guides);
  int s = Fl_Widget_Type::default_size;
  if (s<=8) def_widget_size[0]->setonly();
  else if (s<=11) def_widget_size[1]->setonly();
  else if (s<=14) def_widget_size[2]->setonly();
  else if (s<=18) def_widget_size[3]->setonly();
  else if (s<=24) def_widget_size[4]->setonly();
  else if (s<=32) def_widget_size[5]->setonly();
  grid_window->hotspot(grid_window);
  grid_window->show();
}

void show_settings_cb(Fl_Widget *, void *) {
  settings_window->hotspot(settings_window);
  settings_window->show();
}

void show_global_settings_cb(Fl_Widget *, void *) {
  global_settings_window->hotspot(global_settings_window);
  show_global_settings_window();
}

void header_input_cb(Fl_Input* i, void*) {
  if (strcmp(P.header_file_name, i->value()))
    set_modflag(1);
  P.header_file_name = i->value();
}
void code_input_cb(Fl_Input* i, void*) {
  if (strcmp(P.code_file_name, i->value()))
    set_modflag(1);
  P.code_file_name = i->value();
}

void include_H_from_C_button_cb(Fl_Check_Button* b, void*) {
  if (P.include_H_from_C != b->value()) {
    set_modflag(1);
    P.include_H_from_C = b->value();
  }
}

void use_FL_COMMAND_button_cb(Fl_Check_Button* b, void*) {
  if (P.use_FL_COMMAND != b->value()) {
    set_modflag(1);
    P.use_FL_COMMAND = b->value();
  }
}

void utf8_in_src_cb(Fl_Check_Button *b, void*) {
  if (P.utf8_in_src != b->value()) {
    set_modflag(1);
    P.utf8_in_src = b->value();
  }
}

void avoid_early_includes_cb(Fl_Check_Button *b, void*) {
  if (P.avoid_early_includes != b->value()) {
    set_modflag(1);
    P.avoid_early_includes = b->value();
  }
}

////////////////////////////////////////////////////////////////

Fl_Menu_Item window_type_menu[] = {
  {"Single",0,0,(void*)FL_WINDOW},
  {"Double",0,0,(void*)(FL_WINDOW+1)},
  {0}};

static int overlays_invisible;

// The following Fl_Widget is used to simulate the windows.  It has
// an overlay for the fluid ui, and special-cases the FL_NO_BOX.

class Overlay_Window : public Fl_Overlay_Window {
  void draw();
  void draw_overlay();
public:
  Fl_Window_Type *window;
  int handle(int);
  Overlay_Window(int W,int H) : Fl_Overlay_Window(W,H) {Fl_Group::current(0);}
  void resize(int,int,int,int);
  uchar *read_image(int &ww, int &hh);
};
void Overlay_Window::draw() {
  const int CHECKSIZE = 8;
  // see if box is clear or a frame or rounded:
  if ((damage()&FL_DAMAGE_ALL) &&
      (!box() || (box()>=4&&!(box()&2)) || box()>=_FL_ROUNDED_BOX)) {
    // if so, draw checkerboard so user can see what areas are clear:
    for (int Y = 0; Y < h(); Y += CHECKSIZE)
      for (int X = 0; X < w(); X += CHECKSIZE) {
        fl_color(((Y/(2*CHECKSIZE))&1) != ((X/(2*CHECKSIZE))&1) ?
                 FL_WHITE : FL_BLACK);
        fl_rectf(X,Y,CHECKSIZE,CHECKSIZE);
      }
  }
  Fl_Overlay_Window::draw();
}

extern Fl_Window *main_window;

// Read an image of the overlay window
uchar *Overlay_Window::read_image(int &ww, int &hh) {
  // Create an off-screen buffer for the window...
  //main_window->make_current();
  make_current();

  ww = w();
  hh = h();

  Fl_Offscreen offscreen = fl_create_offscreen(ww, hh);
  uchar *pixels;

  // Redraw the window into the offscreen buffer...
  fl_begin_offscreen(offscreen);

  if (!shown()) image(Fl::scheme_bg_);

  redraw();
  draw();

  // Read the screen image...
  pixels = fl_read_image(0, 0, 0, ww, hh);

  fl_end_offscreen();

  // Cleanup and return...
  fl_delete_offscreen(offscreen);
  main_window->make_current();
  return pixels;
}

void Overlay_Window::draw_overlay() {
  window->draw_overlay();
}
int Overlay_Window::handle(int e) {
  int ret =  window->handle(e);
  if (ret==0) {
    switch (e) {
      case FL_SHOW:
      case FL_HIDE:
        ret = Fl_Overlay_Window::handle(e);
    }
  }
  return ret;
}

/**
 Make and add a new WIndow node.
 \param[in] strategy is kAddAsLastChild or kAddAfterCurrent
 \return new node
 */
Fl_Type *Fl_Window_Type::make(Strategy strategy) {
  Fl_Type *p = Fl_Type::current;
  while (p && !p->is_code_block()) p = p->parent;
  if (!p) {
    fl_message("Please select a function");
    return 0;
  }
  Fl_Window_Type *myo = new Fl_Window_Type();
  if (!this->o) {// template widget
    this->o = new Fl_Window(100,100);
    Fl_Group::current(0);
  }
  // Set the size ranges for this window; in order to avoid opening the
  // X display we use an arbitrary maximum size...
  ((Fl_Window *)(this->o))->size_range(gridx, gridy, 6144, 4096);
  myo->factory = this;
  myo->drag = 0;
  myo->numselected = 0;
  Overlay_Window *w = new Overlay_Window(100,100);
  w->window = myo;
  myo->o = w;
  myo->add(p, strategy);
  myo->modal = 0;
  myo->non_modal = 0;
  return myo;
}

void Fl_Window_Type::add_child(Fl_Type* cc, Fl_Type* before) {
  if (!cc->is_widget()) return;
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  Fl_Widget* b = before ? ((Fl_Widget_Type*)before)->o : 0;
  ((Fl_Window*)o)->insert(*(c->o), b);
  o->redraw();
}

void Fl_Window_Type::remove_child(Fl_Type* cc) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  ((Fl_Window*)o)->remove(c->o);
  o->redraw();
}

void Fl_Window_Type::move_child(Fl_Type* cc, Fl_Type* before) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  ((Fl_Window*)o)->remove(c->o);
  Fl_Widget* b = before ? ((Fl_Widget_Type*)before)->o : 0;
  ((Fl_Window*)o)->insert(*(c->o), b);
  o->redraw();
}

////////////////////////////////////////////////////////////////

// Double-click on window widget shows the window, or if already shown,
// it shows the control panel.
void Fl_Window_Type::open() {
  Overlay_Window *w = (Overlay_Window *)o;
  if (w->shown()) {
    w->show();
    Fl_Widget_Type::open();
  } else {
    Fl_Widget *p = w->resizable();
    if (!p) w->resizable(w);
    w->show();
    w->resizable(p);
  }

  w->image(Fl::scheme_bg_);
  w->size_range(gridx, gridy, Fl::w(), Fl::h());
}

// Read an image of the window
uchar *Fl_Window_Type::read_image(int &ww, int &hh) {
  Overlay_Window *w = (Overlay_Window *)o;

  int hidden = !w->shown();
  w->show(); // make it the front window

  // Read the screen image...
  uchar *idata = w->read_image(ww, hh);
  if (hidden)
    w->hide();
  return idata;
}


// control panel items:

void modal_cb(Fl_Light_Button* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_window()) {i->hide(); return;}
    i->show();
    i->value(((Fl_Window_Type *)current_widget)->modal);
  } else {
    ((Fl_Window_Type *)current_widget)->modal = i->value();
    set_modflag(1);
  }
}

void non_modal_cb(Fl_Light_Button* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_window()) {i->hide(); return;}
    i->show();
    i->value(((Fl_Window_Type *)current_widget)->non_modal);
  } else {
    ((Fl_Window_Type *)current_widget)->non_modal = i->value();
    set_modflag(1);
  }
}

void border_cb(Fl_Light_Button* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_window()) {i->hide(); return;}
    i->show();
    i->value(((Fl_Window*)(current_widget->o))->border());
  } else {
    ((Fl_Window*)(current_widget->o))->border(i->value());
    set_modflag(1);
  }
}

void xclass_cb(Fl_Input* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_window()) {
      i->hide();
      i->parent()->hide(); // hides the "X Class:" label as well
      return;
    }
    i->show();
    i->parent()->show();
    i->value(((Fl_Widget_Type *)current_widget)->xclass);
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        mod = 1;
        Fl_Widget_Type* w = (Fl_Widget_Type*)o;
        if (w->is_window() || w->is_button())
          storestring(i->value(),w->xclass);
        if (w->is_window()) ((Fl_Window*)(w->o))->xclass(w->xclass);
        else if (w->is_menu_item()) w->redraw();
      }
    }
    if (mod) set_modflag(1);
  }
}

////////////////////////////////////////////////////////////////

void Fl_Window_Type::setlabel(const char *n) {
  if (o) ((Fl_Window *)o)->label(n);
}

// make() is called on this widget when user picks window off New menu:
Fl_Window_Type Fl_Window_type;

// Resize from window manager...
void Overlay_Window::resize(int X,int Y,int W,int H) {
  Fl_Widget* t = resizable(); resizable(0);

  // do not set the mod flag if the window was not resized. In FLUID, all
  // windows are opened without a given x/y position, so modifying x/y
  // should not mark the project as dirty
  if (W!=w() || H!=h())
    set_modflag(1);

  Fl_Overlay_Window::resize(X,Y,W,H);
  resizable(t);
  update_xywh();
}

// calculate actual move by moving mouse position (mx,my) to
// nearest multiple of gridsize, and snap to original position
void Fl_Window_Type::newdx() {
  int mydx, mydy;
  if (Fl::event_state(FL_ALT) || !snap) {
    mydx = mx-x1;
    mydy = my-y1;

    if (abs(mydx) < 2 && abs(mydy) < 2) mydx = mydy = 0;
  } else {
    int dx0 = mx-x1;
    int ix = (drag&RIGHT) ? br : bx;
    mydx = gridx ? ((ix+dx0+gridx/2)/gridx)*gridx - ix : dx0;
    if (dx0 > snap) {
      if (mydx < 0) mydx = 0;
    } else if (dx0 < -snap) {
      if (mydx > 0) mydx = 0;
    } else
      mydx = 0;
    int dy0 = my-y1;
    int iy = (drag&BOTTOM) ? by : bt;
    mydy = gridy ? ((iy+dy0+gridy/2)/gridy)*gridy - iy : dy0;
    if (dy0 > snap) {
      if (mydy < 0) mydy = 0;
    } else if (dy0 < -snap) {
      if (mydy > 0) mydy = 0;
    } else
      mydy = 0;
  }

  if (!(drag & (DRAG | BOX | LEFT | RIGHT))) {
    mydx = 0;
    dx = 0;
  }

  if (!(drag & (DRAG | BOX | TOP | BOTTOM))) {
    mydy = 0;
    dy = 0;
  }

  if (dx != mydx || dy != mydy) {
    dx = mydx; dy = mydy;
    ((Overlay_Window *)o)->redraw_overlay();
  }
}

// Move a widget according to dx and dy calculated above
void Fl_Window_Type::newposition(Fl_Widget_Type *myo,int &X,int &Y,int &R,int &T) {
  X = myo->o->x();
  Y = myo->o->y();
  R = X+myo->o->w();
  T = Y+myo->o->h();
  if (!drag) return;
  if (drag&DRAG) {
    X += dx;
    Y += dy;
    R += dx;
    T += dy;
  } else {
    if (drag&LEFT) {
      if (X==bx) {
        X += dx;
      } else {
        if (X<bx+dx) X = bx+dx;
      }
    }
    if (drag&TOP) {
      if (Y==by) {
        Y += dy;
      } else {
        if (Y<by+dy) Y = by+dy;
      }
    }
    if (drag&RIGHT) {
      if (R==br) {
        R += dx;
      } else {
        if (R>br+dx) R = br+dx;
      }
    }
    if (drag&BOTTOM) {
      if (T==bt) {
        T += dy;
      } else {
        if (T>bt+dx) T = bt+dx;
      }
    }
  }
  if (R<X) {int n = X; X = R; R = n;}
  if (T<Y) {int n = Y; Y = T; T = n;}
}

// draw a vertical arrow pointing toward y2
static void draw_v_arrow(int x, int y1, int y2) {
  int dy = (y1>y2) ? -1 : 1 ;
  fl_yxline(x, y1, y2);
  fl_xyline(x-4, y2, x+4);
  fl_line(x-2, y2-dy*5, x, y2-dy);
  fl_line(x+2, y2-dy*5, x, y2-dy);
}

static void draw_h_arrow(int x1, int y, int x2) {
  int dx = (x1>x2) ? -1 : 1 ;
  fl_xyline(x1, y, x2);
  fl_yxline(x2, y-4, y+4);
  fl_line(x2-dx*5, y-2, x2-dx, y);
  fl_line(x2-dx*5, y+2, x2-dx, y);
}

static void draw_top_brace(const Fl_Widget *w) {
  fl_yxline(w->x(), w->y()-2, w->y()+6);
  fl_yxline(w->x()+w->w()-1, w->y()-2, w->y()+6);
  fl_xyline(w->x()-2, w->y(), w->x()+w->w()+1);
}

static void draw_left_brace(const Fl_Widget *w) {
  fl_xyline(w->x()-2, w->y(), w->x()+6);
  fl_xyline(w->x()-2, w->y()+w->h()-1, w->x()+6);
  fl_yxline(w->x(), w->y()-2, w->y()+w->h()+1);
}

static void draw_right_brace(const Fl_Widget *w) {
  int xx = w->x() + w->w() - 1;
  fl_xyline(xx-6, w->y(), xx+2);
  fl_xyline(xx-6, w->y()+w->h()-1, xx+2);
  fl_yxline(xx, w->y()-2, w->y()+w->h()+1);
}

static void draw_bottom_brace(const Fl_Widget *w) {
  int yy = w->y() + w->h() - 1;
  fl_yxline(w->x(), yy-6, yy+2);
  fl_yxline(w->x()+w->w()-1, yy-6, yy+2);
  fl_xyline(w->x()-2, yy, w->x()+w->w()+1);
}

static void draw_height(int x, int y, int b, Fl_Align a) {
  char buf[16];
  int h = b - y;
  sprintf(buf, "%d", h);
  fl_font(FL_HELVETICA, 9);
  int lw = (int)fl_width(buf);
  int lx;

  b --;
  if (h < 30) {
    // Move height to the side...
    if (a == FL_ALIGN_LEFT) lx = x - lw - 2;
    else lx = x + 2;

    fl_yxline(x, y, b);
  } else {
    // Put height inside the arrows...
    lx = x - lw / 2;

    fl_yxline(x, y, y + (h - 11) / 2);
    fl_yxline(x, y + (h + 11) / 2, b);
  }

  // Draw the height...
  fl_draw(buf, lx, y + (h + 9) / 2);

  // Draw the arrowheads...
  fl_line(x-2, y+5, x, y+1, x+2, y+5);
  fl_line(x-2, b-5, x, b-1, x+2, b-5);

  // Draw the end lines...
  fl_xyline(x - 4, y, x + 4);
  fl_xyline(x - 4, b, x + 4);
}

static void draw_width(int x, int y, int r, Fl_Align a) {
  char buf[16];
  int w = r-x;
  sprintf(buf, "%d", w);
  fl_font(FL_HELVETICA, 9);
  int lw = (int)fl_width(buf);
  int ly = y + 4;

  r --;

  if (lw > (w - 20)) {
    // Move width above/below the arrows...
    if (a == FL_ALIGN_TOP) ly -= 10;
    else ly += 10;

    fl_xyline(x, y, r);
  } else {
    // Put width inside the arrows...
    fl_xyline(x, y, x + (w - lw - 2) / 2);
    fl_xyline(x + (w + lw + 2) / 2, y, r);
  }

  // Draw the width...
  fl_draw(buf, x + (w - lw) / 2, ly);

  // Draw the arrowheads...
  fl_line(x+5, y-2, x+1, y, x+5, y+2);
  fl_line(r-5, y-2, r-1, y, r-5, y+2);

  // Draw the end lines...
  fl_yxline(x, y - 4, y + 4);
  fl_yxline(r, y - 4, y + 4);
}

void Fl_Window_Type::draw_overlay() {
  if (recalc) {
    bx = o->w(); by = o->h(); br = 0; bt = 0;
    numselected = 0;
    for (Fl_Type *q=next; q && q->level>level; q=q->next)
      if (q->selected && q->is_widget() && !q->is_menu_item()) {
        numselected++;
        Fl_Widget_Type* myo = (Fl_Widget_Type*)q;
        if (myo->o->x() < bx) bx = myo->o->x();
        if (myo->o->y() < by) by = myo->o->y();
        if (myo->o->x()+myo->o->w() > br) br = myo->o->x()+myo->o->w();
        if (myo->o->y()+myo->o->h() > bt) bt = myo->o->y()+myo->o->h();
      }
    recalc = 0;
    sx = bx; sy = by; sr = br; st = bt;
  }
  fl_color(FL_RED);
  if (drag==BOX && (x1 != mx || y1 != my)) {
    int x = x1; int r = mx; if (x > r) {x = mx; r = x1;}
    int y = y1; int b = my; if (y > b) {y = my; b = y1;}
    fl_rect(x,y,r-x,b-y);
  }
  if (overlays_invisible && !drag) return;
  if (selected) fl_rect(0,0,o->w(),o->h());
  if (!numselected) return;
  int mybx,myby,mybr,mybt;
  int mysx,mysy,mysr,myst;
  mybx = mysx = o->w(); myby = mysy = o->h(); mybr = mysr = 0; mybt = myst = 0;
  Fl_Type *selection = 0L; // used to store the one selected widget (if n==1)
  for (Fl_Type *q=next; q && q->level>level; q = q->next)
    if (q->selected && q->is_widget() && !q->is_menu_item()) {
      selection = q;
      Fl_Widget_Type* myo = (Fl_Widget_Type*)q;
      int x,y,r,t;
      newposition(myo,x,y,r,t);
      if (!show_guides || !drag || numselected != 1) {
        if (Fl_Flex_Type::parent_is_flex(q) && !Fl_Flex_Type::is_fixed(q)) {
          if (((Fl_Flex*)((Fl_Flex_Type*)q->parent)->o)->horizontal()) {
            int yh = y + (t-y)/2;
            fl_begin_loop();
            fl_vertex(x+2, yh); fl_vertex(x+12, yh+5); fl_vertex(x+12, yh-5);
            fl_end_loop();
            fl_begin_loop();
            fl_vertex(r-3, yh); fl_vertex(r-13, yh+5); fl_vertex(r-13, yh-5);
            fl_end_loop();
          } else {
            int xh = x + (r-x)/2;
            fl_begin_loop();
            fl_vertex(xh, y+2); fl_vertex(xh+5, y+12); fl_vertex(xh-5, y+12);
            fl_end_loop();
            fl_begin_loop();
            fl_vertex(xh, t-3); fl_vertex(xh+5, t-13); fl_vertex(xh-5, t-13);
            fl_end_loop();
          }
        }
        fl_rect(x,y,r-x,t-y);
      }
      if (x < mysx) mysx = x;
      if (y < mysy) mysy = y;
      if (r > mysr) mysr = r;
      if (t > myst) myst = t;
      if (!(myo->o->align() & FL_ALIGN_INSIDE)) {
        // Adjust left/right/top/bottom for top/bottom labels...
        int ww, hh;
        ww = (myo->o->align() & FL_ALIGN_WRAP) ? myo->o->w() : 0;
        hh = myo->o->labelsize();
        myo->o->measure_label(ww, hh);
        if (myo->o->align() & FL_ALIGN_TOP) y -= hh;
        else if (myo->o->align() & FL_ALIGN_BOTTOM) t += hh;
        else if (myo->o->align() & FL_ALIGN_LEFT) x -= ww + 4;
        else if (myo->o->align() & FL_ALIGN_RIGHT) r += ww + 4;
      }
      if (x < mybx) mybx = x;
      if (y < myby) myby = y;
      if (r > mybr) mybr = r;
      if (t > mybt) mybt = t;
    }
  if (selected) return;

  if (show_guides && drag) {
    // draw overlays for UI Guideline distances
    // - check for distance to the window edge
    //    * FLTK suggests 10 pixels from the edge
    int d;
    int xsp, ysp;
    int mybx_bak = mybx, myby_bak = myby, mybr_bak = mybr, mybt_bak = mybt;
    Fl_Widget_Type *mysel = (Fl_Widget_Type *)selection;


    ideal_spacing(xsp, ysp);

    if (drag) {
      // Check top spacing...
      if (abs(d = myby - ysp) < 3) {
        dy -= d;
        if (drag & DRAG) mybt -= d;
        myby -= d;
        draw_v_arrow(mybx+5, myby, 0);
      }

      // Check bottom spacing...
      if (abs(d = o->h() - mybt - ysp) < 3) {
        dy += d;
        if (drag & DRAG) myby += d;
        mybt += d;
        draw_v_arrow(mybx+5, mybt, o->h());
      }

      // Check left spacing...
      if (abs(d = mybx - xsp) < 3) {
        dx -= d;
        if (drag & DRAG) mybr -= d;
        mybx -= d;
        draw_h_arrow(mybx, myby+5, 0);
      }

      // Check right spacing...
      if (abs(d = o->w() - mybr - xsp) < 3) {
        dx += d;
        if (drag & DRAG) mybx += d;
        mybr += d;
        draw_h_arrow(mybr, myby+5, o->w());
      }
    }

    if (numselected==1 && selection && !(drag & DRAG)) {
      // Check ideal sizes
      int x,y,r,t;
      newposition(mysel,x,y,r,t);
      int w = r-x;
      int h = t-y;
      int iw = w, ih = h;

      mysel->ideal_size(iw, ih);

      if (drag & (TOP | BOTTOM)) {
        // Check height
        if (abs(d = ih - h) < 5) {
          // Resize height
          if (drag & TOP) {
            myby -= d;
            y -= d;
            dy -= d;
          } else {
            mybt += d;
            t += d;
            dy += d;
          }
        }

        // Draw height guide
        draw_height(x < 50 ? x+10 : x-10, y, t,
                    x < 50 ? FL_ALIGN_RIGHT : FL_ALIGN_LEFT);
      }

      if (drag & (LEFT | RIGHT)) {
        // Check width
        if (abs(d = iw - w) < 5) {
          // Resize width
          if (drag & LEFT) {
            mybx -= d;
            x -= d;
            dx -= d;
          } else {
            mybr += d;
            r += d;
            dx += d;
          }
        }

        // Draw width guide
        draw_width(x, y < 50 ? y+10 : y-10, r,
                   y < 50 ? FL_ALIGN_BOTTOM : FL_ALIGN_TOP);
      }
    }

    // Check spacing and alignment between individual widgets
    if (drag && selection->is_widget()) {
      for (Fl_Type *q=next; q && q->level>level; q = q->next)
        if (q != selection && q->is_widget()) {
          Fl_Widget_Type *qw = (Fl_Widget_Type*)q;
          // Only check visible widgets...
          if (!qw->o->visible_r()) continue;

          // Get bounding box of widget...
          int qx = qw->o->x();
          int qr = qw->o->x() + qw->o->w();
          int qy = qw->o->y();
          int qt = qw->o->y() + qw->o->h();

          if (!(qw->o->align() & FL_ALIGN_INSIDE)) {
            // Adjust top/bottom for top/bottom labels...
            int ww, hh;
            ww = qw->o->w();
            hh = qw->o->labelsize();
            qw->o->measure_label(ww, hh);
            if (qw->o->align() & FL_ALIGN_TOP) qy -= hh;
            if (qw->o->align() & FL_ALIGN_BOTTOM) qt += hh;
          }

          // Do horizontal alignment when the widget is within 25
          // pixels vertically...
          if (fl_min(abs(qy - mysel->o->y() - mysel->o->h()),
                     abs(mysel->o->y() - qt)) < 25) {
            // Align to left of other widget...
            if ((drag & (LEFT | DRAG)) && abs(d = mybx - qx) < 3) {
              dx += d;
              mybx += d;
              if (drag & DRAG) mybr += d;

              draw_left_brace(qw->o);
            }

            // Align to right of other widget...
            if ((drag & (RIGHT | DRAG)) &&
                abs(d = qr - mybr) < 3) {
              dx += d;
              if (drag & DRAG) mybx += d;
              mybr += d;

              draw_right_brace(qw->o);
            }
          }

          // Align to top of other widget...
          if ((drag & (TOP | DRAG)) && abs(d = myby - qy) < 3) {
            dy += d;
            myby += d;
            if (drag & DRAG) mybt += d;

            draw_top_brace(qw->o);
          }

          // Align to bottom of other widget...
          if ((drag & (BOTTOM | DRAG)) && abs(d = qt - mybt) < 3) {
            dy += d;
            if (drag & DRAG) myby += d;
            mybt += d;

            draw_bottom_brace(qw->o);
          }

          // Check spacing between widgets
          if (mysel->is_group()) mysel->ideal_spacing(xsp, ysp);
          else qw->ideal_spacing(xsp, ysp);

          if ((qt)>=myby && qy<=mybt) {
            if (drag & (LEFT | DRAG)) {
              // Compare left of selected to left of current
              if (abs(d = qx - mybx - xsp) >= 3)
                d = qx - mybx + xsp;

              if (abs(d) < 3) {
                dx += d;
                mybx += d;
                if (drag & DRAG) mybr += d;

                // Draw left arrow
                draw_h_arrow(mybx, (myby+mybt)/2, qx);
              }

              // Compare left of selected to right of current
              if (abs(d = qr - mybx - xsp) >= 3)
                d = qr - mybx + xsp;

              if (abs(d) < 3) {
                dx += d;
                mybx += d;
                if (drag & DRAG) mybr += d;

                // Draw left arrow
                draw_h_arrow(mybx, (myby+mybt)/2, qr);
              }
            }

            if (drag & (RIGHT | DRAG)) {
              // Compare right of selected to left of current
              if (abs(d = qx - mybr - xsp) >= 3)
                d = qx - mybr + xsp;

              if (abs(d) < 3) {
                dx += d;
                if (drag & DRAG) mybx += d;
                mybr += d;

                // Draw right arrow
                draw_h_arrow(mybr, (myby+mybt)/2, qx);
              }

              // Compare right of selected to right of current
              if (abs(d = qr - mybr + xsp) >= 3)
                d = qr - mybr - xsp;

              if (abs(d) < 3) {
                dx += d;
                if (drag & DRAG) mybx += d;
                mybr += d;

                // Draw right arrow
                draw_h_arrow(mybr, (myby+mybt)/2, qr);
              }
            }
          }

          if (qr>=mybx && qx<=mybr) {
            // Compare top of selected to top of current
            if (drag & (TOP | DRAG)) {
              if (abs(d = qy - myby - ysp) >= 3)
                d = qy - myby + ysp;

              if (abs(d) < 3) {
                dy += d;
                myby += d;
                if (drag & DRAG) mybt += d;

                // Draw up arrow...
                draw_v_arrow((mybx+mybr)/2, myby, qy);
              }

              // Compare top of selected to bottom of current
              if (abs(d = qt - myby - ysp) >= 3)
                d = qt - myby + ysp;

              if (abs(d) < 3) {
                dy += d;
                myby += d;
                if (drag & DRAG) mybt += d;

                // Draw up arrow...
                draw_v_arrow((mybx+mybr)/2, myby, qt);
              }
            }

            // Compare bottom of selected to top of current
            if (drag & (BOTTOM | DRAG)) {
              if (abs(d = qy - mybt - ysp) >= 3)
                d = qy - mybt + ysp;

              if (abs(d) < 3) {
                dy += d;
                if (drag & DRAG) myby += d;
                mybt += d;

                // Draw down arrow...
                draw_v_arrow((mybx+mybr)/2, mybt, qy);
              }

              // Compare bottom of selected to bottom of current
              if (abs(d = qt - mybt - ysp) >= 3)
                d = qt - mybt + ysp;

              if (abs(d) < 3) {
                dy += d;
                if (drag & DRAG) myby += d;
                mybt += d;

                // Draw down arrow...
                draw_v_arrow((mybx+mybr)/2, mybt, qt);
              }
            }
          }
        }
    }
    mysx += mybx-mybx_bak; mysr += mybr-mybr_bak;
    mysy += myby-myby_bak; myst += mybt-mybt_bak;
  }
  // align the snapping selection box with the box we draw.
  sx = mysx; sy = mysy; sr = mysr; st = myst;

  // Draw selection box + resize handles...
  // draw box including all labels
  fl_line_style(FL_DOT);
  fl_rect(mybx,myby,mybr-mybx,mybt-myby);
  fl_line_style(FL_SOLID);
  // draw box excluding labels
  fl_rect(mysx,mysy,mysr-mysx,myst-mysy);
  fl_rectf(mysx,mysy,5,5);
  fl_rectf(mysr-5,mysy,5,5);
  fl_rectf(mysr-5,myst-5,5,5);
  fl_rectf(mysx,myst-5,5,5);
}

extern Fl_Menu_Item Main_Menu[];

// Calculate new bounding box of selected widgets:
void Fl_Window_Type::fix_overlay() {
  overlay_item->label("Hide O&verlays");
  overlays_invisible = 0;
  recalc = 1;
  ((Overlay_Window *)(this->o))->redraw_overlay();
}

// check if we must redraw any parent of tabs/wizard type
void check_redraw_corresponding_parent(Fl_Type *s) {
    Fl_Widget_Type * prev_parent = 0;
    if( !s || !s->selected || !s->is_widget()) return;
    for (Fl_Type *i=s; i && i->parent; i=i->parent) {
        if (i->is_group() && prev_parent &&
            (!strcmp(i->type_name(), "Fl_Tabs") ||
             !strcmp(i->type_name(), "Fl_Wizard"))) {
             ((Fl_Tabs*)((Fl_Widget_Type*)i)->o)->value(prev_parent->o);
             return;
        }
        if (i->is_group() && s->is_widget())
            prev_parent = (Fl_Widget_Type*)i;
    }
}

// do that for every window (when selected set changes):
void redraw_overlays() {
  for (Fl_Type *o=Fl_Type::first; o; o=o->next)
    if (o->is_window()) ((Fl_Window_Type*)o)->fix_overlay();
}

void toggle_overlays(Fl_Widget *,void *) {
  overlays_invisible = !overlays_invisible;

  if (overlays_invisible)
    overlay_item->label("Show O&verlays");
  else
    overlay_item->label("Hide O&verlays");

  for (Fl_Type *o=Fl_Type::first; o; o=o->next)
    if (o->is_window()) {
      Fl_Widget_Type* w = (Fl_Widget_Type*)o;
      ((Overlay_Window*)(w->o))->redraw_overlay();
    }
}

extern void select(Fl_Type *,int);
extern void select_only(Fl_Type *);
extern void deselect();
extern Fl_Type* in_this_only;
extern void fix_group_size(Fl_Type *t);

extern Fl_Menu_Item Main_Menu[];
extern Fl_Menu_Item New_Menu[];

// move the selected children according to current dx,dy,drag state:
void Fl_Window_Type::moveallchildren()
{
  undo_checkpoint();
  Fl_Type *i;
  for (i=next; i && i->level>level;) {
    if (i->selected && i->is_widget() && !i->is_menu_item()) {
      Fl_Widget_Type* myo = (Fl_Widget_Type*)i;
      int x,y,r,t,ow=myo->o->w(),oh=myo->o->h();
      newposition(myo,x,y,r,t);
      myo->o->resize(x,y,r-x,t-y);
      if (Fl_Flex_Type::parent_is_flex(myo)) {
        Fl_Flex_Type* ft = (Fl_Flex_Type*)myo->parent;
        Fl_Flex* f = (Fl_Flex*)ft->o;
        if (f->horizontal()) {
          if (myo->o->w()!=ow) {
            f->set_size(myo->o, myo->o->w());
            f->layout();
          }
        } else {
          if (myo->o->h()!=oh) {
            f->set_size(myo->o, myo->o->h());
            f->layout();
          }
        }
      }
      // move all the children, whether selected or not:
      Fl_Type* p;
      for (p = myo->next; p && p->level>myo->level; p = p->next)
        if (p->is_widget() && !p->is_menu_item() && !myo->is_flex()) {
          Fl_Widget_Type* myo2 = (Fl_Widget_Type*)p;
          int X,Y,R,T;
          newposition(myo2,X,Y,R,T);
          myo2->o->resize(X,Y,R-X,T-Y);
        }
      i = p;
    } else {
      i = i->next;
    }
  }
  for (i=next; i && i->level>level; i=i->next)
    fix_group_size(i);
  o->redraw();
  recalc = 1;
  ((Overlay_Window *)(this->o))->redraw_overlay();
  set_modflag(1);
  dx = dy = 0;

  update_xywh();
}

int Fl_Window_Type::popupx = 0x7FFFFFFF; // mark as invalid (MAXINT)
int Fl_Window_Type::popupy = 0x7FFFFFFF;

int Fl_Window_Type::handle(int event) {
  static Fl_Type* selection = NULL;
  switch (event) {
  case FL_DND_ENTER:
    Fl::belowmouse(o);
  case FL_DND_DRAG:
    {
      // find the innermost item clicked on:
      selection = this;
      for (Fl_Type* i=next; i && i->level>level; i=i->next)
        if (i->is_group()) {
          Fl_Widget_Type* myo = (Fl_Widget_Type*)i;
          for (Fl_Widget *o1 = myo->o; o1; o1 = o1->parent())
            if (!o1->visible()) goto CONTINUE_DND;
          if (Fl::event_inside(myo->o)) {
            selection = myo;
            if (Fl::event_clicks()==1)
              reveal_in_browser(myo);
          }
        }
    CONTINUE_DND:
      if (selection && !selection->selected) {
        select_only(selection);
        ((Overlay_Window *)o)->redraw_overlay();
      }
    }
  case FL_DND_RELEASE:
    return 1;
  case FL_PASTE:
    { Fl_Type *prototype = typename_to_prototype(Fl::event_text());
      if (prototype==NULL)
        return 0;

      in_this_only = this;
      popupx = Fl::event_x();
      popupy = Fl::event_y();
      // If the selected widget at dnd start and the drop target are the same,
      // or in the same group, add after selection. Otherwise, just add
      // at the end of the selected group.
      if (   Fl_Type::current_dnd->group()
          && selection->group()
          && Fl_Type::current_dnd->group()==selection->group())
      {
        Fl_Type *cc = Fl_Type::current;
        Fl_Type::current = Fl_Type::current_dnd;
        add_new_widget_from_user(prototype, kAddAfterCurrent);
        Fl_Type::current = cc;
      } else {
        add_new_widget_from_user(prototype, kAddAsLastChild);
      }
      popupx = 0x7FFFFFFF;
      popupy = 0x7FFFFFFF; // mark as invalid (MAXINT)
      in_this_only = NULL;
      widget_browser->display(Fl_Type::current);
      widget_browser->rebuild();
      return 1;
    }
  case FL_PUSH:
    x1 = mx = Fl::event_x();
    y1 = my = Fl::event_y();
    drag = dx = dy = 0;
    // test for popup menu:
    if (Fl::event_button() >= 3) {
      in_this_only = this; // modifies how some menu items work.
      static const Fl_Menu_Item* myprev;
      popupx = mx; popupy = my;
      const Fl_Menu_Item* m = New_Menu->popup(mx,my,"New",myprev);
      if (m && m->callback()) {myprev = m; m->do_callback(this->o);}
      popupx = 0x7FFFFFFF; popupy = 0x7FFFFFFF; // mark as invalid (MAXINT)
      in_this_only = 0;
      return 1;
    }
    // find the innermost item clicked on:
    selection = this;
    {for (Fl_Type* i=next; i && i->level>level; i=i->next)
      if (i->is_widget() && !i->is_menu_item()) {
      Fl_Widget_Type* myo = (Fl_Widget_Type*)i;
      for (Fl_Widget *o1 = myo->o; o1; o1 = o1->parent())
        if (!o1->visible()) goto CONTINUE2;
      if (Fl::event_inside(myo->o)) {
        selection = myo;
        if (Fl::event_clicks()==1)
          reveal_in_browser(myo);
      }
    CONTINUE2:;
    }}
    // see if user grabs edges of selected region:
    if (numselected && !(Fl::event_state(FL_SHIFT)) &&
        mx<=br+snap && mx>=bx-snap && my<=bt+snap && my>=by-snap) {
      int snap1 = snap>5 ? snap : 5;
      int w1 = (br-bx)/4; if (w1 > snap1) w1 = snap1;
      if (mx>=br-w1) drag |= RIGHT;
      else if (mx<bx+w1) drag |= LEFT;
      w1 = (bt-by)/4; if (w1 > snap1) w1 = snap1;
      if (my<=by+w1) drag |= TOP;
      else if (my>bt-w1) drag |= BOTTOM;
      if (!drag) drag = DRAG;
    }
    // do object-specific selection of other objects:
    {Fl_Type* t = selection->click_test(mx, my);
    if (t) {
      //if (t == selection) return 1; // indicates mouse eaten w/o change
      if (Fl::event_state(FL_SHIFT)) {
        Fl::event_is_click(0);
        select(t, !t->selected);
      } else {
        deselect();
        select(t, 1);
        if (t->is_menu_item()) t->open();
      }
      selection = t;
      drag = 0;
    } else {
      if (!drag) drag = BOX; // if all else fails, start a new selection region
    }}
    return 1;

  case FL_DRAG:
    if (!drag) return 0;
    mx = Fl::event_x();
    my = Fl::event_y();
    newdx();
    return 1;

  case FL_RELEASE:
    if (!drag) return 0;
    mx = Fl::event_x();
    my = Fl::event_y();
    if (drag != BOX && (dx || dy || !Fl::event_is_click())) {
      if (dx || dy) moveallchildren();
    } else if ((Fl::event_clicks() || Fl::event_state(FL_CTRL))) {
      Fl_Widget_Type::open();
    } else {
      if (mx<x1) {int t = x1; x1 = mx; mx = t;}
      if (my<y1) {int t = y1; y1 = my; my = t;}
      int n = 0;
      int toggle = Fl::event_state(FL_SHIFT);
      // clear selection on everything:
      if (!toggle) deselect(); else Fl::event_is_click(0);
      // select everything in box:
      for (Fl_Type*i=next; i&&i->level>level; i=i->next)
        if (i->is_widget() && !i->is_menu_item()) {
        Fl_Widget_Type* myo = (Fl_Widget_Type*)i;
        for (Fl_Widget *o1 = myo->o; o1; o1 = o1->parent())
          if (!o1->visible()) goto CONTINUE;
        if (Fl::event_inside(myo->o)) selection = myo;
        if (myo->o->x()>=x1 && myo->o->y()>y1 &&
            myo->o->x()+myo->o->w()<mx && myo->o->y()+myo->o->h()<my) {
          n++;
          select(myo, toggle ? !myo->selected : 1);
        }
      CONTINUE:;
      }
      // if nothing in box, select what was clicked on:
      if (!n) {
        select(selection, toggle ? !selection->selected : 1);
      }
    }
    drag = 0;
    ((Overlay_Window *)o)->redraw_overlay();
    return 1;

  case FL_KEYBOARD: {

    int backtab = 0;
    switch (Fl::event_key()) {

    case FL_Escape:
      ((Fl_Window*)o)->hide();
      return 1;

    case FL_Tab: {
      if (Fl::event_state(FL_SHIFT)) backtab = 1;
      // find current child:
      Fl_Type *i = Fl_Type::current;
      while (i && (!i->is_widget() || i->is_menu_item())) i = i->parent;
      if (!i) return 0;
      Fl_Type *p = i->parent;
      while (p && p != this) p = p->parent;
      if (!p || !p->is_widget()) {
        i = next; if (!i || i->level <= level) return 0;
      }
      p = i;
      for (;;) {
        i = backtab ? i->prev : i->next;
        if (!i || i->level <= level) {i = p; break;}
        if (i->is_widget() && !i->is_menu_item()) break;
      }
      deselect(); select(i,1);
      return 1;}

    case FL_Left:  dx = -1; dy = 0; goto ARROW;
    case FL_Right: dx = +1; dy = 0; goto ARROW;
    case FL_Up:    dx = 0; dy = -1; goto ARROW;
    case FL_Down:  dx = 0; dy = +1; goto ARROW;
    ARROW:
      drag = (Fl::event_state(FL_SHIFT)) ? (RIGHT|BOTTOM) : DRAG;
      if (Fl::event_state(FL_COMMAND)) {dx *= gridx; dy *= gridy;}
      moveallchildren();
      drag = 0;
      return 1;

    case 'o':
      toggle_overlays(0, 0);
      break;

    default:
      return 0;
    }}

  case FL_SHORTCUT: {
    in_this_only = this; // modifies how some menu items work.
    const Fl_Menu_Item* m = Main_Menu->test_shortcut();
    if (m && m->callback()) m->do_callback(this->o);
    in_this_only = 0;
    return (m != 0);}

  default:
    return 0;
  }
}

////////////////////////////////////////////////////////////////

void Fl_Window_Type::write_code1() {
  Fl_Widget_Type::write_code1();
}

void Fl_Window_Type::write_code2() {
  const char *var = is_class() ? "this" : name() ? name() : "o";
  write_extra_code();
  if (modal) write_c("%s%s->set_modal();\n", indent(), var);
  else if (non_modal) write_c("%s%s->set_non_modal();\n", indent(), var);
  if (!((Fl_Window*)o)->border()) {
    write_c("%s%s->clear_border();\n", indent(), var);
  }
  if (xclass) {
    write_c("%s%s->xclass(", indent(), var);
    write_cstring(xclass);
    write_c(");\n");
  }
  if (sr_max_w || sr_max_h) {
    write_c("%s%s->size_range(%d, %d, %d, %d);\n", indent(), var,
            sr_min_w, sr_min_h, sr_max_w, sr_max_h);
  } else if (sr_min_w || sr_min_h) {
    write_c("%s%s->size_range(%d, %d);\n", indent(), var, sr_min_w, sr_min_h);
  }
  write_c("%s%s->end();\n", indent(), var);
  if (((Fl_Window*)o)->resizable() == o)
    write_c("%s%s->resizable(%s);\n", indent(), var, var);
  write_block_close();
}

void Fl_Window_Type::write_properties() {
  Fl_Widget_Type::write_properties();
  if (modal) write_string("modal");
  else if (non_modal) write_string("non_modal");
  if (!((Fl_Window*)o)->border()) write_string("noborder");
  if (xclass) {write_string("xclass"); write_word(xclass);}
  if (sr_min_w || sr_min_h || sr_max_w || sr_max_h)
    write_string("size_range {%d %d %d %d}", sr_min_w, sr_min_h, sr_max_w, sr_max_h);
  if (o->visible()) write_string("visible");
}

void Fl_Window_Type::read_property(const char *c) {
  if (!strcmp(c,"modal")) {
    modal = 1;
  } else if (!strcmp(c,"non_modal")) {
    non_modal = 1;
  } else if (!strcmp(c, "visible")) {
    if (Fl::first_window()) open(); // only if we are using user interface
  } else if (!strcmp(c,"noborder")) {
    ((Fl_Window*)o)->border(0);
  } else if (!strcmp(c,"xclass")) {
    storestring(read_word(),xclass);
    ((Fl_Window*)o)->xclass(xclass);
  } else if (!strcmp(c,"size_range")) {
    int mw, mh, MW, MH;
    if (sscanf(read_word(),"%d %d %d %d",&mw,&mh,&MW,&MH) == 4) {
      sr_min_w = mw; sr_min_h = mh; sr_max_w = MW; sr_max_h = MH;
    }
  } else if (!strcmp(c,"xywh")) {
    Fl_Widget_Type::read_property(c);
    pasteoffset = 0; // make it not apply to contents
  } else {
    Fl_Widget_Type::read_property(c);
  }
}

int Fl_Window_Type::read_fdesign(const char* propname, const char* value) {
  int x;
  o->box(FL_NO_BOX); // because fdesign always puts an Fl_Box next
  if (!strcmp(propname,"Width")) {
    if (sscanf(value,"%d",&x) == 1) o->size(x,o->h());
  } else if (!strcmp(propname,"Height")) {
    if (sscanf(value,"%d",&x) == 1) o->size(o->w(),x);
  } else if (!strcmp(propname,"NumberofWidgets")) {
    return 1; // we can figure out count from file
  } else if (!strcmp(propname,"border")) {
    if (sscanf(value,"%d",&x) == 1) ((Fl_Window*)o)->border(x);
  } else if (!strcmp(propname,"title")) {
    label(value);
  } else {
    return Fl_Widget_Type::read_fdesign(propname,value);
  }
  return 1;
}

///////////////////////////////////////////////////////////////////////

Fl_Widget_Class_Type Fl_Widget_Class_type;
Fl_Widget_Class_Type *current_widget_class = 0;

/**
 Create and add a new Widget Class node.
 \param[in] strategy add after current or as last child
 \return new node
 */
Fl_Type *Fl_Widget_Class_Type::make(Strategy strategy) {
  Fl_Type *p = Fl_Type::current;
  while (p && (!p->is_decl_block() || (p->is_widget() && p->is_class()))) p = p->parent;
  Fl_Widget_Class_Type *myo = new Fl_Widget_Class_Type();
  myo->name("UserInterface");

  if (!this->o) {// template widget
    this->o = new Fl_Window(100,100);
    Fl_Group::current(0);
  }
  // Set the size ranges for this window; in order to avoid opening the
  // X display we use an arbitrary maximum size...
  ((Fl_Window *)(this->o))->size_range(gridx, gridy, 6144, 4096);
  myo->factory = this;
  myo->drag = 0;
  myo->numselected = 0;
  Overlay_Window *w = new Overlay_Window(100,100);
  w->window = myo;
  myo->o = w;
  myo->add(p, strategy);
  myo->modal = 0;
  myo->non_modal = 0;
  myo->wc_relative = 0;

  return myo;
}

void Fl_Widget_Class_Type::write_properties() {
  Fl_Window_Type::write_properties();
  if (wc_relative==1)
    write_string("position_relative");
  else if (wc_relative==2)
    write_string("position_relative_rescale");
}

void Fl_Widget_Class_Type::read_property(const char *c) {
  if (!strcmp(c,"position_relative")) {
    wc_relative = 1;
  } else if (!strcmp(c,"position_relative_rescale")) {
      wc_relative = 2;
  } else {
    Fl_Window_Type::read_property(c);
  }
}

// Convert A::B::C::D to D (i.e. keep only innermost name)
// This is useful for classes that contain a namespace component
static const char *trimclassname(const char *n) {
  if (!n)
    return NULL;
  const char *nn;
  while((nn = strstr(n, "::"))) {
    n = nn + 2;
  }
  return(n);
}


void Fl_Widget_Class_Type::write_code1() {
#if 0
  Fl_Widget_Type::write_code1();
#endif // 0

  current_widget_class = this;
  write_public_state = 1;

  const char *c = subclass();
  if (!c) c = "Fl_Group";

  write_c("\n");
  write_comment_h();
  write_h("\nclass %s : public %s {\n", name(), c);
  if (strstr(c, "Window")) {
    write_h("%svoid _%s();\n", indent(1), trimclassname(name()));
    write_h("public:\n");
    write_h("%s%s(int X, int Y, int W, int H, const char *L = 0);\n", indent(1), trimclassname(name()));
    write_h("%s%s(int W, int H, const char *L = 0);\n", indent(1), trimclassname(name()));
    write_h("%s%s();\n", indent(1), trimclassname(name()));

    // a constructor with all four dimensions plus label
    write_c("%s::%s(int X, int Y, int W, int H, const char *L) :\n", name(), trimclassname(name()));
    write_c("%s%s(X, Y, W, H, L)\n{\n", indent(1), c);
    write_c("%s_%s();\n", indent(1), trimclassname(name()));
    write_c("}\n\n");

    // a constructor with just the size and label. The window manager will position the window
    write_c("%s::%s(int W, int H, const char *L) :\n", name(), trimclassname(name()));
    write_c("%s%s(0, 0, W, H, L)\n{\n", indent(1), c);
    write_c("%sclear_flag(16);\n", indent(1));
    write_c("%s_%s();\n", indent(1), trimclassname(name()));
    write_c("}\n\n");

    // a constructor that takes size and label from the Fluid database
    write_c("%s::%s() :\n", name(), trimclassname(name()));
    write_c("%s%s(0, 0, %d, %d, ", indent(1), c, o->w(), o->h());
    const char *cstr = label();
    if (cstr) write_cstring(cstr);
    else write_c("0");
    write_c(")\n{\n");
    write_c("%sclear_flag(16);\n", indent(1));
    write_c("%s_%s();\n", indent(1), trimclassname(name()));
    write_c("}\n\n");

    write_c("void %s::_%s() {\n", name(), trimclassname(name()));
//    write_c("%s%s *w = this;\n", indent(1), name());
  } else {
    write_h("public:\n");
    write_h("%s%s(int X, int Y, int W, int H, const char *L = 0);\n",
            indent(1), trimclassname(name()));
    write_c("%s::%s(int X, int Y, int W, int H, const char *L) :\n", name(), trimclassname(name()));
    if (wc_relative==1)
      write_c("%s%s(0, 0, W, H, L)\n{\n", indent(1), c);
    else if (wc_relative==2)
      write_c("%s%s(0, 0, %d, %d, L)\n{\n", indent(1), c, o->w(), o->h());
    else
      write_c("%s%s(X, Y, W, H, L)\n{\n", indent(1), c);
  }

//  write_c("%s%s *o = this;\n", indent(1), name());

  indentation++;
  write_widget_code();
}

void Fl_Widget_Class_Type::write_code2() {
  write_extra_code();
  if (wc_relative==1)
    write_c("%sposition(X, Y);\n", indent());
  else if (wc_relative==2)
    write_c("%sresize(X, Y, W, H);\n", indent());
  if (modal) write_c("%sset_modal();\n", indent());
  else if (non_modal) write_c("%sset_non_modal();\n", indent());
  if (!((Fl_Window*)o)->border()) write_c("%sclear_border();\n", indent());
  if (xclass) {
    write_c("%sxclass(", indent());
    write_cstring(xclass);
    write_c(");\n");
  }
  write_c("%send();\n", indent());
  if (((Fl_Window*)o)->resizable() == o)
    write_c("%sresizable(this);\n", indent());
  indentation--;
  write_c("}\n");
}

////////////////////////////////////////////////////////////////
// live mode support

Fl_Widget *Fl_Window_Type::enter_live_mode(int) {
  Fl_Window *win = new Fl_Window(o->x(), o->y(), o->w(), o->h());
  live_widget = win;
  if (live_widget) {
    copy_properties();
    Fl_Type *n;
    for (n = next; n && n->level > level; n = n->next) {
      if (n->level == level+1)
        n->enter_live_mode();
    }
    win->end();
  }
  return live_widget;
}

void Fl_Window_Type::leave_live_mode() {
}

/**
 copy all properties from the edit widget to the live widget
 */
void Fl_Window_Type::copy_properties() {
  Fl_Widget_Type::copy_properties();
  /// \todo copy resizing constraints over
}
