/*	Fl_Value_Slider.H

	A slider with a box displaying the current value

*/

#ifndef Fl_Value_Slider_H
#define Fl_Value_Slider_H

#include "Fl_Slider.H"

class Fl_Value_Slider : public Fl_Slider {
    uchar textfont_, textsize_, textcolor_;
public:
    void draw();
    int handle(int);
    Fl_Value_Slider(int x,int y,int w,int h, const char *l = 0);
    Fl_Font textfont() const {return (Fl_Font)textfont_;}
    void textfont(uchar s) {textfont_ = s;}
    uchar textsize() const {return textsize_;}
    void textsize(uchar s) {textsize_ = s;}
    Fl_Color textcolor() const {return (Fl_Color)textcolor_;}
    void textcolor(uchar s) {textcolor_ = s;}
};

#endif
