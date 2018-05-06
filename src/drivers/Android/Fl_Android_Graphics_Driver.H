//
// "$Id$"
//
// Definition of classes Fl_Graphics_Driver, Fl_Surface_Device, Fl_Display_Device
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2018 by Bill Spitzak and others.
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

/**
 \file Fl_Android_Graphics_Driver.H
 \brief Definition of Android graphics driver.
 */

#ifndef FL_ANDROID_GRAPHICS_DRIVER_H
#define FL_ANDROID_GRAPHICS_DRIVER_H

#include <FL/Fl_Graphics_Driver.H>
#include "Fl_Android_Graphics_Clipping.H"
#include "Fl_Android_Graphics_Font.H"
#include <limits.h>


class Fl_Android_Window_Driver;
class Fl_Android_Bytemap;
class Fl_Android_565A_Map;

/**
  \brief The Windows-specific graphics driver class.

  This class is implemented only on the Windows platform.
*/
class FL_EXPORT Fl_Android_Graphics_Driver : public Fl_Graphics_Driver
{
  // easy access to the super class
  typedef Fl_Graphics_Driver super;

protected:
  class Vertex; // see below

  // --- this is a copy of Fl_Graphics_Driver ----------------------------------
  // - use this to find unimplementet calls in the derived driver
  // - excluded by #if/#endif means that we have not implemneted this yet
  // - methods marked with // super: use the implemnetation of the super class
  // - virtual ... override functions are implemented for Android
private:
  virtual void draw_fixed(Fl_RGB_Image * rgb,int XP, int YP, int WP, int HP, int cx, int cy) override;
  virtual void draw_fixed(Fl_Pixmap * pxm,int XP, int YP, int WP, int HP, int cx, int cy) override;
  virtual void draw_fixed(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) override;
  // some platforms may need to reimplement this
  // This is called from the surface device, see: end_current_()
  // super: virtual void set_current_();
protected:
  /** Sets the current value of the scaling factor */
  // super: virtual void scale(float f) { scale_ = f; } // we do not support any scaling at this point
protected:
  // set fl_gc, which we do not use in the Android port at this point
  // super: virtual void global_gc();
  /** Support function for Fl_Pixmap drawing */
  virtual void cache(Fl_Pixmap *img) override;
  /** Support function for Fl_Bitmap drawing */
  virtual void cache(Fl_Bitmap *img) override;
  virtual void cache(Fl_RGB_Image *img) override;
  /** Support function for Fl_RGB_Image drawing */
  virtual void uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_) override;
  // --- implementation is in src/drivers/xxx/Fl_xxx_Graphics_Driver_image.cxx
  /** see fl_draw_image(const uchar* buf, int X,int Y,int W,int H, int D, int L) */
  virtual void draw_image(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0) override;
  /** see fl_draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D, int L) */
  virtual void draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0) override;
  /** see fl_draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) */
  virtual void draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3) override;
  /** see fl_draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) */
  virtual void draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1) override;
#if 0
  virtual void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy);

  /** Support function for image drawing */
  virtual Fl_Bitmask create_bitmask(int w, int h, const uchar *array) {return 0; }
#endif
  // Support function for image drawing
  virtual void uncache_pixmap(fl_uintptr_t p) override;
  // Support function for image drawing
  virtual void delete_bitmask(Fl_Bitmask bm) override;
public:
  /** Constructor, C++11 initialises member variables in-line */
  Fl_Android_Graphics_Driver();
  /** destructor */
  virtual ~Fl_Android_Graphics_Driver() override;
  /** Return whether the graphics driver can do alpha blending */
  virtual char can_do_alpha_blending() override { return 0; }
  // --- implementation is in src/fl_rect.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_rect.cxx
  /** see fl_point() */
  virtual void point(int x, int y) override;
  /** see fl_rect() */
  virtual void rect(int x, int y, int w, int h) override;
  // super: virtual void focus_rect(int x, int y, int w, int h);
  /** see fl_rectf() */
  virtual void rectf(int x, int y, int w, int h) override;
  /** see fl_line(int, int, int, int) */
  virtual void line(int x, int y, int x1, int y1) override;
  /** see fl_line(int, int, int, int, int, int) */
  virtual void line(int x, int y, int x1, int y1, int x2, int y2) override;
  /** see fl_xyline(int, int, int) */
  virtual void xyline(int x, int y, int x1) override;
  /** see fl_xyline(int, int, int, int) */
  virtual void xyline(int x, int y, int x1, int y2) override;
  /** see fl_xyline(int, int, int, int, int) */
  virtual void xyline(int x, int y, int x1, int y2, int x3) override;
  /** see fl_yxline(int, int, int) */
  virtual void yxline(int x, int y, int y1) override;
  /** see fl_yxline(int, int, int, int) */
  virtual void yxline(int x, int y, int y1, int x2) override;
  /** see fl_yxline(int, int, int, int, int) */
  virtual void yxline(int x, int y, int y1, int x2, int y3) override;
  /** see fl_loop(int, int, int, int, int, int) */
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2) override;
  /** see fl_loop(int, int, int, int, int, int, int, int) */
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) override;
  /** see fl_polygon(int, int, int, int, int, int) */
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2) override;
  /** see fl_polygon(int, int, int, int, int, int, int, int) */
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) override;
  // --- clipping
  /** see fl_push_clip() */
  virtual void push_clip(int x, int y, int w, int h) override;
  /** see fl_clip_box() */
  virtual int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) override;
  /** see fl_not_clipped() */
  virtual int not_clipped(int x, int y, int w, int h) override;
  /** see fl_push_no_clip() */
  virtual void push_no_clip() override;
  /** see fl_pop_clip() */
  virtual void pop_clip() override;
  virtual Fl_Region clip_region() override;
  virtual void clip_region(Fl_Region r) override;
  virtual void restore_clip() override;
  // --- implementation is in src/fl_vertex.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_vertex.cxx
  // super: virtual void push_matrix();
  // super: virtual void pop_matrix();
  // super: virtual void mult_matrix(double a, double b, double c, double d, double x, double y);
  // super: virtual void rotate(double d);
  // super: virtual void translate(double x,double y);
  virtual void begin_points() override;
  virtual void begin_line() override;
  virtual void begin_loop() override;
  virtual void begin_polygon() override;
  virtual void begin_complex_polygon() override;
  // super: virtual double transform_x(double x, double y);
  // super: virtual double transform_y(double x, double y);
  // super: virtual double transform_dx(double x, double y);
  // super: virtual double transform_dy(double x, double y);
  /** see fl_transformed_vertex() */
  virtual void transformed_vertex(double xf, double yf) override;
  /** see fl_vertex() */
  virtual void vertex(double x, double y) override;
  /** see fl_end_points() */
  virtual void end_points() override;
  /** see fl_end_line() */
  virtual void end_line() override;
  /** see fl_end_loop() */
  virtual void end_loop() override;
  /** see fl_end_polygon() */
  virtual void end_polygon() override;
  /** see fl_end_complex_polygon() */
  virtual void end_complex_polygon() override;
  /** see fl_gap() */
  virtual void gap() override;
  /** see fl_circle() */
  virtual void circle(double x, double y, double r) override;
  // --- implementation is in src/fl_arc.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_arc.cxx if needed
  virtual void arc(double x, double y, double r, double start, double end) override { super::arc(x, y, r, start, end); }
  // --- implementation is in src/fl_arci.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_arci.cxx
  /** see fl_arc(int x, int y, int w, int h, double a1, double a2) */
  virtual void arc(int x, int y, int w, int h, double a1, double a2) override;
  /** see fl_pie() */
  virtual void pie(int x, int y, int w, int h, double a1, double a2) override;
  // --- implementation is in src/fl_curve.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_curve.cxx if needed
  // super: virtual void curve(double X0, double Y0, double X1, double Y1, double X2, double Y2, double X3, double Y3);
  // --- implementation is in src/fl_line_style.cxx which includes src/cfg_gfx/xxx_line_style.cxx
  // TODO: line_style()
  /** see fl_line_style() */
  virtual void line_style(int style, int width=0, char* dashes=0) override;
  // --- implementation is in src/fl_color.cxx which includes src/cfg_gfx/xxx_color.cxx
  /** see fl_color(Fl_Color) */
  virtual void color(Fl_Color c) override { super::color(c); }
  virtual void set_color(Fl_Color i, unsigned int c) override;
  // super: virtual void free_color(Fl_Color i, int overlay);
  /** see fl_color(void) */
  virtual Fl_Color color() override { return super::color(); }
  /** see fl_color(uchar, uchar, uchar) */
  virtual void color(uchar r, uchar g, uchar b) override;
  /** see fl_draw(const char *str, int n, int x, int y) */
  virtual void draw(const char *str, int n, int x, int y) override;
  /** Draw the first \p n bytes of the string \p str starting at position \p x , \p y */
  // super: virtual void draw(const char *str, int n, float x, float y);
  /** see fl_draw(int angle, const char *str, int n, int x, int y) */
  // TODO: drawing text at an angle is not supported
  virtual void draw(int angle, const char *str, int n, int x, int y) override { draw(str, n, x, y); }
  /** see fl_rtl_draw(const char *str, int n, int x, int y) */
  // TODO: drawing text right-to-left is not supported
  virtual void rtl_draw(const char *str, int n, int x, int y) override { draw(str, n, x, y); }
  /** Returns non-zero if the graphics driver possesses the \p feature */
  // super: virtual int has_feature(driver_feature feature)
  /** see fl_font(Fl_Font, Fl_Fontsize) */
  virtual void font(Fl_Font face, Fl_Fontsize fsize) override;
  /** see fl_font(void) */
  virtual Fl_Font font() override { return super::font(); }
  /** Return the current font size */
  virtual Fl_Fontsize size() override;
  /** Compute the width of the first \p n bytes of the string \p str if drawn with current font */
  virtual double width(const char *str, int n) override;
  /** Compute the width of Unicode character \p c if drawn with current font */
  virtual double width(unsigned int c) override;
  virtual void text_extents(const char*, int n, int& dx, int& dy, int& w, int& h) override;
  /** Return the current line height */
  virtual int height() override;
  /** Return the current line descent */
  virtual int descent() override;
  /** Return the current Fl_Font_Descriptor */
  // super: inline Fl_Font_Descriptor *font_descriptor() { return font_descriptor_;}
  /** Set the current Fl_Font_Descriptor */
  // super: virtual void font_descriptor(Fl_Font_Descriptor *d) { font_descriptor_ = d;}
#if 0
  /** Sets the value of the driver-specific graphics context. */
  virtual void gc(void*) {}
  /** Returns the driver-specific graphics context, of NULL if there's none. */
  virtual void *gc(void) {return NULL;}
  /** Support for pixmap drawing */
  virtual uchar **mask_bitmap() { return 0; }
  /** Support for pixmap drawing */
  virtual void mask_bitmap(uchar **) {}
#endif
  // default implementation may be enough
  /** Support for PostScript drawing */
  virtual float scale_font_for_PostScript(Fl_Font_Descriptor *desc, int s) override { return float(s); }
  // default implementation may be enough
  /** Support for PostScript drawing - no documentation found on this call*/
  // super: virtual float scale_bitmap_for_PostScript() { return 2; }
  // super: virtual void set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win);
  // super: virtual void reset_spot();
  // each platform implements these 3 functions its own way
  /* TODO: Android: we can implement this to have a redraw region based on Fl::damage
   * calls. Currently, we do not implement damage regions, but we can probably
   * implement this using our clipping regions. This may become neccesary when
   * we allow desktop-style window movement.
   */
  // super: virtual void add_rectangle_to_region(Fl_Region r, int x, int y, int w, int h);
  // super: virtual Fl_Region XRectangleRegion(int x, int y, int w, int h);
  // super: virtual void XDestroyRegion(Fl_Region r);
  /** Support for Fl::get_font_name() */
  virtual const char* get_font_name(Fl_Font fnum, int* ap) override;
  /** Support for Fl::get_font_sizes() */
  virtual int get_font_sizes(Fl_Font fnum, int*& sizep) override;
  /** Support for Fl::set_fonts() */
  virtual Fl_Font set_fonts(const char *name) override;
  /** Some platforms may need to implement this to support fonts */
  // super: virtual Fl_Fontdesc* calc_fl_fonts(void) {return NULL;}
  /** Support for Fl::set_font() */
  // super: virtual unsigned font_desc_size();
  /** Support for Fl::get_font() */
  virtual const char *font_name(int num) override;
  /** Support for Fl::set_font() */
  virtual void font_name(int num, const char *name) override;
  /** Support function for fl_overlay_rect() and scaled GUI.
   Defaut implementation may be enough */
  // super: virtual void overlay_rect(int x, int y, int w , int h) { loop(x, y, x+w-1, y, x+w-1, y+h-1, x, y+h-1); }
  // --- end of original Fl_Graphics_Driver header -----------------------------

  // --- start of Android additions --------------------------------------------
  // start drawing with this driver into the given window
  // The virtual call `set_current_()` changes surface, not windows
  void make_current(Fl_Window*);

protected:
  static uint16_t make565(uchar r, uchar g, uchar b);
  static uint16_t make565(Fl_Color crgba);
  void rectf_unclipped(int x, int y, int w, int h);
  void xyline_unclipped(int x, int y, int x1);
  void yxline_unclipped(int x, int y, int y1);
  void end_polygon(int begin, int end);
  void ellipse(double xt, double yt, double rx, double ry);
  void draw(int xx, int yy, Fl_Android_565A_Map *bm, Fl_Rect_Region &r);
  void draw(int x, int y, Fl_Android_Bytemap *bm, Fl_Rect_Region &r);
  int render_letter(int xx, int yy, uint32_t c, Fl_Rect_Region &r);

  // pointer into the screen buffer at the top left corner of the current window
  uint16_t *pBits = nullptr;

  // advance to next line in screen buffer
  int32_t pStride = 0;

  // TODO: current line style, temp kludge to make focus rect work.
  int pLineStyle = 0;

  // Clipping region of the current window in window coordinates (see: pStride and pBits)
  Fl_Rect_Region pWindowRegion;

  // clipping region of the window minus overlapping other windows
  Fl_Complex_Region pDesktopWindowRegion;

  // Final clipping region for all graphics calls to this class.
  Fl_Complex_Region pClippingRegion;

  // store vertices for begin_.../end_... drawing
  class Vertex {
  public:
    void set(float x, float y, bool gap = false) { pX = x; pY = y; pIsGap = gap; }
    float pX, pY;
    bool pIsGap;
  };

  void begin_vertices();
  void add_vertex(float x, float y, bool gap=false);
  int pnVertex = 0, pNVertex = 0, pVertexGapStart = 0;
  Vertex *pVertex = nullptr;
};


#if 0
/**
  The graphics driver used when printing on Android.
*/
class FL_EXPORT Fl_Android_Printer_Graphics_Driver : public Fl_Android_Graphics_Driver {

#if 0

private:
  typedef BOOL (WINAPI* transparent_f_type) (HDC,int,int,int,int,HDC,int,int,int,int,UINT);
  transparent_f_type TransparentBlt();
public:
  virtual int has_feature(driver_feature mask) { return mask & (NATIVE | PRINTER); }
  void draw_unscaled(Fl_Pixmap *pxm, float s, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_unscaled(Fl_Bitmap *bm, float s, int XP, int YP, int WP, int HP, int cx, int cy);

#endif

};
#endif


#endif // FL_ANDROID_GRAPHICS_DRIVER_H

//
// End of "$Id$".
//
