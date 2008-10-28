//
// "$Id$"
//
// Portable drawing function header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2008 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

/**
  \file fl_draw.H
  \brief utility header to pull drawing functions together
*/

#ifndef fl_draw_H
#define fl_draw_H

#include "Enumerations.H"  // for the color names

// Image class...
class Fl_Image;

// Label flags...
FL_EXPORT extern char fl_draw_shortcut;

// Colors:
FL_EXPORT void	fl_color(Fl_Color i); // select indexed color
/** for back compatibility - use fl_color(Fl_Color c) instead */
inline void fl_color(int c) {fl_color((Fl_Color)c);}
FL_EXPORT void	fl_color(uchar r, uchar g, uchar b); // select actual color
extern FL_EXPORT Fl_Color fl_color_;
/**
  Returns the last fl_color() that was set.
  This can be used for state save/restore.
*/
inline Fl_Color fl_color() {return fl_color_;}

// clip:
FL_EXPORT void fl_push_clip(int x, int y, int w, int h);
/** The fl_clip() name is deprecated and will be removed from future releases */
#define fl_clip fl_push_clip
FL_EXPORT void fl_push_no_clip();
FL_EXPORT void fl_pop_clip();
FL_EXPORT int fl_not_clipped(int x, int y, int w, int h);
FL_EXPORT int fl_clip_box(int, int, int, int, int& x, int& y, int& w, int& h);

// points:
FL_EXPORT void fl_point(int x, int y);

// line type:
FL_EXPORT void fl_line_style(int style, int width=0, char* dashes=0);
enum {
  FL_SOLID	= 0,		///< line style: <tt>___________</tt>
  FL_DASH	= 1,		///< line style: <tt>_ _ _ _ _ _</tt>
  FL_DOT	= 2,		///< line style: <tt>. . . . . .</tt>
  FL_DASHDOT	= 3,		///< line style: <tt>_ . _ . _ .</tt>
  FL_DASHDOTDOT	= 4,		///< line style: <tt>_ . . _ . .</tt>

  FL_CAP_FLAT	= 0x100,	///< cap style: end is flat
  FL_CAP_ROUND	= 0x200,	///< cap style: end is round
  FL_CAP_SQUARE	= 0x300,	///< cap style: end wraps end point

  FL_JOIN_MITER	= 0x1000,	///< join style: line join extends to a point
  FL_JOIN_ROUND	= 0x2000,	///< join style: line join is rounded
  FL_JOIN_BEVEL	= 0x3000	///< join style: line join is tidied
};

// rectangles tweaked to exactly fill the pixel rectangle:
FL_EXPORT void fl_rect(int x, int y, int w, int h);
/** Draw a 1-pixel border \e inside the given bounding box */
inline void fl_rect(int x, int y, int w, int h, Fl_Color c) {fl_color(c); fl_rect(x,y,w,h);}
FL_EXPORT void fl_rectf(int x, int y, int w, int h);
/** Color a rectangle that exactly fills the given bounding box */
inline void fl_rectf(int x, int y, int w, int h, Fl_Color c) {fl_color(c); fl_rectf(x,y,w,h);}

// line segments:
FL_EXPORT void fl_line(int x, int y, int x1, int y1);
FL_EXPORT void fl_line(int x, int y, int x1, int y1, int x2, int y2);

// closed line segments:
FL_EXPORT void fl_loop(int x, int y, int x1, int y1, int x2, int y2);
FL_EXPORT void fl_loop(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3);

// filled polygons
FL_EXPORT void fl_polygon(int x, int y, int x1, int y1, int x2, int y2);
FL_EXPORT void fl_polygon(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3);

// draw rectilinear lines, horizontal segment first:
FL_EXPORT void fl_xyline(int x, int y, int x1);
FL_EXPORT void fl_xyline(int x, int y, int x1, int y2);
FL_EXPORT void fl_xyline(int x, int y, int x1, int y2, int x3);

// draw rectilinear lines, vertical segment first:
FL_EXPORT void fl_yxline(int x, int y, int y1);
FL_EXPORT void fl_yxline(int x, int y, int y1, int x2);
FL_EXPORT void fl_yxline(int x, int y, int y1, int x2, int y3);

// circular lines and pie slices (code in fl_arci.C):
FL_EXPORT void fl_arc(int x, int y, int w, int h, double a1, double a2);
FL_EXPORT void fl_pie(int x, int y, int w, int h, double a1, double a2);
/** fl_chord declaration is a place holder - the function does not yet exist */
FL_EXPORT void fl_chord(int x, int y, int w, int h, double a1, double a2); // nyi

// scalable drawing code (code in fl_vertex.C and fl_arc.C):
FL_EXPORT void fl_push_matrix();
FL_EXPORT void fl_pop_matrix();
FL_EXPORT void fl_scale(double x, double y);
FL_EXPORT void fl_scale(double x);
FL_EXPORT void fl_translate(double x, double y);
FL_EXPORT void fl_rotate(double d);
FL_EXPORT void fl_mult_matrix(double a, double b, double c, double d, double x,double y);
FL_EXPORT void fl_begin_points();
FL_EXPORT void fl_begin_line();
FL_EXPORT void fl_begin_loop();
FL_EXPORT void fl_begin_polygon();
FL_EXPORT void fl_vertex(double x, double y);
FL_EXPORT void fl_curve(double X0, double Y0, double X1, double Y1, double X2, double Y2, double X3, double Y3);
FL_EXPORT void fl_arc(double x, double y, double r, double start, double a);
FL_EXPORT void fl_circle(double x, double y, double r);
FL_EXPORT void fl_end_points();
FL_EXPORT void fl_end_line();
FL_EXPORT void fl_end_loop();
FL_EXPORT void fl_end_polygon();
FL_EXPORT void fl_begin_complex_polygon();
FL_EXPORT void fl_gap();
FL_EXPORT void fl_end_complex_polygon();
// get and use transformed positions:
FL_EXPORT double fl_transform_x(double x, double y);
FL_EXPORT double fl_transform_y(double x, double y);
FL_EXPORT double fl_transform_dx(double x, double y);
FL_EXPORT double fl_transform_dy(double x, double y);
FL_EXPORT void fl_transformed_vertex(double x, double y);

/* NOTE: doxygen comments here to avoid triplication in os-specific sources */
/**
  Set the current font, which is then used in various drawing routines,
  You may call this outside a draw context if necessary to cal fl_width(),
  but on X this will open the display.

  The font is identified by a \a face and a \a size.
  The size of the font is measured in pixels and not "points".
  Lines should be spaced \a size pixels apart or more.
*/
FL_EXPORT void fl_font(Fl_Font face, Fl_Fontsize size);
/** current font index */
extern FL_EXPORT Fl_Font fl_font_;
/**
  Returns the \a face set by the most recent call to fl_font().
  Tgis can be used to save/restore the font.
*/
inline Fl_Font fl_font() {return fl_font_;}
/** current font size */
extern FL_EXPORT Fl_Fontsize fl_size_;
/**
  Returns the \a face set by the most recent call to fl_font().
  Tgis can be used to save/restore the font.
*/
inline Fl_Fontsize fl_size() {return fl_size_;}

// information you can get about the current font:
/**
  Recommended minimum line spacing for the current font.
  You can also use the value of \a size passed to fl_font()
*/
FL_EXPORT int   fl_height();	// using "size" should work ok
/**
  Dummy passthru function called only in Fl_Text_Display that simply returns
  the font height as given by the \a size parameter in the same call!

  \todo Is fl_height(int, int size) required for Fl_Text_Dispay?
        Why not use \a size parameter directly?
*/
inline int fl_height(int, int size) {return size;}
/**
  Recommended distance above the bottom of a fl_height() tall box to
  draw the text at so it looks centered vertically in that box.
*/
FL_EXPORT int   fl_descent();
/** Return the pixel width of a nul-terminated string */
FL_EXPORT double fl_width(const char* txt);
/** Return the pixel width of a sequence of \a n characters */
FL_EXPORT double fl_width(const char* txt, int n);
/** Return the pixed width of a single character */
FL_EXPORT double fl_width(Fl_Unichar);

/**
  Draw a nul-terminated string starting at the given location.
  Text is aligned to the left and to the baseline of the font.
  To alighn to the bottom, subtract fl_descent() from \a y.
  To alignn to the top, subtract fl_descent() and add fl_height().
  This version fo fl_draw provides direct access to the text drawing
  function of the underlying OS. It does not apply any special handling
  to control characters.
*/
FL_EXPORT void fl_draw(const char* str, int x, int y);
/**
  Draw an array of \a n characters starting at the given location.
*/
FL_EXPORT void fl_draw(const char* str, int n, int x, int y);
/**
  Draw and array of \a n characters right to left starting at given location.
*/
FL_EXPORT void fl_rtl_draw(const char*, int n, int x, int y);
FL_EXPORT void fl_measure(const char* str, int& x, int& y,
                          int draw_symbols = 1);
FL_EXPORT void fl_draw(const char* str, int x, int y, int w, int h,
                       Fl_Align align,
                       Fl_Image* img=0, int draw_symbols = 1);
FL_EXPORT void fl_draw(const char* str, int x, int y, int w, int h,
                       Fl_Align align,
                       void (*callthis)(const char *,int,int,int),
                       Fl_Image* img=0, int draw_symbols = 1);

// font encoding:
FL_EXPORT const char *fl_latin1_to_local(const char *, int n=-1);
FL_EXPORT const char *fl_local_to_latin1(const char *, int n=-1);
FL_EXPORT const char *fl_mac_roman_to_local(const char *, int n=-1);
FL_EXPORT const char *fl_local_to_mac_roman(const char *, int n=-1);

// boxtypes:
FL_EXPORT void fl_frame(const char* s, int x, int y, int w, int h);
FL_EXPORT void fl_frame2(const char* s, int x, int y, int w, int h);
FL_EXPORT void fl_draw_box(Fl_Boxtype, int x, int y, int w, int h, Fl_Color);

// images:
/** signature of some image drawing functions passed as parameters */
typedef void (*Fl_Draw_Image_Cb)(void*,int,int,int,uchar*);
FL_EXPORT void fl_draw_image(const uchar*, int,int,int,int, int delta=3, int ldelta=0);
FL_EXPORT void fl_draw_image_mono(const uchar*, int,int,int,int, int delta=1, int ld=0);
FL_EXPORT void fl_draw_image(Fl_Draw_Image_Cb, void*, int,int,int,int, int delta=3);
FL_EXPORT void fl_draw_image_mono(Fl_Draw_Image_Cb, void*, int,int,int,int, int delta=1);
FL_EXPORT void fl_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b);
FL_EXPORT char fl_can_do_alpha_blending();

FL_EXPORT uchar *fl_read_image(uchar *p, int x,int y, int w, int h, int alpha=0);

// pixmaps:
FL_EXPORT int fl_draw_pixmap(/*const*/ char* const* data, int x,int y,Fl_Color=FL_GRAY);
FL_EXPORT int fl_measure_pixmap(/*const*/ char* const* data, int &w, int &h);
FL_EXPORT int fl_draw_pixmap(const char* const* data, int x,int y,Fl_Color=FL_GRAY);
FL_EXPORT int fl_measure_pixmap(const char* const* data, int &w, int &h);

// other:
FL_EXPORT void fl_scroll(int X, int Y, int W, int H, int dx, int dy,
                         void (*draw_area)(void*, int,int,int,int), void* data);
FL_EXPORT const char* fl_shortcut_label(int);
FL_EXPORT void fl_overlay_rect(int,int,int,int);
FL_EXPORT void fl_overlay_clear();
FL_EXPORT void fl_cursor(Fl_Cursor, Fl_Color=FL_BLACK, Fl_Color=FL_WHITE);
FL_EXPORT const char* fl_expand_text(const char* from, char* buf, int maxbuf,
                                     double maxw, int& n, double &width,
                                     int wrap, int draw_symbols = 0);

// XIM:
FL_EXPORT void fl_set_status(int X, int Y, int W, int H);
FL_EXPORT void fl_set_spot(int font, int size, int x, int y, int w, int h);
FL_EXPORT void fl_reset_spot(void);



// XForms symbols:
FL_EXPORT int fl_draw_symbol(const char* label,int x,int y,int w,int h, Fl_Color);
FL_EXPORT int fl_add_symbol(const char* name, void (*drawit)(Fl_Color), int scalable);

#endif

//
// End of "$Id$".
//
