// Fl_Color_Chooser.H

// The color chooser object and the color chooser popup.  The popup
// is just a window containing a single color chooser and some boxes
// to indicate the current and cancelled color.

#ifndef Fl_Color_Chooser_H
#define Fl_Color_Chooser_H

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Value_Input.H>

class Flcc_HueBox : public Fl_Widget {
  int handle(int);
  void draw();
  int px, py;
public:
  Flcc_HueBox(int X, int Y, int W, int H) : Fl_Widget(X,Y,W,H) {
  px = py = 0;}
};

class Flcc_ValueBox : public Fl_Widget {
  int handle(int);
  void draw();
  int py;
public:
  Flcc_ValueBox(int X, int Y, int W, int H) : Fl_Widget(X,Y,W,H) {
  py = 0;}
};

class Flcc_Value_Input : public Fl_Value_Input {
public:
  int format(char*);
  Flcc_Value_Input(int X, int Y, int W, int H) : Fl_Value_Input(X,Y,W,H) {}
};

class Fl_Color_Chooser : public Fl_Group {
  Flcc_HueBox huebox;
  Flcc_ValueBox valuebox;
  Fl_Choice choice;
  Flcc_Value_Input rvalue;
  Flcc_Value_Input gvalue;
  Flcc_Value_Input bvalue;
  Fl_Box resize_box;
  double hue_, saturation_, value_;
  double r_, g_, b_;
  void set_valuators();
  static void rgb_cb(Fl_Widget*, void*);
  static void mode_cb(Fl_Widget*, void*);
public:
  int mode() {return choice.value();}
  double hue() const {return hue_;}
  double saturation() const {return saturation_;}
  double value() const {return value_;}
  double r() const {return r_;}
  double g() const {return g_;}
  double b() const {return b_;}
  int hsv(double,double,double);
  int rgb(double,double,double);
  static void hsv2rgb(double, double, double,double&,double&,double&);
  static void rgb2hsv(double, double, double,double&,double&,double&);
  Fl_Color_Chooser(int,int,int,int,const char* = 0);
};

int fl_color_chooser(const char* name, double& r, double& g, double& b);
int fl_color_chooser(const char* name, uchar& r, uchar& g, uchar& b);

#endif
