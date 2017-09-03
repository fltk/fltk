//
// "$Id: Fl_SVG_Image.H 12239 2017-05-17 11:54:18Z manolo $"
//
// SVG Image header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2017 by Bill Spitzak and others.
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

#ifndef FL_SVG_IMAGE_H
#define FL_SVG_IMAGE_H

#include <FL/Fl_Image.H>

struct NSVGimage;

/** The Fl_SVG_Image class supports loading, caching and drawing of scalable vector graphics (SVG) images.
 The FLTK library performs parsing and rasterization of SVG data using a modified version 
 of the \c nanosvg software (https://github.com/memononen/nanosvg) © 2013-14 Mikko Mononen 
 (memon@inside.org). The software modification allows the option to change the image ratio 
 while performing rasterization.
 
 Use Fl_Image::fail() to check if the Fl_SVG_Image failed to load. fail() returns ERR_FILE_ACCESS 
 if the file could not be opened or read, and ERR_FORMAT if the SVG format could not be decoded. 
 If the image has loaded correctly, w(), h(), and d() should return values greater than zero.
 
 Rasterization is not done until the image is first drawn or resize() is called. Therefore, 
 array() is NULL until then. The delayed rasterization ensures an Fl_Shared_Image based on 
 an SVG image and scaled to its display size by calling Fl_Shared_Image::scale() will be 
 always rasterized to the exact screen resolution.
 
 The Fl_SVG_Image class draws images computed by \c nanosvg: one known limitation is that text
 within \c <text\></text\> blocks is not rendered.
 
 The FLTK library can optionally be built without SVG support; in that case, 
 class Fl_SVG_Image is unavailable.
 */
class FL_EXPORT Fl_SVG_Image : public Fl_RGB_Image {
private:
  typedef struct {
    NSVGimage* svg_image;
    int ref_count;
  } counted_NSVGimage;
  counted_NSVGimage* counted_svg_image_;
  bool rasterized_;
  int raster_w_, raster_h_;
  bool to_desaturate_;
  Fl_Color average_color_;
  float average_weight_;
  float svg_scaling_(int W, int H);
  void rasterize_(int W, int H);
  void init_(const char *filename, char *filedata, Fl_SVG_Image *copy_source);
  virtual int draw_scaled_(int X, int Y, int W, int H);
  Fl_SVG_Image(Fl_SVG_Image *source);
public:
  /** Set this to \c false to allow image re-scaling that alters the image aspect ratio.
   Upon object creation, \c proportional is set to \c true, and the aspect ratio is kept constant.*/
  bool proportional;
  Fl_SVG_Image(const char *filename, char *filedata = NULL);
  virtual ~Fl_SVG_Image();
  virtual Fl_Image *copy(int W, int H);
  void resize(int width, int height);
  virtual void desaturate();
  virtual void color_average(Fl_Color c, float i);
  virtual void draw(int X, int Y, int W, int H, int cx = 0, int cy = 0);
  void draw(int X, int Y) { draw(X, Y, w(), h(), 0, 0); }
};

#endif // FL_SVG_IMAGE_H

//
// End of "$Id: Fl_SVG_Image.H 12239 2017-05-17 11:54:18Z manolo $".
//
