//
// "GTK" drawing routines for the Fast Light Tool Kit (FLTK).
//
// These box types provide a GTK+ look, based on Red Hat's Bluecurve
// theme...
//
// Copyright 2006-2010 by Michael Sweet.
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

// Box drawing code for an obscure box type.
// These box types are in separate files so they are not linked
// in if not used.

#include <FL/Fl.H>
#include <FL/fl_draw.H>

extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*);


static void gtk_color(Fl_Color c) {
  Fl::set_box_color(c);
}

static void gtk_up_frame(int x, int y, int w, int h, Fl_Color c) {
  gtk_color(fl_color_average(FL_WHITE, c, 0.5));
  fl_xyline(x + 2, y + 1, x + w - 3);
  fl_yxline(x + 1, y + 2, y + h - 3);

  gtk_color(fl_color_average(FL_BLACK, c, 0.5));
  fl_begin_loop();
    fl_vertex(x, y + 2);
    fl_vertex(x + 2, y);
    fl_vertex(x + w - 3, y);
    fl_vertex(x + w - 1, y + 2);
    fl_vertex(x + w - 1, y + h - 3);
    fl_vertex(x + w - 3, y + h - 1);
    fl_vertex(x + 2, y + h - 1);
    fl_vertex(x, y + h - 3);
  fl_end_loop();
}


static void gtk_up_box(int x, int y, int w, int h, Fl_Color c) {
  gtk_up_frame(x, y, w, h, c);

  gtk_color(fl_color_average(FL_WHITE, c, 0.4f));
  fl_xyline(x + 2, y + 2, x + w - 3);
  gtk_color(fl_color_average(FL_WHITE, c, 0.2f));
  fl_xyline(x + 2, y + 3, x + w - 3);
  gtk_color(fl_color_average(FL_WHITE, c, 0.1f));
  fl_xyline(x + 2, y + 4, x + w - 3);
  gtk_color(c);
  fl_rectf(x + 2, y + 5, w - 4, h - 7);
  gtk_color(fl_color_average(FL_BLACK, c, 0.025f));
  fl_xyline(x + 2, y + h - 4, x + w - 3);
  gtk_color(fl_color_average(FL_BLACK, c, 0.05f));
  fl_xyline(x + 2, y + h - 3, x + w - 3);
  gtk_color(fl_color_average(FL_BLACK, c, 0.1f));
  fl_xyline(x + 2, y + h - 2, x + w - 3);
  fl_yxline(x + w - 2, y + 2, y + h - 3);
}


static void gtk_down_frame(int x, int y, int w, int h, Fl_Color c) {
  gtk_color(fl_color_average(FL_BLACK, c, 0.5));
  fl_begin_loop();
    fl_vertex(x, y + 2);
    fl_vertex(x + 2, y);
    fl_vertex(x + w - 3, y);
    fl_vertex(x + w - 1, y + 2);
    fl_vertex(x + w - 1, y + h - 3);
    fl_vertex(x + w - 3, y + h - 1);
    fl_vertex(x + 2, y + h - 1);
    fl_vertex(x, y + h - 3);
  fl_end_loop();

  gtk_color(fl_color_average(FL_BLACK, c, 0.1f));
  fl_xyline(x + 2, y + 1, x + w - 3);
  fl_yxline(x + 1, y + 2, y + h - 3);

  gtk_color(fl_color_average(FL_BLACK, c, 0.05f));
  fl_yxline(x + 2, y + h - 2, y + 2, x + w - 2);
}


static void gtk_down_box(int x, int y, int w, int h, Fl_Color c) {
  gtk_down_frame(x, y, w, h, c);

  gtk_color(c);
  fl_rectf(x + 3, y + 3, w - 5, h - 4);
  fl_yxline(x + w - 2, y + 3, y + h - 3);
}


static void gtk_thin_up_frame(int x, int y, int w, int h, Fl_Color c) {
  gtk_color(fl_color_average(FL_WHITE, c, 0.6f));
  fl_xyline(x + 1, y, x + w - 2);
  fl_yxline(x, y + 1, y + h - 2);

  gtk_color(fl_color_average(FL_BLACK, c, 0.4f));
  fl_xyline(x + 1, y + h - 1, x + w - 2);
  fl_yxline(x + w - 1, y + 1, y + h - 2);
}


static void gtk_thin_up_box(int x, int y, int w, int h, Fl_Color c) {
  gtk_thin_up_frame(x, y, w, h, c);

  gtk_color(fl_color_average(FL_WHITE, c, 0.4f));
  fl_xyline(x + 1, y + 1, x + w - 2);
  gtk_color(fl_color_average(FL_WHITE, c, 0.2f));
  fl_xyline(x + 1, y + 2, x + w - 2);
  gtk_color(fl_color_average(FL_WHITE, c, 0.1f));
  fl_xyline(x + 1, y + 3, x + w - 2);
  gtk_color(c);
  fl_rectf(x + 1, y + 4, w - 2, h - 8);
  gtk_color(fl_color_average(FL_BLACK, c, 0.025f));
  fl_xyline(x + 1, y + h - 4, x + w - 2);
  gtk_color(fl_color_average(FL_BLACK, c, 0.05f));
  fl_xyline(x + 1, y + h - 3, x + w - 2);
  gtk_color(fl_color_average(FL_BLACK, c, 0.1f));
  fl_xyline(x + 1, y + h - 2, x + w - 2);
}


static void gtk_thin_down_frame(int x, int y, int w, int h, Fl_Color c) {
  gtk_color(fl_color_average(FL_BLACK, c, 0.4f));
  fl_xyline(x + 1, y, x + w - 2);
  fl_yxline(x, y + 1, y + h - 2);

  gtk_color(fl_color_average(FL_WHITE, c, 0.6f));
  fl_xyline(x + 1, y + h - 1, x + w - 2);
  fl_yxline(x + w - 1, y + 1, y + h - 2);
}


static void gtk_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
  gtk_thin_down_frame(x, y, w, h, c);

  gtk_color(c);
  fl_rectf(x + 1, y + 1, w - 2, h - 2);
}

//------------------------
// new GTK+ style for round buttons
#if 1

static void fl_arc_i(int x,int y,int w,int h,double a1,double a2) {
  fl_arc(x,y,w,h,a1,a2);
}

enum {UPPER_LEFT, LOWER_RIGHT, CLOSED, FILL};

static void draw(int which, int x,int y,int w,int h, int inset)
{
  if (inset*2 >= w) inset = (w-1)/2;
  if (inset*2 >= h) inset = (h-1)/2;
  x += inset;
  y += inset;
  w -= 2*inset;
  h -= 2*inset;
  int d = w <= h ? w : h;
  if (d <= 1) return;
  void (*f)(int,int,int,int,double,double);
  f = (which==FILL) ? fl_pie : fl_arc_i;
  if (which >= CLOSED) {
    f(x+w-d, y, d, d, w<=h ? 0 : -90, w<=h ? 180 : 90);
    f(x, y+h-d, d, d, w<=h ? 180 : 90, w<=h ? 360 : 270);
  } else if (which == UPPER_LEFT) {
    f(x+w-d, y, d, d, 45, w<=h ? 180 : 90);
    f(x, y+h-d, d, d, w<=h ? 180 : 90, 225);
  } else { // LOWER_RIGHT
    f(x, y+h-d, d, d, 225, w<=h ? 360 : 270);
    f(x+w-d, y, d, d, w<=h ? 360 : 270, 360+45);
  }
  if (which == FILL) {
    if (w < h)
      fl_rectf(x, y+d/2, w, h-(d&-2));
    else if (w > h)
      fl_rectf(x+d/2, y, w-(d&-2), h);
  } else {
    if (w < h) {
      if (which != UPPER_LEFT) fl_yxline(x+w-1, y+d/2-1, y+h-d/2+1);
      if (which != LOWER_RIGHT) fl_yxline(x, y+d/2-1, y+h-d/2+1);
    } else if (w > h) {
      if (which != UPPER_LEFT) fl_xyline(x+d/2-1, y+h-1, x+w-d/2+1);
      if (which != LOWER_RIGHT) fl_xyline(x+d/2-1, y, x+w-d/2+1);
    }
  }
}

static void gtk_round_up_box(int x, int y, int w, int h, Fl_Color c) {
  gtk_color(c);
  draw(FILL,        x,   y, w,   h, 2);

  gtk_color(fl_color_average(FL_BLACK, c, 0.025f));
  draw(LOWER_RIGHT, x+1, y, w-2, h, 2);
  draw(LOWER_RIGHT, x,   y, w,   h, 3);
  gtk_color(fl_color_average(FL_BLACK, c, 0.05f));
  draw(LOWER_RIGHT, x+1, y, w-2, h, 1);
  draw(LOWER_RIGHT, x,   y, w,   h, 2);
  gtk_color(fl_color_average(FL_BLACK, c, 0.1f));
  draw(LOWER_RIGHT, x+1, y, w-2, h, 0);
  draw(LOWER_RIGHT, x,   y, w,   h, 1);

  gtk_color(fl_color_average(FL_WHITE, c, 0.1f));
  draw(UPPER_LEFT,  x,   y, w,   h, 4);
  draw(UPPER_LEFT,  x+1, y, w-2, h, 3);
  gtk_color(fl_color_average(FL_WHITE, c, 0.2f));
  draw(UPPER_LEFT,  x,   y, w,   h, 3);
  draw(UPPER_LEFT,  x+1, y, w-2, h, 2);
  gtk_color(fl_color_average(FL_WHITE, c, 0.4f));
  draw(UPPER_LEFT,  x,   y, w,   h, 2);
  draw(UPPER_LEFT,  x+1, y, w-2, h, 1);
  gtk_color(fl_color_average(FL_WHITE, c, 0.5f));
  draw(UPPER_LEFT,  x,   y, w,   h, 1);
  draw(UPPER_LEFT,  x+1, y, w-2, h, 0);

  gtk_color(fl_color_average(FL_BLACK, c, 0.5f));
  draw(CLOSED,      x,   y, w,   h, 0);
}

static void gtk_round_down_box(int x, int y, int w, int h, Fl_Color c) {
  gtk_color(c);
  draw(FILL,        x,   y, w,   h, 2);

  gtk_color(fl_color_average(FL_WHITE, c, 0.1f));
  draw(LOWER_RIGHT, x+1, y, w-2, h, 2);
  draw(LOWER_RIGHT, x,   y, w,   h, 3);
  gtk_color(fl_color_average(FL_WHITE, c, 0.2f));
  draw(LOWER_RIGHT, x+1, y, w-2, h, 1);
  draw(LOWER_RIGHT, x,   y, w,   h, 2);
  gtk_color(fl_color_average(FL_WHITE, c, 0.5f));
  draw(LOWER_RIGHT, x+1, y, w-2, h, 0);
  draw(LOWER_RIGHT, x,   y, w,   h, 1);

  gtk_color(fl_color_average(FL_BLACK, c, 0.05f));
  draw(UPPER_LEFT,  x,   y, w,   h, 2);
  draw(UPPER_LEFT,  x+1, y, w-2, h, 1);
  gtk_color(fl_color_average(FL_BLACK, c, 0.1f));
  draw(UPPER_LEFT,  x,   y, w,   h, 1);
  draw(UPPER_LEFT,  x+1, y, w-2, h, 0);

  gtk_color(fl_color_average(FL_BLACK, c, 0.5f));
  draw(CLOSED,      x,   y, w,   h, 0);
}

#else

static void gtk_round_up_box(int x, int y, int w, int h, Fl_Color c) {
  gtk_color(c);
  fl_pie(x, y, w, h, 0.0, 360.0);
  gtk_color(fl_color_average(FL_WHITE, c, 0.5f));
  fl_arc(x, y, w, h, 45.0, 180.0);
  gtk_color(fl_color_average(FL_WHITE, c, 0.25f));
  fl_arc(x, y, w, h, 180.0, 405.0);
  gtk_color(fl_color_average(FL_BLACK, c, 0.5f));
  fl_arc(x, y, w, h, 225.0, 360.0);
}


static void gtk_round_down_box(int x, int y, int w, int h, Fl_Color c) {
  gtk_color(c);
  fl_pie(x, y, w, h, 0.0, 360.0);
  gtk_color(fl_color_average(FL_BLACK, c, 0.2));
  fl_arc(x + 1, y, w, h, 90.0, 210.0);
  gtk_color(fl_color_average(FL_BLACK, c, 0.6));
  fl_arc(x, y, w, h, 0.0, 360.0);
}

#endif

Fl_Boxtype fl_define_FL_GTK_UP_BOX() {
  fl_internal_boxtype(_FL_GTK_UP_BOX, gtk_up_box);
  fl_internal_boxtype(_FL_GTK_DOWN_BOX, gtk_down_box);
  fl_internal_boxtype(_FL_GTK_UP_FRAME, gtk_up_frame);
  fl_internal_boxtype(_FL_GTK_DOWN_FRAME, gtk_down_frame);
  fl_internal_boxtype(_FL_GTK_THIN_UP_BOX, gtk_thin_up_box);
  fl_internal_boxtype(_FL_GTK_THIN_DOWN_BOX, gtk_thin_down_box);
  fl_internal_boxtype(_FL_GTK_THIN_UP_FRAME, gtk_thin_up_frame);
  fl_internal_boxtype(_FL_GTK_THIN_DOWN_FRAME, gtk_thin_down_frame);
  fl_internal_boxtype(_FL_GTK_ROUND_UP_BOX, gtk_round_up_box);
  fl_internal_boxtype(_FL_GTK_ROUND_DOWN_BOX, gtk_round_down_box);

  return _FL_GTK_UP_BOX;
}
