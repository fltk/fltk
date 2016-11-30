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
#include <FL/Fl_Graphics_Driver.H> // for Fl_Offscreen


/** 
 \brief Directs all graphics requests to an Fl_Image.
 
 After creation of an Fl_Image_Surface object, call set_current() on it, and all
 subsequent graphics requests will be recorded in the image. It's possible to 
 draw widgets (using Fl_Image_Surface::draw()) or to use any of the 
 \ref fl_drawings or the \ref fl_attributes. Finally, call image() on the object
 to obtain a newly allocated Fl_RGB_Image object.
 Fl_GL_Window objects can be drawn in the image as well.
 
 Usage example:
 \code
 // this is the widget that you want to draw into an image
 Fl_Widget *g = ...;

 // create an Fl_Image_Surface object
 Fl_Image_Surface *image_surface = new Fl_Image_Surface(g->w(), g->h());
 
 // direct all further graphics requests to the image
 image_surface->set_current();

 // draw a white background
 fl_color(FL_WHITE); 
 fl_rectf(0, 0, g->w(), g->h());

 // draw the g widget in the image
 image_surface->draw(g);

 // get the resulting image
 Fl_RGB_Image* image = image_surface->image();
 
 // delete the image_surface object, but not the image itself
 delete image_surface;

 // direct graphics requests back to the screen
 Fl_Display_Device::display_device()->set_current();
 \endcode
*/
class FL_EXPORT Fl_Image_Surface : public Fl_Widget_Surface {
  friend FL_EXPORT Fl_Offscreen fl_create_offscreen(int w, int h);
  friend FL_EXPORT void fl_begin_offscreen(Fl_Offscreen ctx);
  friend FL_EXPORT void fl_end_offscreen(void);
  friend FL_EXPORT void fl_delete_offscreen(Fl_Offscreen ctx);
private:
  class Fl_Image_Surface_Driver *platform_surface;
protected:
  void translate(int x, int y);
  void untranslate();
public:
  Fl_Image_Surface(int w, int h, int high_res = 0, Fl_Offscreen off = 0);
  ~Fl_Image_Surface();
  void set_current();
  Fl_RGB_Image *image();
  Fl_Shared_Image *highres_image();
  void origin(int *x, int *y);
  void origin(int x, int y);
  int printable_rect(int *w, int *h);
  Fl_Offscreen get_offscreen_before_delete();
  Fl_Offscreen offscreen();
};


/** A base class describing the interface between FLTK and draw-to-image operations.
 This class is only for internal use by the FLTK library.
 A supported platform should implement the virtual methods of this class
 in order to support drawing to an Fl_RGB_Image through class Fl_Image_Surface.
 */
class Fl_Image_Surface_Driver : public Fl_Widget_Surface {
  friend class Fl_Image_Surface;
protected:
  int width;
  int height;
  Fl_Offscreen offscreen;
  Fl_Image_Surface_Driver(int w, int h, int high_res, Fl_Offscreen off) : Fl_Widget_Surface(NULL), width(w), height(h), offscreen(off) {}
  virtual ~Fl_Image_Surface_Driver() {}
  virtual void set_current() {}
  virtual void translate(int x, int y) {}
  virtual void untranslate() {}
  int printable_rect(int *w, int *h) {*w = width; *h = height; return 0;}
  virtual Fl_RGB_Image *image() {return NULL;}
  /** Each platform implements this function its own way.
   It returns an object implementing all virtual functions
   of class Fl_Image_Surface_Driver for the plaform.
   */
  static Fl_Image_Surface_Driver *newImageSurfaceDriver(int w, int h, int high_res, Fl_Offscreen off);
};

#endif // Fl_Image_Surface_H

//
// End of "$Id$".
//
