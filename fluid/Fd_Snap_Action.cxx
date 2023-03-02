//
// Snap action code file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2023 by Bill Spitzak and others.
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

#include "Fd_Snap_Action.h"

#include <FL/fl_draw.H>

int fd_left_window_margin   = 15;
int fd_right_window_margin  = 15;
int fd_top_window_margin    = 15;
int fd_bottom_window_margin = 15;

static void draw_h_arrow(int, int, int);
static void draw_v_arrow(int x, int y1, int y2);
static void draw_left_brace(const Fl_Widget *w);
static void draw_right_brace(const Fl_Widget *w);
static void draw_top_brace(const Fl_Widget *w);
static void draw_bottom_brace(const Fl_Widget *w);

/** \class Fd_Snap_Action
 */

void Fd_Snap_Action::check_x_(Fd_Snap_Data &d, int x_ref, int x_snap) {
  int dd = x_ref + d.dx - x_snap;
  int d2 = dd * dd;
  dx = d.dx - dd;
  if (d2 <= d.x_dist) {
    d.dx_out = dx;
    d.x_dist = d2;
  }
}

void Fd_Snap_Action::check_y_(Fd_Snap_Data &d, int y_ref, int y_snap) {
  int dd = y_ref + d.dy - y_snap;
  int d2 = dd * dd;
  dy = d.dy - dd;
  if (d2 <= d.y_dist) {
    d.dy_out = dy;
    d.y_dist = d2;
  }
}

bool Fd_Snap_Action::matches(Fd_Snap_Data &d) {
  switch (type) {
    case 1: return (d.drag & mask) && (d.dx==dx);
    case 2: return (d.drag & mask) && (d.dy==dy);
    case 3: return (d.drag & mask) && (d.dx==dx) && (d.dy==dy);
  }
  return false;
}

void Fd_Snap_Action::check_all(Fd_Snap_Data &data) {
  for (int i=0; list[i]; i++) {
    if (list[i]->mask & data.drag)
      list[i]->check(data);
  }
}

void Fd_Snap_Action::draw_all(Fd_Snap_Data &data) {
  for (int i=0; list[i]; i++) {
    if (list[i]->matches(data))
      list[i]->draw(data);
  }
}

// ---- window snapping ------------------------------------------------ MARK: -

/**
 All widgets snap their left edge against the left window edge.
 */
class Fd_Snap_Left_Window_Edge : public Fd_Snap_Action {
public:
  Fd_Snap_Left_Window_Edge() { type = 1; mask = FD_LEFT|FD_DRAG; }
  void check(Fd_Snap_Data &d) FL_OVERRIDE { check_x_(d, d.bx, 0); }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE { draw_left_brace(d.win->o); };
};
Fd_Snap_Left_Window_Edge snap_left_window_edge;

/**
 All widgets snap their right edge against the rigth window edge.
 */
class Fd_Snap_Right_Window_Edge : public Fd_Snap_Action {
public:
  Fd_Snap_Right_Window_Edge() { type = 1; mask = FD_RIGHT|FD_DRAG; }
  void check(Fd_Snap_Data &d) FL_OVERRIDE { check_x_(d, d.br, d.win->o->w()); }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE { draw_right_brace(d.win->o); };
};
Fd_Snap_Right_Window_Edge snap_right_window_edge;

/**
 All widgets snap their top edge against the top window edge.
 */
class Fd_Snap_Top_Window_Edge : public Fd_Snap_Action {
public:
  Fd_Snap_Top_Window_Edge() { type = 2; mask = FD_TOP|FD_DRAG; }
  void check(Fd_Snap_Data &d) FL_OVERRIDE { check_y_(d, d.by, 0); }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE { draw_top_brace(d.win->o); };
};
Fd_Snap_Top_Window_Edge snap_top_window_edge;

/**
 All widgets snap their bottom edge against the bottom window edge.
 */
class Fd_Snap_Bottom_Window_Edge : public Fd_Snap_Action {
public:
  Fd_Snap_Bottom_Window_Edge() { type = 2; mask = FD_BOTTOM|FD_DRAG; }
  void check(Fd_Snap_Data &d) FL_OVERRIDE { check_y_(d, d.bt, d.win->o->h()); }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE { draw_bottom_brace(d.win->o); };
};
Fd_Snap_Bottom_Window_Edge snap_bottom_window_edge;

/**
 If the selection is the child of the window, align to the window margin.
 */
class Fd_Snap_Left_Window_Margin : public Fd_Snap_Action {
public:
  Fd_Snap_Left_Window_Margin() { type = 1; mask = FD_LEFT|FD_DRAG; }
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    if (d.wgt && d.wgt->parent == d.win) check_x_(d, d.bx, fd_left_window_margin);
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_h_arrow(d.bx, (d.by+d.bt)/2, 0);
  };
};
Fd_Snap_Left_Window_Margin snap_left_window_margin;

/**
 If the selection is the child of the window, align to the window margin.
 */
class Fd_Snap_Right_Window_Margin : public Fd_Snap_Action {
public:
  Fd_Snap_Right_Window_Margin() { type = 1; mask = FD_RIGHT|FD_DRAG; }
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    if (d.wgt && d.wgt->parent == d.win) check_x_(d, d.br, d.win->o->w()-fd_right_window_margin);
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_h_arrow(d.br, (d.by+d.bt)/2, d.win->o->w()-1);
  };
};
Fd_Snap_Right_Window_Margin snap_right_window_margin;

/**
 If the selection is the child of the window, align to the window margin.
 */
class Fd_Snap_Top_Window_Margin : public Fd_Snap_Action {
public:
  Fd_Snap_Top_Window_Margin() { type = 2; mask = FD_TOP|FD_DRAG; }
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    if (d.wgt && d.wgt->parent == d.win) check_y_(d, d.by, fd_top_window_margin);
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_v_arrow((d.bx+d.br)/2, d.by, 0);
  };
};
Fd_Snap_Top_Window_Margin snap_top_window_margin;

/**
 If the selection is the child of the window, align to the window margin.
 */
class Fd_Snap_Bottom_Window_Margin : public Fd_Snap_Action {
public:
  Fd_Snap_Bottom_Window_Margin() { type = 2; mask = FD_BOTTOM|FD_DRAG; }
  void check(Fd_Snap_Data &d) FL_OVERRIDE {
    if (d.wgt && d.wgt->parent == d.win) check_y_(d, d.bt, d.win->o->h()-fd_bottom_window_margin);
  }
  void draw(Fd_Snap_Data &d) FL_OVERRIDE {
    draw_v_arrow((d.bx+d.br)/2, d.bt, d.win->o->h()-1);
  };
};
Fd_Snap_Bottom_Window_Margin snap_bottom_window_margin;

// ---- snap actions list ---------------------------------------------- MARK: -

Fd_Snap_Action *Fd_Snap_Action::list[] = {
  &snap_left_window_edge,
  &snap_right_window_edge,
  &snap_top_window_edge,
  &snap_bottom_window_edge,

  &snap_left_window_margin,
  &snap_right_window_margin,
  &snap_top_window_margin,
  &snap_bottom_window_margin,
  NULL
};

// ---- draw alignment marks ------------------------------------------- MARK: -

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
  int x = w->as_window() ? 0 : w->x();
  int y = w->as_window() ? 0 : w->y();
  fl_yxline(x, y-2, y+6);
  fl_yxline(x+w->w()-1, y-2, y+6);
  fl_xyline(x-2, y, x+w->w()+1);
}

static void draw_left_brace(const Fl_Widget *w)  {
  int x = w->as_window() ? 0 : w->x();
  int y = w->as_window() ? 0 : w->y();
  fl_xyline(x-2, y, x+6);
  fl_xyline(x-2, y+w->h()-1, x+6);
  fl_yxline(x, y-2, y+w->h()+1);
}

static void draw_right_brace(const Fl_Widget *w) {
  int x = w->as_window() ? w->w() - 1 : w->x() + w->w() - 1;
  int y = w->as_window() ? 0 : w->y();
  fl_xyline(x-6, y, x+2);
  fl_xyline(x-6, y+w->h()-1, x+2);
  fl_yxline(x, y-2, y+w->h()+1);
}

static void draw_bottom_brace(const Fl_Widget *w) {
  int x = w->as_window() ? 0 : w->x();
  int y = w->as_window() ? w->h() - 1 : w->y() + w->h() - 1;
  fl_yxline(x, y-6, y+2);
  fl_yxline(x+w->w()-1, y-6, y+2);
  fl_xyline(x-2, y, x+w->w()+1);
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

