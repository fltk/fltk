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
#include <FL/Fl_Window.H>

#include <stdio.h>
#include <stdlib.h>

#define BORDER 2
#define OV_BORDER 2
#define EXTRASPACE 10
#define SELECTION_BORDER 5
#define EXTRAGAP 2
#define MARGIN 20

enum {LEFT, RIGHT, SELECTED};

static int fl_min(int a, int b) { return a < b ? a : b; }

/** Make sure that we redraw all tabs when new children are added. */
int Fl_Tabs::on_insert(Fl_Widget* candidate, int index) {
  redraw_tabs();
  return Fl_Group::on_insert(candidate, index);
}

/** Make sure that we redraw all tabs when children are moved. */
int Fl_Tabs::on_move(int a, int b) {
  redraw_tabs();
  return Fl_Group::on_move(a, b);
}

/** Make sure that we redraw all tabs when new children are removed. */
void Fl_Tabs::on_remove(int index) {
  redraw_tabs();
  if (child(index)->visible()) {
    if (index+1<children())
      value(child(index+1));
    else if (index>0)
      value(child(index-1));
  }
  if (children()==1)
    damage(FL_DAMAGE_ALL);
  Fl_Group::on_remove(index);
}

/** Make sure that we redraw all tabs when the widget size changes. */
void Fl_Tabs::resize(int X, int Y, int W, int H) {
  redraw_tabs();
  Fl_Group::resize(X, Y, W, H);
}

/** Ensure proper placement of selected tab. */
void Fl_Tabs::show() {
  Fl::damage(FL_DAMAGE_SCROLL);
  Fl_Group::show();
}

/** Calculate tab positions and widths.

  This protected method calculates the horizontal display positions and
  widths of all tabs. If the number of children \c 'nc' (see below) is \> 0
  three internal arrays are allocated, otherwise the arrays are free'd
  and the pointers are set to NULL. Note that the first array is larger
  (nc+1).

  - tab_pos[nc+1] : The left edges of each tab plus a fake left edge
    for a tab past the right-hand one.
  - tab_width[nc] : The width of each tab
  - tab_flags[nc] : Flags, bit 0 is set if the tab is compressed

  If needed, these arrays are (re)allocated.

  These positions are actually of the left edge of the slope.
  They are either separated by the correct distance
  or by EXTRASPACE or by zero.

  In OVERFLOW_COMPRESS mode, tab positions and widths are compressed to make
  the entire tabs bar fit into the width of Fl_Tabs while keeping the selected
  tab fully visible.

  In other overflow modes, the tabs area may be dragged horizontally using
  \ref tab_offset. The tab_pos array is not adjusted to the horizontal offset,
  but starts at this->x() plus the box's left margin.

  The protected variable `tab_count` is set to the currently allocated
  size, i.e. the number of children (`nc`).

  \returns Index of the selected item
  \retval -1 If the number of children is 0 (zero).

  \note Return values in 1.3 were not documented. Return values before Sep 2023
        were documented as 1 based index and 0 if there were no children. This
        was actually never the case. It always returned a 0 based index and
        the (useless) value of also 0 if there were no children. The current
        version return -1 if there are no children.

  \note For this method to work, only on single child should be selected.
        Calling the method \ref value() before calling \ref tab_positions()
        will ensure that exactly one child is selected and return a pointer
        to that child.

  \see clear_tab_positions()
*/
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
  if (nc == 0) return -1;
  int selected = 0;
  Fl_Widget*const* a = array();
  int i;
  char prev_draw_shortcut = fl_draw_shortcut;
  fl_draw_shortcut = 1;

  int l = tab_pos[0] = Fl::box_dx(box());
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

  if (overflow_type == OVERFLOW_COMPRESS) {
    int r = w() - Fl::box_dw(box());;
    if ( (nc > 1) && (tab_pos[nc] > r) ) {
      int wdt = r - l;
      // extreme case: the selected tab is wider than Fl_Tabs itself
      int available = wdt - tab_width[selected];
      if (available <= 8*nc) {
        // if the current tab is so huge that it doesn't fit Fl_Tabs, we make
        // shrink all other tabs to 8 pixels and give the selected tab the rest
        for (i = 0; i < nc; i++) {
          if (i < selected) {
            tab_pos[i] = l + 8*i;
            tab_flags[i] |= 1;
          } else if (i>selected) {
            tab_pos[i] = r - (nc-i)*8;
            tab_flags[i] |= 1;
          } else {
            tab_pos[i] = l + 8*i;;
            tab_flags[i] &= ~1;
          }
          tab_pos[nc] = r;
        }
      } else {
        // This method tries to keep as many visible tabs to the left and right
        // of the selected tab. All other tabs are compressed until they are
        // no smaller than 8 pixels.
        // Overlap to the left and right of the selection is proportional
        // to the left and right total tabs widths.
        // The dynamic of this method is really nice to watch: start FLUID and
        // edit test/tabs. Select any tab and change the label to make the tab
        // wider and smaller. All other tabs will move nicely to make room for
        // the bigger label. Even if two tabs are each wider than Fl_Tabs.
        int overflow = tab_pos[nc] - r;
        int left_total = tab_pos[selected] - l;
        int right_total = tab_pos[nc] - tab_pos[selected+1];
        int left_overflow = left_total+right_total ? overflow * left_total / (left_total+right_total) : overflow;
        int right_overflow = overflow - left_overflow;
        // now clip the left tabs until we compensated overflow on the left
        int xdelta = 0; // accumulate the tab x correction
        for (i=0; i<selected; i++) {          // do this for all tabs on the left of selected
          int tw = tab_width[i];              // get the current width of this tab
          if (left_overflow > 0) {            // do we still need to compensate?
            tw -= left_overflow;              // try to compensate everything
            if (tw < 8) tw = 8;               // but keep a minimum width of 8
            int wdelta = tab_width[i] - tw;   // how many pixels did we actually take?
            left_overflow -= wdelta;          // remove that and keep the remaining overflow
            xdelta += wdelta;                 // accumulate amount of pixel shift
            if (wdelta > 16) tab_flags[i] |= 1; // remove the close button if we overlap too much
          }
          tab_pos[i+1] -= xdelta;             // fix the overlap by moving the tab on the right
        }
        // and clip the right tabs until we compensated overflow on the right
        xdelta = 0;
        for (i=nc-1; i>selected; i--) {
          int tw = tab_width[i];
          if (right_overflow > 0) {
            tw -= right_overflow;
            if (tw < 8) tw = 8;
            int wdelta = tab_width[i] - tw;
            right_overflow -= wdelta;
            xdelta += wdelta;
            // with the close button on the left, overlapping gets more confusing,
            // so remove the button sooner
            if (wdelta > 4) tab_flags[i] |= 1;
          }
          tab_pos[i] -= overflow - xdelta;
        }
        tab_pos[nc] = r;
      }
    }
  }
  return selected;
}

/**
  Return space (height) in pixels usable for tabs.

  The calculated height is the largest space between all children
  and the upper and lower widget boundaries, respectively. If the
  space at the bottom is larger than at the top, the value will be
  negative and the tabs should be placed at the bottom.

  \returns Vertical space that can be used for the tabs.
  \retval  > 0  To put the tabs at the top of the widget.
  \retval  < 0  To put the tabs on the bottom.
  \retval Full height, if children() == 0.
*/
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
 \param event_x, event_y event coordinates
 \return 1 if we hit the close button, and 0 otherwise
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

/**  Determine if the coordinates are in the area of the overflow menu button.

 \param event_x, event_y event coordinates
 \return 1 if we hit the overflow menu button, and 0 otherwise
 */
int Fl_Tabs::hit_overflow_menu(int event_x, int event_y) {
  if (!has_overflow_menu)
    return 0;
  int H = tab_height();
  if (event_x < x()+w()-abs(H)+OV_BORDER)
    return 0;
  if (H >= 0) {
    if (event_y > y()+H)
      return 0;
  } else {
    if (event_y < y()+h()+H)
      return 0;
  }
  return 1;
}

/**  Determine if the coordinates are within the tabs area.

 \param event_x, event_y event coordinates
 \return 1 if we hit the tabs area, and 0 otherwise
 */
int Fl_Tabs::hit_tabs_area(int event_x, int event_y) {
  int H = tab_height();
  if (H >= 0) {
    if (event_y > y()+H)
      return 0;
  } else {
    if (event_y < y()+h()+H)
      return 0;
  }
  if (has_overflow_menu && event_x > x()+w()-abs(H)+OV_BORDER)
    return 0;
  return 1;
}

/**
 Check if the tabs overflow and sets the has_overflow_menu flag accordingly.
 */
void Fl_Tabs::check_overflow_menu() {
  int nc = children();
  int H = tab_height(); if (H < 0) H = -H;
  if (tab_pos[nc] > w()-H+OV_BORDER) {
    has_overflow_menu = 1;
  } else {
    has_overflow_menu = 0;
  }
}

/**
 Take keyboard focus if o is not NULL.
 \param[in] o selected tab
 */
void Fl_Tabs::take_focus(Fl_Widget *o) {
  if (o && Fl::visible_focus() && Fl::focus()!=this) {
    Fl::focus(this);
    redraw_tabs();
  }
}

/**
 Set tab o as selected an call callbacks if needed.
 \param[in] o the newly selected tab
 \return 0 if o is invalide or was deleted by the callback and must no longer be used
 */
int Fl_Tabs::maybe_do_callback(Fl_Widget *o) {
  // chaeck if o is valid
  if ( o == NULL )
    return 0;

  // set the new tab value
  int tab_changed = value(o);
  if ( tab_changed )
    set_changed();

  // do we need to call the callback?
  if ( tab_changed || ( when() & (FL_WHEN_NOT_CHANGED) ) ) {
    Fl_Widget_Tracker wp(o);          // we want to know if the widget lives on
    do_callback(FL_REASON_SELECTED);  // this may delete the tab
    if (wp.deleted()) return 0;       // if it did, return 0
  }

  // if o is still valid, do remaining tasks
  Fl_Tooltip::current(o);
  return 1;
}

/**
 This is called when the user clicks the overflow pulldown menu button.

 This method creates a menu item array that contains the titles of all
 tabs in the Fl_Tabs group. Visible and invisible tabs are separated
 by dividers to indicate their state.

 The menu is then presented until the user selects an item or cancels.
 The chosen tab is then selected and made visible.

 The menu item array is the deleted.
 */
void Fl_Tabs::handle_overflow_menu() {
  int nc = children();
  int H = tab_height(); if (H < 0) H = -H;
  int i, fv=-1, lv=nc; // first and last visible tab
  if (nc <= 0) return;

  // count visible children
  for (i = 0; i < nc; i++) {
    if (tab_pos[i]+tab_offset < 0) fv = i;
    if (tab_pos[i]+tab_width[i]+tab_offset <= w()-H+OV_BORDER) lv = i;
  }

  // create a menu with all children
  Fl_Menu_Item* overflow_menu = new Fl_Menu_Item[nc+1];
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
  const Fl_Menu_Item *m = overflow_menu->popup(x()+w()-H+OV_BORDER, (tab_height()>0)?(y()+H):(y()+h()-OV_BORDER));
  if (m) {
    Fl_Widget *o = (Fl_Widget*)m->user_data();
    push(0);
    take_focus(o);
    maybe_do_callback(o);
  }

  // delete the menu until we need it next time
  if (overflow_menu) {
    delete[] overflow_menu;
    overflow_menu = NULL;
  }
}

/**
 Draw square button-like graphics with a down arrow in the top or bottom right corner.
 */
void Fl_Tabs::draw_overflow_menu_button() {
  int H = tab_height();
  int X, Y;
  if (H > 0) {
    X = x() + w() - H + OV_BORDER;
    if (OV_BORDER > 0)
      fl_rectf(X, y(), H - OV_BORDER, OV_BORDER, color());
    Y = y() + OV_BORDER;
  } else {
    H = -H;
    X = x() + w() - H + OV_BORDER;
    Y = y() + h() - H;
    if (OV_BORDER > 0)
      fl_rectf(X, Y + H - OV_BORDER, H - OV_BORDER, OV_BORDER, color());
  }
  H -= OV_BORDER;
  draw_box(box(), X, Y, H, H, color());
  Fl_Rect r(X, Y, H, H);
  // labelcolor() is historically used to contrast selectioncolor() and is
  // useless her, so we fall back to contrast the background color on the
  // gray ramp.
  Fl_Color arrow_color = fl_contrast(FL_GRAY_RAMP+0, color());
  if (!active_r())
    arrow_color = fl_inactive(arrow_color);
  fl_draw_arrow(r, FL_ARROW_CHOICE, FL_ORIENT_NONE, arrow_color);
}

/**
  Redraw all tabs (and only the tabs).

  This method sets the Fl_Tab's damage flags so the tab area is redrawn.
*/
void Fl_Tabs::redraw_tabs() {
  int H = tab_height();
  if (H >= 0) {
    damage(FL_DAMAGE_EXPOSE, x(), y(), w(), H + SELECTION_BORDER);
  } else {
    H = -H;
    damage(FL_DAMAGE_EXPOSE, x(), y() + h() - H - SELECTION_BORDER, w(), H + SELECTION_BORDER);
  }
}

/**
 Handle all events in the tabs area and forward the rest to the selected child.

 \param[in] event handle this event
 \return 1 if the event was handled
 */
int Fl_Tabs::handle(int event) {
  static int initial_x = 0;
  static int initial_tab_offset = 0;
  static int forward_motion_to_group = 0;
  static Fl_Widget *o_push_drag = NULL;
  Fl_Widget *o;
  int i;

  switch (event) {

  case FL_MOUSEWHEEL:
    if (   ( (overflow_type == OVERFLOW_DRAG) || (overflow_type == OVERFLOW_PULLDOWN) )
        && hit_tabs_area(Fl::event_x(), Fl::event_y()) ) {
      int original_tab_offset = tab_offset;
      tab_offset -= 2 * Fl::event_dx();
      if (tab_offset > 0)
        tab_offset = 0;
      int m = 0;
      if (overflow_type == OVERFLOW_PULLDOWN) m = abs(tab_height());
      int dw = tab_pos[children()] + tab_offset - w();
      if (dw < -m)
        tab_offset -= dw+m;
      if (tab_offset != original_tab_offset)
        redraw_tabs();
      return 1;
    }
    return Fl_Group::handle(event);
  case FL_PUSH:
    initial_x = Fl::event_x();
    initial_tab_offset = tab_offset;
    forward_motion_to_group = 0;
    if (hit_overflow_menu(Fl::event_x(), Fl::event_y())) {
      handle_overflow_menu();
      return 1;
    }
    if (!hit_tabs_area(Fl::event_x(), Fl::event_y())) {
      forward_motion_to_group = 1;
    }
    /* FALLTHROUGH */
  case FL_DRAG:
      o_push_drag = which(Fl::event_x(), Fl::event_y());
  case FL_RELEASE:
    if (forward_motion_to_group) {
      return Fl_Group::handle(event);
    }
    o = which(Fl::event_x(), Fl::event_y());
    if (event == FL_RELEASE && o != o_push_drag) { // see issue #1075
      return 1;
    }
    if ( (overflow_type == OVERFLOW_DRAG) || (overflow_type == OVERFLOW_PULLDOWN) ) {
      if (tab_pos[children()] < w() && tab_offset == 0) {
        // fall through
      } else if (!Fl::event_is_click()) {
        tab_offset = initial_tab_offset + Fl::event_x() - initial_x;
        int m = 0;
        if (overflow_type == OVERFLOW_PULLDOWN) m = abs(tab_height()) - OV_BORDER;
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
        redraw_tabs();
        return 1;
      }
    }
    if (event == FL_RELEASE) {
      push(0);
      take_focus(o);
      if (o && (o->when() & FL_WHEN_CLOSED) && hit_close(o, Fl::event_x(), Fl::event_y())) {
        o->do_callback(FL_REASON_CLOSED);
        return 1; // o may be deleted at this point
      }
      maybe_do_callback(o);
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
  // always make sure that the selected tab is visible
  if (   (selected >= 0)
      && ( (overflow_type == OVERFLOW_DRAG)
        || (overflow_type == OVERFLOW_PULLDOWN) ) ) {
    int m = MARGIN;
    if ( (selected == 0) || (selected == children()-1) ) m = BORDER;
    int mr = m;
    tab_positions();
    if (overflow_type == OVERFLOW_PULLDOWN) mr += abs(tab_height() - OV_BORDER);
    if (tab_pos[selected]+tab_width[selected]+tab_offset+mr > w()) {
      tab_offset = w() - tab_pos[selected] - tab_width[selected] - mr;
    } else if (tab_pos[selected]+tab_offset-m < 0) {
      tab_offset = -tab_pos[selected]+m;
    }
  }
  redraw_tabs();
  return ret;
}

/**
 Draw the tabs area, the optional pulldown button, and all children.
 */
void Fl_Tabs::draw() {
  //
  //  FL_DAMAGE_CHILD   : this is set if any of the children asked for a redraw
  //                      Fl_Tabs forwards this by calling `update_child(v)`
  //  FL_DAMAGE_EXPOSE  : this is set if some setting in a widget changed
  //                      Fl_Tabs uses this to indicate that the tabs area needs a full redraw
  //  FL_DAMAGE_SCROLL  : this is used as a custom flag in various widgets
  //                      Fl_Tabs uses FL_DAMAGE_EXPOSE to indicate that the
  //                      tabs bar needs repositioning and teh tabs must be
  //                      redrawn
  //  FL_DAMAGE_ALL     : just recalculate and redraw everything

  // Anatomy of tabs on top:
  //  +------+                    <<-- selected tabs start at y()
  //  | text | +------+    +---+   <-- other tabs are offset down by BORDER
  //  |      | | text |    | ▼ |   <-- the pulldown button width equals H - OV_BORDER
  // ++      +-------------+---+   <-- tab_height() to tab_height + Fl::box_dx(box())
  // +-------------------------+   <-- tab_height + SELECTION_BORDER
  // |                         |
  //   ↑____↑ this area within the SELECTION_BORDER is called "stem"
  //
  // tab_height() calculates the distance from y() to the "highest" child.
  // Note that the SELECTION_BORDER bleeds into the child area!
  // Note that the clear area under the selected tab also bleeds into the child.
  // Note that children have FL_NO_BOX and Fl_Tabs must draw the background.
  //
  // Horizontally, we start tabs at x() + Fl::box_dx()
  // When uncompressed, the space between tabs is EXTRASPACE
  // EXTRAGAP is the space between the close cross and the label
  // MARGIN is the minimal distance to the left and right edge of Fl_Tabs for
  //  the selected tabs if the overflow mode allows scrolling

  if (children() == 0) {
    fl_rectf(x(), y(), w(), h(), color());
    if (align() & FL_ALIGN_INSIDE)
      draw_label();
    clear_damage();
    return;
  }

  Fl_Widget *selected_child = value();        // return the first visible child and hide all others
  tab_positions();
  int selected = find(selected_child);        // find that child in the list and return 0..children()-1
  if (selected == children()) selected = -1;  // if anything fails, selected is -1 and
  int H = tab_height();
  Fl_Color selected_tab_color = selected_child ? selected_child->color() : color();
  bool tabs_at_top = (H > 0);
  bool colored_selection_border = (selection_color() != selected_tab_color);

  int tabs_y, tabs_h;
  int child_area_y, child_area_h;
  int clipped_child_area_y, clipped_child_area_h;
  int selection_border_y, selection_border_h;

  selection_border_h = colored_selection_border ? SELECTION_BORDER : Fl::box_dx(box());

  if (tabs_at_top) {
    tabs_h = H;
    tabs_y = y();
    selection_border_y = y() + tabs_h;
    child_area_y = y() + tabs_h;
    child_area_h = h() - tabs_h;
    clipped_child_area_y = y() + tabs_h + selection_border_h;
    clipped_child_area_h = h() - tabs_h - selection_border_h;
  } else {
    tabs_h = -H;
    tabs_y = y() + h() - tabs_h;
    selection_border_y = tabs_y - selection_border_h;
    child_area_y = y();
    child_area_h = h() - tabs_h;
    clipped_child_area_y = y();
    clipped_child_area_h = h() - tabs_h - selection_border_h;
  }

  // ---- recalculate the tabs so that the selected tab is visible
  if (damage() & (FL_DAMAGE_ALL|FL_DAMAGE_SCROLL)) {
    Fl_Widget *selected_tab = value();
    if (selected_tab)
      value(selected_tab);
  }

  // ---- draw the tabs and the selection border
  if (damage() & (FL_DAMAGE_ALL|FL_DAMAGE_EXPOSE|FL_DAMAGE_SCROLL))
  {
    // -- draw tabs background
    if (parent()) {
      Fl_Widget *p = parent();
      fl_push_clip(x(), tabs_y, w(), tabs_h);
      if (Fl_Window *win = p->as_window()) {
        fl_draw_box(p->box(), 0, 0, p->w(), p->h(), p->color());
        win->draw_backdrop();
      } else {
        fl_draw_box(p->box(), p->x(), p->y(), p->w(), p->h(), p->color());
      }
      fl_pop_clip();
    } else {
      fl_rectf(x(), tabs_y, w(), tabs_h, color());
    }

    // -- draw selection border
    fl_push_clip(x(), selection_border_y, w(), selection_border_h);
    if (colored_selection_border) {
      draw_box(box(), x(), y(), w(), h(), selected_tab_color);
      draw_box(box(), x(), selection_border_y, w(), selection_border_h, selection_color());
    } else {
      draw_box(box(), x(), child_area_y, w(), child_area_h, selected_tab_color);
    }
    // draw the stem, the area that reaches from the tab into the selection border
    if (selected != -1) {
      int stem_x = x() + tab_pos[selected] + tab_offset;
      int stem_w = fl_min(tab_pos[selected+1] - tab_pos[selected], tab_width[selected]);
      if (colored_selection_border) {
        if (tabs_at_top)
          fl_rectf(stem_x, selection_border_y, stem_w, selection_border_h/2, selection_color());
        else
          fl_rectf(stem_x, selection_border_y+selection_border_h-selection_border_h/2, stem_w, selection_border_h/2, selection_color());
      } else {
        fl_rectf(stem_x, child_area_y-tabs_h, stem_w, child_area_h+2*tabs_h, selection_color());
      }
    }
    fl_pop_clip();

    // -- draw all tabs
    fl_push_clip(x(), tabs_y, w(), tabs_h);
    int i, clip_left, clip_right;
    int safe_selected = selected == -1 ? children() : selected;
    // draw all tabs from the leftmost up to the selected one, stacking them
    // visually as needed. The clipping assures that no tabs shine through gaps
    // between tabs.
    clip_left = x();
    for (i=0; i<safe_selected; i++) {
      clip_right = (i<tab_count-1) ? x()+(tab_offset+tab_pos[i+1]+tab_width[i+1]/2) : x() + w();
      fl_push_clip(clip_left, tabs_y, clip_right-clip_left, tabs_h);
      draw_tab(x()+tab_pos[i], x()+tab_pos[i+1],
               tab_width[i], H, child(i), tab_flags[i], LEFT);
      fl_pop_clip();
    }
    // draw all tabs from the rightmost back to the selected one, also visually stacking them
    clip_right = x() + w();
    for (i=children()-1; i > safe_selected; i--) {
      clip_left = (i>0) ? (tab_offset+tab_pos[i]-tab_width[i-1]/2) : x();
      fl_push_clip(clip_left, tabs_y, clip_right-clip_left, tabs_h);
      draw_tab(x()+tab_pos[i], x()+tab_pos[i+1],
               tab_width[i], H, child(i), tab_flags[i], RIGHT);
      fl_pop_clip();
    }
    // if there is a selected tab, draw it last over all other tabs
    if (selected > -1)
      draw_tab(x()+tab_pos[selected], x()+tab_pos[selected+1],
               tab_width[selected], H, selected_child, tab_flags[selected], SELECTED);
    fl_pop_clip();

    // -- draw the overflow menu button
    if (overflow_type == OVERFLOW_PULLDOWN)
      check_overflow_menu();
    if (has_overflow_menu)
      draw_overflow_menu_button();
  }

  // ---- draw the child area
  if (damage() & (FL_DAMAGE_ALL|FL_DAMAGE_CHILD)) {
    // clip to area below selection border
    fl_push_clip(x(), clipped_child_area_y, w(), clipped_child_area_h);
    if (damage() & (FL_DAMAGE_ALL)) {
      // draw the box and background around the child
      if (colored_selection_border)
        draw_box(box(), x(), y(), w(), h(), selected_tab_color);
      else
        draw_box(box(), x(), child_area_y, w(), child_area_h, selected_tab_color);
      // force draw the selected child
      if (selected_child)
        draw_child(*selected_child);
    } else if (damage() & (FL_DAMAGE_CHILD)) {
      // draw the selected child
      if (selected_child)
        update_child(*selected_child);
    }
    // stop clipping
    fl_pop_clip();
  }

  clear_damage();
}

/**
 Draw a tab in the top or bottom tabs area.

 Tabs can be selected, or on the left or right side of the selected tab. If
 overlapping, left tabs are drawn bottom to top using clipping. The selected
 tab is then the topmost, followed by the right side tabs drawn top to bottom.

 Tabs with the FL_WHEN_CLOSE bit set will draw a cross on their left side
 only if they are not compressed/overlapping.

 \param[in] x1 horizontal position of the left visible edge of the tab
 \param[in] x2 horizontal position of the following tab
 \param[in] W, H width and height of the tab
 \param[in] o the child widget that corresponds to this tab
 \param[in] flags if bit 1 is set, this tab is overlapped by another tab
 \param[in] what can be LEFT, SELECTED, or RIGHT to indicate if the tab is to
    the left side or the right side of the selected tab, or the selected tab itself
 */
void Fl_Tabs::draw_tab(int x1, int x2, int W, int H, Fl_Widget* o, int flags, int what) {
  x1 += tab_offset;
  x2 += tab_offset;
  int sel = (what == SELECTED);
  int dh = Fl::box_dh(box());
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
    H += dh;

    draw_box(bt, x1, y() + yofs, W, H + 10 - yofs, bc);

    // Draw the label using the current color...
    o->labelcolor(sel ? labelcolor() : o->labelcolor());

    // Draw the "close" button if requested
    if ( (o->when() & FL_WHEN_CLOSED) && !(flags & 1) ) {
      int sz = labelsize()/2, sy = (H - sz)/2;
      Fl_Color close_color = fl_contrast(FL_GRAY_RAMP+0, bc);
      if (!active_r())
        close_color = fl_inactive(close_color);
      fl_draw_symbol("@3+", x1 + EXTRASPACE/2, y() + yofs/2 + sy, sz, sz, close_color);
      wc = sz + EXTRAGAP;
    }

    // Draw the label text
    o->draw_label(x1 + wc, y() + yofs, W - wc, H - yofs, tab_align());

    // Draw the focus box
    if (Fl::focus() == this && o->visible())
      draw_focus(bt, x1, y(), W, H, bc);
  } else {
    H = -H;
    H += dh;

    draw_box(bt, x1, y() + h() - H - 10, W, H + 10 - yofs, bc);

    // Draw the label using the current color...
    o->labelcolor(sel ? labelcolor() : o->labelcolor());

    // Draw the "close" button if requested
    if ( (o->when() & FL_WHEN_CLOSED) && (x1+W < x2) ) {
      int sz = labelsize()/2, sy = (H - sz)/2;
      Fl_Color close_color = fl_contrast(FL_GRAY_RAMP+0, bc);
      if (!active_r())
        close_color = fl_inactive(close_color);
      fl_draw_symbol("@3+", x1 + EXTRASPACE/2, y() + h() - H -yofs/2 + sy, sz, sz, close_color);
      wc = sz + EXTRAGAP;
    }

    // Draw the label text
    o->draw_label(x1 + wc, y() + h() - H, W - wc, H - yofs, tab_align());

    // Draw the focus box
    if (Fl::focus() == this && o->visible())
      draw_focus(bt, x1, y()+h()-H+1, W, H, bc);
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
}

/**
 Delete allocated resources and destroy all children.
 */
Fl_Tabs::~Fl_Tabs() {
  clear_tab_positions();
}

/**
  Returns the position and size available to be used by its children.

  If there isn't any child yet the \p tabh parameter will be used to
  calculate the return values. This assumes that the children's labelsize
  is the same as the Fl_Tabs' labelsize and adds a small border.

  If there are already children, the values of child(0) are returned, and
  \p tabh is ignored.

  \note Children should always use the same positions and sizes.

  \note This function requires access to the display drivers to determine the
  selected font height. If `client_area` is invoked before the first window is
  displayed, ensure that `fl_open_display()` is called beforehand.

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

/**
  Clear internal array of tab positions and widths.

  \see tab_positions().
*/
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

 \see OVERFLOW_COMPRESS, OVERFLOW_CLIP, OVERFLOW_PULLDOWN, OVERFLOW_DRAG
 */
void Fl_Tabs::handle_overflow(int ov) {
  overflow_type = ov;
  tab_offset = 0;
  has_overflow_menu = 0;
  damage(FL_DAMAGE_SCROLL);
  redraw();
}

