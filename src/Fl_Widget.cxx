//
// Base widget class for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Tooltip.H>
#include <FL/fl_draw.H>
#include <FL/fl_string_functions.h>
#include <stdlib.h>
#include "flstring.h"


////////////////////////////////////////////////////////////////
// for compatibility with Forms, all widgets without callbacks are
// inserted into a "queue" when they are activated, and the forms
// compatibility interaction functions (fl_do_events, etc.) will
// read one widget at a time from this queue and return it:

const int QUEUE_SIZE = 20;

static Fl_Widget *obj_queue[QUEUE_SIZE];
static int obj_head, obj_tail;

void Fl_Widget::default_callback(Fl_Widget *widget, void * /*v*/) {
  obj_queue[obj_head++] = widget;
  if (obj_head >= QUEUE_SIZE) obj_head = 0;
  if (obj_head == obj_tail) {
    obj_tail++;
    if (obj_tail >= QUEUE_SIZE) obj_tail = 0;
  }
}
/**
    Reads the default callback queue and returns the first widget.

    All Fl_Widgets that don't have a callback defined use the default
    callback \p static Fl_Widget::default_callback() that puts a pointer
    to the widget in a queue. This method reads the oldest widget out
    of this queue.

    The queue (FIFO) is limited (currently 20 items). If the queue
    overflows, the oldest entry (Fl_Widget *) is discarded.

    Relying on the default callback and reading the callback queue with
    Fl::readqueue() is not recommended. If you need a callback, you should
    set one with Fl_Widget::callback(Fl_Callback *cb, void *data)
    or one of its variants.

    \see Fl_Widget::callback()
    \see Fl_Widget::callback(Fl_Callback *cb, void *data)
    \see Fl_Widget::default_callback()
*/
Fl_Widget *Fl::readqueue() {
  if (obj_tail==obj_head) return 0;
  Fl_Widget *widget = obj_queue[obj_tail++];
  if (obj_tail >= QUEUE_SIZE) obj_tail = 0;
  return widget;
}
/*
    This static internal function removes all pending callbacks for a
    specific widget from the default callback queue (Fl::readqueue()).
    It is only called from Fl_Widget's destructor if the widget
    doesn't have an own callback.
    Note: There's no need to have this in the Fl:: namespace.
*/
static void cleanup_readqueue(Fl_Widget *w) {

  if (obj_tail==obj_head) return;

  // Read the entire queue and copy over all valid entries.
  // The new head will be determined after the last copied entry.

  int old_head = obj_head;      // save newest entry
  int entry = obj_tail;         // oldest entry
  obj_head = obj_tail;          // new queue start
  for (;;) {
    Fl_Widget *o = obj_queue[entry++];
    if (entry >= QUEUE_SIZE) entry = 0;
    if (o != w) { // valid entry
      obj_queue[obj_head++] = o;
      if (obj_head >= QUEUE_SIZE) obj_head = 0;
    } // valid entry
    if (entry == old_head) break;
  }
  return;
}
////////////////////////////////////////////////////////////////

int Fl_Widget::handle(int) {
  return 0;
}

/** Default font size for widgets */
Fl_Fontsize FL_NORMAL_SIZE = 14;

Fl_Widget::Fl_Widget(int X, int Y, int W, int H, const char* L) {

  x_ = X; y_ = Y; w_ = W; h_ = H;

  label_.value   = L;
  label_.image   = 0;
  label_.deimage = 0;
  label_.type    = FL_NORMAL_LABEL;
  label_.font    = FL_HELVETICA;
  label_.size    = FL_NORMAL_SIZE;
  label_.color   = FL_FOREGROUND_COLOR;
  label_.align_  = FL_ALIGN_CENTER;
  tooltip_       = 0;
  callback_      = default_callback;
  user_data_     = 0;
  type_          = 0;
  flags_         = VISIBLE_FOCUS;
  damage_        = 0;
  box_           = FL_NO_BOX;
  color_         = FL_GRAY;
  color2_        = FL_GRAY;
  when_          = FL_WHEN_RELEASE;

  parent_ = 0;
  if (Fl_Group::current()) Fl_Group::current()->add(this);
}

/** Updates the size and position of widgets using hints and requests.

 parametric_resize() handles additional information in the width and height
 parameters of the resize request. Size request are created using predifined
 macros and an optional delta value. For example,
 `size(FL_CURRENT_SIZE+10, FL_LABEL_SIZE+10)` will resize the widget so that it
 contains its label, wrapping at the current widget width, with a 10 pixel
 margin added to the width and height. Deltas are always applied last.

 Currently implemented are the following combinations:

 | width | height |  |
 |-------|--------|--|
 | `int` or FL_CURRENT_SIZE | `int` or FL_CURRENT_SIZE | use the current size of the widget, possibly adding a delta value |
 | `int` or FL_CURRENT_SIZE | FL_LABEL_SIZE | calculate the label height, wrapping the label at `width` |
 | FL_LABEL_SIZE | FL_LABEL_SIZE | resize the widget, so it fits around the label text |

 The size request can be or'd with the `FL_GROW` or `FL_SHRINK` hint, requesting
 that the widget can only grow or shrink in size.

 Planned requests include `FL_TEXT_SIZE` for `Fl_Input_`, `Fl_Text_Display`, and
 derived widgets, and `FL_CHILDREN_SIZE` and `FL_CHILDREN_FIT` for widgets
 derived from `Fl_Group`.

 It may make sense to add requests for sibling coordinates, for example
 `FL_SIBLING_BOTTOM` to generate the widget y coordinate below the previous
 widget in the group.

 For groups, it may make sense to differentiate between resizing only the group
 widget, and resizing and changing the layout of the children. This could be
 an additional `FL_GROW_...` hint.

 This feature uses the fact that screen coordinates are not useful
 beyond a 20 bit integer size. As the arguments for resize() are 32 bit, we can
 use some of the remaining 12 bits to encode resizing hints and requests.
 ```
 32        24        16        8         0
 |----+----|----+----|----+----|----+----|
 |0kkk|kkkk|rrgg|dddd|dddd|dddd|dddd|dddd|
```
 - bit 31 is 0 so negative coordinates don't spoil this scheme
 - k = kind of request, 1..127, so that any positive value with any of bits
 24...30 set is marked as a resize request.
 - r = reserved
 - g = grow bits, if set for width or height, the widget and only shrink (1)
 or grow(2) in size
 - d = delta - 0x80000, and easy way to adjust the resulting size in a last step

 \param[inout] X, Y widget position
 \param[inout] W, H requests a widget size using tokens and returns the
    calculated size
 */
void Fl_Widget::parametric_resize(int &X, int &Y, int &W, int &H) {
  int ww = 0, hh = 0;
  int w_type = W >> 24;
  int w_grow = (W >> 20) & 3;
  int w_delta = w_type ? (W & 0xfffff) - 0x80000 : 0;
  int h_type = H >> 24;
  int h_grow = (H >> 20) & 3;
  int h_delta = h_type ? (H & 0xfffff) - 0x80000 : 0;
  if (h_type == (FL_LABEL_SIZE>>24)) {  // label size
    if (w_type == 0) ww = W;
    else if (w_type == (FL_CURRENT_SIZE>>24)) ww = w_;
    measure_label(ww, hh);
    if (w_type == 0) ww = W;
    else if (w_type == (FL_CURRENT_SIZE>>24)) ww = w_;
  }
  hh += h_delta;
  if (h_grow==0 || (h_grow==1 && hh<h_) || (h_grow==2 && hh>h_)) H = hh;
  else H = h_;
  ww += w_delta;
  if (w_grow==0 || (w_grow==1 && ww<w_) || (w_grow==2 && ww>w_)) W = ww;
  else W = w_;
}

void Fl_Widget::resize(int X, int Y, int W, int H) {
  if (((W > 0) && (W & 0x7f000000)) || ((H > 0) && (H & 0x7f000000)))
    parametric_resize(X, Y, W, H);
  x_ = X; y_ = Y; w_ = W; h_ = H;
}

// this is useful for parent widgets to call to resize children:
int Fl_Widget::damage_resize(int X, int Y, int W, int H) {
  if (x() == X && y() == Y && w() == W && h() == H) return 0;
  resize(X, Y, W, H);
  redraw();
  return 1;
}

int Fl_Widget::take_focus() {
  if (!takesevents()) return 0;
  if (!visible_focus()) return 0;
  if (!handle(FL_FOCUS)) return 0; // see if it wants it
  if (contains(Fl::focus())) return 1; // it called Fl::focus for us
  Fl::focus(this);
  return 1;
}

extern void fl_throw_focus(Fl_Widget*); // in Fl_x.cxx

/*
   Destroys the widget, taking care of throwing focus before if any.
   Destruction removes the widget from any parent group! And groups when
   destroyed destroy all their children. This is convenient and fast.
*/
Fl_Widget::~Fl_Widget() {
  Fl::clear_widget_pointer(this);
  if (flags() & COPIED_LABEL) free((void *)(label_.value));
  if (flags() & COPIED_TOOLTIP) free((void *)(tooltip_));
  image(NULL);
  deimage(NULL);
  // remove from parent group
  if (parent_) parent_->remove(this);
#ifdef DEBUG_DELETE
  if (parent_) { // this should never happen
    printf("*** Fl_Widget: parent_->remove(this) failed [%p,%p]\n",parent_,this);
  }
#endif // DEBUG_DELETE
  parent_ = 0; // Don't throw focus to a parent widget.
  fl_throw_focus(this);
  // remove stale entries from default callback queue (Fl::readqueue())
  if (callback_ == default_callback) cleanup_readqueue(this);
  if ( (flags_ & AUTO_DELETE_USER_DATA) && user_data_)
    delete (Fl_Callback_User_Data*)user_data_;
}

/**
  Draws a focus box for the widget at the given position and size.

  This method does nothing if
  - the global option Fl::visible_focus() or
  - the per-widget option visible_focus()
  is false (off).

  This means that Fl_Widget::draw_focus() or one of the more specialized
  methods can be called without checking these visible focus options.

  \note This method must only be called if the widget has the focus.
        This is not tested internally.

  The boxtype \p bt is used to calculate the inset so the focus box is drawn
  inside the box borders.

  The default focus box drawing color is black. The background color \p bg
  is used to determine a better visible color if necessary by using
  fl_contrast() with the given background color.

  \param[in]  bt        Boxtype that needs to be considered (frame width)
  \param[in]  X,Y,W,H   Bounding box
  \param[in]  bg        Background color

  \see Fl_Widget::draw_focus()
  \see Fl_Widget::draw_focus(Fl_Boxtype, int, int, int, int) const
*/
void Fl_Widget::draw_focus(Fl_Boxtype bt, int X, int Y, int W, int H, Fl_Color bg) const {
  if (!Fl::visible_focus()) return;
  if (!visible_focus()) return;
  switch (bt) {
    case FL_DOWN_BOX:
    case FL_DOWN_FRAME:
    case FL_THIN_DOWN_BOX:
    case FL_THIN_DOWN_FRAME:
      X ++;
      Y ++;
    default:
      break;
  }
  X += Fl::box_dx(bt);
  Y += Fl::box_dy(bt);
  W -= Fl::box_dw(bt)+1;
  H -= Fl::box_dh(bt)+1;

  Fl_Color savecolor = fl_color();
  fl_color(fl_contrast(FL_BLACK, bg));
  fl_focus_rect(X, Y, W, H);
  fl_color(savecolor);
}

void Fl_Widget::activate() {
  if (!active()) {
    clear_flag(INACTIVE);
    if (active_r()) {
      redraw();
      redraw_label();
      handle(FL_ACTIVATE);
      if (inside(Fl::focus())) Fl::focus()->take_focus();
    }
  }
}

void Fl_Widget::deactivate() {
  if (active_r()) {
    set_flag(INACTIVE);
    redraw();
    redraw_label();
    handle(FL_DEACTIVATE);
    fl_throw_focus(this);
  } else {
    set_flag(INACTIVE);
  }
}

int Fl_Widget::active_r() const {
  for (const Fl_Widget* o = this; o; o = o->parent())
    if (!o->active()) return 0;
  return 1;
}

void Fl_Widget::show() {
  if (!visible()) {
    clear_flag(INVISIBLE);
    if (visible_r()) {
      redraw();
      redraw_label();
      handle(FL_SHOW);
      if (inside(Fl::focus())) Fl::focus()->take_focus();
    }
  }
}

void Fl_Widget::hide() {
  if (visible_r()) {
    set_flag(INVISIBLE);
    for (Fl_Widget *p = parent(); p; p = p->parent())
      if (p->box() || !p->parent()) {p->redraw(); break;}
    handle(FL_HIDE);
    fl_throw_focus(this);
  } else {
    set_flag(INVISIBLE);
  }
}

int Fl_Widget::visible_r() const {
  for (const Fl_Widget* o = this; o; o = o->parent())
    if (!o->visible()) return 0;
  return 1;
}

// return true if widget is inside (or equal to) this:
// Returns false for null widgets.
int Fl_Widget::contains(const Fl_Widget *o) const {
  for (; o; o = o->parent_) if (o == this) return 1;
  return 0;
}


void Fl_Widget::label(const char *a) {
  if (flags() & COPIED_LABEL) {
    // reassigning a copied label remains the same copied label
    if (label_.value == a)
      return;
    free((void *)(label_.value));
    clear_flag(COPIED_LABEL);
  }
  label_.value=a;
  redraw_label();
}


void Fl_Widget::copy_label(const char *a) {
  // reassigning a copied label remains the same copied label
  if ((flags() & COPIED_LABEL) && (label_.value == a))
    return;
  if (a) {
    label(fl_strdup(a));
    set_flag(COPIED_LABEL);
  } else {
    label(0);
  }
}

void Fl_Widget::image(Fl_Image* img) {
  if (image_bound()) {
    if (label_.image && (label_.image != img)) {
      label_.image->release();
    }
    bind_image(0);
  }
  label_.image = img;
}

void Fl_Widget::image(Fl_Image& img) {
  image(&img);
}

void Fl_Widget::bind_image(Fl_Image* img) {
  image(img);
  bind_image( (img != NULL) );
}

void Fl_Widget::deimage(Fl_Image* img) {
  if (deimage_bound()) {
    if (label_.deimage && (label_.deimage != img))  {
      label_.deimage->release();
    }
    bind_deimage(0);
  }
  label_.deimage = img;
}

void Fl_Widget::deimage(Fl_Image& img) {
  deimage(&img);
}

void Fl_Widget::bind_deimage(Fl_Image* img) {
  deimage(img);
  bind_deimage( (img != NULL) );
}

/** Calls the widget callback function with arbitrary arguments.

 All overloads of do_callback() call this method.
 It does nothing if the widget's callback() is NULL.
 It clears the widget's \e changed flag \b after the callback was
 called unless the callback is the default callback. Hence it is not
 necessary to call clear_changed() after calling do_callback()
 in your own widget's handle() method.

 A \p reason must be set for widgets if different actions can trigger
 the same callback.

 \note It is legal to delete the widget in the callback (i.e. in user code),
 but you must not access the widget in the handle() method after
 calling do_callback() if the widget was deleted in the callback.
 We recommend to use Fl_Widget_Tracker to check whether the widget
 was deleted in the callback.

 \param[in] widget call the callback with \p widget as the first argument
 \param[in] arg use \p arg as the user data (second) argument
 \param[in] reason give a reason to why this callback was called, defaults to \ref FL_REASON_UNKNOWN

 \see default_callback()
 \see callback()
 \see class Fl_Widget_Tracker
 \see Fl::callback_reason()
 */
void Fl_Widget::do_callback(Fl_Widget *widget, void *arg, Fl_Callback_Reason reason) {
  Fl::callback_reason_ = reason;
  if (!callback_) return;
  Fl_Widget_Tracker wp(this);
  callback_(widget, arg);
  if (wp.deleted()) return;
  if (callback_ != default_callback)
    clear_changed();
}

/*
 \brief Sets the user data for this widget.
 Sets the new user data (void *) argument that is passed to the callback function.
 \param[in] v new user data
 */
void Fl_Widget::user_data(void* v) {
  if ((flags_ & AUTO_DELETE_USER_DATA) && user_data_)
    delete (Fl_Callback_User_Data*)user_data_;
  clear_flag(AUTO_DELETE_USER_DATA);
  user_data_ = v;
}

/*
 \brief Sets the user data for this widget.
 Sets the new user data (void *) argument that is passed to the callback function.
 \param[in] v new user data
 \param[in] auto_free if set, the widget will free user data when destroyed; defaults to false
 */
void Fl_Widget::user_data(Fl_Callback_User_Data* v, bool auto_free) {
  user_data((void*)v);
  if (auto_free)
    set_flag(AUTO_DELETE_USER_DATA);
}

