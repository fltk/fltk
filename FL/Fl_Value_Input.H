// Fl_Value_Input.H

// Edit text or drag to adjust floating point value.
// as suggested by Pierre Jasmin

// Current behavior:
// Dragging left mouse moves by step()
// Middle mouse is 5*step()
// Right mouse is 20*step()
// clicking any mouse button puts insertion cursor there
// You can also navigate there with tab

// Bugs (?):
// No way to select region of text
// Hard to paste in new values

#ifndef Fl_Value_Input_H
#define Fl_Value_Input_H

#include "Fl_Valuator.H"
#include "Fl_Input.H"

class Fl_Value_Input : public Fl_Valuator {
public:
  Fl_Input input;
private:
  char soft_;
  static void input_cb(Fl_Widget*,void*);
  virtual void value_damage(); // cause damage() due to value() changing
public:
  int handle(int);
  void draw();
  void resize(int,int,int,int);
  Fl_Value_Input(int x,int y,int w,int h,const char *l=0);

  void soft(char x) {soft_ = x;}
  char soft() const {return soft_;}

  Fl_Font textfont() const {return input.textfont();}
  void textfont(uchar s) {input.textfont(s);}
  uchar textsize() const {return input.textsize();}
  void textsize(uchar s) {input.textsize(s);}
  Fl_Color textcolor() const {return input.textcolor();}
  void textcolor(uchar n) {input.textcolor(n);}
  Fl_Color cursor_color() const {return input.cursor_color();}
  void cursor_color(uchar n) {input.cursor_color(n);}

};

#endif
