//
// "$Id: Fl_Abstract_Printer.H 7330 2010-03-25 13:59:00Z manolo $"
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

/** \file Fl_Abstract_Printer.H 
 \brief declaration of class Fl_Abstract_Printer.
 */

#ifndef Fl_Abstract_Printer_H
#define Fl_Abstract_Printer_H

#include <FL/Fl_Device.H>

/**
 \brief A virtual class for print support with several platform-specific implementations.
 *
 This class has no public constructor: don't instantiate it; use Fl_Printer or Fl_PSfile_Device instead.
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

#endif // Fl_Abstract_Printer_H

//
// End of "$Id$"
//

