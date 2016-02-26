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

#include <FL/Fl_Copy_Surface.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Shared_Image.H>


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
private:
  Fl_Offscreen offscreen;
  int width;
  int height;
#ifdef __APPLE__ // PORTME: Fl_Surface_Driver - platform image surface driver
#elif defined(WIN32)
  HDC _sgc; 
  Window _sw; 
  Fl_Surface_Device *_ss;
  int _savedc;
#elif defined(FL_PORTING)
# pragma message "FL_PORTING: define variables to hold image data for Fl_Image_Surface"
#else
  Fl_Surface_Device *previous;
  Window pre_window;
#endif
protected:
  void translate(int x, int y);
  void untranslate();
public:
  Fl_Image_Surface(int w, int h, int highres = 0);
  ~Fl_Image_Surface();
  void set_current();
  Fl_RGB_Image *image();
  Fl_Shared_Image *highres_image();
};

#endif // Fl_Image_Surface_H

//
// End of "$Id$".
//
