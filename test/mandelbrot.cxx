//
// Mandelbrot set demo for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

#include "mandelbrot_ui.h"
#include <FL/fl_draw.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Printer.H>
#include <stdio.h>
#include <stdlib.h>

const unsigned int palette[] = {
  0xCC0000, 0xCA0002, 0xC90004, 0xC70006, 0xC60008, 0xC4000A, 0xC3000C, 0xC1000E, 0xBF0010, 0xBE0012,
  0xBC0014, 0xBB0016, 0xB90018, 0xB8001A, 0xB6001B, 0xB4001D, 0xB3001F, 0xB10021, 0xB00023, 0xAE0025,
  0xAD0027, 0xAB0029, 0xA9002B, 0xA8002D, 0xA6002F, 0xA50031, 0xA30033, 0xA20035, 0xA00037, 0x9E0039,
  0x9D003B, 0x9B003D, 0x9A003F, 0x980041, 0x970043, 0x950045, 0x940047, 0x920049, 0x90004B, 0x8F004D,
  0x8D004E, 0x8C0050, 0x8A0052, 0x890054, 0x870056, 0x850058, 0x84005A, 0x82005C, 0x81005E, 0x7F0060,
  0x7E0062, 0x7C0064, 0x7A0066, 0x790068, 0x77006A, 0x76006C, 0x74006E, 0x730070, 0x710072, 0x6F0074,
  0x6E0076, 0x6C0078, 0x6B007A, 0x69007C, 0x68007E, 0x660080, 0x640081, 0x630083, 0x610085, 0x600087,
  0x5E0089, 0x5D008B, 0x5B008D, 0x59008F, 0x580091, 0x560093, 0x550095, 0x530097, 0x520099, 0x50009B,
  0x4E009D, 0x4D009F, 0x4B00A1, 0x4A00A3, 0x4800A5, 0x4700A7, 0x4500A9, 0x4300AB, 0x4200AD, 0x4000AF,
  0x3F00B1, 0x3D00B3, 0x3C00B4, 0x3A00B6, 0x3800B8, 0x3700BA, 0x3500BC, 0x3400BE, 0x3200C0, 0x3100C2,
  0x2F00C4, 0x2E00C6, 0x2C00C8, 0x2A00CA, 0x2900CC, 0x2700CE, 0x2600D0, 0x2400D2, 0x2300D4, 0x2100D6,
  0x1F00D8, 0x1E00DA, 0x1C00DC, 0x1B00DE, 0x1900E0, 0x1800E2, 0x1600E4, 0x1400E6, 0x1300E7, 0x1100E9,
  0x1000EB, 0x0E00ED, 0x0D00EF, 0x0B00F1, 0x0900F3, 0x0800F5, 0x0600F7, 0x0500F9, 0x0300FB, 0x0200FD,
  0x0000FF, 0x0202FD, 0x0404FB, 0x0606F9, 0x0808F7, 0x0A0AF5, 0x0C0CF3, 0x0E0EF1, 0x1010EF, 0x1212ED,
  0x1414EB, 0x1616E9, 0x1818E7, 0x1A1AE6, 0x1B1BE4, 0x1D1DE2, 0x1F1FE0, 0x2121DE, 0x2323DC, 0x2525DA,
  0x2727D8, 0x2929D6, 0x2B2BD4, 0x2D2DD2, 0x2F2FD0, 0x3131CE, 0x3333CC, 0x3535CA, 0x3737C8, 0x3939C6,
  0x3B3BC4, 0x3D3DC2, 0x3F3FC0, 0x4141BE, 0x4343BC, 0x4545BA, 0x4747B8, 0x4949B6, 0x4B4BB4, 0x4D4DB3,
  0x4E4EB1, 0x5050AF, 0x5252AD, 0x5454AB, 0x5656A9, 0x5858A7, 0x5A5AA5, 0x5C5CA3, 0x5E5EA1, 0x60609F,
  0x62629D, 0x64649B, 0x666699, 0x686897, 0x6A6A95, 0x6C6C93, 0x6E6E91, 0x70708F, 0x72728D, 0x74748B,
  0x767689, 0x787887, 0x7A7A85, 0x7C7C83, 0x7E7E81, 0x808080, 0x81817E, 0x83837C, 0x85857A, 0x878778,
  0x898976, 0x8B8B74, 0x8D8D72, 0x8F8F70, 0x91916E, 0x93936C, 0x95956A, 0x979768, 0x999966, 0x9B9B64,
  0x9D9D62, 0x9F9F60, 0xA1A15E, 0xA3A35C, 0xA5A55A, 0xA7A758, 0xA9A956, 0xABAB54, 0xADAD52, 0xAFAF50,
  0xB1B14E, 0xB3B34D, 0xB4B44B, 0xB6B649, 0xB8B847, 0xBABA45, 0xBCBC43, 0xBEBE41, 0xC0C03F, 0xC2C23D,
  0xC4C43B, 0xC6C639, 0xC8C837, 0xCACA35, 0xCCCC33, 0xCECE31, 0xD0D02F, 0xD2D22D, 0xD4D42B, 0xD6D629,
  0xD8D827, 0xDADA25, 0xDCDC23, 0xDEDE21, 0xE0E01F, 0xE2E21D, 0xE4E41B, 0xE6E61A, 0xE7E718, 0xE9E916,
  0xEBEB14, 0xEFEF10, 0xF3F30C, 0xF7F708, 0xFBFB04, 0xFFFF00
};

const size_t palette_size = sizeof(palette)/sizeof(palette[0]);

void get_color(float t, uchar& r, uchar& g, uchar& b) {
  int index = (int)((1-t) * palette_size);
  Fl_Color c = palette[index % palette_size] << 8;
  Fl::get_color(c, r, g, b);
}

Drawing_Window mbrot;
Drawing_Window jbrot;

void idle(void*) {
  if (!mbrot.d->idle() && !(jbrot.d && jbrot.d->idle())) Fl::remove_idle(idle);
}

void set_idle() {
  Fl::add_idle(idle);
}

static void window_callback(Fl_Widget*, void*) {exit(0);}

static void print(Fl_Widget *o, void *data) {
  Fl_Printer printer;
  Fl_Window *win = o->window();
  if(!win->visible()) return;
  win->make_current();
  uchar *image_data = fl_read_image(NULL, 0, 0, win->w(), win->h(), 0);
  if( printer.start_job(1) ) return;
  if( printer.start_page() ) return;
  printer.scale(.7f,.7f);
  fl_draw_image(image_data, 0,0, win->w(), win->h());
  printer.end_page();
  delete image_data;
  printer.end_job();
}

static void toggle_color(Fl_Widget *o, void *data) {
  mbrot.d->use_colors = !mbrot.d->use_colors; mbrot.d->new_buffer();
  if(jbrot.d) { jbrot.d->use_colors = mbrot.d->use_colors; jbrot.d->new_buffer(); }
}

int main(int argc, char **argv) {
  mbrot.make_window();
  mbrot.window->begin();
  Fl_Button* o = new Fl_Button(0, 0, 0, 0, NULL);
  o->callback(print,NULL);
  o->shortcut(FL_CTRL+'p');
  o = new Fl_Button(0, 0, 0, 0, NULL);
  o->callback(toggle_color,NULL);
  o->shortcut(FL_CTRL+'m');
  mbrot.window->end();

  mbrot.d->X = -.75;
  mbrot.d->scale = 2.5;
  mbrot.update_label();
  int i = 0;
  if (Fl::args(argc,argv,i) < argc) Fl::fatal(Fl::help);
  Fl::visual(FL_RGB);
  mbrot.window->callback(window_callback);
  mbrot.window->show(argc,argv);
  Fl::run();
  return 0;
}

void Drawing_Window::update_label() {
  char buffer[128];
  snprintf(buffer, 128, "%+.10f", d->X); x_input->value(buffer);
  snprintf(buffer, 128, "%+.10f", d->Y); y_input->value(buffer);
  snprintf(buffer, 128, "%.2g", d->scale); w_input->value(buffer);
}

void Drawing_Area::draw() {
  if (!dx) {
    dx = Fl::box_dx(box());
    dy = Fl::box_dy(box());
    dw = Fl::box_dw(box());
    dh = Fl::box_dh(box());
    W -= dw;
    H -= dh;
  }
  draw_box();
  drawn = 0;
  set_idle();
}

int Drawing_Area::idle() {
  if (!window()->visible()) return 0;
  if (drawn < nextline) {
    window()->make_current();
    int yy = drawn+y()+dy;
    if (yy >= sy && yy <= sy+sh) erase_box();
    if (use_colors)
      fl_draw_image(buffer+drawn*W*3, x()+dx, yy, W, 1, 3);
    else
      fl_draw_image_mono(buffer+drawn*W, x()+dx, yy, W, 1, 1, W);
    drawn++;
    return 1;
  }
  int linebytes = use_colors ? W*3 : W;
  if (nextline < H) {
    if (!buffer) buffer = new uchar[linebytes*H];
    double yy = Y+(H/2-nextline)*scale/W;
    double yi = yy; if (julia) yy = jY;
    uchar *p = buffer+nextline*linebytes;
    for (int xi = 0; xi < W; xi++) {
      double xx = X+(xi-W/2)*scale/W;
      double wx = xx; double wy = yi;
      if (julia) xx = jX;
      for (int i=0; ; i++) {
        if (i >= iterations) {
          *p = 0;
          if (use_colors) {
           *(p+1) = 0;
           *(p+2) = 0;
          }
          break;
        }
        double t = wx*wx - wy*wy + xx;
        wy = 2*wx*wy + yy;
        wx = t;
        if (wx*wx + wy*wy > 4) {
          wx = t = 1-double(i)/(1<<10);
          if (t <= 0) t = 0; else for (i=brightness; i--;) t*=wx;
          if (use_colors) {
            get_color((float)t, *p, *(p+1), *(p+2));
          } else {
            *p = 255 - int(254*t);
          }
          break;
        }
      }
      p += use_colors ? 3 : 1;
    }
    nextline++;
    return nextline <= H;
  }
  return 0;
}

void Drawing_Area::erase_box() {
  window()->make_current();
  fl_overlay_clear();
}

int Drawing_Area::handle(int event) {
  static int ix, iy;
  static int dragged;
  static int button;
  int x2,y2;
  switch (event) {
  case FL_PUSH:
    erase_box();
    ix = Fl::event_x(); if (ix<x()) ix=x(); if (ix>=x()+w()) ix=x()+w()-1;
    iy = Fl::event_y(); if (iy<y()) iy=y(); if (iy>=y()+h()) iy=y()+h()-1;
    dragged = 0;
    button = Fl::event_button();
    return 1;
  case FL_DRAG:
    dragged = 1;
    erase_box();
    x2 = Fl::event_x(); if (x2<x()) x2=x(); if (x2>=x()+w()) x2=x()+w()-1;
    y2 = Fl::event_y(); if (y2<y()) y2=y(); if (y2>=y()+h()) y2=y()+h()-1;
    if (button != 1) {ix = x2; iy = y2; return 1;}
    if (ix < x2) {sx = ix; sw = x2-ix;} else {sx = x2; sw = ix-x2;}
    if (iy < y2) {sy = iy; sh = y2-iy;} else {sy = y2; sh = iy-y2;}
    window()->make_current();
    fl_overlay_rect(sx,sy,sw,sh);
    return 1;
  case FL_RELEASE:
    if (button == 1) {
      erase_box();
      if (dragged && sw > 3 && sh > 3) {
        X = X + (sx+sw/2-x()-W/2)*scale/W;
        Y = Y + (-sy-sh/2+y()+H/2)*scale/W;
        scale = sw*scale/W;
      } else if (!dragged) {
        scale = 2*scale;
        if (julia) {
          if (scale >= 4) {
            scale = 4;
            X = Y = 0;
          }
        } else {
          if (scale >= 2.5) {
            scale = 2.5;
            X = -.75;
            Y = 0;
          }
        }
      } else return 1;
      ((Drawing_Window*)(user_data()))->update_label();
      new_display();
    } else if (!julia) {
      if (!jbrot.d) {
        jbrot.make_window();
        jbrot.d->julia = 1;
        jbrot.d->X = 0;
        jbrot.d->Y = 0;
        jbrot.d->scale = 4;
        jbrot.d->use_colors = mbrot.d->use_colors;
        jbrot.update_label();
      }
      jbrot.d->jX = X + (ix-x()-W/2)*scale/W;
      jbrot.d->jY = Y + (H/2-iy+y())*scale/W;
      static char s[128];
      snprintf(s, 128, "Julia %.7f %.7f",jbrot.d->jX,jbrot.d->jY);
      jbrot.window->label(s);
      jbrot.window->show();
      jbrot.d->new_display();
    }
    return 1;
  }
  return 0;
}

void Drawing_Area::new_display() {
  drawn = nextline = 0;
  set_idle();
}

void Drawing_Area::new_buffer() {
  if (buffer) {delete[] buffer; buffer = 0; new_display();}
}

void Drawing_Area::resize(int XX,int YY,int WW,int HH) {
  if (WW != w() || HH != h()) {
    W = WW - dw;
    H = HH - dh;
    if (buffer) {delete[] buffer; buffer = 0; new_display();}
  }
  Fl_Box::resize(XX,YY,WW,HH);
}
