//
// "$Id"
//
// Portable drawing function header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#ifndef fl_draw_H
#define fl_draw_H

#include "Enumerations.H"  // for the color names

// Colors:
void	fl_color(Fl_Color); // select indexed color
inline void fl_color(int c) {fl_color((Fl_Color)c);} // for back compatability
void	fl_color(uchar, uchar, uchar); // select actual color
extern Fl_Color fl_color_; inline Fl_Color fl_color() {return fl_color_;}

// clip:
void fl_clip(int x, int y, int w, int h);
#define fl_push_clip fl_clip
void fl_push_no_clip();
void fl_pop_clip();
int fl_not_clipped(int x, int y, int w, int h);
int fl_clip_box(int, int, int, int, int& x, int& y, int& w, int& h);

// points:
void fl_point(int x, int y);

// rectangles tweaked to exactly fill the pixel rectangle:
void fl_rect(int x, int y, int w, int h);
void fl_rectf(int x, int y, int w, int h);

// line segments:
void fl_line(int,int, int,int);
void fl_line(int,int, int,int, int,int);

// closed line segments:
void fl_loop(int,int, int,int, int,int);
void fl_loop(int,int, int,int, int,int, int,int);

// filled polygons
void fl_polygon(int,int, int,int, int,int);
void fl_polygon(int,int, int,int, int,int, int,int);

// draw rectilinear lines, horizontal segment first:
void fl_xyline(int x, int y, int x1);
void fl_xyline(int x, int y, int x1, int y2);
void fl_xyline(int x, int y, int x1, int y2, int x3);

// draw rectilinear lines, vertical segment first:
void fl_yxline(int x, int y, int y1);
void fl_yxline(int x, int y, int y1, int x2);
void fl_yxline(int x, int y, int y1, int x2, int y3);

// circular lines and pie slices (code in fl_arci.C):
void fl_arc(int x, int y, int w, int h, double a1, double a2);
void fl_pie(int x, int y, int w, int h, double a1, double a2);
void fl_chord(int x, int y, int w, int h, double a1, double a2); // nyi

// scalable drawing code (code in fl_vertex.C and fl_arc.C):
void fl_push_matrix();
void fl_pop_matrix();
void fl_scale(double x, double y);
void fl_scale(double x);
void fl_translate(double x, double y);
void fl_rotate(double d);
void fl_mult_matrix(double a, double b, double c, double d, double x,double y);
void fl_begin_points();
void fl_begin_line();
void fl_begin_loop();
void fl_begin_polygon();
void fl_vertex(double x, double y);
void fl_curve(double, double, double, double, double, double, double, double);
void fl_arc(double x, double y, double r, double start, double a);
void fl_circle(double x, double y, double r);
void fl_end_points();
void fl_end_line();
void fl_end_loop();
void fl_end_polygon();
void fl_begin_complex_polygon();
void fl_gap();
void fl_end_complex_polygon();
// get and use transformed positions:
double fl_transform_x(double x, double y);
double fl_transform_y(double x, double y);
double fl_transform_dx(double x, double y);
double fl_transform_dy(double x, double y);
void fl_transformed_vertex(double x, double y);

// current font:
void fl_font(int face, int size);
void fl_font(int face, int size, Fl_Font default_font, int default_size);
extern int fl_font_; inline int fl_font() {return fl_font_;}
extern int fl_size_; inline int fl_size() {return fl_size_;}

// information you can get about the current font:
int   fl_height();	// using "size" should work ok
int   fl_descent();
double fl_width(const char*);
double fl_width(const char*, int n);
double fl_width(uchar);

// draw using current font:
void fl_draw(const char*, int x, int y);
void fl_draw(const char*, int n, int x, int y);
void fl_draw(const char*, int x, int y, int w, int h, Fl_Align);
void fl_measure(const char*, int& x, int& y);

// boxtypes:
void fl_frame(const char* s, int x, int y, int w, int h);
void fl_frame2(const char* s, int x, int y, int w, int h);
void fl_draw_box(Fl_Boxtype, int x, int y, int w, int h, Fl_Color);

// images:
void fl_draw_image(const uchar*, int,int,int,int, int delta=3, int ldelta=0);
void fl_draw_image_mono(const uchar*, int,int,int,int, int delta=1, int ld=0);
typedef void (*Fl_Draw_Image_Cb)(void*,int,int,int,uchar*);
void fl_draw_image(Fl_Draw_Image_Cb, void*, int,int,int,int, int delta=3);
void fl_draw_image_mono(Fl_Draw_Image_Cb, void*, int,int,int,int, int delta=1);
void fl_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b);

// pixmaps:
int fl_draw_pixmap(/*const*/ char* const* data, int x,int y,Fl_Color=FL_GRAY);
int fl_measure_pixmap(/*const*/ char* const* data, int &w, int &h);

// other:
extern void fl_scroll(int X, int Y, int W, int H, int dx, int dy,
	       void (*draw_area)(void*, int,int,int,int), void* data);
const char* fl_shortcut_label(int);
void fl_overlay_rect(int,int,int,int);
void fl_overlay_clear();
void fl_cursor(Fl_Cursor, Fl_Color=FL_BLACK, Fl_Color=FL_WHITE);

// XForms symbols:
int fl_draw_symbol(const char* label,int x,int y,int w,int h, Fl_Color);
int fl_add_symbol(const char* name, void (*drawit)(Fl_Color), int scalable);

#endif

//
// End of "$Id: fl_draw.H,v 1.2 1998/10/19 21:39:21 mike Exp $".
//
