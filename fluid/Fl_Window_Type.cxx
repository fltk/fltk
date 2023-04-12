//
// Window type code file for the Fast Light Tool Kit (FLTK).
//
// The widget describing an Fl_Window.  This is also all the code
// for interacting with the overlay, which allows the user to
// select, move, and resize the children widgets.
//
// Copyright 1998-2023 by Bill Spitzak and others.
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
#include "Fd_Snap_Action.h"

#include <FL/Fl.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/fl_message.H>
#include <FL/fl_draw.H>
#include <FL/platform.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Shared_Image.H>
#include "../src/flstring.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

extern Fl_Preferences   fluid_prefs;

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

void i18n_type_cb(Fl_Choice *c, void *v) {
  if (v == LOAD) {
    c->value(g_project.i18n_type);
  } else {
    undo_checkpoint();
    g_project.i18n_type = c->value();
    set_modflag(1);
  }
  switch (g_project.i18n_type) {
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
      g_project.i18n_include = i18n_include_input->value();
      i18n_conditional_input->value("");
      g_project.i18n_conditional = i18n_conditional_input->value();
      i18n_function_input->value("gettext");
      g_project.i18n_function = i18n_function_input->value();
      i18n_static_function_input->value("gettext_noop");
      g_project.i18n_static_function = i18n_static_function_input->value();
      i18n_include_input->show();
      i18n_conditional_input->show();
      i18n_file_input->hide();
      i18n_set_input->hide();
      i18n_function_input->show();
      i18n_static_function_input->show();
      break;
  case 2 : /* POSIX cat */
      i18n_include_input->value("<nl_types.h>");
      g_project.i18n_include = i18n_include_input->value();
      i18n_conditional_input->value("");
      g_project.i18n_conditional = i18n_conditional_input->value();
      i18n_file_input->value("");
      g_project.i18n_file = i18n_file_input->value();
      i18n_set_input->value("1");
      g_project.i18n_set = i18n_set_input->value();
      i18n_include_input->show();
      i18n_conditional_input->show();
      i18n_file_input->show();
      i18n_set_input->show();
      i18n_function_input->hide();
      i18n_static_function_input->hide();
      break;
  }

}

void show_grid_cb(Fl_Widget *, void *) {
  settings_window->show();
  w_settings_tabs->value(w_settings_layout_tab);
}

void show_settings_cb(Fl_Widget *, void *) {
  settings_window->hotspot(settings_window);
  settings_window->show();
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
  void draw() FL_OVERRIDE;
  void draw_overlay() FL_OVERRIDE;
public:
  Fl_Window_Type *window;
  int handle(int) FL_OVERRIDE;
  Overlay_Window(int W,int H) : Fl_Overlay_Window(W,H) {Fl_Group::current(0);}
  void resize(int,int,int,int) FL_OVERRIDE;
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
 Make and add a new Window node.
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
    undo_checkpoint();
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
    undo_checkpoint();
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
    undo_checkpoint();
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
    undo_checkpoint();
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
  mydx = mx-x1;
  mydy = my-y1;

  if (!(drag & (FD_DRAG | FD_BOX | FD_LEFT | FD_RIGHT))) {
    mydx = 0;
    dx = 0;
  }

  if (!(drag & (FD_DRAG | FD_BOX | FD_TOP | FD_BOTTOM))) {
    mydy = 0;
    dy = 0;
  }

  if (show_guides && (drag & (FD_DRAG|FD_TOP|FD_LEFT|FD_BOTTOM|FD_RIGHT))) {
    Fl_Type *selection = 0L; // special power for the first selected widget
    for (Fl_Type *q=next; q && q->level>level; q = q->next) {
      if (q->selected && q->is_widget() && !q->is_menu_item()) {
        selection = q;
        break;
      }
    }
    Fd_Snap_Data data = { mydx, mydy, bx, by, br, bt, drag, 4, 4, mydx, mydy, (Fl_Widget_Type*)selection, this };
    Fd_Snap_Action::check_all(data);
    if (data.x_dist < 4) mydx = data.dx_out;
    if (data.y_dist < 4) mydy = data.dy_out;
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
  if (drag&FD_DRAG) {
    X += dx;
    Y += dy;
    R += dx;
    T += dy;
  } else {
    if (drag&FD_LEFT) {
      if (X==bx) {
        X += dx;
      } else {
        if (X<bx+dx) X = bx+dx;
      }
    }
    if (drag&FD_TOP) {
      if (Y==by) {
        Y += dy;
      } else {
        if (Y<by+dy) Y = by+dy;
      }
    }
    if (drag&FD_RIGHT) {
      if (R==br) {
        R += dx;
      } else {
        if (R>br+dx) R = br+dx;
      }
    }
    if (drag&FD_BOTTOM) {
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
  if (drag==FD_BOX && (x1 != mx || y1 != my)) {
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
  Fl_Type *selection = 0L; // special power for the first selected widget
  for (Fl_Type *q=next; q && q->level>level; q = q->next)
    if (q->selected && q->is_widget() && !q->is_menu_item()) {
      if (!selection) selection = q;
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

  if (show_guides && (drag & (FD_DRAG|FD_TOP|FD_LEFT|FD_BOTTOM|FD_RIGHT))) {
    Fd_Snap_Data data = { dx, dy, sx, sy, sr, st, drag, 4, 4, dx, dy, (Fl_Widget_Type*)selection, this};
    Fd_Snap_Action::draw_all(data);
  }
}

extern Fl_Menu_Item Main_Menu[];

// Calculate new bounding box of selected widgets:
void Fl_Window_Type::fix_overlay() {
  overlay_item->label("Hide O&verlays");
  if (overlay_button) overlay_button->label("Hide &Overlays");
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

  if (overlays_invisible) {
    overlay_item->label("Show O&verlays");
    if (overlay_button) overlay_button->label("Show &Overlays");
  } else {
    overlay_item->label("Hide O&verlays");
    if (overlay_button) overlay_button->label("Hide &Overlays");
  }

  for (Fl_Type *o=Fl_Type::first; o; o=o->next)
    if (o->is_window()) {
      Fl_Widget_Type* w = (Fl_Widget_Type*)o;
      ((Overlay_Window*)(w->o))->redraw_overlay();
    }
}

void toggle_guides(Fl_Widget *,void *) {
  show_guides = !show_guides;
  fluid_prefs.set("show_guides", show_guides);

  if (show_guides) {
    guides_item->label("Hide Guides");
    if (guides_button) guides_button->label("Hide &Guides");
  } else {
    guides_item->label("Show Guides");
    if (guides_button) guides_button->label("Show &Guides");
  }

  for (Fl_Type *o=Fl_Type::first; o; o=o->next) {
    if (o->is_window()) {
      Fl_Widget_Type* w = (Fl_Widget_Type*)o;
      ((Overlay_Window*)(w->o))->redraw_overlay();
    }
  }
}

void guides_cb(Fl_Button *o, void *v) {
  toggle_guides(NULL, NULL);
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
            f->fixed(myo->o, myo->o->w());
            f->layout();
          }
        } else {
          if (myo->o->h()!=oh) {
            f->fixed(myo->o, myo->o->h());
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
    // printf("DND enter\n");
  case FL_DND_DRAG:
    // printf("DND drag\n");
    {
      // find the innermost item clicked on:
      selection = this;
      for (Fl_Type* i=next; i && i->level>level; i=i->next)
        if (i->is_group()) {
          Fl_Widget_Type* myo = (Fl_Widget_Type*)i;
          if (Fl::event_inside(myo->o) && myo->o->visible_r()) {
            selection = myo;
            if (Fl::event_clicks()==1)
              reveal_in_browser(myo);
          }
        }
      if (selection && !selection->selected) {
        select_only(selection);
        ((Overlay_Window *)o)->redraw_overlay();
      }
    }
    Fl::belowmouse(o);
    return 1;
  case FL_DND_RELEASE:
    // printf("DND release\n");
    Fl::belowmouse(o);
    return 1;
  case FL_PASTE:
    // printf("DND paste\n");
    { Fl_Type *prototype = typename_to_prototype(Fl::event_text());
      if (prototype==NULL) {
        // it's not a FLUID type, so it could be the filename of an image
        const char *cfn = Fl::event_text();
        // printf("DND is filename %s?\n", cfn);
        if ((cfn == NULL) || (*cfn == 0)) return 0;
        if (strlen(cfn) >= FL_PATH_MAX) return 0;
        char fn[FL_PATH_MAX+1];
        // some platform prepend "file://" or "computer://" or similar text
        const char *sep = strstr(cfn, "://");
        if (sep)
          strcpy(fn, sep+3);
        else
          strcpy(fn, cfn);
        // remove possibly trailing \r\n
        int n = (int)strlen(fn)-1;
        if (fn[n] == '\n') fn[n--] = 0;
        if (fn[n] == '\r') fn[n--] = 0;
        // on X11 and Wayland (?), filenames need to be decoded
#if (defined(FLTK_USE_X11) || defined(FLTK_USE_WAYLAND))
        fl_decode_uri(fn);
#endif
        // does a file by that name actually exist?
        if (fl_access(fn, 4)==-1) return 0;
        // but is this an image file?
        Fl_Image *img = Fl_Shared_Image::get(fn);
        if (!img || (img->ld() < 0)) return 0;
        // ok, so it is an image - now add it as image() or deimage() to the widget
        // printf("DND check for target %s\n", fn);
        Fl_Widget_Type *tgt = NULL;
        for (Fl_Type* i=next; i && i->level>level; i=i->next) {
          if (i->is_widget()) {
            Fl_Widget_Type* myo = (Fl_Widget_Type*)i;
            if (Fl::event_inside(myo->o) && myo->o->visible_r())
              tgt = myo;
          }
        }
        if (tgt) {
          char rel[FL_PATH_MAX+1];
          enter_project_dir();
          fl_filename_relative(rel, FL_PATH_MAX, fn);
          leave_project_dir();
          // printf("DND image = %s\n", fn);
          if (Fl::get_key(FL_Alt_L) || Fl::get_key(FL_Alt_R)) {
          //if (Fl::event_alt()) { // TODO: X11/Wayland does not set the e_state on DND events
            tgt->inactive_name(rel);
            tgt->compress_deimage_ = 1;
            tgt->bind_deimage_ = 0;
          } else {
            tgt->image_name(rel);
            tgt->compress_image_ = 1;
            tgt->bind_image_ = 0;
          }
          select_only(tgt);
          tgt->open();
        }
        return 1;
      }

      in_this_only = this;
      popupx = Fl::event_x();
      popupy = Fl::event_y();
      // If the selected widget at dnd start and the drop target are the same,
      // or in the same group, add after selection. Otherwise, just add
      // at the end of the selected group.
      if (   Fl_Type::current_dnd->group()
          && selection && selection->group()
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
        mx<=br+2 && mx>=bx-2 && my<=bt+2 && my>=by-2) {
      if (mx >= br-5) drag |= FD_RIGHT;
      else if (mx <= bx+5) drag |= FD_LEFT;
      if (my >= bt-5) drag |= FD_BOTTOM;
      else if (my <= by+5) drag |= FD_TOP;
      if (!drag) drag = FD_DRAG;
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
      if (!drag) drag = FD_BOX; // if all else fails, start a new selection region
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
    if (drag != FD_BOX && (dx || dy || !Fl::event_is_click())) {
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
        if (myo && myo->o && myo->o->x()>=x1 && myo->o->y()>y1 &&
            myo->o->x()+myo->o->w()<mx && myo->o->y()+myo->o->h()<my) {
          n++;
          select(myo, toggle ? !myo->selected : 1);
        }
      CONTINUE:;
      }
      // if nothing in box, select what was clicked on:
      if (selection && !n) {
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
      drag = (Fl::event_state(FL_SHIFT)) ? (FD_RIGHT|FD_BOTTOM) : FD_DRAG;
      if (Fl::event_state(FL_COMMAND)) {
        int x_step, y_step;
        if (drag & (FD_RIGHT|FD_BOTTOM))
          Fd_Snap_Action::get_resize_stepsize(x_step, y_step);
        else
          Fd_Snap_Action::get_move_stepsize(x_step, y_step);
        dx *= x_step;
        dy *= y_step;
      }
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

void Fl_Window_Type::write_code1(Fd_Code_Writer& f) {
  Fl_Widget_Type::write_code1(f);
}

void Fl_Window_Type::write_code2(Fd_Code_Writer& f) {
  const char *var = is_class() ? "this" : name() ? name() : "o";
  write_extra_code(f);
  if (modal) f.write_c("%s%s->set_modal();\n", f.indent(), var);
  else if (non_modal) f.write_c("%s%s->set_non_modal();\n", f.indent(), var);
  if (!((Fl_Window*)o)->border()) {
    f.write_c("%s%s->clear_border();\n", f.indent(), var);
  }
  if (xclass) {
    f.write_c("%s%s->xclass(", f.indent(), var);
    f.write_cstring(xclass);
    f.write_c(");\n");
  }
  if (sr_max_w || sr_max_h) {
    f.write_c("%s%s->size_range(%d, %d, %d, %d);\n", f.indent(), var,
            sr_min_w, sr_min_h, sr_max_w, sr_max_h);
  } else if (sr_min_w || sr_min_h) {
    f.write_c("%s%s->size_range(%d, %d);\n", f.indent(), var, sr_min_w, sr_min_h);
  }
  f.write_c("%s%s->end();\n", f.indent(), var);
  if (((Fl_Window*)o)->resizable() == o)
    f.write_c("%s%s->resizable(%s);\n", f.indent(), var, var);
  write_block_close(f);
}

void Fl_Window_Type::write_properties(Fd_Project_Writer &f) {
  Fl_Widget_Type::write_properties(f);
  if (modal) f.write_string("modal");
  else if (non_modal) f.write_string("non_modal");
  if (!((Fl_Window*)o)->border()) f.write_string("noborder");
  if (xclass) {f.write_string("xclass"); f.write_word(xclass);}
  if (sr_min_w || sr_min_h || sr_max_w || sr_max_h)
    f.write_string("size_range {%d %d %d %d}", sr_min_w, sr_min_h, sr_max_w, sr_max_h);
  if (o->visible()) f.write_string("visible");
}

void Fl_Window_Type::read_property(Fd_Project_Reader &f, const char *c) {
  if (!strcmp(c,"modal")) {
    modal = 1;
  } else if (!strcmp(c,"non_modal")) {
    non_modal = 1;
  } else if (!strcmp(c, "visible")) {
    if (Fl::first_window()) open(); // only if we are using user interface
  } else if (!strcmp(c,"noborder")) {
    ((Fl_Window*)o)->border(0);
  } else if (!strcmp(c,"xclass")) {
    storestring(f.read_word(),xclass);
    ((Fl_Window*)o)->xclass(xclass);
  } else if (!strcmp(c,"size_range")) {
    int mw, mh, MW, MH;
    if (sscanf(f.read_word(),"%d %d %d %d",&mw,&mh,&MW,&MH) == 4) {
      sr_min_w = mw; sr_min_h = mh; sr_max_w = MW; sr_max_h = MH;
    }
  } else if (!strcmp(c,"xywh")) {
    Fl_Widget_Type::read_property(f, c);
    pasteoffset = 0; // make it not apply to contents
  } else {
    Fl_Widget_Type::read_property(f, c);
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

void Fl_Widget_Class_Type::write_properties(Fd_Project_Writer &f) {
  Fl_Window_Type::write_properties(f);
  if (wc_relative==1)
    f.write_string("position_relative");
  else if (wc_relative==2)
    f.write_string("position_relative_rescale");
}

void Fl_Widget_Class_Type::read_property(Fd_Project_Reader &f, const char *c) {
  if (!strcmp(c,"position_relative")) {
    wc_relative = 1;
  } else if (!strcmp(c,"position_relative_rescale")) {
      wc_relative = 2;
  } else {
    Fl_Window_Type::read_property(f, c);
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


void Fl_Widget_Class_Type::write_code1(Fd_Code_Writer& f) {
#if 0
  Fl_Widget_Type::write_code1(Fd_Code_Writer& f);
#endif // 0

  current_widget_class = this;
  write_public_state = 1;

  const char *c = subclass();
  if (!c) c = "Fl_Group";

  f.write_c("\n");
  write_comment_h(f);
  f.write_h("\nclass %s : public %s {\n", name(), c);
  if (strstr(c, "Window")) {
    f.write_h("%svoid _%s();\n", f.indent(1), trimclassname(name()));
    f.write_h("public:\n");
    f.write_h("%s%s(int X, int Y, int W, int H, const char *L = 0);\n", f.indent(1), trimclassname(name()));
    f.write_h("%s%s(int W, int H, const char *L = 0);\n", f.indent(1), trimclassname(name()));
    f.write_h("%s%s();\n", f.indent(1), trimclassname(name()));

    // a constructor with all four dimensions plus label
    f.write_c("%s::%s(int X, int Y, int W, int H, const char *L) :\n", name(), trimclassname(name()));
    f.write_c("%s%s(X, Y, W, H, L)\n{\n", f.indent(1), c);
    f.write_c("%s_%s();\n", f.indent(1), trimclassname(name()));
    f.write_c("}\n\n");

    // a constructor with just the size and label. The window manager will position the window
    f.write_c("%s::%s(int W, int H, const char *L) :\n", name(), trimclassname(name()));
    f.write_c("%s%s(0, 0, W, H, L)\n{\n", f.indent(1), c);
    f.write_c("%sclear_flag(16);\n", f.indent(1));
    f.write_c("%s_%s();\n", f.indent(1), trimclassname(name()));
    f.write_c("}\n\n");

    // a constructor that takes size and label from the Fluid database
    f.write_c("%s::%s() :\n", name(), trimclassname(name()));
    f.write_c("%s%s(0, 0, %d, %d, ", f.indent(1), c, o->w(), o->h());
    const char *cstr = label();
    if (cstr) f.write_cstring(cstr);
    else f.write_c("0");
    f.write_c(")\n{\n");
    f.write_c("%sclear_flag(16);\n", f.indent(1));
    f.write_c("%s_%s();\n", f.indent(1), trimclassname(name()));
    f.write_c("}\n\n");

    f.write_c("void %s::_%s() {\n", name(), trimclassname(name()));
//    f.write_c("%s%s *w = this;\n", f.indent(1), name());
  } else {
    f.write_h("public:\n");
    f.write_h("%s%s(int X, int Y, int W, int H, const char *L = 0);\n",
            f.indent(1), trimclassname(name()));
    f.write_c("%s::%s(int X, int Y, int W, int H, const char *L) :\n", name(), trimclassname(name()));
    if (wc_relative==1)
      f.write_c("%s%s(0, 0, W, H, L)\n{\n", f.indent(1), c);
    else if (wc_relative==2)
      f.write_c("%s%s(0, 0, %d, %d, L)\n{\n", f.indent(1), c, o->w(), o->h());
    else
      f.write_c("%s%s(X, Y, W, H, L)\n{\n", f.indent(1), c);
  }

//  f.write_c("%s%s *o = this;\n", f.indent(1), name());

  f.indentation++;
  write_widget_code(f);
}

void Fl_Widget_Class_Type::write_code2(Fd_Code_Writer& f) {
  write_extra_code(f);
  if (wc_relative==1)
    f.write_c("%sposition(X, Y);\n", f.indent());
  else if (wc_relative==2)
    f.write_c("%sresize(X, Y, W, H);\n", f.indent());
  if (modal) f.write_c("%sset_modal();\n", f.indent());
  else if (non_modal) f.write_c("%sset_non_modal();\n", f.indent());
  if (!((Fl_Window*)o)->border()) f.write_c("%sclear_border();\n", f.indent());
  if (xclass) {
    f.write_c("%sxclass(", f.indent());
    f.write_cstring(xclass);
    f.write_c(");\n");
  }
  f.write_c("%send();\n", f.indent());
  if (((Fl_Window*)o)->resizable() == o)
    f.write_c("%sresizable(this);\n", f.indent());
  f.indentation--;
  f.write_c("}\n");
}

////////////////////////////////////////////////////////////////
// live mode support

Fl_Widget *Fl_Window_Type::enter_live_mode(int) {
  Fl_Window *win = new Fl_Window(o->x(), o->y(), o->w(), o->h());
  return propagate_live_mode(win);
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
