//
// Contrast function test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022-2024 by Bill Spitzak and others.
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

// Note: this test and demo program is work in progress. It is published
// because it is helpful but it needs some more work to be "perfect" ;-)
// AlbrechtS

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/Fl_Terminal.H>
#include <FL/fl_draw.H>

#include <math.h>
#include <stdlib.h>
#include <string.h>

// program version

const char *version = "0.9.1";

// prototypes and forward declarations

static void button_cb(Fl_Widget *w, void *v);
static Fl_Color calc_contrast(Fl_Color fg, Fl_Color bg, Fl_Fontsize fs);

// class Button

class Button : public Fl_Button {
  char lbuf[20];      // private label buffer
  Fl_Color ocol_;     // "original" (label) color
  int idx_;           // button index (0 - 255)
public:
  Button(int X, int Y, int W, int H, int n)
    : Fl_Button(X, Y, W, H, "") {
    idx_ = n;
    box(FL_THIN_DOWN_BOX);
    label(lbuf);
    callback(button_cb);
    color((Fl_Color)n);
    sprintf(lbuf, "%03d", n);
    set_labelcolor(n);
    labelsize(15);
    labelfont(FL_HELVETICA);
  }

  void set_labelcolor(Fl_Color col) {
    ocol_ = col;
    labelcolor(calc_contrast(col, color(), labelsize()));
  }

  Fl_Color ocol() {
    return ocol_;
  }

  int idx() {
    return idx_;
  }

  void draw() FL_OVERRIDE {
    draw_box();
    // draw small filled rectangle with "original" color
    fl_color(ocol_);
    fl_rectf(x() + 5, y() + 5, 10, h() - 10);
    // measure and draw label
    int lw = 0, lh = 0;
    fl_font(labelfont(), labelsize());
    fl_measure(lbuf, lw, lh);
    fl_color(labelcolor());
    fl_draw(lbuf, x() + 15 + (w() - lw - 15) / 2, y() + h() - (h() - lh) / 2 - lh/4);
    fl_color(FL_BLACK);
  }

}; // class Button

// global variables

Fl_Terminal *term = 0;

double g_lfg;                   // perceived lightness of foreground color
double g_lbg;                   // perceived lightness of background color
double g_lcref;                 // calculated contrast reference (CIELAB, L*a*b*)
int g_selected = -1;            // selected button: -1 = none, 0 - 255 = valid button

Fl_Fontsize g_fs = 15;          // fontsize for button labels
int g_level = 0;                // *init* fl_contrast_level (sensitivity)

int g_algo = FL_CONTRAST_CIELAB;  // contrast algorithm: 0 = none, 1 = legacy (1.3.x), 2 = CIELAB, 3 = custom
const char *alch = "";            // algorithm as char: "LEGACY", "CIELAB" , or "CUSTOM"

Fl_Color lcolor = FL_BLACK;     // label color, set by slider callback
Button *buttons[256];           // array of color buttons
Fl_Value_Slider *sliders[6];    // array of sliders (gray, red, green, blue, level, fontsize)
Fl_Output *color_out = 0;       // color output (RRGGBB)

// Custom contrast algorithm: currently a dummy function (returns fg).
// This may be used to define a "better" contrast function in user code

static Fl_Color custom_contrast(Fl_Color fg, Fl_Color bg, Fl_Fontsize fs, int) {
  return fg;
}

/*
  Local function to calculate the contrast and store it in some
  global variables for display purposes and logging.

  This function is a wrapper around fl_contrast() in this demo program.
*/
static Fl_Color calc_contrast(Fl_Color fg, Fl_Color bg, Fl_Fontsize fs) {

  // Compute and set global *perceived* lightness L* (Lstar) and contrast for display

  g_lfg      = fl_lightness(fg);
  g_lbg      = fl_lightness(bg);
  g_lcref    = g_lfg - g_lbg;     // perceived contrast (light on dark = positive)

  switch (g_algo) {
    case FL_CONTRAST_NONE:              // 0 = none (return fg)
    case FL_CONTRAST_LEGACY:            // 1 = legacy (FLTK 1.3.x)
    case FL_CONTRAST_CIELAB:            // 2 = CIELAB (L*a*b*)
    case FL_CONTRAST_CUSTOM:            // 3 = Custom
      return fl_contrast(fg, bg, fs);
    default:
      break;
  }
  return fg;
}

// set all button label colors and adjust fontsize (labelsize)

static void update_labels() {
  for (int i = 0; i < 256; i++) {
    buttons[i]->set_labelcolor(lcolor);
    buttons[i]->labelsize(g_fs);
  }
}

static void button_cb(Fl_Widget *w, void *v) {
  Button *b = (Button *)w;
  g_selected = b->idx();                          // selected button index
  Fl_Color ocol = Fl::get_color(b->ocol());       // button's "original" label color (RGB0)
  Fl_Color fg = Fl::get_color(b->labelcolor());   // button's label color (RGB0)
  Fl_Color bg = Fl::get_color(b->color());        // button's background color (RGB0)
  calc_contrast(ocol, bg, g_fs);                  // calculate values to be displayed
  const char *color = "";                         // calculated label color (text)
  if (fg == ocol)            color = "fg";
  else if (fg == 0xffffff00) color = "WHITE";
  else if (fg == 0x0)        color = "BLACK";
  term->printf("[%s] fg: %06x, bg: %06x, lfg: %6.2f, lbg: %6.2f, lc: %7.2f, %s => %-5s",
                  b->label(), ocol >> 8, bg >> 8, g_lfg, g_lbg, g_lcref, alch, color);
  if (g_algo == FL_CONTRAST_LEGACY ||
      g_algo == FL_CONTRAST_CIELAB)
    term->printf(" (level = %3d)\n", g_level);
  else
    term->printf("\n");
}

void lf_cb(Fl_Widget *w, void *v) {
  term->printf("\n");
}

// callback for color (gray and R, G, B) sliders

void color_slider_cb(Fl_Widget *w, void *v) {
  int n = fl_int(v);                        // slider type: 0 = gray, 1 = color
  unsigned int r, g, b;
  if (n == 0) {                             // gray slider
    int val = (int)sliders[0]->value();
    lcolor = fl_rgb_color(val, val, val);   // set gray value
    sliders[1]->value(val);                 // set r/g/b values as well
    sliders[2]->value(val);
    sliders[3]->value(val);
    r = g = b = val;
  } else {                                  // any color slider
    r = (unsigned int)sliders[1]->value();
    g = (unsigned int)sliders[2]->value();
    b = (unsigned int)sliders[3]->value();
    lcolor = fl_rgb_color(r, g, b);         // set color value
  }
  // update button label colors
  update_labels();
  // output label color
  char color_buf[10];
  sprintf(color_buf, "%02X %02X %02X", r, g, b);
  color_out->value(color_buf);
  w->window()->redraw();
}

// callback for "level" and "fontsize" sliders

void slider_cb(Fl_Widget *w, void *v) {
  int n = fl_int(v);                            // slider type: 1 = level, 2 = fontsize
  switch (n) {
    case 1:                                     // fl_contrast_level()
      g_level = (int)sliders[n + 3]->value();
      fl_contrast_level(g_level);               // set/store current contrast level
      break;
    case 2:                                     // 2nd slider: fontsize (labelsize)
      g_fs = (int)sliders[n + 3]->value();
      break;
    default:
      break;
  }
  // update button label colors
  update_labels();
  w->window()->redraw();
}


// callback for the "random color" button

void rc_cb(Fl_Widget *w, void *v) {
  static bool first = true;
  unsigned int r, g, b;

  if (first) {
    // Seed the random number generator...
    srand((unsigned int)time(NULL));
    first = false;
    r = g = b = 0;  // initialize with black
  } else {
    r = rand() % 256;
    g = rand() % 256;
    b = rand() % 256;
  }

  sliders[1] -> value(r);
  sliders[2] -> value(g);
  sliders[3] -> value(b);

  // update button label colors
  lcolor = fl_rgb_color(r, g, b);         // set color value
  update_labels();
  // output label color
  char color_buf[10];
  sprintf(color_buf, "%02X %02X %02X", r, g, b);
  color_out->value(color_buf);
  w->window()->redraw();
}

// callback for contrast algorithm (radio buttons)

void algo_cb(Fl_Widget *w, void *v) {
  int val = fl_int(v);
  g_algo = val;
  switch(val) {
    case FL_CONTRAST_LEGACY: alch = "LEGACY"; fl_contrast_mode(val); break; // legacy 1.3.x
    case FL_CONTRAST_CIELAB: alch = "CIELAB"; fl_contrast_mode(val); break; // CIELAB L*a*b*
    case FL_CONTRAST_CUSTOM: alch = "CUSTOM"; fl_contrast_mode(val); break; // custom
    case FL_CONTRAST_NONE:
    default:
      alch = "none  ";
      fl_contrast_mode(FL_CONTRAST_NONE);
      break;
  }
  g_level = fl_contrast_level();  // get current contrast level (per mode)
  sliders[4]->value(g_level);     // set level slider value
  update_labels();                // update all button labels

  // print selected button's attributes
  if (g_selected >= 0) {
    button_cb(buttons[g_selected], (void *)0);
  }
  if (w)
    w->window()->redraw();
}

// color chooser callback
void color_cb(Fl_Widget *w, void *v) {
  Fl_Color_Chooser *cc = (Fl_Color_Chooser *)w;
  int r = (int)(cc->r() * 255);
  int g = (int)(cc->g() * 255);
  int b = (int)(cc->b() * 255);
  Fl_Color c = fl_rgb_color(r, g, b);
  Button *bt = buttons[255]; // last button
  bt->color(c);
  bt->set_labelcolor(lcolor);
  bt->redraw();
}

// ===============================================================
// ======================   main() program  ======================
// ===============================================================

int main(int argc, char **argv) {

  const int bw = 58;
  const int bh = 30;
  int cw = 16 * bw + 10;
  int ch = 16 * bh + 10;
  int ww = cw + 10;
  int wh = 16 * bh + 135 + 10 + 170 /* terminal */ + 10;
  Fl_Double_Window window(ww, wh, "fl_contrast test");

  int n = 0;
  Button **b = buttons;
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      *b = new Button(x * bw + 10, y * bh + 10, bw, bh, n);
      (*b)->set_labelcolor(n);
      b++;
      n++;
    }
  }

  // sliders for label color (gray, red, green, blue)

  const int sx = 10 + bw;
  const int sw = 5 * bw;
  const int sh = 25;
  int sy = ch + 10;

  Fl_Hor_Value_Slider *gray = new Fl_Hor_Value_Slider(sx, sy, sw, sh, "gray");
  gray->color(0xdddddd00);
  gray->textsize(13);
  gray->align(FL_ALIGN_LEFT);
  gray->value(0);
  gray->bounds(0, 255);
  gray->step(1);
  gray->callback(color_slider_cb, (void *)0);
  sy += sh + 10;

  Fl_Hor_Value_Slider *red = new Fl_Hor_Value_Slider(sx, sy, sw, sh, "red");
  red->color(FL_RED);
  red->textcolor(FL_WHITE);
  red->textsize(13);
  red->align(FL_ALIGN_LEFT);
  red->value(0);
  red->bounds(0, 255);
  red->step(1);
  red->callback(color_slider_cb, (void *)1);
  sy += sh + 5;

  Fl_Hor_Value_Slider *green = new Fl_Hor_Value_Slider(sx, sy, sw, sh, "green");
  green->color(FL_GREEN);
  green->textsize(13);
  green->align(FL_ALIGN_LEFT);
  green->value(0);
  green->bounds(0, 255);
  green->step(1);
  green->callback(color_slider_cb, (void *)1);
  sy += sh + 5;

  Fl_Hor_Value_Slider *blue = new Fl_Hor_Value_Slider(sx, sy, sw, sh, "blue");
  blue->color(FL_BLUE);
  blue->textcolor(FL_WHITE);
  blue->textsize(13);
  blue->align(FL_ALIGN_LEFT);
  blue->value(0);
  blue->bounds(0, 255);
  blue->step(1);
  blue->callback(color_slider_cb, (void *)1);

  sliders[0] = gray;
  sliders[1] = red;
  sliders[2] = green;
  sliders[3] = blue;

  // contrast algorithm selection group

  int cgx = 10 + 6*bw + 10;
  int cgy = ch + 30;
  int cgw =  90;
  int cgh = 100;
  int abh = 25;

  Fl_Group *cg = new Fl_Group(cgx, cgy, cgw, cgh, "fl_contrast:");
  cg->align(FL_ALIGN_TOP);
  cg->box(FL_FRAME);

  Fl_Radio_Round_Button *anon = new Fl_Radio_Round_Button(cgx, cgy,      cgw, abh, "none");
  Fl_Radio_Round_Button *aleg = new Fl_Radio_Round_Button(cgx, cgy + 25, cgw, abh, "LEGACY");
  Fl_Radio_Round_Button *acie = new Fl_Radio_Round_Button(cgx, cgy + 50, cgw, abh, "CIELAB");
  Fl_Radio_Round_Button *aapc = new Fl_Radio_Round_Button(cgx, cgy + 75, cgw, abh, "CUSTOM");
  acie->value(1);
  anon->callback(algo_cb, (void *)0);
  aleg->callback(algo_cb, (void *)1);
  acie->callback(algo_cb, (void *)2);
  aapc->callback(algo_cb, (void *)3);

  cg->end();

  color_out = new Fl_Output(10 + 10 * bw, ch + 10, 100, 30, "label color:");
  color_out->align(FL_ALIGN_LEFT);
  color_out->textfont(FL_COURIER);
  color_out->textsize(16);
  color_out->value("00 00 00");

  // light blue "level" slider

  Fl_Hor_Value_Slider *s_level = new Fl_Hor_Value_Slider(10 + 9 * bw, red->y(), 3 * bw - 15, sh, "level");
  s_level->color(231);
  s_level->textcolor(224);
  s_level->textsize(13);
  s_level->align(FL_ALIGN_LEFT);
  s_level->step(1);
  s_level->bounds(0, 100);
  s_level->value(g_level);
  s_level->callback(slider_cb, (void *)1);
  s_level->tooltip("set contrast sensitivity level (0-100), default: 50");

  // labelsize slider

  Fl_Hor_Value_Slider *s_fs = new Fl_Hor_Value_Slider(10 + 9 * bw, green->y(), 3 * bw - 15, sh, "labelsize");
  s_fs->color(231);
  s_fs->textcolor(224);
  s_fs->textsize(13);
  s_fs->align(FL_ALIGN_LEFT);
  s_fs->step(1);
  s_fs->bounds(8, 24);
  s_fs->value(15);
  s_fs->callback(slider_cb, (void *)2);
  s_fs->tooltip("set label/text fontsize");

  sliders[4] = s_level;
  sliders[5] = s_fs;

  // line feed (LF) button

  Fl_Button *lf = new Fl_Button(10 + 8 * bw, blue->y(), bw, sh, "LF");
  lf->tooltip("Click to output a linefeed to the log.");
  lf->callback(lf_cb);

  // random color (R) button

  Fl_Button *rc = new Fl_Button(10 + 8 * bw + lf->w() + 2, blue->y(), bw*3/4, sh, "&RC");
  rc->tooltip("Click to select a random text color.");
  rc->callback(rc_cb);

  // color chooser for field #255

  int ccx = 10 + 12 * bw;
  int ccy = ch + 10;
  int ccw = 4 * bw;
  int cch = 120;

  Fl_Color_Chooser *color_chooser = new Fl_Color_Chooser(ccx, ccy, ccw, cch);
  color_chooser->callback(color_cb);
  color_chooser->label("bg color [255] @->");
  color_chooser->rgb(1, 1, 1);
  color_chooser->mode(1); // byte mode
  color_chooser->align(FL_ALIGN_LEFT_BOTTOM);

  // set contrast mode and level, update button label colors

  fl_contrast_mode(g_algo);
  fl_contrast_function(custom_contrast);  // dummy contrast function
  algo_cb(acie, fl_voidptr(2));

  // Fl_Terminal for output

  int ttx = 10;
  int tty = color_chooser->y() + cch + 10;
  int ttw = window.w() - 20;
  int tth = window.h() - tty - 10;

  term = new Fl_Terminal(ttx, tty, ttw, tth);
  term->color(FL_WHITE);
  term->textfgcolor(FL_BLACK);
  term->textsize(13);

  term->printf("FLTK %d.%d.%d fl_contrast() test program with different contrast algorithms, version %s\n",
               FL_MAJOR_VERSION, FL_MINOR_VERSION, FL_PATCH_VERSION, version);
  term->printf(" - Select a foreground (text) color with the gray or red/green/blue sliders (displayed inside each field).\n");
  term->printf(" - Select an arbitrary background color for field #255 with the color chooser.\n");
  term->printf(" - Select a colored field (by clicking on it) to display its attributes.\n");
  term->printf(" - Select the contrast algorithm by clicking on the radio buttons.\n");
  term->printf(" - Tune the contrast algorithm with the light blue \"level\" slider (default: %d).\n", fl_contrast_level());
  term->printf(" - Select a random foreground (text) color by clicking the RC button\n");

  window.resizable(term);
  window.end();
  window.show(argc, argv);
  rc_cb(rc, 0); // update button labels - must be called after show()
  return Fl::run();
}
