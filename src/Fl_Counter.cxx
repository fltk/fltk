//
// Counter widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

void Fl_Counter::draw() {
  int i; Fl_Boxtype boxtype[5];
  Fl_Color selcolor;

  boxtype[0] = box();
  if (boxtype[0] == FL_UP_BOX) boxtype[0] = FL_DOWN_BOX;
  if (boxtype[0] == FL_THIN_UP_BOX) boxtype[0] = FL_THIN_DOWN_BOX;
  for (i=1; i<5; i++)
    if (mouseobj == i)
      boxtype[i] = fl_down(box());
    else
      boxtype[i] = box();

  int xx[5], ww[5];
  if (type() == FL_NORMAL_COUNTER) {
    int W = w()*15/100;
    xx[1] = x();         ww[1] = W;
    xx[2] = x()+1*W;     ww[2] = W;
    xx[0] = x()+2*W;     ww[0] = w()-4*W;
    xx[3] = x()+w()-2*W; ww[3] = W;
    xx[4] = x()+w()-1*W; ww[4] = W;
  } else {
    int W = w()*20/100;
    xx[1] = 0;           ww[1] = 0;
    xx[2] = x();         ww[2] = W;
    xx[0] = x()+W;       ww[0] = w()-2*W;
    xx[3] = x()+w()-1*W; ww[3] = W;
    xx[4] = 0;           ww[4] = 0;
  }

  draw_box(boxtype[0], xx[0], y(), ww[0], h(), FL_BACKGROUND2_COLOR);
  fl_font(textfont(), textsize());
  fl_color(active_r() ? textcolor() : fl_inactive(textcolor()));
  char str[128]; format(str);
  fl_draw(str, xx[0], y(), ww[0], h(), FL_ALIGN_CENTER);
  if (Fl::focus() == this) draw_focus(boxtype[0], xx[0], y(), ww[0], h());
  if (!(damage()&FL_DAMAGE_ALL)) return; // only need to redraw text

  if (active_r())
    selcolor = labelcolor();
  else
    selcolor = fl_inactive(labelcolor());

  if (type() == FL_NORMAL_COUNTER) {
    draw_box(boxtype[1], xx[1], y(), ww[1], h(), color());
    fl_draw_symbol("@-4<<", xx[1], y(), ww[1], h(), selcolor);
  }
  draw_box(boxtype[2], xx[2], y(), ww[2], h(), color());
  fl_draw_symbol("@-4<",  xx[2], y(), ww[2], h(), selcolor);
  draw_box(boxtype[3], xx[3], y(), ww[3], h(), color());
  fl_draw_symbol("@-4>",  xx[3], y(), ww[3], h(), selcolor);
  if (type() == FL_NORMAL_COUNTER) {
    draw_box(boxtype[4], xx[4], y(), ww[4], h(), color());
    fl_draw_symbol("@-4>>", xx[4], y(), ww[4], h(), selcolor);
  }
}

void Fl_Counter::increment_cb() {
  if (!mouseobj) return;
  double v = value();
  switch (mouseobj) {
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
  if (b->mouseobj) {
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
    if (mouseobj) {
      Fl::remove_timeout(repeat_callback, this);
      mouseobj = 0;
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
    if (i != mouseobj) {
      Fl::remove_timeout(repeat_callback, this);
      mouseobj = (uchar)i;
      if (i) Fl::add_timeout(INITIALREPEAT, repeat_callback, this);
      Fl_Widget_Tracker wp(this);
      increment_cb();
      if (wp.deleted()) return 1;
      redraw();
    }
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
  case FL_FOCUS : /* FALLTHROUGH */
  case FL_UNFOCUS :
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
  mouseobj = 0;
  textfont_ = FL_HELVETICA;
  textsize_ = FL_NORMAL_SIZE;
  textcolor_ = FL_FOREGROUND_COLOR;
}


Fl_Simple_Counter::Fl_Simple_Counter(int X,int Y,int W,int H, const char *L)
: Fl_Counter(X,Y,W,H,L) {
  type(FL_SIMPLE_COUNTER);
}
