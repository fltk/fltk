/*	Fl_Value_Slider.C	*/

#include <FL/Fl.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/fl_draw.H>
#include <math.h>
#include <FL/Fl_Input_.H> // for default_font

Fl_Value_Slider::Fl_Value_Slider(int x,int y,int w,int h, const char*l)
: Fl_Slider(x,y,w,h,l) {
  step(1,100);
  textfont_ = FL_HELVETICA;
  textsize_ = 10;
  textcolor_ = FL_BLACK;
}

void Fl_Value_Slider::draw() {
  int sxx = x(), syy = y(), sww = w(), shh = h();
  int bxx = x(), byy = y(), bww = w(), bhh = h();
  if (horizontal()) {
    bww = 35; sxx += 35; sww -= 35;
  } else {
    syy += 25; bhh = 25; shh -= 25;
  }
  Fl_Slider::draw(sxx,syy,sww,shh);
  draw_box(box(),bxx,byy,bww,bhh,color());
  char buf[128];
  format(buf);
  fl_font(textfont(), textsize(),
	  Fl_Input_::default_font(), Fl_Input_::default_size());
  fl_color(active_r() ? textcolor() : inactive(textcolor()));
  fl_draw(buf, bxx, byy, bww, bhh, FL_ALIGN_CLIP);
}

int Fl_Value_Slider::handle(int event) {
  int sxx = x(), syy = y(), sww = w(), shh = h();
  if (horizontal()) {
    sxx += 35; sww -= 35;
  } else {
    syy += 25; shh -= 25;
  }
  return Fl_Slider::handle(event,sxx,syy,sww,shh);
}

// End of Fl_Value_Slider.C
