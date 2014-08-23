//
// "$Id$"
//
// Engraved label drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2011 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

// Drawing code for XForms style engraved & embossed labels

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

// data[] is dx, dy, color triples

static void innards(
    const Fl_Label* o, int X, int Y, int W, int H, Fl_Align align,
    const int data[][3], int n)
{
  Fl_Align a1 = align;
  if (a1 & FL_ALIGN_CLIP) {
    fl_push_clip(X, Y, W, H); a1 = (Fl_Align)(a1&~FL_ALIGN_CLIP);}
  fl_font((Fl_Font)o->font, o->size);
  for (int i = 0; i < n; i++) {
    fl_color((Fl_Color)(i < n-1 ? data[i][2] : o->color));
    fl_draw(o->value, X+data[i][0], Y+data[i][1], W, H, a1);
  }
  if (align & FL_ALIGN_CLIP) fl_pop_clip();
}

static void fl_shadow_label(
    const Fl_Label* o, int X, int Y, int W, int H, Fl_Align align)
{
  static const int data[2][3] = {{2,2,FL_DARK3},{0,0,0}};
  innards(o, X, Y, W, H, align, data, 2);
}

static void fl_engraved_label(
    const Fl_Label* o, int X, int Y, int W, int H, Fl_Align align)
{
  static const int data[7][3] = {
    {1,0,FL_LIGHT3},{1,1,FL_LIGHT3},{0,1,FL_LIGHT3},
    {-1,0,FL_DARK3},{-1,-1,FL_DARK3},{0,-1,FL_DARK3},
    {0,0,0}};
  innards(o, X, Y, W, H, align, data, 7);
}

static void fl_embossed_label(
    const Fl_Label* o, int X, int Y, int W, int H, Fl_Align align)
{
  static const int data[7][3] = {
    {-1,0,FL_LIGHT3},{-1,-1,FL_LIGHT3},{0,-1,FL_LIGHT3},
    {1,0,FL_DARK3},{1,1,FL_DARK3},{0,1,FL_DARK3},
    {0,0,0}};
  innards(o, X, Y, W, H, align, data, 7);
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

//
// End of "$Id$".
//
