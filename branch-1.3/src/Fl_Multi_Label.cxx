//
// "$Id$"
//
// Multi-label widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

// Allows two labels to be used on a widget (by having one of them
// be one of these it allows an infinte number!)

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Multi_Label.H>

static void multi_labeltype(
    const Fl_Label* o, int x, int y, int w, int h, Fl_Align a)
{
  Fl_Multi_Label* b = (Fl_Multi_Label*)(o->value);
  Fl_Label local = *o;
  local.value = b->labela;
  local.type = b->typea;
  int W = w; int H = h; local.measure(W, H);
  local.draw(x,y,w,h,a);
  if (a & FL_ALIGN_BOTTOM) h -= H;
  else if (a & FL_ALIGN_TOP) {y += H; h -= H;}
  else if (a & FL_ALIGN_RIGHT) w -= W;
  else if (a & FL_ALIGN_LEFT) {x += W; w -= W;}
  else {int d = (h+H)/2; y += d; h -= d;}
  local.value = b->labelb;
  local.type = b->typeb;
  local.draw(x,y,w,h,a);
}

// measurement is only correct for left-to-right appending...
static void multi_measure(const Fl_Label* o, int& w, int& h) {
  Fl_Multi_Label* b = (Fl_Multi_Label*)(o->value);
  Fl_Label local = *o;
  local.value = b->labela;
  local.type = b->typea;
  local.measure(w,h);
  local.value = b->labelb;
  local.type = b->typeb;
  int W = 0; int H = 0; local.measure(W,H);
  w += W; if (H>h) h = H;
}

void Fl_Multi_Label::label(Fl_Widget* o) {
  Fl::set_labeltype(_FL_MULTI_LABEL, multi_labeltype, multi_measure);
  o->label(_FL_MULTI_LABEL, (const char*)this);
}

void Fl_Multi_Label::label(Fl_Menu_Item* o) {
  Fl::set_labeltype(_FL_MULTI_LABEL, multi_labeltype, multi_measure);
  o->label(_FL_MULTI_LABEL, (const char*)this);
}

//
// End of "$Id$".
//
