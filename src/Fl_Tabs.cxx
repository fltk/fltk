//
// Tab widget for the Fast Light Tool Kit (FLTK).
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

// This is the "file card tabs" interface to allow you to put lots and lots
// of buttons and switches in a panel, as popularized by many toolkits.

// Each child widget is a card, and its label() is printed on the card tab.
// Clicking the tab makes that card visible.

#include <FL/Fl.H>
#include <FL/Fl_Tabs.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Menu_Item.H>

#include <stdio.h>
#include <stdlib.h>

#define BORDER 2
#define EXTRASPACE 10
#define SELECTION_BORDER 5
#define EXTRAGAP 2
#define MARGIN 20


/** Make sure that we redraw all tabs when new children are added. */
int Fl_Tabs::on_insert(Fl_Widget* candidate, int index) {
  redraw_tabs();
  damage(FL_DAMAGE_EXPOSE);
  return Fl_Group::on_insert(candidate, index);
}

/** Make sure that we redraw all tabs when children are moved. */
int Fl_Tabs::on_move(int a, int b) {
  redraw_tabs();
  damage(FL_DAMAGE_EXPOSE);
  return Fl_Group::on_move(a, b);
}

/** Make sure that we redraw all tabs when new children are removed. */
void Fl_Tabs::on_remove(int index) {
  redraw_tabs();
  damage(FL_DAMAGE_EXPOSE|FL_DAMAGE_ALL);
  if (child(index)->visible()) {
    if (index+1<children())
      value(child(index+1));
    else if (index>0)
      value(child(index-1));
  }
  Fl_Group::on_remove(index);
}

/** Make sure that we redraw all tabs when the widget size changes. */
void Fl_Tabs::resize(int X, int Y, int W, int H) {
  damage(FL_DAMAGE_EXPOSE);
  Fl_Group::resize(X, Y, W, H);
}

// Return the left edges of each tab (plus a fake left edge for a tab
// past the right-hand one).  These positions are actually of the left
// edge of the slope.  They are either separated by the correct distance
// or by EXTRASPACE or by zero.
// These positions are updated in the private arrays tab_pos[] and
// tab_width[], resp.. If needed, these arrays are (re)allocated.
// Return value is the index of the selected item.

int Fl_Tabs::tab_positions() {
  const int nc = children();
  if (nc != tab_count) {
    clear_tab_positions();
    if (nc) {
      tab_pos   = (int*)malloc((nc+1)*sizeof(int));
      tab_width = (int*)malloc((nc)*sizeof(int));
      tab_flags = (int*)malloc((nc)*sizeof(int));
    }
    tab_count = nc;
  }
  if (nc == 0) return 0;
  int selected = 0;
  Fl_Widget*const* a = array();
  int i;
  char prev_draw_shortcut = fl_draw_shortcut;
  fl_draw_shortcut = 1;

  tab_pos[0] = Fl::box_dx(box());
  for (i=0; i<nc; i++) {
    Fl_Widget* o = *a++;
    if (o->visible()) selected = i;

    int wt = 0; int ht = 0;
    Fl_Labeltype ot = o->labeltype();
    Fl_Align oa = o->align();
    if (ot == FL_NO_LABEL) {
      o->labeltype(FL_NORMAL_LABEL);
    }
    o->align(tab_align());
    o->measure_label(wt,ht);
    o->labeltype(ot);
    o->align(oa);

    if (o->when() & FL_WHEN_CLOSED)
      wt += labelsize()/2 + EXTRAGAP;

    tab_width[i] = wt + EXTRASPACE;
    tab_pos[i+1] = tab_pos[i] + tab_width[i] + BORDER;
    tab_flags[i] = 0;
  }
  fl_draw_shortcut = prev_draw_shortcut;

  int r = w();
  if (tab_pos[i] <= r) return selected;

  if (overflow_type == OVERFLOW_COMPRESS) {
    // uh oh, they are too big:
    // pack them against right edge:
    tab_pos[i] = r;
    for (i = nc; i--;) {
      int l = r-tab_width[i];
      if (tab_pos[i+1] < l) l = tab_pos[i+1];
      if (tab_pos[i] <= l) break;
      tab_pos[i] = l;
      tab_flags[i] |= 1;
      r -= EXTRASPACE;
    }
    // pack them against left edge and truncate width if they still don't fit:
    for (i = 0; i<nc; i++) {
      if (tab_pos[i] >= i*EXTRASPACE) break;
      tab_pos[i] = i*EXTRASPACE;
      tab_flags[i] |= 1;
      int W = w()-1-EXTRASPACE*(nc-i) - tab_pos[i];
      if (tab_width[i] > W) tab_width[i] = W;
    }
    // adjust edges according to visiblity:
    for (i = nc; i > selected; i--) {
      tab_pos[i] = tab_pos[i-1] + tab_width[i-1];
    }
    if ((selected > 0) && (tab_pos[selected-1]+tab_width[selected-1]>tab_pos[selected]))
      tab_flags[selected] |= 1;
    tab_flags[selected] &= ~1;
  }
  return selected;
}

// Returns space (height) in pixels needed for tabs. Negative to put them on the bottom.
// Returns full height, if children() = 0.
int Fl_Tabs::tab_height() {
  if (children() == 0) return h();
  int H = h();
  int H2 = y();
  Fl_Widget*const* a = array();
  for (int i=children(); i--;) {
    Fl_Widget* o = *a++;
    if (o->y() < y()+H) H = o->y()-y();
    if (o->y()+o->h() > H2) H2 = o->y()+o->h();
  }
  H2 = y()+h()-H2;
  if (H2 > H) return (H2 <= 0) ? 0 : -H2;
  else return (H <= 0) ? 0 : H;
}

/** Return a pointer to the child widget with a tab at the given coordinates.

  The Fl_Tabs::which() method returns a pointer to the child widget of the
  Fl_Tabs container that corresponds to the tab at the given event coordinates.
  If the event coordinates are outside the area of the tabs or if the Fl_Tabs
  container has no children, the method returns NULL.

  \param event_x, event_y event coordinates
  \returns pointer to the selected child widget, or NULL
*/
Fl_Widget *Fl_Tabs::which(int event_x, int event_y) {
  if (children() == 0) return 0;
  int H = tab_height();
  if (H < 0) {
    if (event_y > y()+h() || event_y < y()+h()+H) return 0;
  } else {
    if (event_y > y()+H || event_y < y()) return 0;
  }
  if (event_x < x()) return 0;
  Fl_Widget *ret = 0L;
  const int nc = children();
  tab_positions();
  for (int i=0; i<nc; i++) {
    if (event_x < x()+tab_pos[i+1]+tab_offset) {
      ret = child(i);
      break;
    }
  }
  return ret;
}

/**  Check whether the coordinates fall within the "close" button area of the tab.

 The Fl_Tabs::hit_close() method checks whether the given event coordinates
 fall within the area of the "close" button on the tab of the specified
 child widget. This method should be called after the Fl_Tabs::which() method,
 which updates a lookup table used to determine the width of each tab.

 \param o check the tab of this widget
 \param event_x, event_y event coordinatese
 \return 1 if we hit the close button, and 0 otherwie
 */
int Fl_Tabs::hit_close(Fl_Widget *o, int event_x, int event_y) {
  (void)event_y;
  for (int i=0; i<children(); i++) {
    if (child(i)==o) {
      // never hit the "close" button on a compressed tab unless it's the active one
      if (tab_flags[i] & 1)
        return 0;
      // did we hit the area of teh "x"?
      int tab_x = tab_pos[i] + tab_offset + x();
      return (   (event_x >= tab_x)
              && (event_x <  tab_x + (labelsize()+EXTRASPACE+EXTRAGAP)/2) );
    }
  }
  return 0;
}

void Fl_Tabs::check_overflow_menu() {
  int nc = children();
  int H = tab_height(); if (H < 0) H = -H;
  if (tab_pos[nc] > w()-H) {
    has_overflow_menu = 1;
  } else {
    has_overflow_menu = 0;
  }
}

void Fl_Tabs::handle_overflow_menu() {
  int nc = children();
  int H = tab_height(); if (H < 0) H = -H;
  int i, fv=-1, lv=nc; // first and last visible tab
  if (nc==0) return;

  // count visibel children
  for (i = 0; i < nc; i++) {
    if (tab_pos[i]+tab_offset < 0) fv = i;
    if (tab_pos[i]+tab_width[i]+tab_offset <= w()-H) lv = i;
  }

  // create a menu with all children
  overflow_menu = new Fl_Menu_Item[nc+1];
  memset(overflow_menu, 0, sizeof(Fl_Menu_Item)*(nc+1));
  for (i = 0; i < nc; i++) {
    overflow_menu[i].label(child(i)->label());
    overflow_menu[i].user_data(child(i));
    overflow_menu[i].labelfont(labelfont());
    overflow_menu[i].labelsize(labelsize());
    if ( (i == fv) || (i == lv) )
      overflow_menu[i].flags |= FL_MENU_DIVIDER;
    if (child(i)->visible())
      overflow_menu[i].labelfont_ |= FL_BOLD;
  }

  // show the menu and handle the selection
  const Fl_Menu_Item *m = overflow_menu->popup(x()+w()-H, (tab_height()>0)?(y()+H):(y()+h()));
  if (m)
    value((Fl_Widget*)m->user_data());

  // delete the menu until we need it next time
  if (overflow_menu) {
    delete[] overflow_menu;
    overflow_menu = NULL;
  }
}

void Fl_Tabs::draw_overflow_menu_button() {
  int H = tab_height();
  int X, Y;
  if (H > 0) {
    X = x() + w() - H;
    Y = y();
  } else {
    H = -H;
    X = x() + w() - H;
    Y = y() + h() - H - 1;
  }
  fl_draw_box(box(), X, Y, H, H+1, color());
  Fl_Rect r(X, Y, H, H+1);
  fl_draw_arrow(r, FL_ARROW_CHOICE, FL_ORIENT_NONE, fl_contrast(FL_BLACK, color()));
}

void Fl_Tabs::redraw_tabs()
{
  int H = tab_height();
  if (H >= 0) {
    H += Fl::box_dy(box());
    damage(FL_DAMAGE_SCROLL, x(), y(), w(), H);
  } else {
    H = Fl::box_dy(box()) - H;
    damage(FL_DAMAGE_SCROLL, x(), y() + h() - H, w(), H);
  }
}

int Fl_Tabs::handle(int event) {
  static int initial_x = 0;
  static int initial_tab_offset = 0;
  static int forward_motion_to_group = 0;
  Fl_Widget *o;
  int i;

  switch (event) {

  case FL_PUSH:
    initial_x = Fl::event_x();
    initial_tab_offset = tab_offset;
    forward_motion_to_group = 0;
    {
      int H = tab_height();
      if (H >= 0) {
        if (Fl::event_y() > y()+H) {
          forward_motion_to_group = 1;
          return Fl_Group::handle(event);
        }
      } else {
        if (Fl::event_y() < y()+h()+H) {
          forward_motion_to_group = 1;
          return Fl_Group::handle(event);
        }
        H = - H;
      }
      if (has_overflow_menu && Fl::event_x() > x()+w()-H) {
        handle_overflow_menu();
        return 1;
      }
    }
    /* FALLTHROUGH */
  case FL_DRAG:
  case FL_RELEASE:
    if (forward_motion_to_group) {
      return Fl_Group::handle(event);
    }
    o = which(Fl::event_x(), Fl::event_y());
    if ( (overflow_type == OVERFLOW_DRAG) || (overflow_type == OVERFLOW_PULLDOWN) ) {
      if (tab_pos[children()] < w() && tab_offset == 0) {
        // fall through
      } else if (!Fl::event_is_click()) {
        tab_offset = initial_tab_offset + Fl::event_x() - initial_x;
        int m = 0;
        if (overflow_type == OVERFLOW_PULLDOWN) m = abs(tab_height());
        if (tab_offset > 0) {
          initial_tab_offset -= tab_offset;
          tab_offset = 0;
        } else {
          int dw = tab_pos[children()] + tab_offset - w();
          if (dw < -m) {
            initial_tab_offset -= dw+m;
            tab_offset -= dw+m;
          }
        }
        damage(FL_DAMAGE_EXPOSE|FL_DAMAGE_SCROLL);
        return 1;
      }
    }
    if (event == FL_RELEASE) {
      push(0);
      if (o && Fl::visible_focus() && Fl::focus()!=this) {
        Fl::focus(this);
        redraw_tabs();
      }
      if (o && (o->when() & FL_WHEN_CLOSED) && hit_close(o, Fl::event_x(), Fl::event_y())) {
        o->do_callback(FL_REASON_CLOSED);
        return 1; // o may be deleted at this point
      }
      if (o &&                              // Released on a tab and..
          (value(o) ||                      // tab changed value or..
           (when()&(FL_WHEN_NOT_CHANGED))   // ..no change but WHEN_NOT_CHANGED set,
          )                                 // handles FL_WHEN_RELEASE_ALWAYS too.
         ) {
        Fl_Widget_Tracker wp(o);
        set_changed();
        do_callback(FL_REASON_SELECTED);
        if (wp.deleted()) return 1;
      }
      Fl_Tooltip::current(o);
    } else {
      push(o);
    }
    return 1;
  case FL_MOVE: {
    int ret = Fl_Group::handle(event);
    Fl_Widget *tooltip_widget = Fl_Tooltip::current();
    Fl_Widget *n; // initialized later
    int H = tab_height();
    if ( (H >= 0) && (Fl::event_y() > y()+H) )
      return ret;
    else if ( (H < 0) && (Fl::event_y() < y()+h()+H) )
      return ret;
    else {
      n = which(Fl::event_x(), Fl::event_y());
      if (!n) n = this;
    }
    if (n != tooltip_widget)
      Fl_Tooltip::enter(n);
    return ret; }
  case FL_FOCUS:
  case FL_UNFOCUS:
    if (!Fl::visible_focus()) return Fl_Group::handle(event);
    if (Fl::event() == FL_RELEASE ||
        Fl::event() == FL_SHORTCUT ||
        Fl::event() == FL_KEYBOARD ||
        Fl::event() == FL_FOCUS ||
        Fl::event() == FL_UNFOCUS) {
      redraw_tabs();
      if (Fl::event() == FL_FOCUS) return Fl_Group::handle(event);
      if (Fl::event() == FL_UNFOCUS) return 0;
      else return 1;
    } else return Fl_Group::handle(event);
  case FL_KEYBOARD:
    switch (Fl::event_key()) {
      case FL_Left:
        if (!children()) return 0;
        if (child(0)->visible()) return 0;
        for (i = 1; i < children(); i ++)
          if (child(i)->visible()) break;
        value(child(i - 1));
        set_changed();
        do_callback(FL_REASON_SELECTED);
        return 1;
      case FL_Right:
        if (!children()) return 0;
        if (child(children() - 1)->visible()) return 0;
        for (i = 0; i < children()-1; i++)
          if (child(i)->visible()) break;
        value(child(i + 1));
        set_changed();
        do_callback(FL_REASON_SELECTED);
        return 1;
      case FL_Down:
        redraw();
        return Fl_Group::handle(FL_FOCUS);
      default:
        break;
    }
    return Fl_Group::handle(event);
  case FL_SHORTCUT:
    for (i = 0; i < children(); ++i) {
      Fl_Widget *c = child(i);
      if (c->test_shortcut(c->label())) {
        char sc = !c->visible();
        value(c);
        if (sc) {
          set_changed();
          do_callback(FL_REASON_SELECTED);
        } else {
          do_callback(FL_REASON_RESELECTED);
        }
        return 1;
      }
    }
    return Fl_Group::handle(event);
  case FL_SHOW:
    value(); // update visibilities and fall through
  default:
    return Fl_Group::handle(event);

  }
}

/**
  This is called by the tab widget's handle() method to set the
  tab group widget the user last FL_PUSH'ed on. Set back to zero
  on FL_RELEASE.

  As of this writing, the value is mainly used by draw_tab()
  to determine whether or not to draw a 'down' box for the tab
  when it's clicked, and to turn it off if the user drags off it.

  \see push().
*/
int Fl_Tabs::push(Fl_Widget *o) {
  if (push_ == o) return 0;
  if ( (push_ && !push_->visible()) || (o && !o->visible()) )
    redraw_tabs();
  push_ = o;
  return 1;
}

/**
  Gets the currently visible widget/tab.

  The Fl_Tabs::value() method returns a pointer to the currently visible child
  widget of the Fl_Tabs container. The visible child is the first child that
  is currently being displayed, or the last child if none of the children are
  being displayed.

  If child widgets have been added, moved, or deleted, this method ensures that
  only one tab is visible at a time.

  \return a pointer to the currently visible child
*/
Fl_Widget* Fl_Tabs::value() {
  Fl_Widget* v = 0;
  Fl_Widget*const* a = array();
  for (int i=children(); i--;) {
    Fl_Widget* o = *a++;
    if (v) o->hide();
    else if (o->visible()) v = o;
    else if (!i) {o->show(); v = o;}
  }
  return v;
}

/** Sets the widget to become the current visible widget/tab.

  The Fl_Tabs::value() method allows you to set a particular child widget of
  the Fl_Tabs container to be the currently visible widget. If the specified
  widget is a child of the Fl_Tabs container, it will be made visible and all
  other children will be hidden. The method returns 1 if the value was changed,
  and 0 if the specified value was already set.

  \param[in] newvalue a poiner to a child widget
  \return 1 if a different tab was chosen
  \return 0 if there was no change (new value already set)
*/
int Fl_Tabs::value(Fl_Widget *newvalue) {
  Fl_Widget*const* a = array();
  int ret = 0;
  int selected = -1;
  for (int i=children(); i--;) {
    Fl_Widget* o = *a++;
    if (o == newvalue) {
      if (!o->visible()) ret = 1;
      o->show();
      selected = children()-i-1;
    } else {
      o->hide();
    }
  }
  // make sure that the selected tab is visible
  if (   (selected >= 0)
      && (ret == 1)
      && ( (overflow_type == OVERFLOW_DRAG)
        || (overflow_type == OVERFLOW_PULLDOWN) ) ) {
    int m = MARGIN;
    if ( (selected == 0) || (selected == children()-1) ) m = 0;
    int mr = m;
    if (overflow_type == OVERFLOW_PULLDOWN) mr += abs(tab_height());
    tab_positions();
    if (tab_pos[selected]+tab_width[selected]+tab_offset+mr > w()) {
      tab_offset = w() - tab_pos[selected] - tab_width[selected] - mr;
      damage(FL_DAMAGE_EXPOSE);
    } else if (tab_pos[selected]+tab_offset-m < 0) {
      tab_offset = -tab_pos[selected]+m;
      damage(FL_DAMAGE_EXPOSE);
    }
  }
  return ret;
}

enum {LEFT, RIGHT, SELECTED};

void Fl_Tabs::draw() {
  Fl_Widget *v = value();
  int H = tab_height();
  int ty, th;
  if (H >= 0) {
    ty = y(); th = H;
  } else {
    ty = y() + h() + H; th = -H;
  }
  Fl_Color c = v ? v->color() : color();

  if (damage() & FL_DAMAGE_ALL) { // redraw the children
    draw_box(box(), x(), y()+(H>=0?H:0), w(), h()-(H>=0?H:-H), c);
  }
  if (damage() & (FL_DAMAGE_SCROLL|FL_DAMAGE_ALL)) {
    if (selection_color() != c) {
      // Draw the top or bottom SELECTION_BORDER lines of the tab pane in the
      // selection color so that the user knows which tab is selected...
      int clip_y = (H >= 0) ? y() + H : y() + h() + H - SELECTION_BORDER;
      fl_push_clip(x(), clip_y, w(), SELECTION_BORDER);
      draw_box(box(), x(), clip_y, w(), SELECTION_BORDER, selection_color());
      fl_pop_clip();
    }
  }
  if (damage() & FL_DAMAGE_ALL) { // redraw the children
    if (v) draw_child(*v);
  } else { // redraw the child
    if (v) update_child(*v);
  }
  if (damage() & FL_DAMAGE_EXPOSE) { // redraw the tab bar background
    if (parent()) {
      Fl_Widget *p = parent();
      fl_push_clip(x(), ty, w(), th);
      if (p->as_window())
        fl_draw_box(p->box(), 0, 0, p->w(), p->h(), p->color());
      else
        fl_draw_box(p->box(), p->x(), p->y(), p->w(), p->h(), p->color());
      fl_pop_clip();
    } else {
      fl_rectf(x(), ty, w(), th, color());
    }
  }
  if (damage() & (FL_DAMAGE_SCROLL|FL_DAMAGE_ALL)) {
    const int nc = children();
    int selected = tab_positions();
    int i;
    if (H>0)
      fl_push_clip(x(), ty, w(), th+BORDER);
    else
      fl_push_clip(x(), ty-BORDER, w(), th+BORDER);
    Fl_Widget*const* a = array();
    for (i=0; i<selected; i++)
      draw_tab(x()+tab_pos[i], x()+tab_pos[i+1],
               tab_width[i], H, a[i], tab_flags[i], LEFT);
    for (i=nc-1; i > selected; i--)
      draw_tab(x()+tab_pos[i], x()+tab_pos[i+1],
               tab_width[i], H, a[i], tab_flags[i], RIGHT);
    if (v) {
      i = selected;
      draw_tab(x()+tab_pos[i], x()+tab_pos[i+1],
               tab_width[i], H, a[i], tab_flags[i], SELECTED);
    }
    if (overflow_type == OVERFLOW_PULLDOWN)
      check_overflow_menu();
    if (has_overflow_menu)
      draw_overflow_menu_button();
    fl_pop_clip();
  }
}

void Fl_Tabs::draw_tab(int x1, int x2, int W, int H, Fl_Widget* o, int flags, int what) {
  x1 += tab_offset;
  x2 += tab_offset;
  int sel = (what == SELECTED);
  int dh = Fl::box_dh(box());
  int dy = Fl::box_dy(box());
  int wc = 0; // width of "close" button if drawn, or 0
  char prev_draw_shortcut = fl_draw_shortcut;
  fl_draw_shortcut = 1;

  Fl_Boxtype bt = (o == push_ && !sel) ? fl_down(box()) : box();
  Fl_Color bc = sel ? selection_color() : o->selection_color();

  // Save the label color and label type
  Fl_Color oc = o->labelcolor();
  Fl_Labeltype ot = o->labeltype();

  // Set a labeltype that really draws a label
  if (ot == FL_NO_LABEL)
    o->labeltype(FL_NORMAL_LABEL);

  // compute offsets to make selected tab look bigger
  int yofs = sel ? 0 : BORDER;

  if ((x2 < x1+W) && what == RIGHT) x1 = x2 - W;

  if (H >= 0) {
    if (sel) fl_push_clip(x1, y(), x2 - x1, H + dh - dy);
    else fl_push_clip(x1, y(), x2 - x1, H);

    H += dh;

    draw_box(bt, x1, y() + yofs, W, H + 10 - yofs, bc);

    // Draw the label using the current color...
    o->labelcolor(sel ? labelcolor() : o->labelcolor());

    // Draw the "close" button if requested
    if ( (o->when() & FL_WHEN_CLOSED) && !(flags & 1) ) {
      int sz = labelsize()/2, sy = (H - sz)/2;
      fl_draw_symbol("@3+", x1 + EXTRASPACE/2, y() + yofs/2 + sy, sz, sz, o->labelcolor());
      wc = sz + EXTRAGAP;
    }

    // Draw the label text
    o->draw_label(x1 + wc, y() + yofs, W - wc, H - yofs, tab_align());

    // Draw the focus box
    if (Fl::focus() == this && o->visible())
      draw_focus(bt, x1, y(), W, H, bc);

    fl_pop_clip();
  } else {
    H = -H;

    if (sel) fl_push_clip(x1, y() + h() - H - dy, x2 - x1, H + dy);
    else fl_push_clip(x1, y() + h() - H, x2 - x1, H);

    H += dh;

    draw_box(bt, x1, y() + h() - H - 10, W, H + 10 - yofs, bc);

    // Draw the label using the current color...
    o->labelcolor(sel ? labelcolor() : o->labelcolor());

    // Draw the "close" button if requested
    if ( (o->when() & FL_WHEN_CLOSED) && (x1+W < x2) ) {
      int sz = labelsize()/2, sy = (H - sz)/2;
      fl_draw_symbol("@3+", x1 + EXTRASPACE/2, y() + h() - H -yofs/2 + sy, sz, sz, o->labelcolor());
      wc = sz + EXTRAGAP;
    }

    // Draw the label text
    o->draw_label(x1 + wc, y() + h() - H, W - wc, H - yofs, tab_align());

    // Draw the focus box
    if (Fl::focus() == this && o->visible())
      draw_focus(bt, x1, y() + h() - H, W, H, bc);

    fl_pop_clip();
  }
  fl_draw_shortcut = prev_draw_shortcut;

  // Restore the original label color and label type
  o->labelcolor(oc);
  o->labeltype(ot);
}

/**
  Creates a new Fl_Tabs widget using the given position, size,
  and label string. The default boxtype is FL_THIN_UP_BOX.

  Use add(Fl_Widget*) to add each child, which are usually
  Fl_Group widgets. The children should be sized to stay
  away from the top or bottom edge of the Fl_Tabs widget,
  which is where the tabs will be drawn.

  All children of Fl_Tabs should have the same size and exactly fit on top of
  each other. They should only leave space above or below where the tabs will
  go, but not on the sides. If the first child of Fl_Tabs is set to
  "resizable()", the riders will not resize when the tabs are resized.

  The destructor <I>also deletes all the children</I>. This
  allows a whole tree to be deleted at once, without having to
  keep a pointer to all the children in the user code. A kludge
  has been done so the Fl_Tabs and all of its children
  can be automatic (local) variables, but you must declare the
  Fl_Tabs widget <I>first</I> so that it is destroyed last.
*/
Fl_Tabs::Fl_Tabs(int X, int Y, int W, int H, const char *L) :
  Fl_Group(X,Y,W,H,L)
{
  box(FL_THIN_UP_BOX);
  push_ = 0;
  overflow_type = OVERFLOW_COMPRESS;
  tab_offset = 0;
  tab_pos = 0;
  tab_width = 0;
  tab_flags = NULL;
  tab_count = 0;
  tab_align_ = FL_ALIGN_CENTER;
  has_overflow_menu = 0;
  overflow_menu = NULL;
}

Fl_Tabs::~Fl_Tabs() {
  clear_tab_positions();
  if (overflow_menu)
    delete[] overflow_menu;
}

/**
  Returns the position and size available to be used by its children.

  If there isn't any child yet the \p tabh parameter will be used to
  calculate the return values. This assumes that the children's labelsize
  is the same as the Fl_Tabs' labelsize and adds a small border.

  If there are already children, the values of child(0) are returned, and
  \p tabh is ignored.

  \note Children should always use the same positions and sizes.

  \p tabh can be one of
  \li    0: calculate label size, tabs on top
  \li   -1: calculate label size, tabs on bottom
  \li >  0: use given \p tabh value, tabs on top (height = tabh)
  \li < -1: use given \p tabh value, tabs on bottom (height = -tabh)

  \param[in]    tabh            position and optional height of tabs (see above)
  \param[out]   rx,ry,rw,rh     (x,y,w,h) of client area for children

  \since        FLTK 1.3.0
*/
void Fl_Tabs::client_area(int &rx, int &ry, int &rw, int &rh, int tabh) {

  if (children()) {                     // use existing values

    rx = child(0)->x();
    ry = child(0)->y();
    rw = child(0)->w();
    rh = child(0)->h();

  } else {                              // calculate values

    int y_offset;
    int label_height = fl_height(labelfont(), labelsize()) + BORDER*2;

    if (tabh == 0)                      // use default (at top)
      y_offset = label_height;
    else if (tabh == -1)                // use default (at bottom)
      y_offset = -label_height;
    else
      y_offset = tabh;                  // user given value

    rx = x();
    rw = w();

    if (y_offset >= 0) {                // labels at top
      ry = y() + y_offset;
      rh = h() - y_offset;
    } else {                            // labels at bottom
      ry = y();
      rh = h() + y_offset;
    }
  }
}

void Fl_Tabs::clear_tab_positions() {
  if (tab_pos) {
    free(tab_pos);
    tab_pos = 0;
  }
  if (tab_width){
    free(tab_width);
    tab_width = 0;
  }
  if (tab_flags){
    free(tab_flags);
    tab_flags = NULL;
  }
}

/** Set a method to handle an overflowing tab bar.

 The Fl_Tabs widget allows you to specify how to handle the situation where
 there are more tabs than can be displayed at once. The available options are:

 - \c OVERFLOW_COMPRESS: Tabs will be compressed and overlaid on top of each other.
 - \c OVERFLOW_CLIP: Only the first tabs that fit will be displayed.
 - \c OVERFLOW_PULLDOWN: Tabs that do not fit will be placed in a pull-down menu.
 - \c OVERFLOW_DRAG: The tab bar can be dragged horizontally to reveal additional tabs.

 You can set the desired behavior using the overflow() method.

 \param ov overflow type
 */
void Fl_Tabs::handle_overflow(int ov) {
  overflow_type = ov;
  tab_offset = 0;
  has_overflow_menu = 0;
  if (overflow_menu) {
    delete[] overflow_menu;
    overflow_menu = NULL;
  }
  damage(FL_DAMAGE_EXPOSE|FL_DAMAGE_ALL);
  redraw();
}

