//
// Label drawing routines for the Fast Light Tool Kit (FLTK).
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

// Drawing code for the (one) common label types.
// Other label types (symbols) are in their own source files
// to avoid linking if not used.

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Image.H>

void
fl_no_label(const Fl_Label*,int,int,int,int,Fl_Align) {}

void
fl_normal_label(const Fl_Label* o, int X, int Y, int W, int H, Fl_Align align)
{
  fl_font(o->font, o->size);
  fl_color((Fl_Color)o->color);
  fl_draw(o->value, X, Y, W, H, align, o->image, 1, o->spacing);
}

void
fl_normal_measure(const Fl_Label* o, int& W, int& H) {
  fl_font(o->font, o->size);
  fl_measure(o->value, W, H);
  if (o->image) {
    int iw = o->image->w(), ih = o->image->h();
    if (o->align_ & FL_ALIGN_IMAGE_BACKDROP) {          // backdrop: ignore
      // ignore backdrop image for calculation
    } else if (o->align_ & FL_ALIGN_IMAGE_NEXT_TO_TEXT) { // text and image side by side
      W += iw + o->spacing;
      if (ih > H) H = ih;
    } else {
      if (iw > W) W = iw;
      H += ih + o->spacing;
    }
  }
}

#define MAX_LABELTYPE 16

static Fl_Label_Draw_F* table[MAX_LABELTYPE] = {
  fl_normal_label,
  fl_no_label,
  fl_normal_label,      // _FL_SHADOW_LABEL,
  fl_normal_label,      // _FL_ENGRAVED_LABEL,
  fl_normal_label,      // _FL_EMBOSSED_LABEL,
  fl_no_label,          // _FL_MULTI_LABEL,
  fl_no_label,          // _FL_ICON_LABEL,
  // FL_FREE_LABELTYPE+n:
  fl_no_label, fl_no_label, fl_no_label,
  fl_no_label, fl_no_label, fl_no_label,
  fl_no_label, fl_no_label, fl_no_label
};

static Fl_Label_Measure_F* measure[MAX_LABELTYPE];

/** Sets the functions to call to draw and measure a specific labeltype. */
void Fl::set_labeltype(Fl_Labeltype t,Fl_Label_Draw_F* f,Fl_Label_Measure_F*m)
{
  table[t] = f; measure[t] = m;
}

////////////////////////////////////////////////////////////////

/** Draws a label with arbitrary alignment in an arbitrary box. */
void Fl_Label::draw(int X, int Y, int W, int H, Fl_Align align) const {
  if (!value && !image) return;
  switch (align&(FL_ALIGN_TOP|FL_ALIGN_BOTTOM)) {
    case 0: Y += v_margin_; H -= 2*v_margin_; break;
    case FL_ALIGN_TOP: Y += v_margin_; H -= v_margin_; break;
    case FL_ALIGN_BOTTOM: H -= v_margin_; break;
  }
  switch (align&(FL_ALIGN_LEFT|FL_ALIGN_RIGHT)) {
    case 0: X += h_margin_; W -= 2*h_margin_; break;
    case FL_ALIGN_LEFT: X += h_margin_; W -= h_margin_; break;
    case FL_ALIGN_RIGHT: W -= h_margin_; break;
  }
  table[type](this, X, Y, W, H, align);
}
/**
    Measures the size of the label.
    \param[in,out] W, H : this is the requested size for the label text plus image;
         on return, this will contain the size needed to fit the label
*/
void Fl_Label::measure(int& W, int& H) const {
  if (!value && !image) {
    W = H = 0;
    return;
  }

//  if (W > 0) W -= h_margin_;
  Fl_Label_Measure_F* f = ::measure[type]; if (!f) f = fl_normal_measure;
  f(this, W, H);
}

/** Draws the widget's label at the defined label position.
    This is the normal call for a widget's draw() method.
 */
void Fl_Widget::draw_label() const {
  int X = x_+Fl::box_dx(box());
  int W = w_-Fl::box_dw(box());
  if (W > 11 && align()&(FL_ALIGN_LEFT|FL_ALIGN_RIGHT)) {X += 3; W -= 6;}
  draw_label(X, y_+Fl::box_dy(box()), W, h_-Fl::box_dh(box()));
}

/** Draws the label in an arbitrary bounding box.
    draw() can use this instead of draw_label(void) to change the bounding box
 */
void Fl_Widget::draw_label(int X, int Y, int W, int H) const {
  // quit if we are not drawing a label inside the widget:
  if ((align()&15) && !(align() & FL_ALIGN_INSIDE)) return;
  draw_label(X,Y,W,H,align());
}

/** Draws the label in an arbitrary bounding box with an arbitrary alignment.
    Anybody can call this to force the label to draw anywhere.
 */
void Fl_Widget::draw_label(int X, int Y, int W, int H, Fl_Align a) const {
  if (flags()&SHORTCUT_LABEL) fl_draw_shortcut = 1;
  Fl_Label l1 = label_;
  if (!active_r()) {
    l1.color = fl_inactive((Fl_Color)l1.color);
    if (l1.deimage) l1.image = l1.deimage;
  }
  l1.draw(X,Y,W,H,a);
  fl_draw_shortcut = 0;
}

// include these vars here so they can be referenced without including
// Fl_Input_ code:
#include <FL/Fl_Input_.H>
