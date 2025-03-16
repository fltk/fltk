//
// Window Node code file for the Fast Light Tool Kit (FLTK).
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

//
// The widget describing an Fl_Window.  This is also all the code
// for interacting with the overlay, which allows the user to
// select, move, and resize the children widgets.

#include "nodes/Window_Node.h"

#include "app/Snap_Action.h"
#include "Fluid.h"
#include "Project.h"
#include "proj/undo.h"
#include "io/Project_Reader.h"
#include "io/Project_Writer.h"
#include "io/Code_Writer.h"
#include "nodes/factory.h"
#include "nodes/Group_Node.h"
#include "nodes/Grid_Node.h"
#include "panels/settings_panel.h"
#include "panels/widget_panel.h"
#include "widgets/Node_Browser.h"

#include <FL/Fl.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/fl_message.H>
#include <FL/fl_draw.H>
#include <FL/platform.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Tooltip.H>
#include "../src/flstring.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#undef min
#undef max
#include <algorithm>

extern Fl_Window *the_panel;
extern void draw_width(int x, int y, int r, Fl_Align a);
extern void draw_height(int x, int y, int b, Fl_Align a);

// Update the XYWH values in the widget panel...
static void update_xywh() {
  if (current_widget && current_widget->is_widget()) {
    Fl_Widget *o = ((Widget_Node *)current_widget)->o;
    widget_x_input->value(o->x());
    widget_y_input->value(o->y());
    widget_w_input->value(o->w());
    widget_h_input->value(o->h());
    if (Flex_Node::parent_is_flex(current_widget)) {
      widget_flex_size->value(Flex_Node::size(current_widget));
      widget_flex_fixed->value(Flex_Node::is_fixed(current_widget));
    }
  }
}

void i18n_type_cb(Fl_Choice *c, void *v) {
  if (v == LOAD) {
    c->value(static_cast<int>(Fluid.proj.i18n_type));
  } else {
    Fluid.proj.undo.checkpoint();
    Fluid.proj.i18n_type = static_cast<fld::I18n_Type>(c->value());
    Fluid.proj.set_modflag(1);
  }
  switch (Fluid.proj.i18n_type) {
  case fld::I18n_Type::NONE : /* None */
      i18n_gnu_group->hide();
      i18n_posix_group->hide();
      break;
  case fld::I18n_Type::GNU : /* GNU gettext */
      i18n_gnu_group->show();
      i18n_posix_group->hide();
      break;
  case fld::I18n_Type::POSIX : /* POSIX cat */
      i18n_gnu_group->hide();
      i18n_posix_group->show();
      break;
  }
  // make sure that the outside labels are redrawn too.
  w_settings_i18n_tab->redraw();
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
  {"Single",0,nullptr,(void*)FL_WINDOW},
  {"Double",0,nullptr,(void*)(FL_DOUBLE_WINDOW)},
  {nullptr}};

static int overlays_invisible;

// The following Fl_Widget is used to simulate the windows.  It has
// an overlay for the fluid ui, and special-cases the FL_NO_BOX.

class Overlay_Window : public Fl_Overlay_Window {
  void draw() override;
  void draw_overlay() override;
  static void close_cb(Overlay_Window *self, void*);
public:
  Window_Node *window;
  int handle(int) override;
  Overlay_Window(int W,int H) : Fl_Overlay_Window(W,H) {
    Fl_Group::current(nullptr);
    callback((Fl_Callback*)close_cb);
  }
  void resize(int,int,int,int) override;
  uchar *read_image(int &ww, int &hh);
};

/**
 \brief User closes the window, so we mark the .fl file as changed.
 Mark the .fl file a changed, but don;t mark the source files as changed.
 \param self pointer to this window
 */
void Overlay_Window::close_cb(Overlay_Window *self, void*) {
  if (self->visible())
    Fluid.proj.set_modflag(1, -2);
  self->hide();
}

// Use this when drawing flat boxes while editing, so users can see the outline,
// even if the group and its parent have the same color.
static void fd_flat_box_ghosted(int x, int y, int w, int h, Fl_Color c) {
  fl_rectf(x, y, w, h, Fl::box_color(c));
  fl_rect(x, y, w, h, Fl::box_color(fl_color_average(FL_FOREGROUND_COLOR, c, .1f)));
}

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
  if (Fluid.show_ghosted_outline) {
    Fl_Box_Draw_F *old_flat_box = Fl::get_boxtype(FL_FLAT_BOX);
    Fl::set_boxtype(FL_FLAT_BOX, fd_flat_box_ghosted, 0, 0, 0, 0);
    Fl_Overlay_Window::draw();
    Fl::set_boxtype(FL_FLAT_BOX, old_flat_box, 0, 0, 0, 0);
  } else {
    Fl_Overlay_Window::draw();
  }
}

// Read an image of the overlay window
uchar *Overlay_Window::read_image(int &ww, int &hh) {
  // Create an off-screen buffer for the window...
  //Fluid.main_window->make_current();
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
  pixels = fl_read_image(nullptr, 0, 0, ww, hh);

  fl_end_offscreen();

  // Cleanup and return...
  fl_delete_offscreen(offscreen);
  Fluid.main_window->make_current();
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
 \param[in] strategy is Strategy::AS_LAST_CHILD or Strategy::AFTER_CURRENT
 \return new node
 */
Node *Window_Node::make(Strategy strategy) {
  Node *anchor = Fluid.proj.tree.current, *p = anchor;
  if (p && (strategy.placement() == Strategy::AFTER_CURRENT)) p = p->parent;
  while (p && (!p->is_code_block() || p->is_a(Type::Widget_Class))) {
    anchor = p;
    strategy.placement(Strategy::AFTER_CURRENT);
    p = p->parent;
  }
  if (!p) {
    fl_message("Please select a function");
    return nullptr;
  }
  Window_Node *myo = new Window_Node();
  if (!this->o) {// template widget
    this->o = new Fl_Window(100,100);
    Fl_Group::current(nullptr);
  }
  myo->factory = this;
  myo->drag = 0;
  myo->numselected = 0;
  Overlay_Window *w = new Overlay_Window(100, 100);
  w->size_range(10, 10);
  w->window = myo;
  myo->o = w;
  myo->add(anchor, strategy);
  myo->modal = 0;
  myo->non_modal = 0;
  return myo;
}

void Window_Node::add_child(Node* cc, Node* before) {
  if (!cc->is_widget()) return;
  Widget_Node* c = (Widget_Node*)cc;
  Fl_Widget* b = before ? ((Widget_Node*)before)->o : nullptr;
  ((Fl_Window*)o)->insert(*(c->o), b);
  o->redraw();
}

void Window_Node::remove_child(Node* cc) {
  Widget_Node* c = (Widget_Node*)cc;
  ((Fl_Window*)o)->remove(c->o);
  o->redraw();
}

void Window_Node::move_child(Node* cc, Node* before) {
  Widget_Node* c = (Widget_Node*)cc;
  ((Fl_Window*)o)->remove(c->o);
  Fl_Widget* b = before ? ((Widget_Node*)before)->o : nullptr;
  ((Fl_Window*)o)->insert(*(c->o), b);
  o->redraw();
}

////////////////////////////////////////////////////////////////

/**
 \brief Show the Window Type editor window without setting the modified flag.
 \see Window_Node::open()
 */
void Window_Node::open_() {
  Overlay_Window *w = (Overlay_Window *)o;
  if (w->shown()) {
    w->show();
    Widget_Node::open();
  } else {
    Fl_Widget *p = w->resizable();
    if (!p) w->resizable(w);
    w->show();
    w->resizable(p);
  }
  w->image(Fl::scheme_bg_);
}

/**
 \brief Show the Window Type editor window and set the modified flag if needed.
 Double-click on window widget shows the window, or if already shown, it shows
 the control panel.
 \see Window_Node::open_()
 */
void Window_Node::open() {
  Overlay_Window *w = (Overlay_Window *)o;
  if (!w->visible()) {
    Fluid.proj.set_modflag(1, -2);
  }
  open_();
}

// Read an image of the window
uchar *Window_Node::read_image(int &ww, int &hh) {
  Overlay_Window *w = (Overlay_Window *)o;

  int hidden = !w->shown();
  w->show(); // make it the front window

  // Read the screen image...
  uchar *idata = w->read_image(ww, hh);
  if (hidden)
    w->hide();
  return idata;
}

void Window_Node::ideal_size(int &w, int &h) {
  w = 480; h = 320;
  if (Fluid.main_window) {
    int sx, sy, sw, sh;
    Fl_Window *win = Fluid.main_window;
    int screen = Fl::screen_num(win->x(), win->y());
    Fl::screen_work_area(sx, sy, sw, sh, screen);
    w = std::min(w, sw*3/4); h = std::min(h, sh*3/4);
  }
  fld::app::Snap_Action::better_size(w, h);
}



////////////////////////////////////////////////////////////////

Window_Node Window_Node::prototype;

void Window_Node::setlabel(const char *n) {
  if (o) ((Fl_Window *)o)->label(n);
}

// Resize from window manager...
void Overlay_Window::resize(int X,int Y,int W,int H) {
  // Make sure we don't create undo checkpoints if the window does not actually change.
  // Some WMs seem to send spurious resize events.
  if (X!=x() || Y!=y() || W!=w() || H!=h()) {
    // Set a checkpoint on the first resize event, ignore further resizes until
    // a different type of checkpoint is triggered.
    if (Fluid.proj.undo.checkpoint(fld::proj::Undo::OnceType::WINDOW_RESIZE))
      Fluid.proj.set_modflag(1);
  }

  Fl_Widget* t = resizable();
  if (Fluid.proj.tree.allow_layout == 0) {
    resizable(nullptr);
  }

  // do not set the mod flag if the window was not resized. In FLUID, all
  // windows are opened without a given x/y position, so modifying x/y
  // should not mark the project as dirty
  if (W!=w() || H!=h())
    Fluid.proj.set_modflag(1);

  Fl_Overlay_Window::resize(X,Y,W,H);
  resizable(t);
  update_xywh();
}

// calculate actual move by moving mouse position (mx,my) to
// nearest multiple of gridsize, and snap to original position
void Window_Node::newdx() {
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

  if (Fluid.show_guides && (drag & (FD_DRAG|FD_TOP|FD_LEFT|FD_BOTTOM|FD_RIGHT))) {
    Node *selection = nullptr; // special power for the first selected widget
    for (Node *q=next; q && q->level>level; q = q->next) {
      if (q->selected && q->is_true_widget()) {
        selection = q;
        break;
      }
    }
    fld::app::Snap_Data data = { mydx, mydy, bx, by, br, bt, drag, 4, 4, mydx, mydy, (Widget_Node*)selection, this };
    fld::app::Snap_Action::check_all(data);
    if (data.x_dist < 4) mydx = data.dx_out;
    if (data.y_dist < 4) mydy = data.dy_out;
  }

  if (dx != mydx || dy != mydy) {
    dx = mydx; dy = mydy;
    ((Overlay_Window *)o)->redraw_overlay();
  }
}

// Move a widget according to dx and dy calculated above
void Window_Node::newposition(Widget_Node *myo,int &X,int &Y,int &R,int &T) {
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

void fd_hatch(int x, int y, int w, int h, int size=6, int offset=0, int pad=3) {
  x -= pad; y -= pad; w += 2*pad; h += 2*pad;
  int yp = (x+offset+y*size-1-y)%size;
  if (w > h) {
    for (; yp < h; yp+=size)
      fl_line(x, y+yp, x+yp, y);
    for (; yp < w; yp+=size)
      fl_line(x+yp-h, y+h, x+yp, y);
    for (; yp < w+h; yp+=size)
      fl_line(x+yp-h, y+h, x+w, y+yp-w);
  } else {
    for (; yp < w; yp+=size)
      fl_line(x, y+yp, x+yp, y);
    for (; yp < h; yp+=size)
      fl_line(x, y+yp, x+w, y+yp-w);
    for (; yp < h+w; yp+=size)
      fl_line(x+yp-h, y+h, x+w, y+yp-w);
  }
}

/**
 \brief Draw a hatch pattern over all children that overlap the bounds of this box.
 \param[in] group check all children of this group
 \param[in] x, y, w, h bounding box of this group
 */
void Window_Node::draw_out_of_bounds(Widget_Node *group, int x, int y, int w, int h) {
  for (Node *p = group->next; p && p->level>group->level; p = p->next) {
    if (p->level == group->level+1 && p->is_true_widget()) {
      Fl_Widget *o = ((Widget_Node*)p)->o;
      if (o->x() < x) fd_hatch(o->x(), o->y(), x-o->x(), o->h());
      if (o->y() < y) fd_hatch(o->x(), o->y(), o->w(), y-o->y());
      if (o->x()+o->w() > x+w) fd_hatch(x+w, o->y(), (o->x()+o->w())-(x+w), o->h());
      if (o->y()+o->h() > y+h) fd_hatch(o->x(), y+h, o->w(), (o->y()+o->h())-(y+h));
    }
  }
}

/**
 \brief Draw a hatch pattern for all groups that have out of bounds children.
 */
void Window_Node::draw_out_of_bounds() {
  // get every group in the hierarchy, then draw any overlap of a direct child with that group
  fl_color(FL_DARK_RED);
  draw_out_of_bounds(this, 0, 0, o->w(), o->h());
  for (Node *q=next; q && q->level>level; q = q->next) {
    // don't do this for Fl_Scroll (which we currently can't handle in FLUID anyway)
    if (q->is_a(Type::Group) && !q->is_a(Type::Scroll)) {
      Widget_Node *w = (Widget_Node*)q;
      draw_out_of_bounds(w, w->o->x(), w->o->y(), w->o->w(), w->o->h());
    }
  }
  fl_color(FL_RED);
}

/**
 \brief Compare all children in the same level and hatch overlapping areas.
 */
void Window_Node::draw_overlaps() {
  fl_color(FL_DARK_YELLOW);
  // loop through all widgets in this window
  for (Node *q=next; q && q->level>level; q = q->next) {
    // is it a valid widget
    if (q->is_true_widget()) {
      Widget_Node *w = (Widget_Node*)q;
      // is the widget visible
      if (w->o->visible()) {
        int x = w->o->x(), y = w->o->y();
        int r = x + w->o->w(), b = y + w->o->h();
        for (Node *p=q->next; p && p->level>=q->level; p = p->next) {
          if (p->level==q->level && p->is_true_widget()) {
            Widget_Node *wp = (Widget_Node*)p;
            if (wp->o->visible()) {
              int px = std::max(x, wp->o->x());
              int py = std::max(y, wp->o->y());
              int pr = std::min(r, wp->o->x() + wp->o->w());
              int pb = std::min(b, wp->o->y() + wp->o->h());
              if (pr > px && pb > py)
                fd_hatch(px, py, pr-px, pb-py);
            }
          }
        }
      } else {
        int l = q->level;
        for (; q && q->next && q->next->level>l; q = q->next) { }
      }
    }
  }
  fl_color(FL_RED);
}

void Window_Node::draw_overlay() {
  if (recalc) {
    bx = o->w(); by = o->h(); br = 0; bt = 0;
    numselected = 0;
    for (Node *q=next; q && q->level>level; q=q->next)
      if (q->selected && q->is_true_widget()) {
        numselected++;
        Widget_Node* myo = (Widget_Node*)q;
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

  if (Fluid.show_restricted) {
    draw_out_of_bounds();
    draw_overlaps();
    // TODO: for Fl_Tile, find all areas that are not covered by visible children
  }

  if (selected) fl_rect(0,0,o->w(),o->h());
  if (!numselected) return;
  int mybx,myby,mybr,mybt;
  int mysx,mysy,mysr,myst;
  mybx = mysx = o->w(); myby = mysy = o->h(); mybr = mysr = 0; mybt = myst = 0;
  Node *selection = nullptr; // special power for the first selected widget
  for (Node *q=next; q && q->level>level; q = q->next)
    if (q->selected && q->is_true_widget()) {
      if (!selection) selection = q;
      Widget_Node* myo = (Widget_Node*)q;
      int x,y,r,t;
      newposition(myo,x,y,r,t);
      if (Fluid.show_guides) {
        // If we are in a drag operation, and the parent is a grid, show the grid overlay
        if (drag && q->parent && q->parent->is_a(Type::Grid)) {
          Fl_Grid_Proxy *grid = ((Fl_Grid_Proxy*)((Grid_Node*)q->parent)->o);
          grid->draw_overlay();
        }
      }
      if (!Fluid.show_guides || !drag || numselected != 1) {
        if (Flex_Node::parent_is_flex(q) && Flex_Node::is_fixed(q)) {
          Fl_Flex *flex = ((Fl_Flex*)((Flex_Node*)q->parent)->o);
          Fl_Widget *wgt = myo->o;
          if (flex->horizontal()) {
            draw_width(wgt->x(), wgt->y()+15, wgt->x()+wgt->w(), FL_ALIGN_CENTER);
          } else {
            draw_height(wgt->x()+15, wgt->y(), wgt->y()+wgt->h(), FL_ALIGN_CENTER);
          }
        } else if (q->is_a(Type::Grid)) {
          Fl_Grid_Proxy *grid = ((Fl_Grid_Proxy*)((Grid_Node*)q)->o);
          grid->draw_overlay();
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
  fl_focus_rect(mybx,myby,mybr-mybx,mybt-myby); // issue #816
  // draw box excluding labels
  fl_rect(mysx,mysy,mysr-mysx,myst-mysy);
  fl_rectf(mysx,mysy,5,5);
  fl_rectf(mysr-5,mysy,5,5);
  fl_rectf(mysr-5,myst-5,5,5);
  fl_rectf(mysx,myst-5,5,5);

  if (Fluid.show_guides && (drag & (FD_DRAG|FD_TOP|FD_LEFT|FD_BOTTOM|FD_RIGHT))) {
    fld::app::Snap_Data data = { dx, dy, sx, sy, sr, st, drag, 4, 4, dx, dy, (Widget_Node*)selection, this};
    fld::app::Snap_Action::draw_all(data);
  }
}

// Calculate new bounding box of selected widgets:
void Window_Node::fix_overlay() {
  Fluid.overlay_item->label("Hide O&verlays");
  if (overlay_button) overlay_button->label("Hide &Overlays");
  overlays_invisible = 0;
  recalc = 1;
  ((Overlay_Window *)(this->o))->redraw_overlay();
}

// check if we must redraw any parent of tabs/wizard type
void check_redraw_corresponding_parent(Node *s) {
  Widget_Node * prev_parent = nullptr;
  if( !s || !s->selected || !s->is_widget()) return;
  for (Node *i=s; i && i->parent; i=i->parent) {
    if (i->is_a(Type::Group) && prev_parent) {
      if (i->is_a(Type::Tabs)) {
        ((Fl_Tabs*)((Widget_Node*)i)->o)->value(prev_parent->o);
        return;
      }
      if (i->is_a(Type::Wizard)) {
        ((Fl_Wizard*)((Widget_Node*)i)->o)->value(prev_parent->o);
        return;
      }
    }
    if (i->is_a(Type::Group) && s->is_widget())
      prev_parent = (Widget_Node*)i;
  }
}

// do that for every window (when selected set changes):
void redraw_overlays() {
  for (Node *o=Fluid.proj.tree.first; o; o=o->next)
    if (o->is_a(Type::Window)) ((Window_Node*)o)->fix_overlay();
}

void toggle_overlays(Fl_Widget *,void *) {
  overlays_invisible = !overlays_invisible;

  if (overlays_invisible) {
    Fluid.overlay_item->label("Show O&verlays");
    if (overlay_button) overlay_button->label("Show &Overlays");
  } else {
    Fluid.overlay_item->label("Hide O&verlays");
    if (overlay_button) overlay_button->label("Hide &Overlays");
  }

  for (Node *o=Fluid.proj.tree.first; o; o=o->next)
    if (o->is_a(Type::Window)) {
      Widget_Node* w = (Widget_Node*)o;
      ((Overlay_Window*)(w->o))->redraw_overlay();
    }
}

/**
 \brief User changes settings to show positioning guides in layout editor overlay.
 This is called from the main menu and from the check button in the Settings
 dialog.
 */
void toggle_guides(Fl_Widget *,void *) {
  Fluid.show_guides = !Fluid.show_guides;
  Fluid.preferences.set("Fluid.show_guides", Fluid.show_guides);

  if (Fluid.show_guides)
    Fluid.guides_item->label("Hide Guides");
  else
    Fluid.guides_item->label("Show Guides");
  if (guides_button)
    guides_button->value(Fluid.show_guides);

  for (Node *o=Fluid.proj.tree.first; o; o=o->next) {
    if (o->is_a(Type::Window)) {
      Widget_Node* w = (Widget_Node*)o;
      ((Overlay_Window*)(w->o))->redraw_overlay();
    }
  }
}

/**
 \brief User changes settings to show positioning guides in layout editor overlay.
 This is called from the check button in the Settings dialog.
 */
void toggle_guides_cb(Fl_Check_Button *o, void *v) {
  toggle_guides(nullptr, nullptr);
}

/**
 \brief User changes settings to show overlapping and out of bounds widgets.
 This is called from the main menu and from the check button in the Settings
 dialog.
 */
void toggle_restricted(Fl_Widget *,void *) {
  Fluid.show_restricted = !Fluid.show_restricted;
  Fluid.preferences.set("Fluid.show_restricted", Fluid.show_restricted);

  if (Fluid.show_restricted)
    Fluid.restricted_item->label("Hide Restricted");
  else
    Fluid.restricted_item->label("Show Restricted");
  if (restricted_button)
    restricted_button->value(Fluid.show_restricted);

  for (Node *o=Fluid.proj.tree.first; o; o=o->next) {
    if (o->is_a(Type::Window)) {
      Widget_Node* w = (Widget_Node*)o;
      ((Overlay_Window*)(w->o))->redraw_overlay();
    }
  }
}

/**
 \brief User changes settings to show low contrast groups with a ghosted outline.
 */
void toggle_ghosted_outline_cb(Fl_Check_Button *,void *) {
  Fluid.show_ghosted_outline = !Fluid.show_ghosted_outline;
  Fluid.preferences.set("Fluid.show_ghosted_outline", Fluid.show_ghosted_outline);
  for (Node *o=Fluid.proj.tree.first; o; o=o->next) {
    if (o->is_a(Type::Window)) {
      Widget_Node* w = (Widget_Node*)o;
      ((Overlay_Window*)(w->o))->redraw();
    }
  }
}

/**
 \brief User changes settings to show overlapping and out of bounds widgets.
 This is called from the check button in the Settings dialog.
 */
void toggle_restricted_cb(Fl_Check_Button *o, void *v) {
  toggle_restricted(nullptr, nullptr);
}

extern void select(Node *,int);
extern void select_only(Node *);
extern void deselect();
extern Node* in_this_only;
extern void fix_group_size(Node *t);

extern Fl_Menu_Item New_Menu[];

/**
 Move the selected children according to current dx, dy, drag state.

 This is somewhat of a do-all function that received many additions when new
 widget types were added. In the default case, moving a group will simply move
 all descendants with it. When resizing, children are resized to fit within
 the group.

 This is not ideal for widgets that are moved or resized within a group that
 manages the layout of its children. We must create a more universal way to
 modify move events per widget type.

 \param[in] key if key is not 0, it contains the code of the keypress that
      caused this call. This must only be set when handle FL_KEYBOARD events.
 */
void Window_Node::moveallchildren(int key)
{
  bool update_widget_panel = false;
  Fluid.proj.undo.checkpoint();
  Node *i;
  for (i=next; i && i->level>level;) {
    if (i->selected && i->is_true_widget()) {
      Widget_Node* myo = (Widget_Node*)i;
      int x,y,r,t,ow=myo->o->w(),oh=myo->o->h();
      newposition(myo,x,y,r,t);
      if (myo->is_a(Type::Flex) || myo->is_a(Type::Grid)) {
        // Flex and Grid need to be able to layout their children.
        Fluid.proj.tree.allow_layout++;
        myo->o->resize(x,y,r-x,t-y);
        Fluid.proj.tree.allow_layout--;
      } else {
        // Other groups are resized without affecting their children, however
        // they move their children if the entire widget is moved.
        myo->o->resize(x,y,r-x,t-y);
      }
      if (Flex_Node::parent_is_flex(myo)) {
        // If the border of a Flex child is move, give that child a fixed size
        // so that the user request is reflected.
        Flex_Node* ft = (Flex_Node*)myo->parent;
        Fl_Flex* f = (Fl_Flex*)ft->o;
        if (key) {
          ft->keyboard_move_child(myo, key);
        } else if (drag & FD_DRAG) {
          ft->insert_child_at(myo->o, Fl::event_x(), Fl::event_y());
        } else {
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
        // relayout the Flex parent
        Fluid.proj.tree.allow_layout++;
        f->layout();
        Fluid.proj.tree.allow_layout--;
      } else if (myo->parent && myo->parent->is_a(Type::Grid)) {
        Grid_Node* gt = (Grid_Node*)myo->parent;
        Fl_Grid* g = (Fl_Grid*)gt->o;
        if (key) {
          gt->keyboard_move_child(myo, key);
        } else {
          if (drag & FD_DRAG) {
            gt->insert_child_at(myo->o, Fl::event_x(), Fl::event_y());
          } else {
            gt->child_resized(myo);
          }
        }
        Fluid.proj.tree.allow_layout++;
        g->layout();
        Fluid.proj.tree.allow_layout--;
        update_widget_panel = true;
      } else if (myo->parent && myo->parent->is_a(Type::Group)) {
        Group_Node* gt = (Group_Node*)myo->parent;
        ((Fl_Group*)gt->o)->init_sizes();
      }
      // move all the children, whether selected or not:
      Node* p;
      for (p = myo->next; p && p->level>myo->level; p = p->next)
        if (p->is_true_widget() && !myo->is_a(Type::Flex) && !myo->is_a(Type::Grid)) {
          Widget_Node* myo2 = (Widget_Node*)p;
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
  Fluid.proj.set_modflag(1);
  dx = dy = 0;

  update_xywh();
  if (update_widget_panel && the_panel && the_panel->visible()) {
    propagate_load(the_panel, LOAD);
  }
}

int Window_Node::popupx = 0x7FFFFFFF; // mark as invalid (MAXINT)
int Window_Node::popupy = 0x7FFFFFFF;

int Window_Node::handle(int event) {
  static Node* selection = nullptr;
  switch (event) {
  case FL_DND_ENTER:
    // printf("DND enter\n");
  case FL_DND_DRAG:
    // printf("DND drag\n");
    {
      // find the innermost item clicked on:
      selection = this;
      for (Node* i=next; i && i->level>level; i=i->next)
        if (i->is_a(Type::Group)) {
          Widget_Node* myo = (Widget_Node*)i;
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
    { Node *prototype = typename_to_prototype(Fl::event_text());
      if (prototype==nullptr) {
        // it's not a FLUID type, so it could be the filename of an image
        const char *cfn = Fl::event_text();
        // printf("DND is filename %s?\n", cfn);
        if ((cfn == nullptr) || (*cfn == 0)) return 0;
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
        Widget_Node *tgt = nullptr;
        for (Node* i=next; i && i->level>level; i=i->next) {
          if (i->is_widget()) {
            Widget_Node* myo = (Widget_Node*)i;
            if (Fl::event_inside(myo->o) && myo->o->visible_r())
              tgt = myo;
          }
        }
        if (tgt) {
          char rel[FL_PATH_MAX+1];
          Fluid.proj.enter_project_dir();
          fl_filename_relative(rel, FL_PATH_MAX, fn);
          Fluid.proj.leave_project_dir();
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
      if (   Fluid.proj.tree.current_dnd->group()
          && selection && selection->group()
          && Fluid.proj.tree.current_dnd->group()==selection->group())
      {
        Node *cc = Fluid.proj.tree.current;
        Fluid.proj.tree.current = Fluid.proj.tree.current_dnd;
        add_new_widget_from_user(prototype, Strategy::AS_LAST_CHILD);
        Fluid.proj.tree.current = cc;
      } else {
        add_new_widget_from_user(prototype, Strategy::AS_LAST_CHILD);
      }
      popupx = 0x7FFFFFFF;
      popupy = 0x7FFFFFFF; // mark as invalid (MAXINT)
      in_this_only = nullptr;
      widget_browser->display(Fluid.proj.tree.current);
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
      in_this_only = nullptr;
      return 1;
    }
    // find the innermost item clicked on:
    selection = this;
    {for (Node* i=next; i && i->level>level; i=i->next)
      if (i->is_true_widget()) {
      Widget_Node* myo = (Widget_Node*)i;
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
    {Node* t = selection->click_test(mx, my);
    if (t) {
      //if (t == selection) return 1; // indicates mouse eaten w/o change
      if (Fl::event_state(FL_SHIFT)) {
        Fl::event_is_click(0);
        select(t, !t->selected);
      } else {
        deselect();
        select(t, 1);
        if (t->is_a(Type::Menu_Item)) t->open();
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
      Widget_Node::open();
    } else {
      if (mx<x1) {int t = x1; x1 = mx; mx = t;}
      if (my<y1) {int t = y1; y1 = my; my = t;}
      int n = 0;
      int toggle = Fl::event_state(FL_SHIFT);
      // clear selection on everything:
      if (!toggle) deselect(); else Fl::event_is_click(0);
      // select everything in box:
      for (Node*i=next; i&&i->level>level; i=i->next)
        if (i->is_true_widget()) {
        Widget_Node* myo = (Widget_Node*)i;
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
      Node *i = Fluid.proj.tree.current;
      while (i && !i->is_true_widget()) i = i->parent;
      if (!i) return 0;
      Node *p = i->parent;
      while (p && p != this) p = p->parent;
      if (!p || !p->is_widget()) {
        i = next; if (!i || i->level <= level) return 0;
      }
      p = i;
      for (;;) {
        i = backtab ? i->prev : i->next;
        if (!i || i->level <= level) {i = p; break;}
        if (i->is_true_widget()) break;
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
          fld::app::Snap_Action::get_resize_stepsize(x_step, y_step);
        else
          fld::app::Snap_Action::get_move_stepsize(x_step, y_step);
        dx *= x_step;
        dy *= y_step;
      }
      moveallchildren(Fl::event_key());
      drag = 0;
      return 1;

    case 'o':
        toggle_overlays(nullptr, nullptr);
      break;

    default:
      return 0;
    }}

  case FL_SHORTCUT: {
    in_this_only = this; // modifies how some menu items work.
    const Fl_Menu_Item* m = Fluid.main_menu->test_shortcut();
    if (m && m->callback()) m->do_callback(this->o);
    in_this_only = nullptr;
    return (m != nullptr);}

  default:
    return 0;
  }
}

////////////////////////////////////////////////////////////////


/**
 Write the C++ code that comes before the children of the window are written.
 \param f the source code output stream
 */
void Window_Node::write_code1(fld::io::Code_Writer& f) {
  Widget_Node::write_code1(f);
}


/**
 Write the C++ code that comes after the children of the window are written.
 \param f the source code output stream
 */
void Window_Node::write_code2(fld::io::Code_Writer& f) {
  const char *var = is_class() ? "this" : name() ? name() : "o";
  // make the window modal or non-modal
  if (modal) {
    f.write_c("%s%s->set_modal();\n", f.indent(), var);
  } else if (non_modal) {
    f.write_c("%s%s->set_non_modal();\n", f.indent(), var);
  }
  // clear the window border
  if (!((Fl_Window*)o)->border()) {
    f.write_c("%s%s->clear_border();\n", f.indent(), var);
  }
  // set the xclass of the window
  if (xclass) {
    f.write_c("%s%s->xclass(", f.indent(), var);
    f.write_cstring(xclass);
    f.write_c(");\n");
  }
  // make the window resizable
  if (((Fl_Window*)o)->resizable() == o)
    f.write_c("%s%s->resizable(%s);\n", f.indent(), var, var);
  // set the size range last
  if (sr_max_w || sr_max_h) {
    f.write_c("%s%s->size_range(%d, %d, %d, %d);\n", f.indent(), var,
            sr_min_w, sr_min_h, sr_max_w, sr_max_h);
  } else if (sr_min_w || sr_min_h) {
    f.write_c("%s%s->size_range(%d, %d);\n", f.indent(), var, sr_min_w, sr_min_h);
  }
  // insert extra code from user, may call `show()`
  write_extra_code(f);
  // stop adding widgets to this window
  f.write_c("%s%s->end();\n", f.indent(), var);
  write_block_close(f);
}

void Window_Node::write_properties(fld::io::Project_Writer &f) {
  Widget_Node::write_properties(f);
  if (modal) f.write_string("modal");
  else if (non_modal) f.write_string("non_modal");
  if (!((Fl_Window*)o)->border()) f.write_string("noborder");
  if (xclass) {f.write_string("xclass"); f.write_word(xclass);}
  if (sr_min_w || sr_min_h || sr_max_w || sr_max_h)
    f.write_string("size_range {%d %d %d %d}", sr_min_w, sr_min_h, sr_max_w, sr_max_h);
  if (o->visible() || override_visible_) f.write_string("visible");
}

void Window_Node::read_property(fld::io::Project_Reader &f, const char *c) {
  if (!strcmp(c,"modal")) {
    modal = 1;
  } else if (!strcmp(c,"non_modal")) {
    non_modal = 1;
  } else if (!strcmp(c, "visible")) {
    if (Fluid.batch_mode) // don't actually open any windows in batch mode
      override_visible_ = 1;
    else // in interactive mode, we simply show the window
      open_();
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
    Widget_Node::read_property(f, c);
    Fluid.pasteoffset = 0; // make it not apply to contents
  } else {
    Widget_Node::read_property(f, c);
  }
}

int Window_Node::read_fdesign(const char* propname, const char* value) {
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
    return Widget_Node::read_fdesign(propname,value);
  }
  return 1;
}

///////////////////////////////////////////////////////////////////////

Widget_Class_Node Widget_Class_Node::prototype;

Widget_Class_Node *current_widget_class = nullptr;

/**
 Create and add a new Widget Class node.
 \param[in] strategy add after current or as last child
 \return new node
 */
Node *Widget_Class_Node::make(Strategy strategy) {
  Node *anchor = Fluid.proj.tree.current, *p = anchor;
  if (p && (strategy.placement() == Strategy::AFTER_CURRENT)) p = p->parent;
  while (p && (!p->is_decl_block() || (p->is_widget() && p->is_class()))) {
    anchor = p;
    strategy.placement(Strategy::AFTER_CURRENT);
    p = p->parent;
  }
  Widget_Class_Node *myo = new Widget_Class_Node();
  myo->name("UserInterface");

  if (!this->o) {// template widget
    this->o = new Fl_Window(100,100);
    Fl_Group::current(nullptr);
  }
  myo->factory = this;
  myo->drag = 0;
  myo->numselected = 0;
  Overlay_Window *w = new Overlay_Window(100, 100);
  w->size_range(10, 10);
  w->window = myo;
  myo->o = w;
  myo->add(anchor, strategy);
  myo->modal = 0;
  myo->non_modal = 0;
  myo->wc_relative = 0;

  return myo;
}

void Widget_Class_Node::write_properties(fld::io::Project_Writer &f) {
  Window_Node::write_properties(f);
  if (wc_relative==1)
    f.write_string("position_relative");
  else if (wc_relative==2)
    f.write_string("position_relative_rescale");
}

void Widget_Class_Node::read_property(fld::io::Project_Reader &f, const char *c) {
  if (!strcmp(c,"position_relative")) {
    wc_relative = 1;
  } else if (!strcmp(c,"position_relative_rescale")) {
      wc_relative = 2;
  } else {
    Window_Node::read_property(f, c);
  }
}

// Convert A::B::C::D to D (i.e. keep only innermost name)
// This is useful for classes that contain a namespace component
static const char *trimclassname(const char *n) {
  if (!n)
    return nullptr;
  const char *nn;
  while((nn = strstr(n, "::"))) {
    n = nn + 2;
  }
  return(n);
}


void Widget_Class_Node::write_code1(fld::io::Code_Writer& f) {
#if 0
  Widget_Node::write_code1(fld::io::Code_Writer& f);
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

/**
 Write the C++ code that comes after the children of the window are written.
 \param f the source code output stream
 */
void Widget_Class_Node::write_code2(fld::io::Code_Writer& f) {
  // make the window modal or non-modal
  if (modal) {
    f.write_c("%sset_modal();\n", f.indent());
  } else if (non_modal) {
    f.write_c("%sset_non_modal();\n", f.indent());
  }
  // clear the window border
  if (!((Fl_Window*)o)->border()) f.write_c("%sclear_border();\n", f.indent());
  // set the xclass of the window
  if (xclass) {
    f.write_c("%sxclass(", f.indent());
    f.write_cstring(xclass);
    f.write_c(");\n");
  }
  // make the window resizable
  if (((Fl_Window*)o)->resizable() == o)
    f.write_c("%sresizable(this);\n", f.indent());
  // insert extra code from user
  write_extra_code(f);
  // stop adding widgets to this window
  f.write_c("%send();\n", f.indent());
  // reposition or resize the Widget Class to fit into the target
  if (wc_relative==1)
    f.write_c("%sposition(X, Y);\n", f.indent());
  else if (wc_relative==2)
    f.write_c("%sresize(X, Y, W, H);\n", f.indent());
  f.indentation--;
  f.write_c("}\n");
}


////////////////////////////////////////////////////////////////
// live mode support

Fl_Widget *Window_Node::enter_live_mode(int) {
  Fl_Window *win = new Fl_Window(10, 10, o->w(), o->h());
  return propagate_live_mode(win);
}

void Window_Node::leave_live_mode() {
}

/**
 copy all properties from the edit widget to the live widget
 */
void Window_Node::copy_properties() {
  Fl_Window *self = static_cast<Fl_Window*>(o);
  Fl_Window *live = static_cast<Fl_Window*>(live_widget);
  if (self->resizable() == self)
    live->resizable(live);
  Widget_Node::copy_properties();
}
