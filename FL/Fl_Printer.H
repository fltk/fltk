//
// "$Id: Fl_Printer.H 7263 2010-03-14 18:07:24Z AlbrechtS $"
//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
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

#ifndef Fl_Printer_H
#define Fl_Printer_H

#include <FL/Fl_Device.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Bitmap.H>
#include <stdio.h>

/**
 * @brief Provides an OS-independent interface to printing.
 *
 It allows to use all FLTK drawing, color, text, and clip functions, and to have them operate
 on printed page(s). There are two main, non exclusive, ways to use it.
 <ul><li>Print any widget (standard, custom, Fl_Window, Fl_Gl_Window) as it appears 
 on screen, with optional translation and scaling. This is done by calling print_widget() 
 or print_window_part().
 <li>Use a series of FLTK graphics commands (e.g., font, text, lines, colors, clip) to
 compose a page appropriately shaped for printing. 
 </ul>
 In both cases, begin by start_job(), start_page(), printable_rect() and origin() calls
 and finish by end_page() and end_job() calls.
 */
class 
#ifndef FL_DOXYGEN
  Fl_Virtual_Printer
#else
  Fl_Printer
#endif
   : public Fl_Device {
     friend class Fl_Pixmap;
     friend class Fl_RGB_Image;
     friend class Fl_Bitmap;
private:
  struct chain_elt {
    Fl_Image *image;
    const uchar *data;
    struct chain_elt *next;
  };
  void add_image(Fl_Image *image, const uchar *data); // adds an image to the page image list
  void traverse(Fl_Widget *widget); // finds subwindows of widget and prints them
protected:
  int y_offset;
  int x_offset;
  struct chain_elt *image_list_; // chained list of Fl_Image's used in this page
  void *gc; // the printer's graphics context, if there's one, NULL otherwise
  void delete_image_list(); // deletes the page image list
#ifndef FL_DOXYGEN
     Fl_Virtual_Printer(void) { gc = NULL; bg_r_ = bg_g_ = bg_b_ = 0; };
#endif
public:
#ifdef FL_DOXYGEN
     /** 
      @brief The constructor.
      */
     Fl_Printer(void);

     /** 
      @brief The destructor.
      */
     ~Fl_Printer(void);
#endif
     
     /**
      @brief Starts a print job.
      *
      @param[in] pagecount the total number of pages of the job
      @param[out] frompage if non-null, *frompage is set to the first page the user wants printed
      @param[out] topage if non-null, *topage is set to the last page the user wants printed
      @return 0 iff OK
      */
     virtual int start_job(int pagecount, int *frompage = NULL, int *topage = NULL);
     
     /**
      @brief Starts a new printed page
      *
      The page coordinates are initially in points, i.e., 1/72 inch, 
      and with origin at the top left of the printable page area.
      @return 0 iff OK
      */
     virtual int start_page(void);
     
     /**
      @brief Computes the width and height of the printable area of the page.
      *
      Values are in the same unit as that used by FLTK drawing functions,
      are unchanged by calls to origin(), but are changed by scale() calls.
      Values account for the user-selected paper type and print orientation.
      @return 0 iff OK.
      */
     virtual int printable_rect(int *w, int *h);
     
     /**
      @brief Computes the dimensions of margins that lie between the printable page area and
      the full page.
      *
      Values are in the same unit as that used by FLTK drawing functions. They are changed
      by scale() calls.
      @param[out] left If non-null, *left is set to the left margin size.
      @param[out] top If non-null, *top is set to the top margin size.
      @param[out] right If non-null, *right is set to the right margin size.
      @param[out] bottom If non-null, *bottom is set to the bottom margin size.
      */
     virtual void margins(int *left, int *top, int *right, int *bottom);
     
     /**
      @brief Sets the position in page coordinates of the origin of graphics functions.
      *
      Arguments should be expressed relatively to the result of a previous printable_rect() call.
      That is, <tt>printable_rect(&w, &h); origin(w/2, 0);</tt> sets the graphics origin at the
      top center of the page printable area.
      Origin() calls are not affected by rotate() calls.
      Successive origin() calls don't combine their effects.
      @param[in] x Horizontal position in page coordinates of the desired origin of graphics functions.
      @param[in] y Same as above, vertically.
      */
     virtual void origin(int x, int y);
     
     /**
      @brief Computes the page coordinates of the current origin of graphics functions.
      *
      @param[out] x If non-null, *x is set to the horizontal page offset of graphics origin.
      @param[out] y Same as above, vertically.
      */
     void origin(int *x, int *y);
     
     /**
      @brief Changes the scaling of page coordinates.
      *
      This function also resets the origin of graphics functions at top left of printable page area.
      After a scale() call, do a printable_rect() call to get the new dimensions of the printable page area.
      Successive scale() calls don't combine their effects.
      @param scale_x Horizontal dimensions of plot are multiplied by this quantity.
      @param scale_y Same as above, vertically.
      */
     virtual void scale (float scale_x, float scale_y);
     
     /**
      @brief Rotates the graphics operations relatively to paper.
      *
      The rotation is centered on the current graphics origin. Successive rotate() calls don't combine their effects.
      On MSWindows, Fl_RGB_Image's don't rotate well; print_window_part() is an efficient workaround.
      @param angle Rotation angle in counterclockwise degrees.
      */
     virtual void rotate(float angle);
     
     /**
      @brief Translates the current graphics origin accounting for the current rotation.
      *
      This function is only useful after a rotate() call. 
      Each translate() call must be matched by an untranslate() call.
      Successive translate() calls add up their effects.
      */
     virtual void translate(int x, int y);
     
     /**
      @brief Undoes the effect of a previous translate() call.
      */
     virtual void untranslate(void);
     
     /**
      @brief Draws the widget on the printed page.
      *
      The widget's position on the printed page is determined by the last call to origin()
      and by the optional delta_x and delta_y arguments.
      Its dimensions are in points unless there was a previous call to scale().
      <br>Under MSWindows, Fl_RGB_Image's are printed without transparency.
      A workaround is to use the print_window_part() call. 
      @param[in] widget Any FLTK widget (e.g., standard, custom, window).
      @param[in] delta_x Optional horizontal offset for positioning the widget relatively
      to the current origin of graphics functions.
      @param[in] delta_y Same as above, vertically.
      */
     void print_widget(Fl_Widget* widget, int delta_x = 0, int delta_y = 0);
     
     /**
      @brief Prints a rectangular part of an on-screen window.
      *
      @param win The window from where to capture.
      @param x The rectangle left
      @param y The rectangle top
      @param w The rectangle width
      @param h The rectangle height
      @param delta_x Optional horizontal offset from current graphics origin where to print the captured rectangle.
      @param delta_y As above, vertically.
      */
     void print_window_part(Fl_Window *win, int x, int y, int w, int h, int delta_x = 0, int delta_y = 0);
     
     /**
      @brief To be called at the end of each page.
      *
      @return 0 iff OK.
      */
     virtual int end_page (void);
     
     /**
      @brief To be called at the end of a print job.
      */
     virtual void end_job (void);
          
     /**
      @brief Sets this printer as the target of future graphics calls.
      @return The current target device of graphics calls.
      */
     Fl_Device *set_current(void);
   };

#ifdef __APPLE__
class Fl_Quartz_Printer : public Fl_Virtual_Printer {
private:
  float scale_x;
  float scale_y;
  float angle; // rotation angle in radians
  PMPrintSession  printSession;
  PMPageFormat    pageFormat;
  PMPrintSettings printSettings;
public:
  Fl_Quartz_Printer(void);
  int start_job(int pagecount, int *frompage = NULL, int *topage = NULL);
  int start_page (void);
  int printable_rect(int *w, int *h);
  void margins(int *left, int *top, int *right, int *bottom);
  void origin(int x, int y);
  void scale (float scale_x, float scale_y);
  void rotate(float angle);
  void translate(int x, int y);
  void untranslate(void);
  int end_page (void);
  void end_job (void);
};
#endif

#ifdef WIN32
class Fl_GDI_Printer : public Fl_Virtual_Printer {
private:
  int   abortPrint;
  PRINTDLG      pd;
  HDC           hPr;
  int           prerr;
  int left_margin;
  int top_margin;
  void absolute_printable_rect(int *x, int *y, int *w, int *h);
public:
  Fl_GDI_Printer(void);
  int start_job(int pagecount, int *frompage = NULL, int *topage = NULL);
  int start_page (void);
  int printable_rect(int *w, int *h);
  void margins(int *left, int *top, int *right, int *bottom);
  void origin(int x, int y);
  void scale (float scale_x, float scale_y);
  void rotate(float angle);
  void translate(int x, int y);
  void untranslate(void);
  int end_page (void);
  void end_job (void);
};
#endif


/**
 @brief Creates a session where all graphics will go to a local PostScript file. Same API as Fl_Printer class.
 *
 This class has the same API as class Fl_Printer except for start_job() member function.
 \see class Fl_Printer.
 */
class Fl_PSfile_Device : public 
#ifndef FL_DOXYGEN
  Fl_Virtual_Printer 
#else
  Fl_Device 
#endif
  { // defined for all 3 platforms
public: 
    /**
     @brief Possible page formats.
     */
    enum Page_Format{
      A0 = 0, 
      A1,
      A2,
      A3,
      A4,
      A5,
      A6,
      A7,
      A8,
      A9,
      B0,
      B1,
      B2,
      B3,
      B4,
      B5,
      B6,
      B7,
      B8,
      B9,
      B10,
      C5E,
      DLE,
      EXECUTIVE,
      FOLIO,
      LEDGER,
      LEGAL,
      LETTER,
      TABLOID,
      ENVELOPE,
      MEDIA = 0x1000
    };
    
    /**
     @brief Possible page layouts.
     */
    enum Page_Layout {PORTRAIT = 0, LANDSCAPE = 0x100, REVERSED = 0x200, ORIENTATION = 0x300};

#ifndef FL_DOXYGEN
private:
  enum SHAPE{NONE=0, LINE, LOOP, POLYGON, POINTS};
  
  class Clip{
  public:
    int x, y, w, h;
    Clip *prev;
  };
  Clip * clip_;
  
  int lang_level_;
  int font_;
  int size_;
  Fl_Color color_;
  int gap_;
  int pages_;
  
  double width_;
  double height_;
  
  int shape_;
  int linewidth_;// need for clipping, lang level 1-2
  int linestyle_;//
  int interpolate_; //interpolation of images
  unsigned char cr_,cg_,cb_;
  char  linedash_[256];//should be enought
  void concat();  // transform ror scalable dradings...
  void reconcat(); //invert
  void recover(); //recovers the state afrer grestore (such as line styles...)
  void reset();
  
  uchar * mask;
  int mx; // width of mask;
  int my; // mask lines
  //Fl_Color bg_;
  int (*close_cmd_)(FILE *);
  int page_policy_;
  int nPages;
  int orientation_;
  
  float scale_x;
  float scale_y;
  float angle;
  int left_margin;
  int top_margin;

protected:
  struct page_format {
    int width;
    int height;
    const char *name;
  };

  FILE *output;
  double pw_, ph_;
  static const int NO_PAGE_FORMATS=30;
  static const struct page_format page_formats[NO_PAGE_FORMATS];
  
  uchar bg_r, bg_g, bg_b;
  int start_postscript (int pagecount, enum Page_Format format, enum Page_Layout layout);
  /*  int alpha_mask(const uchar * data, int w, int h, int D, int LD=0);
   */
  void draw(const char* s, int n, int x, int y) {transformed_draw(s,n,x,y); };
  void draw(int angle, const char *str, int n, int x, int y);
  void transformed_draw(const char* s, int n, double x, double y); //precise text placing
  void transformed_draw(const char* s, double x, double y);
  int alpha_mask(const uchar * data, int w, int h, int D, int LD=0);
  void draw_scaled_image(const uchar *data, double x, double y, double w, double h, int iw, int ih, int D=3, int LD=0);
  void draw_scaled_image_mono(const uchar *data, double x, double y, double w, double h, int iw, int ih, int D=3, int LD=0);
  void draw_scaled_image(Fl_Draw_Image_Cb call, void *data, double x, double y, double w, double h, int iw, int ih, int D);
  void draw_scaled_image_mono(Fl_Draw_Image_Cb call, void *data, double x, double y, double w, double h, int iw, int ih, int D);

  enum Page_Format page_format_;
  char *ps_filename_;
  // implementation of drawing methods
  void color(Fl_Color c);
  //void bg_color(Fl_Color bg);
  void color(uchar r, uchar g, uchar b);
  Fl_Color color(){return color_;};
  
  void push_clip(int x, int y, int w, int h);
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H);
  int not_clipped(int x, int y, int w, int h);
  void push_no_clip();
  void pop_clip();
  
  void line_style(int style, int width=0, char* dashes=0);
  
  void rect(int x, int y, int w, int h);
  void rectf(int x, int y, int w, int h);
  
  void xyline(int x, int y, int x1);
  void xyline(int x, int y, int x1, int y2);
  void xyline(int x, int y, int x1, int y2, int x3);
  
  void yxline(int x, int y, int y1);
  void yxline(int x, int y, int y1, int x2);
  void yxline(int x, int y, int y1, int x2, int y3);
  
  void line(int x1, int y1, int x2, int y2);
  void line(int x1, int y1, int x2, int y2, int x3, int y3);  
  
  void loop(int x0, int y0, int x1, int y1, int x2, int y2);
  void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  void point(int x, int y);
  
  void begin_points();
  void begin_line();
  void begin_loop();
  void begin_polygon();
  void vertex(double x, double y);
  void curve(double x, double y, double x1, double y1, double x2, double y2, double x3, double y3);
  void circle(double x, double y, double r);
  void arc(double x, double y, double r, double start, double a);
  void arc(int x, int y, int w, int h, double a1, double a2);
  void pie(int x, int y, int w, int h, double a1, double a2);
  void end_points();
  void end_line();
  void end_loop();
  void end_polygon();
  void begin_complex_polygon(){begin_polygon();};
  void gap(){gap_=1;};
  void end_complex_polygon(){end_polygon();};
  void transformed_vertex(double x, double y);
  
  void font(int face, int size);
  int font(){return font_;};
  int size(){return size_;};
  double width(unsigned c);
  double width(const char* s, int n);
  int descent();
  int height();
  
  void draw_image(const uchar* d, int x,int y,int w,int h, int delta=3, int ldelta=0){draw_scaled_image(d,x,y,w,h,w,h,delta,ldelta);};
  void draw_image_mono(const uchar* d, int x,int y,int w,int h, int delta=1, int ld=0){draw_scaled_image_mono(d,x,y,w,h,w,h,delta,ld);};
  void draw_image(Fl_Draw_Image_Cb call, void* data, int x,int y, int w, int h, int delta=3){draw_scaled_image(call,data, x, y, w, h, w, h, delta);};
  void draw_image_mono(Fl_Draw_Image_Cb call, void* data, int x,int y, int w, int h, int delta=1){draw_scaled_image_mono(call, data, x, y, w, h, w, h, delta);};
  
  void draw(Fl_Pixmap * pxm,int XP, int YP, int WP, int HP, int cx, int cy);
  void draw(Fl_RGB_Image * rgb,int XP, int YP, int WP, int HP, int cx, int cy);
  void draw(Fl_Bitmap * bitmap,int XP, int YP, int WP, int HP, int cx, int cy);
    
public:
  void page_policy(int p);
  int page_policy(){return page_policy_;};
  void close_command( int (*cmd)(FILE *)){close_cmd_=cmd;};
  FILE * file() {return output;};
  //void orientation (int o);
  //Fl_PSfile_Device(FILE *o, int lang_level, int pages = 0); // ps (also multi-page) constructor
  //Fl_PSfile_Device(FILE *o, int lang_level, int x, int y, int w, int h); //eps constructor
  void interpolate(int i){interpolate_=i;};
  int interpolate(){return interpolate_;}
  
  void page(double pw, double ph, int media = 0);
  void page(int format);
  
  void place(double x, double y, double tx, double ty, double scale = 1);
  int start_page (void);
  int printable_rect(int *w, int *h);
  void margins(int *left, int *top, int *right, int *bottom);
  void origin(int x, int y);
  void scale (float scale_x, float scale_y);
  void rotate(float angle);
  void translate(int x, int y);
  void untranslate(void);
  int end_page (void);
  void end_job (void);
  
#endif // FL_DOXYGEN
    
    /**
     @brief The constructor.
     */
    Fl_PSfile_Device(void);
    
    /**
     @brief The destructor.
     */
    virtual ~Fl_PSfile_Device();
    
    /**
     @brief Begins the session where all graphics will go to a local PostScript file.
     *
     Opens a file dialog to select an output PostScript file.
     @param pagecount The total number of pages to be created.
     @param format Desired page format.
     @param layout Desired page layout.
     @return 0 iff OK
     */
    int start_job(int pagecount, enum Page_Format format = A4, enum Page_Layout layout = PORTRAIT);
    
    /**
     @brief Begins the session where all graphics will go to named PostScript file.
     *
     @param fname The name of the output local PostScript file.
     @param pagecount The total number of pages to be created.
     @param format Desired page format.
     @param layout Desired page layout.
     @return 0 iff OK
     */
    int start_job(const char *fname, int pagecount, enum Page_Format format = A4, enum Page_Layout layout = PORTRAIT);
    
  };

#ifndef FL_DOXYGEN

#ifdef __APPLE__
typedef Fl_Quartz_Printer Fl_Printer;
#elif defined(WIN32)
typedef Fl_GDI_Printer Fl_Printer;
#else
class Fl_PS_Printer : public Fl_PSfile_Device {
public:
  Fl_PS_Printer(void) {};
  ~Fl_PS_Printer(void) {};
  int start_job(int pages, int *firstpage = NULL, int *lastpage = NULL);
};
typedef Fl_PS_Printer Fl_Printer;

#endif  // __APPLE__

#endif // FL_DOXYGEN

#endif // Fl_Printer_H

//
// End of "$Id: Fl_Printer.H 7263 2010-03-14 18:07:24Z AlbrechtS $"
//
