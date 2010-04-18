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
 \brief declaration of classes Fl_Printer,  Fl_Device_Plugin.
 */

#ifndef Fl_Printer_H
#define Fl_Printer_H

#include <FL/Fl_Abstract_Printer.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Bitmap.H>
#include <stdio.h>

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
 <p><b>Platform specifics</b>
 <ul>
 <li>Xlib-based platforms (<i>e.g.</i>, Linux, Unix): this class is implemented as
 a subclass of Fl_PSfile_Device.
 Use the static public attributes of this class to set the print dialog to other languages 
 than English. For example, the "Printer:" dialog item Fl_Printer::dialog_printer can be set to French with:
 \code
 Fl_Printer::dialog_printer = "Imprimante:";
 Fl_Printer myprinter;
 myprinter.start_job();
 \endcode
 Use Fl_PSfile_Device::file_chooser_title to customize the title of the file chooser dialog that opens
 when using the "Print To File" option of the print dialog. 
 Class Fl_RGB_Image prints but loses its transparency if it has one.
 <li>MSWindows platform: Transparent Fl_RGB_Image 's don't print with exact transparency on most printers.
 Fl_RGB_Image 's don't rotate() well.
 A workaround is to use the print_window_part() call. 
 <li>Mac OS X platform: all graphics requests print as on display.
 </ul>
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

  /** 
   @brief The destructor.
   */
  ~Fl_Printer(void);

#else // Xlib (Linux/Unix)

#include <FL/Fl_PSfile_Device.H>

class Fl_Printer : public Fl_PSfile_Device {

public:

  Fl_Printer(void) {};
  ~Fl_Printer(void) {};
  int start_job(int pages, int *firstpage = NULL, int *lastpage = NULL);

#endif	// Fl_Printer (platform-dependent)

  // Fl_Printer:: common for all platforms

public:	// just to be sure ...

  /** \name These attributes apply to the Xlib platform only.
   \{
   */  
  /** [this text may be customized at run-time] */
  static const char *dialog_title; 
  /** [this text may be customized at run-time] */
  static const char *dialog_printer; 
  /** [this text may be customized at run-time] */
  static const char *dialog_range; 
  /** [this text may be customized at run-time] */
  static const char *dialog_copies; 
  /** [this text may be customized at run-time] */
  static const char *dialog_all; 
  /** [this text may be customized at run-time] */
  static const char *dialog_pages;
  /** [this text may be customized at run-time] */
  static const char *dialog_from;
  /** [this text may be customized at run-time] */
  static const char *dialog_to;
  /** "Properties..." [this text may be customized at run-time] */
  static const char *dialog_properties;
  /** [this text may be customized at run-time] */
  static const char *dialog_copyNo;
  /** [this text may be customized at run-time] */
  static const char *dialog_print_button;
  /** [this text may be customized at run-time] */
  static const char *dialog_cancel_button;
  /** [this text may be customized at run-time] */
  static const char *dialog_print_to_file;
  /** [this text may be customized at run-time] */
  static const char *property_title;
  /** [this text may be customized at run-time] */
  static const char *property_pagesize;
  /** [this text may be customized at run-time] */
  static const char *property_mode;
  /** [this text may be customized at run-time] */
  static const char *property_use;
  /** [this text may be customized at run-time] */
  static const char *property_save;
  /** [this text may be customized at run-time] */
  static const char *property_cancel;
  /** \} */

}; // class Fl_Printer

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
   \param w the widget
   \param x,y offsets where to print relatively to coordinates origin
   \param height height of the current drawing area
   */
  virtual int print(Fl_Widget* w, int x, int y, int height) { return 0; }
};

#endif // Fl_Printer_H

//
// End of "$Id$"
//
