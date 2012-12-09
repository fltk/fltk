//
// "$Id$"
//
// Value slider widget for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/fl_draw.H>
#include <math.h>

/**
  Creates a new Fl_Value_Slider widget using the given
  position, size, and label string. The default boxtype is FL_DOWN_BOX.
*/
Fl_Value_Slider::Fl_Value_Slider(int X, int Y, int W, int H, const char*l)
: Fl_Slider(X,Y,W,H,l) {
  step(1,100);
  textfont_ = FL_HELVETICA;
  textsize_ = 10;
  textcolor_ = FL_FOREGROUND_COLOR;
}

void Fl_Value_Slider::draw() {
  int sxx = x(), syy = y(), sww = w(), shh = h();
  int bxx = x(), byy = y(), bww = w(), bhh = h();
  if (horizontal()) {
    bww = 35; sxx += 35; sww -= 35;
  } else {
    syy += 25; bhh = 25; shh -= 25;
  }
  if (damage()&FL_DAMAGE_ALL) draw_box(box(),sxx,syy,sww,shh,color());
  Fl_Slider::draw(sxx+Fl::box_dx(box()),
		  syy+Fl::box_dy(box()),
		  sww-Fl::box_dw(box()),
		  shh-Fl::box_dh(box()));
  draw_box(box(),bxx,byy,bww,bhh,color());
  char buf[128];
  format(buf);
  fl_font(textfont(), textsize());
  fl_color(active_r() ? textcolor() : fl_inactive(textcolor()));
  fl_draw(buf, bxx, byy, bww, bhh, FL_ALIGN_CLIP);
}

int Fl_Value_Slider::handle(int event) {
  if (event == FL_PUSH && Fl::visible_focus()) {
    Fl::focus(this);
    redraw();
  }
  int sxx = x(), syy = y(), sww = w(), shh = h();
  if (horizontal()) {
    sxx += 35; sww -= 35;
  } else {
    syy += 25; shh -= 25;
  }
  return Fl_Slider::handle(event,
			   sxx+Fl::box_dx(box()),
			   syy+Fl::box_dy(box()),
			   sww-Fl::box_dw(box()),
			   shh-Fl::box_dh(box()));
}


Fl_Hor_Value_Slider::Fl_Hor_Value_Slider(int X,int Y,int W,int H,const char *l)
: Fl_Value_Slider(X,Y,W,H,l) {
  type(FL_HOR_SLIDER);
}


//
// End of "$Id$".
//
