//
// "$Id$"
//
// Definition of classes Fl_Device, Fl_Graphics_Driver, Fl_Surface_Device, Fl_Display_Device
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2014 by Bill Spitzak and others.
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

/** \file Fl_Device.H 
 \brief declaration of classes Fl_Device, Fl_Graphics_Driver, Fl_Surface_Device, 
 Fl_Display_Device, Fl_Device_Plugin.
*/

#ifndef Fl_Device_H
#define Fl_Device_H

#include <FL/x.H>
#include <FL/Fl_Plugin.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Bitmap.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_RGB_Image.H>
#include <stdlib.h>

class Fl_Graphics_Driver;
class Fl_Font_Descriptor;
/** \brief Points to the driver that currently receives all graphics requests */
FL_EXPORT extern Fl_Graphics_Driver *fl_graphics_driver;

/**
 signature of image generation callback function.
 \param[in]  data  user data passed to function
 \param[in]  x,y,w position and width of scan line in image
 \param[out] buf   buffer for generated image data. You must copy \p w
 pixels from scanline \p y, starting at pixel \p x
 to this buffer.
 */
typedef void (*Fl_Draw_Image_Cb)(void* data,int x,int y,int w,uchar* buf);

// typedef what the x,y fields in a point are:
#ifdef WIN32
typedef int COORD_T;
#  define XPOINT XPoint
#elif defined(__APPLE__)
typedef float COORD_T;
typedef struct { float x; float y; } QPoint;
#  define XPOINT QPoint
extern float fl_quartz_line_width_;
#elif defined(FL_PORTING)
# pragma message "FL_PORTING: define types for COORD_T and XPOINT"
typedef int COORD_T; // default if not ported
typedef struct { int x; int y; } QPoint;
#  define XPOINT QPoint
#else
typedef short COORD_T;
#  define XPOINT XPoint
#endif

/**
 All graphical output devices and all graphics systems.
 This class supports a rudimentary system of run-time type information.
 */
class FL_EXPORT Fl_Device {
public:
  /** A string that identifies each subclass of Fl_Device.
     Function class_name() applied to a device of this class returns this string.
   */
  static const char *class_id;
  /** 
   Returns the name of the class of this object. 
   Use of the class_name() function is discouraged because it will be removed from future FLTK versions.
   
   The class of an instance of an Fl_Device subclass can be checked with code such as:
   \code
   if ( instance->class_name() == Fl_Printer::class_id ) { ... }
   \endcode
   */
  virtual const char *class_name() {return class_id;};
  /** 
   Virtual destructor.
   
   The destructor of Fl_Device must be virtual to make the destructors of
   derived classes being called correctly on destruction.
   */
  virtual ~Fl_Device() {};
};

#define FL_REGION_STACK_SIZE 10
#define FL_MATRIX_STACK_SIZE 32
/**
 \brief A virtual class subclassed for each graphics driver FLTK uses.
 Typically, FLTK applications do not use directly objects from this class. Rather, they perform
 drawing operations (e.g., fl_rectf()) that operate on the current drawing surface (see Fl_Surface_Device).
 Drawing operations are functionally presented in \ref drawing and as function lists
 in the \ref fl_drawings and \ref fl_attributes modules. The \ref fl_graphics_driver global variable
 gives at any time the graphics driver used by all drawing operations. Its value changes when
 drawing operations are directed to another drawing surface by Fl_Surface_Device::set_current().
 
 \p The Fl_Graphics_Driver class is of interest if one wants to perform new kinds of drawing operations.
 An example would be to draw to a PDF file. This would involve creating a new Fl_Graphics_Driver derived 
 class. This new class should implement all virtual methods of the Fl_Graphics_Driver class
 to support all FLTK drawing functions.
  */
class FL_EXPORT Fl_Graphics_Driver : public Fl_Device {
  friend class Fl_Pixmap;
  friend class Fl_Bitmap;
  friend class Fl_RGB_Image;
public:
  /** A 2D coordinate transformation matrix */
  struct matrix {double a, b, c, d, x, y;};
  int fl_clip_state_number;
protected:
  static const matrix m0;
  Fl_Font font_; // current font
  Fl_Fontsize size_; // current font size
  Fl_Color color_; // current color
  int sptr;
  static const int matrix_stack_size = FL_MATRIX_STACK_SIZE;
  matrix stack[FL_MATRIX_STACK_SIZE];
  matrix m;
  int n, p_size, gap_;
  XPOINT *p;
  int what;
  int rstackptr;
  static const int region_stack_max = FL_REGION_STACK_SIZE - 1;
  Fl_Region rstack[FL_REGION_STACK_SIZE];
  Fl_Font_Descriptor *font_descriptor_;
#ifndef FL_DOXYGEN
  enum {LINE, LOOP, POLYGON, POINT_};
  inline int vertex_no() { return n; }
  inline XPOINT *vertices() {return p;}
  inline int vertex_kind() {return what;}
#endif
  matrix *fl_matrix; /**< Points to the current coordinate transformation matrix */

  // === all code below in this class has been to the reorganisation FL_PORTING process
public:
  static const char *class_id;
  virtual const char *class_name() {return class_id;};
  Fl_Graphics_Driver();
  virtual ~Fl_Graphics_Driver() { if (p) free(p); }
public:
  // --- implementation is in src/fl_rect.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_rect.cxx
  virtual void point(int x, int y) = 0;
  virtual void rect(int x, int y, int w, int h) = 0;
  virtual void rectf(int x, int y, int w, int h) = 0;
  virtual void line(int x, int y, int x1, int y1) = 0;
  virtual void line(int x, int y, int x1, int y1, int x2, int y2) = 0;
  virtual void xyline(int x, int y, int x1) = 0;
  virtual void xyline(int x, int y, int x1, int y2) = 0;
  virtual void xyline(int x, int y, int x1, int y2, int x3) = 0;
  virtual void yxline(int x, int y, int y1) = 0;
  virtual void yxline(int x, int y, int y1, int x2) = 0;
  virtual void yxline(int x, int y, int y1, int x2, int y3) = 0;
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2) = 0;
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) = 0;
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2) = 0;
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) = 0;
  // --- clipping
  virtual void push_clip(int x, int y, int w, int h) = 0;
  virtual int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) = 0;
  virtual int not_clipped(int x, int y, int w, int h) = 0;
  virtual void push_no_clip() = 0;
  virtual void pop_clip() = 0;
  virtual Fl_Region clip_region();              // has default implementation
  virtual void clip_region(Fl_Region r);        // has default implementation
  virtual void restore_clip();
  // --- implementation is in src/fl_vertex.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_vertex.cxx
  virtual void push_matrix();
  virtual void pop_matrix();
  virtual void mult_matrix(double a, double b, double c, double d, double x, double y);
  virtual void rotate(double d);
  virtual void scale(double x, double y);
  virtual void scale(double x);
  virtual void translate(double x,double y);
  virtual void begin_points();
  virtual void begin_line();
  virtual void begin_loop();
  virtual void begin_polygon();
  virtual void begin_complex_polygon() = 0;
  virtual double transform_x(double x, double y);
  virtual double transform_y(double x, double y);
  virtual double transform_dx(double x, double y);
  virtual double transform_dy(double x, double y);
  virtual void transformed_vertex(double xf, double yf) = 0;
  virtual void vertex(double x, double y) = 0;
  virtual void end_points() = 0;
  virtual void end_line() = 0;
  virtual void end_loop() = 0;
  virtual void end_polygon() = 0;
  virtual void end_complex_polygon() = 0;
  virtual void gap() = 0;
  virtual void circle(double x, double y, double r) = 0;
  // --- implementation is in src/fl_arc.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_arc.cxx if needed
  virtual void arc(double x, double y, double r, double start, double end);
  // --- implementation is in src/fl_arci.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_arci.cxx
  virtual void arc(int x, int y, int w, int h, double a1, double a2) = 0;
  virtual void pie(int x, int y, int w, int h, double a1, double a2) = 0;
  // --- implementation is in src/fl_curve.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_curve.cxx if needed
  virtual void curve(double X0, double Y0, double X1, double Y1, double X2, double Y2, double X3, double Y3);
  // --- implementation is in src/fl_line_style.cxx which includes src/cfg_gfx/xxx_line_style.cxx
  virtual void line_style(int style, int width=0, char* dashes=0) = 0;
  // --- implementation is in src/fl_color.cxx which includes src/cfg_gfx/xxx_color.cxx
  virtual void color(Fl_Color c) { color_ = c; }
  virtual Fl_Color color() { return color_; }
  virtual void color(uchar r, uchar g, uchar b) = 0;
  // --- implementation is in src/fl_font.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_font.cxx
  virtual void draw(const char *str, int n, int x, int y) = 0;
  virtual void draw(const char *str, int n, float x, float y) { draw(str, n, (int)(x+0.5), (int)(y+0.5));}
  virtual void draw(int angle, const char *str, int n, int x, int y) { draw(str, n, x, y); }
  virtual void rtl_draw(const char *str, int n, int x, int y) { draw(str, n, x, y); }
  virtual void font(Fl_Font face, Fl_Fontsize fsize) {font_ = face; size_ = fsize;}
  virtual Fl_Font font() {return font_; }
  virtual Fl_Fontsize size() {return size_; }
  virtual double width(const char *str, int n) { return 0; }
  virtual double width(unsigned int c) { char ch = (char)c; return width(&ch, 1); }
  virtual void text_extents(const char*, int n, int& dx, int& dy, int& w, int& h);
  virtual int height() { return size(); }
  virtual int descent() { return 0; }
  virtual Fl_Font_Descriptor *font_descriptor() { return font_descriptor_;}
  virtual void font_descriptor(Fl_Font_Descriptor *d) { font_descriptor_ = d;}
  // --- implementation is in src/fl_image.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_font.cxx
  virtual   void draw_image(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0) {}
  virtual   void draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0) {}
  virtual   void draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3) {}
  virtual   void draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1) {}
  /** \brief Draws an Fl_RGB_Image object to the device.
   *
   Specifies a bounding box for the image, with the origin (upper left-hand corner) of
   the image offset by the cx and cy arguments.
   */
  virtual   void draw(Fl_RGB_Image * rgb,int XP, int YP, int WP, int HP, int cx, int cy) {}
  /** \brief Draws an Fl_Pixmap object to the device.
   *
   Specifies a bounding box for the image, with the origin (upper left-hand corner) of
   the image offset by the cx and cy arguments.
   */
  virtual   void draw(Fl_Pixmap * pxm,int XP, int YP, int WP, int HP, int cx, int cy) {}
  /** \brief Draws an Fl_Bitmap object to the device.
   *
   Specifies a bounding box for the image, with the origin (upper left-hand corner) of
   the image offset by the cx and cy arguments.
   */
  virtual void draw(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) {}
  virtual int draw_scaled(Fl_Image *img, int X, int Y, int W, int H);
  virtual void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy);

protected:
  // --- implementation is in src/fl_vertex.cxx which includes src/cfg_gfx/xxx_rect.cxx
  virtual void transformed_vertex0(COORD_T x, COORD_T y);
  virtual void fixloop();
};



#if defined(__APPLE__)

// FIXME: add Fl_Quartz_Printer_Graphics_Driver
// FIXME: it should not be required to include this file here. This is nothing that the user should have access to.
#include "src/drivers/Quartz/Fl_Quartz_Graphics_Driver.h"

#elif defined(WIN32) || defined(FL_DOXYGEN)

// FIXME: it should not be required to include this file here. This is nothing that the user should have access to.
#include "src/drivers/GDI/Fl_GDI_Graphics_Driver.h"

/**
 The graphics driver used when printing on MSWindows.
 *
 This class is implemented only on the MSWindows platform. It 's extremely similar to Fl_GDI_Graphics_Driver.
 */
class FL_EXPORT Fl_GDI_Printer_Graphics_Driver : public Fl_GDI_Graphics_Driver {
public:
  static const char *class_id;
  const char *class_name() {return class_id;};
  void draw(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy);
  int draw_scaled(Fl_Image *img, int XP, int YP, int WP, int HP);
};

#elif defined(FL_PORTING)

# pragma message "FL_PORTING: define a native graphics driver Fl_xxx_Graphics_Driver"
class FL_EXPORT Fl_XXX_Graphics_Driver : public Fl_Graphics_Driver {
protected:
  // --- recently moved implementations (see FL_PORTING efforts)
  void point(int x, int y) { }
  void rect(int x, int y, int w, int h) { }
};

#else // X11

// FIXME: it should not be required to include this file here. This is nothing that the user should have access to.
#include "src/drivers/Xlib/Fl_Xlib_Graphics_Driver.h"


#endif



/**
 A drawing surface that's susceptible to receive graphical output.
 Any FLTK application has at any time a current drawing surface to which all drawing requests are directed.
 The current surface is given by Fl_Surface_Device::surface().
 When main() begins running, the current drawing surface has been set to the computer's display, 
 an instance of the Fl_Display_Device class.

 A drawing surface other than the computer's display, is typically used as follows:
 <ol><li> Create \c surface, an object from a particular Fl_Surface_Device derived class (e.g., Fl_Copy_Surface, Fl_Printer).
 <li> Memorize what is the current drawing surface with <tt> Fl_Surface_Device *old_current = Fl_Surface_Device::surface();</tt>
 <li> Call \c surface->set_current(); to redirect all graphics requests to \c surface which becomes the new
 current drawing surface (not necessary with class Fl_Printer because it is done by Fl_Printer::start_job()).
 <li> At this point any of the \ref fl_drawings (e.g., fl_rect()) or the \ref fl_attributes or \ref drawing_images functions
 (e.g., fl_draw_image(), Fl_Image::draw()) operates on the new current drawing surface.
 Certain drawing surfaces allow additional ways to draw to them (e.g., Fl_Printer::print_widget(), Fl_Image_Surface::draw()).
 <li> After all drawing requests have been performed, redirect graphics requests back to their previous destination
 with \c old_current->set_current();.
 <li> Delete \c surface.
 </ol>
 */
class FL_EXPORT Fl_Surface_Device : public Fl_Device {
  /** \brief The graphics driver in use by this surface. */
  Fl_Graphics_Driver *_driver;
  static Fl_Surface_Device *_surface; // the surface that currently receives graphics output
  static Fl_Surface_Device *default_surface(); // create surface is none exists yet
protected:
  /** \brief Constructor that sets the graphics driver to use for the created surface. */
  Fl_Surface_Device(Fl_Graphics_Driver *graphics_driver) {_driver = graphics_driver; };
public:
  static const char *class_id;
  const char *class_name() {return class_id;};
  virtual void set_current(void);
  /** \brief Sets the graphics driver of this drawing surface. */
  inline void driver(Fl_Graphics_Driver *graphics_driver) {_driver = graphics_driver;};
  /** \brief Returns the graphics driver of this drawing surface. */
  inline Fl_Graphics_Driver *driver() {return _driver; };
  /** The current drawing surface.
   In other words, the Fl_Surface_Device object that currently receives all graphics output */
  static inline Fl_Surface_Device *surface() {
    return _surface ? _surface : default_surface();
  };
  /** \brief The destructor. */
  virtual ~Fl_Surface_Device() {}
};

/**
 A display to which the computer can draw.
 When the program begins running, an Fl_Display_Device instance has been created and made the current drawing surface.
 There is no need to create any other object of this class.
 */
class FL_EXPORT Fl_Display_Device : public Fl_Surface_Device {
  friend class Fl_Quartz_Graphics_Driver;
  static Fl_Display_Device *_display; // the platform display device
#ifdef __APPLE__
  friend class Fl_X;
  friend class Fl_Graphics_Driver;
  static bool high_res_window_; //< true when drawing to a window of a retina display (Mac OS X only)
  static bool high_resolution() {return high_res_window_;}
#elif defined(WIN32)
#elif defined(FL_PORTING)
# pragma message "FL_PORTING: implement functions for extra high res drawing if your platform supports it"
#else
#endif
public:
  static const char *class_id;
  const char *class_name() {return class_id;};
  Fl_Display_Device(Fl_Graphics_Driver *graphics_driver);
  static Fl_Display_Device *display_device();
};

/**
 This plugin socket allows the integration of new device drivers for special
 window or screen types. It is currently used to provide an automated printing
 service and screen capture for OpenGL windows, if linked with fltk_gl.
 */
class FL_EXPORT Fl_Device_Plugin : public Fl_Plugin {
public:
  /** \brief The constructor */
  Fl_Device_Plugin(const char *pluginName)
  : Fl_Plugin(klass(), pluginName) { }
  /** \brief Returns the class name */
  virtual const char *klass() { return "fltk:device"; }
  /** \brief Returns the plugin name */
  virtual const char *name() = 0;
  /** \brief Prints a widget 
   \param w the widget
   \param x,y offsets where to print relatively to coordinates origin
   \param height height of the current drawing area
   */
  virtual int print(Fl_Widget* w, int x, int y, int height) = 0;
  /** captures a rectangle of a widget as an image
   \return The captured pixels as an RGB image
   */
  virtual Fl_RGB_Image* rectangle_capture(Fl_Widget *widget, int x, int y, int w, int h) = 0;
};

#endif // Fl_Device_H

//
// End of "$Id$".
//
