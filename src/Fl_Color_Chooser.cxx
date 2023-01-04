//
// Color chooser for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2019 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/fl_draw.H>
#include <FL/math.h>
#include <stdio.h>

// Besides being a useful object on it's own, the Fl_Color_Chooser was
// an attempt to make a complex composite object that could be easily
// imbedded into a user interface.  If you wish to make complex objects
// of your own, be sure to read this code.

// The function fl_color_chooser() creates a window containing a color
// chooser and a few buttons and current-color indicators.  It is an
// easier interface for simple programs that just need a color.

// The "hue box" can be a circle or rectilinear.
// You get a circle by defining this:
#define CIRCLE 1
// And the "hue box" can auto-update when the value changes
// you get this by defining this:
#define UPDATE_HUE_BOX 1

/**
  This \e static method converts HSV colors to RGB colorspace.
  \param[in] H, S, V color components
  \param[out] R, G, B color components
 */
void Fl_Color_Chooser::hsv2rgb(
        double H, double S, double V, double& R, double& G, double& B) {
  if (S < 5.0e-6) {
    R = G = B = V;
  } else {
    int i = (int)H;
    double f = H - (float)i;
    double p1 = V*(1.0-S);
    double p2 = V*(1.0-S*f);
    double p3 = V*(1.0-S*(1.0-f));
    switch (i) {
    case 0: R = V;   G = p3;  B = p1;  break;
    case 1: R = p2;  G = V;   B = p1;  break;
    case 2: R = p1;  G = V;   B = p3;  break;
    case 3: R = p1;  G = p2;  B = V;   break;
    case 4: R = p3;  G = p1;  B = V;   break;
    case 5: R = V;   G = p1;  B = p2;  break;
    }
  }
}

/**
  This \e static method converts RGB colors to HSV colorspace.
  \param[in] R, G, B color components
  \param[out] H, S, V color components
 */
void Fl_Color_Chooser::rgb2hsv(
        double R, double G, double B, double& H, double& S, double& V) {
  double maxv = R > G ? R : G; if (B > maxv) maxv = B;
  V = maxv;
  if (maxv>0) {
    double minv = R < G ? R : G; if (B < minv) minv = B;
    S = 1.0 - double(minv)/maxv;
    if (maxv > minv) {
      if (maxv == R) {H = (G-B)/double(maxv-minv); if (H<0) H += 6.0;}
      else if (maxv == G) H = 2.0+(B-R)/double(maxv-minv);
      else H = 4.0+(R-G)/double(maxv-minv);
    }
  }
}

/** Fl_Color_Chooser modes */
enum {
  M_RGB,        /**< mode() of Fl_Color_Chooser showing RGB values */
  M_BYTE,       /**< mode() of Fl_Color_Chooser showing byte values */
  M_HEX,        /**< mode() of Fl_Color_Chooser showing hex values */
  M_HSV         /**< mode() of Fl_Color_Chooser showing HSV values */
};
static const Fl_Menu_Item mode_menu[] = {
  {"rgb"},
  {"byte"},
  {"hex"},
  {"hsv"},
  {0}
};

#ifndef FL_DOXYGEN
int Flcc_Value_Input::format(char* buf) {
  Fl_Color_Chooser* c = (Fl_Color_Chooser*)parent();
  if (c->mode() == M_HEX) return snprintf(buf, 5,"0x%02X", int(value()));
  else return Fl_Valuator::format(buf);
}
#endif // !FL_DOXYGEN

void Fl_Color_Chooser::set_valuators() {
  switch (mode()) {
  case M_RGB:
    rvalue.range(0,1); rvalue.step(1,1000); rvalue.value(r_);
    gvalue.range(0,1); gvalue.step(1,1000); gvalue.value(g_);
    bvalue.range(0,1); bvalue.step(1,1000); bvalue.value(b_);
    break;
  case M_BYTE: /* FALLTHROUGH */
  case M_HEX:
    rvalue.range(0,255); rvalue.step(1); rvalue.value(int(255*r_+.5));
    gvalue.range(0,255); gvalue.step(1); gvalue.value(int(255*g_+.5));
    bvalue.range(0,255); bvalue.step(1); bvalue.value(int(255*b_+.5));
    break;
  case M_HSV:
    rvalue.range(0,6); rvalue.step(1,1000); rvalue.value(hue_);
    gvalue.range(0,1); gvalue.step(1,1000); gvalue.value(saturation_);
    bvalue.range(0,1); bvalue.step(1,1000); bvalue.value(value_);
    break;
  }
}

/**
  Sets the current rgb color values.
  Does not do the callback. Does not clamp (but out of range values will
  produce psychedelic effects in the hue selector).
  \param[in] R, G, B color components.
  \return 1 if a new rgb value was set, 0 if the rgb value was the previous one.
 */
int Fl_Color_Chooser::rgb(double R, double G, double B) {
  if (R == r_ && G == g_ && B == b_) return 0;
  r_ = R; g_ = G; b_ = B;
  double ph = hue_;
  double ps = saturation_;
  double pv = value_;
  rgb2hsv(R,G,B,hue_,saturation_,value_);
  set_valuators();
  set_changed();
  if (value_ != pv) {
#ifdef UPDATE_HUE_BOX
    huebox.damage(FL_DAMAGE_SCROLL);
#endif
    valuebox.damage(FL_DAMAGE_EXPOSE);}
  if (hue_ != ph || saturation_ != ps) {
    huebox.damage(FL_DAMAGE_EXPOSE);
    valuebox.damage(FL_DAMAGE_SCROLL);
  }
  return 1;
}

/**
  Set the hsv values.
  The passed values are clamped (or for hue, modulus 6 is used) to get
  legal values. Does not do the callback.
  \param[in] H, S, V color components.
  \return 1 if a new hsv value was set, 0 if the hsv value was the previous one.
*/
int Fl_Color_Chooser::hsv(double H, double S, double V) {
  H = fmod(H,6.0); if (H < 0.0) H += 6.0;
  if (S < 0.0) S = 0.0; else if (S > 1.0) S = 1.0;
  if (V < 0.0) V = 0.0; else if (V > 1.0) V = 1.0;
  if (H == hue_ && S == saturation_ && V == value_) return 0;
  double ph = hue_;
  double ps = saturation_;
  double pv = value_;
  hue_ = H; saturation_ = S; value_ = V;
  if (value_ != pv) {
#ifdef UPDATE_HUE_BOX
    huebox.damage(FL_DAMAGE_SCROLL);
#endif
    valuebox.damage(FL_DAMAGE_EXPOSE);}
  if (hue_ != ph || saturation_ != ps) {
    huebox.damage(FL_DAMAGE_EXPOSE);
    valuebox.damage(FL_DAMAGE_SCROLL);
  }
  hsv2rgb(H,S,V,r_,g_,b_);
  set_valuators();
  set_changed();
  return 1;
}

////////////////////////////////////////////////////////////////

static void tohs(double x, double y, double& h, double& s) {
#ifdef CIRCLE
  x = 2*x-1;
  y = 1-2*y;
  s = sqrt(x*x+y*y); if (s > 1.0) s = 1.0;
  h = (3.0/M_PI)*atan2(y,x);
  if (h<0) h += 6.0;
#else
  h = fmod(6.0*x,6.0); if (h < 0.0) h += 6.0;
  s = 1.0-y; if (s < 0.0) s = 0.0; else if (s > 1.0) s = 1.0;
#endif
}

#ifndef FL_DOXYGEN
int Flcc_HueBox::handle(int e) {
  static double ih, is;
  Fl_Color_Chooser* c = (Fl_Color_Chooser*)parent();
  switch (e) {
  case FL_PUSH:
    if (Fl::visible_focus()) {
      Fl::focus(this);
      redraw();
    }
    ih = c->hue();
    is = c->saturation();
  case FL_DRAG: {
    double Xf, Yf, H, S;
    Xf = (Fl::event_x()-x()-Fl::box_dx(box()))/double(w()-Fl::box_dw(box()));
    Yf = (Fl::event_y()-y()-Fl::box_dy(box()))/double(h()-Fl::box_dh(box()));
    tohs(Xf, Yf, H, S);
    if (fabs(H-ih) < 3*6.0/w()) H = ih;
    if (fabs(S-is) < 3*1.0/h()) S = is;
    if (Fl::event_state(FL_CTRL)) H = ih;
    if (c->hsv(H, S, c->value())) c->do_callback(FL_REASON_DRAGGED);
    } return 1;
  case FL_FOCUS : /* FALLTHROUGH */
  case FL_UNFOCUS :
    if (Fl::visible_focus()) {
      redraw();
      return 1;
    }
    else return 1;
  case FL_KEYBOARD :
    return handle_key(Fl::event_key());
  default:
    return 0;
  }
}
#endif // !FL_DOXYGEN

static void generate_image(void* vv, int X, int Y, int W, uchar* buf) {
  Flcc_HueBox* v = (Flcc_HueBox*)vv;
  int iw = v->w()-Fl::box_dw(v->box());
  double Yf = double(Y)/(v->h()-Fl::box_dh(v->box()));
#ifdef UPDATE_HUE_BOX
  const double V = ((Fl_Color_Chooser*)(v->parent()))->value();
#else
  const double V = 1.0;
#endif
  for (int x = X; x < X+W; x++) {
    double Xf = double(x)/iw;
    double H,S; tohs(Xf,Yf,H,S);
    double r=0, g=0, b=0;
    Fl_Color_Chooser::hsv2rgb(H,S,V,r,g,b);
    *buf++ = uchar(255*r+.5);
    *buf++ = uchar(255*g+.5);
    *buf++ = uchar(255*b+.5);
  }
}

#ifndef FL_DOXYGEN
int Flcc_HueBox::handle_key(int key) {
  int w1 = w()-Fl::box_dw(box())-6;
  int h1 = h()-Fl::box_dh(box())-6;
  Fl_Color_Chooser* c = (Fl_Color_Chooser*)parent();

#ifdef CIRCLE
  int X = int(.5*(cos(c->hue()*(M_PI/3.0))*c->saturation()+1) * w1);
  int Y = int(.5*(1-sin(c->hue()*(M_PI/3.0))*c->saturation()) * h1);
#else
  int X = int(c->hue()/6.0*w1);
  int Y = int((1-c->saturation())*h1);
#endif

  switch (key) {
    case FL_Up :
      Y -= 3;
      break;
    case FL_Down :
      Y += 3;
      break;
    case FL_Left :
      X -= 3;
      break;
    case FL_Right :
      X += 3;
      break;
    default :
      return 0;
  }

  double Xf, Yf, H, S;
  Xf = (double)X/(double)w1;
  Yf = (double)Y/(double)h1;
  tohs(Xf, Yf, H, S);
  if (c->hsv(H, S, c->value())) c->do_callback(FL_REASON_CHANGED);

  return 1;
}
#endif // !FL_DOXYGEN

#ifndef FL_DOXYGEN
void Flcc_HueBox::draw() {
  if (damage()&FL_DAMAGE_ALL) draw_box();
  int x1 = x()+Fl::box_dx(box());
  int yy1 = y()+Fl::box_dy(box());
  int w1 = w()-Fl::box_dw(box());
  int h1 = h()-Fl::box_dh(box());
  if (w1>0 && h1>0) {
    if (damage() == FL_DAMAGE_EXPOSE) fl_push_clip(x1+px,yy1+py,6,6);
    fl_draw_image(generate_image, this, x1, yy1, w1, h1);
    if (damage() == FL_DAMAGE_EXPOSE) fl_pop_clip();
  }
  Fl_Color_Chooser* c = (Fl_Color_Chooser*)parent();
#ifdef CIRCLE
  int X = int(.5*(cos(c->hue()*(M_PI/3.0))*c->saturation()+1) * (w1-6));
  int Y = int(.5*(1-sin(c->hue()*(M_PI/3.0))*c->saturation()) * (h1-6));
#else
  int X = int(c->hue()/6.0*(w1-6));
  int Y = int((1-c->saturation())*(h1-6));
#endif
  if (X < 0) X = 0; else if (X > w1-6) X = w1-6;
  if (Y < 0) Y = 0; else if (Y > h1-6) Y = h1-6;
  //  fl_color(c->value()>.75 ? FL_BLACK : FL_WHITE);
  if (w1>0 && h1>0) {
    fl_push_clip(x1,yy1,w1,h1);
    draw_box(FL_UP_BOX,x1+X,yy1+Y,6,6,Fl::focus() == this ? FL_FOREGROUND_COLOR : FL_GRAY);
    fl_pop_clip();
  }
  px = X; py = Y;
}
#endif // !FL_DOXYGEN

////////////////////////////////////////////////////////////////

#ifndef FL_DOXYGEN
int Flcc_ValueBox::handle(int e) {
  static double iv;
  Fl_Color_Chooser* c = (Fl_Color_Chooser*)parent();
  switch (e) {
  case FL_PUSH:
    if (Fl::visible_focus()) {
      Fl::focus(this);
      redraw();
    }
    iv = c->value();
  case FL_DRAG: {
    double Yf;
    Yf = 1-(Fl::event_y()-y()-Fl::box_dy(box()))/double(h()-Fl::box_dh(box()));
    if (fabs(Yf-iv)<(3*1.0/h())) Yf = iv;
    if (c->hsv(c->hue(),c->saturation(),Yf)) c->do_callback(FL_REASON_DRAGGED);
    } return 1;
  case FL_FOCUS : /* FALLTHROUGH */
  case FL_UNFOCUS :
    if (Fl::visible_focus()) {
      redraw();
      return 1;
    }
    else return 1;
  case FL_KEYBOARD :
    return handle_key(Fl::event_key());
  default:
    return 0;
  }
}
#endif // !FL_DOXYGEN

static double tr, tg, tb;
static void generate_vimage(void* vv, int X, int Y, int W, uchar* buf) {
  Flcc_ValueBox* v = (Flcc_ValueBox*)vv;
  double Yf = 255*(1.0-double(Y)/(v->h()-Fl::box_dh(v->box())));
  uchar r = uchar(tr*Yf+.5);
  uchar g = uchar(tg*Yf+.5);
  uchar b = uchar(tb*Yf+.5);
  for (int x = X; x < X+W; x++) {
    *buf++ = r; *buf++ = g; *buf++ = b;
  }
}

#ifndef FL_DOXYGEN
void Flcc_ValueBox::draw() {
  if (damage()&FL_DAMAGE_ALL) draw_box();
  Fl_Color_Chooser* c = (Fl_Color_Chooser*)parent();
  c->hsv2rgb(c->hue(),c->saturation(),1.0,tr,tg,tb);
  int x1 = x()+Fl::box_dx(box());
  int yy1 = y()+Fl::box_dy(box());
  int w1 = w()-Fl::box_dw(box());
  int h1 = h()-Fl::box_dh(box());
  if (w1>0 && h1>0) {
    if (damage() == FL_DAMAGE_EXPOSE) fl_push_clip(x1,yy1+py,w1,6);
    fl_draw_image(generate_vimage, this, x1, yy1, w1, h1);
    if (damage() == FL_DAMAGE_EXPOSE) fl_pop_clip();
  }
  int Y = int((1-c->value()) * (h1-6));
  if (Y < 0) Y = 0; else if (Y > h1-6) Y = h1-6;
  draw_box(FL_UP_BOX,x1,yy1+Y,w1,6,Fl::focus() == this ? FL_FOREGROUND_COLOR : FL_GRAY);
  py = Y;
}
#endif // !FL_DOXYGEN

#ifndef FL_DOXYGEN
int Flcc_ValueBox::handle_key(int key) {
  int h1 = h()-Fl::box_dh(box())-6;
  Fl_Color_Chooser* c = (Fl_Color_Chooser*)parent();

  int Y = int((1-c->value()) * h1);
  if (Y < 0) Y = 0; else if (Y > h1) Y = h1;

  switch (key) {
    case FL_Up :
      Y -= 3;
      break;
    case FL_Down :
      Y += 3;
      break;
    default :
      return 0;
  }

  double Yf;
  Yf = 1-((double)Y/(double)h1);
  if (c->hsv(c->hue(),c->saturation(),Yf)) c->do_callback(FL_REASON_CHANGED);

  return 1;
}
#endif // !FL_DOXYGEN

////////////////////////////////////////////////////////////////

void Fl_Color_Chooser::rgb_cb(Fl_Widget* o, void*) {
  Fl_Color_Chooser* c = (Fl_Color_Chooser*)(o->parent());
  double R = c->rvalue.value();
  double G = c->gvalue.value();
  double B = c->bvalue.value();
  if (c->mode() == M_HSV) {
    if (c->hsv(R,G,B)) c->do_callback(FL_REASON_CHANGED);
    return;
  }
  if (c->mode() != M_RGB) {
    R = R/255;
    G = G/255;
    B = B/255;
  }
  if (c->rgb(R,G,B)) c->do_callback(FL_REASON_CHANGED);
}

void Fl_Color_Chooser::mode_cb(Fl_Widget* o, void*) {
  Fl_Color_Chooser* c = (Fl_Color_Chooser*)(o->parent());
  // force them to redraw even if value is the same:
  c->rvalue.value(-1);
  c->gvalue.value(-1);
  c->bvalue.value(-1);
  c->set_valuators();
}

void Fl_Color_Chooser::mode(int newMode)
{
  choice.value(newMode);
  choice.do_callback(FL_REASON_RESELECTED);
}

// Small local helper function:
// Copy hex color value ('RRGGBB') of Fl_Color_Chooser to clipboard.
// Always returns 1 (event was used).

static int copy_rgb(double r, double g, double b) {
  char buf[8];
  int len;
  len = snprintf(buf, 8, "%02X%02X%02X", int(r * 255 + .5), int(g * 255 + .5), int(b * 255 + .5));
  Fl::copy(buf, len, 1);
  // printf("copied '%s' to clipboard\n", buf); // Debug
  return 1;
}

/**
  Handles all events received by this widget.

  This specific handle() method processes the standard 'copy' function
  as seen in other input widgets. It copies the current color value to
  the clipboard as a string in RGB format ('RRGGBB').

  This format is independent of the Fl_Color_Chooser display format
  setting. No other formats are supplied.

  The keyboard events handled are:
    - ctrl-c
    - ctrl-x
    - ctrl-Insert

  All other events are processed by the parent class \c Fl_Group.

  This enables the \b user to choose a color value, press
  \p ctrl-c to copy the value to the clipboard and paste it into
  a color selection widget in another application window or any
  other text input (e.g. a preferences dialog or an editor).

  \note Keyboard event handling by the current focus widget has
    priority, hence moving the focus to one of the buttons or
    selecting text in one of the input widgets effectively
    disables this special method.

  \param[in]    e  current event
  \returns      1  if event has been handled, 0 otherwise

  \see Fl_Group::handle(int)
*/

int Fl_Color_Chooser::handle(int e) {

  int mods = Fl::event_state() & (FL_META | FL_CTRL | FL_ALT);
  unsigned int shift = Fl::event_state() & FL_SHIFT;

  switch (e) {
    case FL_KEYBOARD:
    case FL_SHORTCUT:
      // ignore CTRL-SHIFT-C, CTRL-SHIFT-X and CTRL-SHIFT-Insert
      if (shift)
        return Fl_Group::handle(e);
      switch (Fl::event_key()) {
        case FL_Insert:
          if (mods == FL_CTRL)
            return copy_rgb(r_, g_, b_);
          break;
        case 'c':
        case 'x':
          if (mods == FL_COMMAND)
            return copy_rgb(r_, g_, b_);
          break;
        default:
          break;
      }
    default:
      break;
  }
  return Fl_Group::handle(e);
}

////////////////////////////////////////////////////////////////

/**
  Creates a new Fl_Color_Chooser widget using the given position, size, and
  label string.
  The recommended dimensions are 200x95. The color is initialized to black.
  \param[in] X, Y, W, H position and size of the widget
  \param[in] L widget label, default is no label
 */
Fl_Color_Chooser::Fl_Color_Chooser(int X, int Y, int W, int H, const char* L)
  : Fl_Group(0,0,195,115,L),
    huebox(0,0,115,115),
    valuebox(115,0,20,115),
    choice(140,0,55,25),
    rvalue(140,30,55,25),
    gvalue(140,60,55,25),
    bvalue(140,90,55,25),
    resize_box(0,0,115,115)
{
  end();
  resizable(resize_box);
  resize(X,Y,W,H);
  r_ = g_ = b_ = 0;
  hue_ = 0.0;
  saturation_ = 0.0;
  value_ = 0.0;
  huebox.box(FL_DOWN_FRAME);
  valuebox.box(FL_DOWN_FRAME);
  choice.menu(mode_menu);
  set_valuators();
  rvalue.callback(rgb_cb);
  gvalue.callback(rgb_cb);
  bvalue.callback(rgb_cb);
  choice.callback(mode_cb);
  choice.box(FL_THIN_UP_BOX);
  choice.textfont(FL_HELVETICA_BOLD_ITALIC);
}

////////////////////////////////////////////////////////////////
// fl_color_chooser():

#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Return_Button.H>

class ColorChip : public Fl_Widget {
  void draw() FL_OVERRIDE;
public:
  uchar r,g,b;
  ColorChip(int X, int Y, int W, int H) : Fl_Widget(X,Y,W,H) {
    box(FL_ENGRAVED_FRAME);}
};

void ColorChip::draw() {
  if (damage()&FL_DAMAGE_ALL) draw_box();
  fl_rectf(x()+Fl::box_dx(box()),
           y()+Fl::box_dy(box()),
           w()-Fl::box_dw(box()),
           h()-Fl::box_dh(box()),r,g,b);
}

static void chooser_cb(Fl_Widget* o, void* vv) {
  Fl_Color_Chooser* c = (Fl_Color_Chooser*)o;
  ColorChip* v = (ColorChip*)vv;
  v->r = uchar(255*c->r()+.5);
  v->g = uchar(255*c->g()+.5);
  v->b = uchar(255*c->b()+.5);
  v->damage(FL_DAMAGE_EXPOSE);
}

extern const char* fl_ok;
extern const char* fl_cancel;

// fl_color_chooser's callback for ok_button (below)
//  [in] o is a pointer to okay_button (below)
//  [in] p is a pointer to an int to receive the return value (1)
// closes the fl_color_chooser window
static void cc_ok_cb (Fl_Widget *o, void *p) {
  *((int *)p) = 1; // set return value
  o->window()->hide();
}

// fl_color_chooser's callback for cancel_button and window close
//  [in] o is a pointer to cancel_button (below) _or_ the dialog window
//  [in] p is a pointer to an int to receive the return value (0)
// closes the fl_color_chooser window
static void cc_cancel_cb (Fl_Widget *o, void *p) {
  *((int *)p) = 0; // set return value
  if (o->window()) // cancel button
    o->window()->hide();
  else // window close
    o->hide();
}

/** \addtogroup  group_comdlg
    @{ */
/**
  \brief Pops up a window to let the user pick an arbitrary RGB color.
  \note \#include <FL/Fl_Color_Chooser.H>
  \image html fl_color_chooser.jpg
  \image latex  fl_color_chooser.jpg "fl_color_chooser" width=8cm
  \param[in] name Title label for the window
  \param[in,out] r, g, b Color components in the range 0.0 to 1.0.
  \param[in] cmode Optional mode for color chooser. See mode(int). Default -1 if none (rgb mode).
  \retval 1 if user confirms the selection
  \retval 0 if user cancels the dialog
  \relates Fl_Color_Chooser
 */
int fl_color_chooser(const char* name, double& r, double& g, double& b, int cmode) {
  int ret = 0;
  Fl_Window window(215,200,name);
  window.callback(cc_cancel_cb,&ret);
  Fl_Color_Chooser chooser(10, 10, 195, 115);
  ColorChip ok_color(10, 130, 95, 25);
  Fl_Return_Button ok_button(10, 165, 95, 25, fl_ok);
  ok_button.callback(cc_ok_cb,&ret);
  ColorChip cancel_color(110, 130, 95, 25);
  cancel_color.r = uchar(255*r+.5); ok_color.r = cancel_color.r;
  ok_color.g = cancel_color.g = uchar(255*g+.5);
  ok_color.b = cancel_color.b = uchar(255*b+.5);
  Fl_Button cancel_button(110, 165, 95, 25, fl_cancel);
  cancel_button.callback(cc_cancel_cb,&ret);
  window.resizable(chooser);
  chooser.rgb(r,g,b);
  chooser.callback(chooser_cb, &ok_color);
  if (cmode!=-1) chooser.mode(cmode);
  window.end();
  window.set_modal();
  window.hotspot(window);
  window.show();
  while (window.shown()) Fl::wait();
  if (ret) { // ok_button or Enter
    r = chooser.r();
    g = chooser.g();
    b = chooser.b();
  }
  return ret;
}

/**
  \brief Pops up a window to let the user pick an arbitrary RGB color.
  \note \#include <FL/Fl_Color_Chooser.H>
  \image html fl_color_chooser.jpg
  \image latex  fl_color_chooser.jpg "fl_color_chooser" width=8cm
  \param[in] name Title label for the window
  \param[in,out] r, g, b Color components in the range 0 to 255.
  \param[in] cmode Optional mode for color chooser. See mode(int). Default -1 if none (rgb mode).
  \retval 1 if user confirms the selection
  \retval 0 if user cancels the dialog
  \relates Fl_Color_Chooser
 */
int fl_color_chooser(const char* name, uchar& r, uchar& g, uchar& b, int cmode) {
  double dr = r/255.0;
  double dg = g/255.0;
  double db = b/255.0;
  if (fl_color_chooser(name,dr,dg,db,cmode)) {
    r = uchar(255*dr+.5);
    g = uchar(255*dg+.5);
    b = uchar(255*db+.5);
    return 1;
  }
  return 0;
}

/** @} */
