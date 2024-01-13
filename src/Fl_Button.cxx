//
// Button widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2014 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file. If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>

#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Toggle_Button.H>


Fl_Widget_Tracker *Fl_Button::key_release_tracker = 0;


// There are a lot of subclasses, named Fl_*_Button.  Some of
// them are implemented by setting the type() value and testing it
// here.  This includes Fl_Radio_Button and Fl_Toggle_Button

/**
  Sets the current value of the button.
  A non-zero value sets the button to 1 (ON), and zero sets it to 0 (OFF).
  \param[in] v button value.
  \see set(), clear()
 */
int Fl_Button::value(int v) {
  v = v ? 1 : 0;
  oldval = v;
  clear_changed();
  if (value_ != v) {
    value_ = v;
    if (box()) redraw();
    else redraw_label();
    return 1;
  } else {
    return 0;
  }
}

/**
  Turns on this button and turns off all other radio buttons in the group
  (calling \c value(1) or \c set() does not do this).
 */
void Fl_Button::setonly() { // set this radio button on, turn others off
  value(1);
  Fl_Group* g = parent();
  Fl_Widget*const* a = g->array();
  for (int i = g->children(); i--;) {
    Fl_Widget* o = *a++;
    if (o != this && o->type()==FL_RADIO_BUTTON) ((Fl_Button*)o)->value(0);
  }
}

void Fl_Button::draw() {
  if (type() == FL_HIDDEN_BUTTON) return;
  Fl_Color col = value() ? selection_color() : color();
  Fl_Boxtype bt = value() ? (down_box()?down_box():fl_down(box())) : box();
  if (compact_ && parent()) {
    Fl_Widget *p = parent();
    int px, py, pw = p->w(), ph = p->h();
    if (p->as_window()) { px = 0; py = 0; } else { px = p->x(); py = p->y(); }
    fl_push_clip(x(), y(), w(), h());
    draw_box(bt, px, py, pw, ph, col);
    fl_pop_clip();
    const int hh = 5, ww = 5;
    Fl_Color divider_color = fl_gray_ramp(FL_NUM_GRAY/3);
    if (!active_r())
      divider_color = fl_inactive(divider_color);
    if (x()+w() != px+pw) {
      fl_color(divider_color);
      fl_yxline(x()+w()-1, y()+hh, y()+h()-1-hh);
    }
    if (y()+h() != py+ph) {
      fl_color(divider_color);
      fl_xyline(x()+ww, y()+h()-1, x()+w()-1-ww);
    }
  } else {
    draw_box(bt, col);
  }
  draw_backdrop();
  if (labeltype() == FL_NORMAL_LABEL && value()) {
    Fl_Color c = labelcolor();
    labelcolor(fl_contrast(c, col));
    draw_label();
    labelcolor(c);
  } else draw_label();
  if (Fl::focus() == this) draw_focus();
}

int Fl_Button::handle(int event) {
  static bool s_key_repeat = false;
  int newval;
  switch (event) {
  case FL_ENTER: /* FALLTHROUGH */
  case FL_LEAVE:
//  if ((value_?selection_color():color())==FL_GRAY) redraw();
    return 1;
  case FL_PUSH:
    if (Fl::visible_focus() && handle(FL_FOCUS)) Fl::focus(this);
    /* FALLTHROUGH */
  case FL_DRAG:
    if (Fl::event_inside(this)) {
      if (type() == FL_RADIO_BUTTON) newval = 1;
      else newval = !oldval;
    } else {
      clear_changed();
      newval = oldval;
    }
    if (newval != value_) {
      value_ = newval;
      set_changed();
      redraw();
      if ((type() == 0) && (when() & FL_WHEN_CHANGED)) do_callback(FL_REASON_CHANGED);
    }
    return 1;
  case FL_RELEASE:
    if (value_ == oldval) {
      clear_changed();
    } else {
      Fl_Widget_Tracker wp(this);
      if (type() == FL_RADIO_BUTTON) {
        setonly();
      } else if (type() == FL_TOGGLE_BUTTON) {
        oldval = value_;
      } else {
        value(oldval);
      }
      set_changed();
    }
    if (changed() && (when() & FL_WHEN_CHANGED))
      do_callback(FL_REASON_CHANGED);
    else if (!changed() && (when() & FL_WHEN_NOT_CHANGED))
      do_callback(FL_REASON_SELECTED);
    else if (when() & FL_WHEN_RELEASE)
      do_callback(FL_REASON_RELEASED);
    return 1;
  case FL_SHORTCUT:
    if (!(shortcut() ?
          Fl::test_shortcut(shortcut()) : test_shortcut())) return 0;
    if (Fl::visible_focus() && handle(FL_FOCUS)) Fl::focus(this);
    goto triggered_by_keyboard;
  case FL_FOCUS :
  case FL_UNFOCUS :
    if (Fl::visible_focus()) {
      if (box() == FL_NO_BOX) {
        // Widgets with the FL_NO_BOX boxtype need a parent to
        // redraw, since it is responsible for redrawing the
        // background...
        int X = x() > 0 ? x() - 1 : 0;
        int Y = y() > 0 ? y() - 1 : 0;
        if (window()) window()->damage(FL_DAMAGE_ALL, X, Y, w() + 2, h() + 2);
      } else redraw();
      return 1;
    } else return 0;
    /* NOTREACHED */
  case FL_KEYUP:
    s_key_repeat = false;
    return 0;
  case FL_KEYBOARD :
    if (Fl::focus() == this && Fl::event_key() == ' ' &&
        !(Fl::event_state() & (FL_SHIFT | FL_CTRL | FL_ALT | FL_META))) {
    triggered_by_keyboard:
      if (s_key_repeat) return 1;
      Fl_Widget_Tracker wp(this);
       if (type() == FL_RADIO_BUTTON) {
        if (!value_) {
          setonly();
          set_changed();
        } else {
          clear_changed();
        }
      } else if (type() == FL_TOGGLE_BUTTON) {
        value(!value());
        set_changed();
      } else {
        set_changed();
      }

      if (changed() && (when() & FL_WHEN_CHANGED))
        do_callback(FL_REASON_CHANGED);
      else if (!changed() && (when() & FL_WHEN_NOT_CHANGED)) 
        do_callback(FL_REASON_SELECTED);
      else if (when() & FL_WHEN_RELEASE) 
        do_callback(FL_REASON_RELEASED);
      s_key_repeat = true;
      if (wp.deleted()) return 1; // leave if the widget was deleted in the callback

      if ((type() != FL_RADIO_BUTTON) && (type() != FL_TOGGLE_BUTTON)) {
        simulate_key_action(); // for visual feedback only
      }
      return 1;
    }
    /* FALLTHROUGH */
  default:
    return 0;
  }
  return 0;
}

void Fl_Button::simulate_key_action()
{
  if (key_release_tracker) {
    Fl::remove_timeout(key_release_timeout, key_release_tracker);
    key_release_timeout(key_release_tracker);
  }
  value(1);
  redraw();
  key_release_tracker = new Fl_Widget_Tracker(this);
  Fl::add_timeout(0.15, key_release_timeout, key_release_tracker);
}

void Fl_Button::key_release_timeout(void *d)
{
  Fl_Widget_Tracker *wt = (Fl_Widget_Tracker*)d;
  if (!wt)
    return;
  if (wt==key_release_tracker)
    key_release_tracker = 0L;
  Fl_Button *btn = (Fl_Button*)wt->widget();
  if (btn) {
    btn->value(0);
    btn->redraw();
  }
  delete wt;
}

/**
  The constructor creates the button using the given position, size, and label.

  The default box type is box(FL_UP_BOX).

  You can control how the button is drawn when ON by setting down_box().
  The default is FL_NO_BOX (0) which will select an appropriate box type
  using the normal (OFF) box type by using fl_down(box()).

  Derived classes may handle this differently.

  A button may request callbacks by setting bits in the \p when() bitfield.
  Only one callback is called for any one event. If multiple bits are set,
  only the first callback in the list below will be called.

  If the `FL_WHEN_CHANGED` bit is set, the callback is called when the mouse
  button is released, the shortcut key was pressed, or the button has focus
  and the space bar was pressed, and the value of the button changed. A regular
  button (not a radio or toggle button) callback is also triggered by key
  repeat events. The callback reason is set to `FL_REASON_CHANGED`.
  `Fl_Button::changed()` is !=0, and `Fl_Button::value()` is set to the new
  value for radio and check buttons.

  If `FL_WHEN_NOT_CHANGED` is set, the callback is called for the same events
  as above, but only if the value did *not* change. The callback reason is
  `FL_REASON_SELECTED`. This bit is usually combined with other bits in `when()`.

  The default setting is `FL_WHEN_RELEASE`. If this flag is set, the callback is
  called when the mouse button or the shortcut key is released. Toggle and radio
  button callbacks are called, even if the value did not change, however the
 `Fl_Button::changed()` flag is set accordingly. The callback reason is given
  as `FL_REASON_RELEASED`.

  When a radio button changes other radio buttons in the same group, only the
  user activated button will trigger its callback according to the flags above.
  The other widgets in the group will change their value, but not call their
  corresponding callbacks.

  \param[in] X, Y, W, H position and size of the widget
  \param[in] L widget label, default is no label
 */
Fl_Button::Fl_Button(int X, int Y, int W, int H, const char *L)
: Fl_Widget(X,Y,W,H,L),
  shortcut_(0),
  value_(0),
  oldval(0),
  down_box_(FL_NO_BOX),
  compact_(0)
{
  box(FL_UP_BOX);
  set_flag(SHORTCUT_LABEL);
}

/**
  The constructor creates the button using the given position, size, and label.

  The Button type() is set to FL_RADIO_BUTTON.

  \param[in] X, Y, W, H position and size of the widget
  \param[in] L widget label, default is no label
 */
Fl_Radio_Button::Fl_Radio_Button(int X,int Y,int W,int H,const char *L)
: Fl_Button(X, Y, W, H, L) {
  type(FL_RADIO_BUTTON);
}

/**
  The constructor creates the button using the given position, size, and label.

  The Button type() is set to FL_TOGGLE_BUTTON.

  \param[in] X, Y, W, H position and size of the widget
  \param[in] L widget label, default is no label
 */
Fl_Toggle_Button::Fl_Toggle_Button(int X,int Y,int W,int H,const char *L)
: Fl_Button(X,Y,W,H,L)
{
  type(FL_TOGGLE_BUTTON);
}

/**
 Decide if buttons should be rendered in compact mode.

 \image html compact_buttons_gtk.png "compact button keypad using GTK+ Scheme"
 \image latex compact_buttons_gtk.png "compact button keypad using GTK+ Scheme" width=4cm

 \image html compact_buttons_gleam.png "compact buttons in Gleam"
 \image latex compact_buttons_gleam.png "compact buttons in Gleam" width=4cm

 In compact mode, the button's surrounding border is altered to visually signal
 that multiple buttons are functionally linked together. To ensure the correct
 rendering of buttons in compact mode, all buttons must be part of the same
 group, positioned close to each other, and aligned with the edges of the
 group. Any button outlines not in contact with the parent group's outline
 will be displayed as separators.

 \param[in] v switch compact mode on (1) or off (0)
 */
void Fl_Button::compact(uchar v) { compact_ = v; }
