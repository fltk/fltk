// Fl_Value_Output.H

// Fltk widget for drag-adjusting a floating point value.
// This is much lighter than Fl_Value_Input because it has no text editor
// If you deactivate() it, it can be used to display a floating-point value

#ifndef Fl_Value_Output_H
#define Fl_Value_Output_H

#ifndef Fl_Valuator_H
#include "Fl_Valuator.H"
#endif

class Fl_Value_Output : public Fl_Valuator {
  uchar textfont_, textsize_, textcolor_, soft_;
public:
  int handle(int);
  void draw();
  Fl_Value_Output(int x,int y,int w,int h,const char *l=0);

  void soft(uchar x) {soft_ = x;}
  uchar soft() const {return soft_;}

  Fl_Font textfont() const {return (Fl_Font)textfont_;}
  void textfont(uchar s) {textfont_ = s;}
  uchar textsize() const {return textsize_;}
  void textsize(uchar s) {textsize_ = s;}
  Fl_Color textcolor() const {return (Fl_Color)textcolor_;}
  void textcolor(uchar s) {textcolor_ = s;}
};

#endif
