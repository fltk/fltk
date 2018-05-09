//
// "$Id$"
//
// Printing support for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2016 by Bill Spitzak and others.
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

/** \file Fl_Printer.H 
 \brief declaration of class Fl_Printer.
 */

#ifndef Fl_Printer_H
#define Fl_Printer_H

#include <FL/Fl_Paged_Device.H>

/**
 * \brief OS-independent print support.
 *
 Fl_Printer allows to use all drawing, color, text, image, and clip FLTK functions, and to have them operate
 on printed page(s). There are two main, non exclusive, ways to use it.
 <ul><li>Print any widget (standard, custom, Fl_Window, Fl_Gl_Window) as it appears 
 on screen, with optional translation, scaling and rotation. This is done by calling print_widget(), 
 print_window() or print_window_part().
 <li>Use a series of FLTK graphics commands (e.g., font, text, lines, colors, clip, image) to
 compose a page appropriately shaped for printing. 
 </ul>
 In both cases, begin by begin_job(), begin_page(), printable_rect() and origin() calls
 and finish by end_page() and end_job() calls.
 <p>Example of use: print a widget centered in a page
 \code
 #include <FL/Fl_Printer.H>
 #include <FL/fl_draw.H>
 int width, height;
 Fl_Widget *widget = ...  // a widget we want printed
 Fl_Printer *printer = new Fl_Printer();
 if (printer->begin_job(1) == 0) {
  printer->begin_page();
  printer->printable_rect(&width, &height);
  fl_color(FL_BLACK);
  fl_line_style(FL_SOLID, 2);
  fl_rect(0, 0, width, height);
  fl_font(FL_COURIER, 12);
  time_t now; time(&now); fl_draw(ctime(&now), 0, fl_height());
  printer->origin(width/2, height/2);
  printer->print_widget(widget, -widget->w()/2, -widget->h()/2);
  printer->end_page();
  printer->end_job();
 }
 delete printer;
 \endcode
 <b>Platform specifics</b>
 <ul>
 <li>Unix/Linux platforms:
 Unless it has been previously changed, the default paper size is A4.
 To change that, press the "Properties" button of the "Print" dialog window
 opened by an Fl_Printer::begin_job() call. This opens a "Printer Properties" window where it's
 possible to select the adequate paper size. Finally press the "Save" button therein to assign
 the chosen paper size to the chosen printer for this and all further print operations.
 <br>Class Fl_RGB_Image prints but loses its transparency if it has one.
 See class Fl_PostScript_Graphics_Driver for a description of how UTF-8 strings appear in print.
 Use the static public attributes of this class to set the print dialog to other languages 
 than English. For example, the "Printer:" dialog item Fl_Printer::dialog_printer can be set to French with:
 \code
 Fl_Printer::dialog_printer = "Imprimante:";
 \endcode
 before creation of the Fl_Printer object.
 Use Fl_PostScript_File_Device::file_chooser_title to customize the title of the file chooser dialog that opens
 when using the "Print To File" option of the print dialog. 
 <li>Windows platform: Transparent Fl_RGB_Image 's don't print with exact transparency on most printers
 (a workaround is to use print_window_part() ).
 Fl_RGB_Image 's don't rotate() well.
 <li>Mac OS X platform: all graphics requests print as on display and accept rotation and scaling.
 </ul>
 */
class FL_EXPORT Fl_Printer : public Fl_Paged_Device {
private:
  Fl_Paged_Device *printer;
public:
  /** The constructor */
  Fl_Printer(void);
  int begin_job(int pagecount, int *frompage = NULL, int *topage = NULL);
  int begin_page(void);
  int printable_rect(int *w, int *h);
  void margins(int *left, int *top, int *right, int *bottom);
  void origin(int *x, int *y);
  void origin(int x, int y);
  void scale(float scale_x, float scale_y = 0.);
  void rotate(float angle);
  void translate(int x, int y);
  void untranslate(void);
  int end_page (void);
  void end_job (void);
  void draw_decorated_window(Fl_Window *win, int x_offset, int y_offset);
  void set_current(void);
 
  /** \name These attributes are useful for the Linux/Unix platform only.
   \{
   */  
  static const char *dialog_title; 
  static const char *dialog_printer; 
  static const char *dialog_range; 
  static const char *dialog_copies; 
  static const char *dialog_all; 
  static const char *dialog_pages;
  static const char *dialog_from;
  static const char *dialog_to;
  static const char *dialog_properties;
  static const char *dialog_copyNo;
  static const char *dialog_print_button;
  static const char *dialog_cancel_button;
  static const char *dialog_print_to_file;
  static const char *property_title;
  static const char *property_pagesize;
  static const char *property_mode;
  static const char *property_use;
  static const char *property_save;
  static const char *property_cancel;
  /** \} */ 
  /** The destructor */
  ~Fl_Printer(void);
};

#endif // Fl_Printer_H

//
// End of "$Id$"
//
