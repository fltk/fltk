//
// "$Id: Fl_Translated_Xlib_Graphics_Driver.H 11217 2016-02-25 17:56:48Z manolo $"
//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#ifndef Fl_Translated_Xlib_Graphics_Driver_h
#define Fl_Translated_Xlib_Graphics_Driver_h

#ifndef FL_DOXYGEN

#include <src/drivers/Xlib/Fl_Xlib_Graphics_Driver.H>

/* graphics driver that translates all graphics coordinates before calling Xlib */
class Fl_Translated_Xlib_Graphics_Driver : public Fl_Xlib_Graphics_Driver {
  int offset_x, offset_y; // translation between user and graphical coordinates: graphical = user + offset
  unsigned depth; // depth of translation stack
  int stack_x[20], stack_y[20]; // translation stack allowing cumulative translations
public:
  Fl_Translated_Xlib_Graphics_Driver();
  virtual ~Fl_Translated_Xlib_Graphics_Driver();
  void translate_all(int dx, int dy);
  void untranslate_all();
  void rect(int x, int y, int w, int h);
  void rectf(int x, int y, int w, int h);
  void xyline(int x, int y, int x1);
  void xyline(int x, int y, int x1, int y2);
  void xyline(int x, int y, int x1, int y2, int x3);
  void yxline(int x, int y, int y1);
  void yxline(int x, int y, int y1, int x2);
  void yxline(int x, int y, int y1, int x2, int y3);
  void line(int x, int y, int x1, int y1);
  void line(int x, int y, int x1, int y1, int x2, int y2);
  void draw(const char* str, int n, int x, int y);
  void draw(int angle, const char *str, int n, int x, int y);
  void rtl_draw(const char* str, int n, int x, int y);
  void draw(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_image(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0);
  void draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3);
  void draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0) ;
  void draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1);
  void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy);
  void push_clip(int x, int y, int w, int h);
  int not_clipped(int x, int y, int w, int h);
  int clip_box(int x, int y, int w, int h, int& X, int& Y, int& W, int& H);
  void pie(int x, int y, int w, int h, double a1, double a2);
  void arc(int x, int y, int w, int h, double a1, double a2);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  void loop(int x0, int y0, int x1, int y1, int x2, int y2);
  void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  void point(int x, int y);
};

#endif // FL_DOXYGEN

#endif /* Fl_Translated_Xlib_Graphics_Driver_h */

//
// End of "$Id: Fl_Translated_Xlib_Graphics_Driver.H 11217 2016-02-25 17:56:48Z manolo $".
//
