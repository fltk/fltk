//
// "$Id$"
//
// Printing support for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010 by Bill Spitzak and others.
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
/** \file Fl_Printer.H 
 \brief declaration of classes Fl_Abstract_Printer, Fl_Printer,  Fl_Device_Plugin.
 */

#ifndef Fl_Printer_H
#define Fl_Printer_H

#include <FL/Fl_Device.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Bitmap.H>
#include <stdio.h>

/**
 \brief A virtual class for print support with several platform-specific implementations.
 *
 This class has no public constructor: don't instantiate it; use Fl_Printer or Fl_PSfile_Device instead.
 \see class Fl_Printer for full documentation of member functions.
 */
class Fl_Abstract_Printer : public Fl_Device {
   friend class Fl_Pixmap;
   friend class Fl_RGB_Image;
   friend class Fl_Bitmap;
private:
#ifdef __APPLE__
  struct chain_elt {
    Fl_Image *image;
    const uchar *data;
    struct chain_elt *next;
  };
  void add_image(Fl_Image *image, const uchar *data); // adds an image to the page image list
#endif
  void traverse(Fl_Widget *widget); // finds subwindows of widget and prints them
protected:
  /** \brief horizontal offset to the origin of graphics coordinates */
  int x_offset;
  /** \brief vertical offset to the origin of graphics coordinates */
  int y_offset;
  /** \brief chained list of Fl_Image's used in this page */
  struct chain_elt *image_list_; 
  /** \brief the printer's graphics context, if there's one, NULL otherwise */
  void *gc; 
  /** \brief the constructor */
  Fl_Abstract_Printer(void) { gc = NULL; bg_r_ = bg_g_ = bg_b_ = 0; };
#ifdef __APPLE__
  /** \brief deletes the page image list */
  void delete_image_list(); 
#endif
public:
   Fl_Device *set_current(void);
   virtual int start_job(int pagecount, int *frompage = NULL, int *topage = NULL);
   virtual int start_page(void);
   virtual int printable_rect(int *w, int *h);
   virtual void margins(int *left, int *top, int *right, int *bottom);
   virtual void origin(int x, int y);
   void origin(int *x, int *y);
   virtual void scale(float scale_x, float scale_y);
   virtual void rotate(float angle);
   virtual void translate(int x, int y);
   virtual void untranslate(void);
   void print_widget(Fl_Widget* widget, int delta_x = 0, int delta_y = 0);
   void print_window_part(Fl_Window *win, int x, int y, int w, int h, int delta_x = 0, int delta_y = 0);
   virtual int end_page (void);
   virtual void end_job (void);
};

#if defined(__APPLE__) || defined(WIN32) || defined(FL_DOXYGEN)
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
class Fl_Printer : public Fl_Abstract_Printer {
private:
#ifdef __APPLE__
  float scale_x;
  float scale_y;
  float angle; // rotation angle in radians
  PMPrintSession  printSession;
  PMPageFormat    pageFormat;
  PMPrintSettings printSettings;
#elif defined(WIN32)
  int   abortPrint;
  PRINTDLG      pd;
  HDC           hPr;
  int           prerr;
  int left_margin;
  int top_margin;
  void absolute_printable_rect(int *x, int *y, int *w, int *h);
#endif
public:
  /** 
   @brief The constructor.
   */
  Fl_Printer(void);
  int start_job(int pagecount, int *frompage = NULL, int *topage = NULL);
  int start_page (void);
  int printable_rect(int *w, int *h);
  void margins(int *left, int *top, int *right, int *bottom);
  void origin(int x, int y);
#ifdef FL_DOXYGEN  
  void origin(int *x, int *y);
#endif
  void scale (float scale_x, float scale_y);
  void rotate(float angle);
  void translate(int x, int y);
  void untranslate(void);
#ifdef FL_DOXYGEN  
  void print_widget(Fl_Widget* widget, int delta_x = 0, int delta_y = 0);
  void print_window_part(Fl_Window *win, int x, int y, int w, int h, int delta_x = 0, int delta_y = 0);
#endif  
  int end_page (void);
  void end_job (void);
};

#else

#include <FL/Fl_PSfile_Device.H>

class Fl_Printer : public Fl_PSfile_Device {
public:
  Fl_Printer(void) {};
  ~Fl_Printer(void) {};
  int start_job(int pages, int *firstpage = NULL, int *lastpage = NULL);
};
#endif  

/**
 This plugin socket allows the integration of new device drivers for special
 window or screen types. It is currently used to provide an automated printing
 service for OpenGL windows, if linked with fltk_gl.
 */
class Fl_Device_Plugin : public Fl_Plugin {
public:
  /** \brief The constructor */
  Fl_Device_Plugin(const char *name)
  : Fl_Plugin(klass(), name) { }
  /** \brief Returns the class name */
  virtual const char *klass() { return "fltk:device"; }
  /** \brief Returns the plugin name */
  virtual const char *name() = 0;
  /** \brief Prints a widget 
   \param p the printer
   \param w the widget
   \param x,y offsets where to print relatively to coordinates origin
   */
  virtual int print(Fl_Abstract_Printer* p, Fl_Widget* w, int x, int y) { return 0; }
};

#endif // Fl_Printer_H

//
// End of "$Id$"
//
