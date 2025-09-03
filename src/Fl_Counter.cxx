//
// Counter widget for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Counter.H>
#include <FL/Fl_Simple_Counter.H>
#include <FL/fl_draw.H>

// This struct describes the four arrow boxes
struct arrow_box {
  int width;
  Fl_Arrow_Type arrow_type;
  Fl_Boxtype boxtype;
  Fl_Orientation orientation;
  arrow_box() { // constructor
    width = 0;
    boxtype = FL_NO_BOX;
    orientation = FL_ORIENT_RIGHT;
    arrow_type = FL_ARROW_SINGLE;
  }
};

/**
  Compute sizes (widths) of arrow boxes.

  This method computes the two sizes of the arrow boxes of Fl_Counter.
  You can override it in a subclass if you want to draw fancy arrows
  or change the layout. However, the basic layout is fixed and can't
  be changed w/o overriding the draw() and handle() methods.

  Basic layout:
  \code
    +------+-----+-------------+-----+------+
    |  <<  |  <  |    value    |  >  |  >>  |
    +------+-----+-------------+-----+------+
  \endcode

  The returned value \p w2 should be zero if the counter type() is FL_SIMPLE_COUNTER.

  \param[out]   w1  width of single arrow box
  \param[out]   w2  width of double arrow box
*/
void Fl_Counter::arrow_widths(int &w1, int &w2) {
  if (type() == FL_SIMPLE_COUNTER) {
    w1 = w() * 20/100;
    w2 = 0;
  } else {
    w1 = w() * 13/100;
    w2 = w() * 17/100;
  }
  // limit arrow box sizes to reserve more space for the text box
  if (w1 > 13) w1 = 13;
  if (w2 > 24) w2 = 24;
}

void Fl_Counter::draw() {
  struct arrow_box ab[4];

  // text box setup
  Fl_Boxtype tbt = box();
  if (tbt == FL_UP_BOX) tbt = FL_DOWN_BOX;
  if (tbt == FL_THIN_UP_BOX) tbt = FL_THIN_DOWN_BOX;

  // arrow boxes
  for (int i = 0; i < 4; i++) {
    if (mouseobj_ == i + 1)
      ab[i].boxtype = fl_down(box());
    else
      ab[i].boxtype = box();
  }

  ab[0].arrow_type = ab[3].arrow_type = FL_ARROW_DOUBLE;      // first and last arrow
  ab[0].orientation = ab[1].orientation = FL_ORIENT_LEFT;     // left arrows

  int w1 = 0, w2 = 0;
  arrow_widths(w1, w2);
  if (type() == FL_SIMPLE_COUNTER)
    w2 = 0;

  ab[0].width = ab[3].width = w2;          // double arrows
  ab[1].width = ab[2].width = w1;          // single arrows

  int tw = w() - 2 * (w1 + w2); // text box width
  int tx = x() + w1 + w2;       // text box position

  // printf("w() = %3d, w1 = %3d, w2 = %3d, tw = %3d\n", w(), w1, w2, tw);

  // always draw text box and text
  draw_box(tbt, tx, y(), tw, h(), FL_BACKGROUND2_COLOR);
  fl_font(textfont(), textsize());
  fl_color(active_r() ? textcolor() : fl_inactive(textcolor()));
  char str[128]; format(str);
  fl_draw(str, tx, y(), tw, h(), FL_ALIGN_CENTER);
  if (Fl::focus() == this) draw_focus(tbt, tx, y(), tw, h());
  if (!(damage()&FL_DAMAGE_ALL)) return; // only need to redraw text

  Fl_Color arrow_color;
  if (active_r())
    arrow_color = labelcolor();
  else
    arrow_color = fl_inactive(labelcolor());

  // draw arrow boxes
  int xo = x();
  for (int i = 0; i < 4; i++) {
    if (ab[i].width > 0) {
      draw_box(ab[i].boxtype, xo, y(), ab[i].width, h(), color());
      Fl_Rect bb(xo, y(), ab[i].width, h(), ab[i].boxtype);
      fl_draw_arrow(bb, ab[i].arrow_type, ab[i].orientation, arrow_color);
      xo += ab[i].width;
    }
    if (i == 1) xo += tw;
  }

} // draw()

void Fl_Counter::increment_cb() {
  if (!mouseobj_) return;
  double v = value();
  switch (mouseobj_) {
  case 1: v -= lstep_; break;
  case 2: v = increment(v, -1); break;
  case 3: v = increment(v, 1); break;
  case 4: v += lstep_; break;
  }
  handle_drag(clamp(round(v)));
}

#define INITIALREPEAT .5
#define REPEAT .1

void Fl_Counter::repeat_callback(void* v) {
  Fl_Counter* b = (Fl_Counter*)v;
  int buttons = Fl::event_state() & FL_BUTTONS; // any mouse button pressed
  int focus = (Fl::focus() == b);               // the widget has focus
  if (b->mouseobj_ && buttons && focus) {
    Fl::add_timeout(REPEAT, repeat_callback, b);
    b->increment_cb();
  }
}

int Fl_Counter::calc_mouseobj() {
  if (type() == FL_NORMAL_COUNTER) {
    int W = w()*15/100;
    if (Fl::event_inside(x(), y(), W, h())) return 1;
    if (Fl::event_inside(x()+W, y(), W, h())) return 2;
    if (Fl::event_inside(x()+w()-2*W, y(), W, h())) return 3;
    if (Fl::event_inside(x()+w()-W, y(), W, h())) return 4;
  } else {
    int W = w()*20/100;
    if (Fl::event_inside(x(), y(), W, h())) return 2;
    if (Fl::event_inside(x()+w()-W, y(), W, h())) return 3;
  }
  return -1;
}

int Fl_Counter::handle(int event) {
  int i;
  switch (event) {
  case FL_RELEASE:
    if (mouseobj_) {
      Fl::remove_timeout(repeat_callback, this);
      mouseobj_ = 0;
      redraw();
    }
    handle_release();
    return 1;
  case FL_PUSH:
    if (Fl::visible_focus()) Fl::focus(this);
    { Fl_Widget_Tracker wp(this);
      handle_push();
      if (wp.deleted()) return 1;
    }
  case FL_DRAG:
    i = calc_mouseobj();
    if (i != mouseobj_) {
      Fl::remove_timeout(repeat_callback, this);
      mouseobj_ = (uchar)i;
      if (i > 0)
        Fl::add_timeout(INITIALREPEAT, repeat_callback, this);
      Fl_Widget_Tracker wp(this);
      increment_cb();
      if (wp.deleted()) return 1;
      redraw();
    }
    return 1;
  case FL_MOUSEWHEEL:
    handle_drag(clamp(increment(value(),(Fl::event_dy() - Fl::event_dx()) / 2 )));
    return 1;
  case FL_KEYBOARD :
    switch (Fl::event_key()) {
      case FL_Left:
        handle_drag(clamp(increment(value(),-1)));
        return 1;
      case FL_Right:
        handle_drag(clamp(increment(value(),1)));
        return 1;
      default:
        return 0;
    }
    // break not required because of switch...
  case FL_UNFOCUS :
    mouseobj_ = 0;
    /* FALLTHROUGH */
  case FL_FOCUS :
    if (Fl::visible_focus()) {
      redraw();
      return 1;
    } else return 0;
  case FL_ENTER : /* FALLTHROUGH */
  case FL_LEAVE :
    return 1;
  default:
    return 0;
  }
}

/**
  Destroys the valuator.
 */
Fl_Counter::~Fl_Counter() {
  Fl::remove_timeout(repeat_callback, this);
}

/**
  Creates a new Fl_Counter widget using the given position, size, and label
  string. The default type is FL_NORMAL_COUNTER.
  \param[in] X, Y, W, H position and size of the widget
  \param[in] L widget label, default is no label
 */
Fl_Counter::Fl_Counter(int X, int Y, int W, int H, const char* L)
  : Fl_Valuator(X, Y, W, H, L) {
  box(FL_UP_BOX);
  selection_color(FL_INACTIVE_COLOR); // was FL_BLUE
  align(FL_ALIGN_BOTTOM);
  bounds(-1000000.0, 1000000.0);
  Fl_Valuator::step(1, 10);
  lstep_ = 1.0;
  mouseobj_ = 0;
  textfont_ = FL_HELVETICA;
  textsize_ = FL_NORMAL_SIZE;
  textcolor_ = FL_FOREGROUND_COLOR;
}


Fl_Simple_Counter::Fl_Simple_Counter(int X,int Y,int W,int H, const char *L)
: Fl_Counter(X,Y,W,H,L) {
  type(FL_SIMPLE_COUNTER);
}
