//
// "$Id$"
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

#ifndef Fl_Image_Surface_H
#define Fl_Image_Surface_H

#include <FL/Fl_Widget_Surface.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/x.H> // for Fl_Offscreen


/** Directs all graphics requests to an Fl_Image.
 
 After creation of an Fl_Image_Surface object, call set_current() on it, and all subsequent graphics requests
 will be recorded in the image. It's possible to draw widgets (using Fl_Image_Surface::draw()) 
 or to use any of the \ref fl_drawings or the \ref fl_attributes.
 Finally, call image() on the object to obtain a newly allocated Fl_RGB_Image object.
 <br> Fl_GL_Window objects can be drawn in the image as well. 
 
 <br> Usage example:
 \code
 Fl_Widget *g = ...; // a widget you want to draw in an image
 Fl_Image_Surface *img_surf = new Fl_Image_Surface(g->w(), g->h()); // create an Fl_Image_Surface object
 img_surf->set_current(); // direct graphics requests to the image
 fl_color(FL_WHITE); fl_rectf(0, 0, g->w(), g->h()); // draw a white background
 img_surf->draw(g); // draw the g widget in the image
 Fl_RGB_Image* image = img_surf->image(); // get the resulting image
 delete img_surf; // delete the img_surf object
 Fl_Display_Device::display_device()->set_current();  // direct graphics requests back to the display
 \endcode
*/
class FL_EXPORT Fl_Image_Surface : public Fl_Widget_Surface {
  friend Fl_Offscreen fl_create_offscreen(int w, int h);
  friend void fl_begin_offscreen(Fl_Offscreen ctx);
  friend void fl_end_offscreen(void);
  friend void fl_delete_offscreen(Fl_Offscreen ctx);
private:
  class Helper;
  Helper *platform_surface;
protected:
  void translate(int x, int y);
  void untranslate();
public:
  Fl_Image_Surface(int w, int h, int high_res = 0);
  Fl_Image_Surface(Fl_Offscreen off, int w, int h);
  ~Fl_Image_Surface();
  void set_current();
  void end_current();
  Fl_RGB_Image *image();
  Fl_Shared_Image *highres_image();
  void origin(int *x, int *y);
  void origin(int x, int y);
  int printable_rect(int *w, int *h);
  Fl_Offscreen get_offscreen_before_delete();
  Fl_Offscreen offscreen();
};

#endif // Fl_Image_Surface_H

//
// End of "$Id$".
//
