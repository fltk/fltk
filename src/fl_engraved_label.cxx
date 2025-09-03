//
// Engraved label drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2011 by Bill Spitzak and others.
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

// Drawing code for XForms style engraved & embossed labels

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

// data[] is dx, dy, color triples

static void innards(const char *str, int len, int X, int Y, const int data[][3], int n)
{
  Fl_Color c = fl_color();
  for (int i = 0; i < n; i++) {
    fl_color((Fl_Color)(i < n-1 ? data[i][2] : c));
    fl_draw(str, len, X+data[i][0], Y+data[i][1]);
  }
  fl_color(c);
}

static void dispatch(const Fl_Label* o,
  int x, int y, int w, int h, Fl_Align align,
  void (*callthis)(const char*,int,int,int))
{
  if ((!o->value || !*(o->value)) && !o->image) return;
  if (w && h && !fl_not_clipped(x, y, w, h) && (align & FL_ALIGN_INSIDE)) return;
  if (align & FL_ALIGN_CLIP)
    fl_push_clip(x, y, w, h);
  fl_font(o->font, o->size);
  fl_color((Fl_Color)o->color);
  fl_draw(o->value, x, y, w, h, align, callthis, o->image, 1, o->spacing);
  if (align & FL_ALIGN_CLIP)
    fl_pop_clip();
}



// Draw text with shadow
static void fl_shadow_label_draw(const char *str, int len, int x, int y) {
  static const int data[2][3] = {{2,2,FL_DARK3},{0,0,0}};
  innards(str, len, x, y, data, 2);
}

// Implement shadowed text label type
static void fl_shadow_label(
    const Fl_Label* o, int X, int Y, int W, int H, Fl_Align align)
{
  dispatch(o, X, Y, W, H, align, fl_shadow_label_draw);
}


// Draw text engraved
static void fl_engraved_label_draw(const char *str, int len, int x, int y) {
  static const int data[7][3] = {
    {1,0,FL_LIGHT3},{1,1,FL_LIGHT3},{0,1,FL_LIGHT3},
    {-1,0,FL_DARK3},{-1,-1,FL_DARK3},{0,-1,FL_DARK3},
    {0,0,0}};
  innards(str, len, x, y, data, 7);
}

// Implement engraved text label type
static void fl_engraved_label(
    const Fl_Label* o, int X, int Y, int W, int H, Fl_Align align)
{
  dispatch(o, X, Y, W, H, align, fl_engraved_label_draw);
}


// Draw text embossed
static void fl_embossed_label_draw(const char *str, int len, int x, int y) {
  static const int data[7][3] = {
    {-1,0,FL_LIGHT3},{-1,-1,FL_LIGHT3},{0,-1,FL_LIGHT3},
    {1,0,FL_DARK3},{1,1,FL_DARK3},{0,1,FL_DARK3},
    {0,0,0}};
  innards(str, len, x, y, data, 7);
}

// Implement embossed text label type
static void fl_embossed_label(
                              const Fl_Label* o, int X, int Y, int W, int H, Fl_Align align)
{
  dispatch(o, X, Y, W, H, align, fl_embossed_label_draw);
}


Fl_Labeltype fl_define_FL_SHADOW_LABEL() {
  Fl::set_labeltype(_FL_SHADOW_LABEL, fl_shadow_label, 0);
  return _FL_SHADOW_LABEL;
}
Fl_Labeltype fl_define_FL_ENGRAVED_LABEL() {
  Fl::set_labeltype(_FL_ENGRAVED_LABEL, fl_engraved_label, 0);
  return _FL_ENGRAVED_LABEL;
}
Fl_Labeltype fl_define_FL_EMBOSSED_LABEL() {
  Fl::set_labeltype(_FL_EMBOSSED_LABEL, fl_embossed_label, 0);
  return _FL_EMBOSSED_LABEL;
}
