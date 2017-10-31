//
// "$Id$"
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
 \ref array is NULL until then. The delayed rasterization ensures an Fl_Shared_Image based on
 an SVG image and scaled to its display size by Fl_Shared_Image::scale() will be 
 always rasterized to the exact screen resolution.
 
 The Fl_SVG_Image class draws images computed by \c nanosvg: one known limitation is that text
 within \c <text\></text\> blocks is not rendered.
 
 The FLTK library can optionally be built without SVG support; in that case, 
 class Fl_SVG_Image is unavailable.

 Example of displaying a hard-coded svg file:
 \code
 #include <FL/Fl.H>
 #include <FL/Fl_Window.H>
 #include <FL/Fl_Box.H>
 #include <FL/Fl_SVG_Image.H>

 // A black rotated rectangle
 const char *svg_data = "<svg viewBox=\"0 0 200 200\" version = \"1.1\">\n"
                        "<rect x=\"25\" y=\"50\" width=\"150\" height=\"100\" fill=\"black\" "
                        "transform=\"rotate(45 100 100)\"> </svg>\n";

 int main(int argc, char **argv) {
     Fl_SVG_Image *svg = new Fl_SVG_Image(0, svg_data);       // create SVG object
     Fl_Window    *win = new Fl_Window(720, 486, "svg test");
     Fl_Box       *box = new Fl_Box(0,0,win->w(),win->h());
     box->image(svg);  // assign svg object to Fl_Box
     win->end();
     win->show(argc,argv);
     return(Fl::run());
 }
 \endcode

 Example of displaying an svg image from a file:
 \code
 #include <errno.h>   // errno
 #include <string.h>  // strerror
 #include <FL/Fl.H>
 #include <FL/Fl_Window.H>
 #include <FL/Fl_Box.H>
 #include <FL/Fl_SVG_Image.H>
 #include <FL/fl_message.H>
 int main(int argc, char **argv) {
     Fl_Window *win = new Fl_Window(720, 486, "svg test");
     Fl_Box    *box = new Fl_Box(0,0,win->w(),win->h());
 
     // Load svg image from disk, assign to a box
     const char *svgpath = "/var/tmp/simple.svg";
     Fl_SVG_Image *svg = new Fl_SVG_Image(svgpath);  // load SVG object from disk
     switch ( svg->fail() ) {
         case Fl_Image::ERR_FILE_ACCESS:
            // File couldn't load? show path + os error to user
            fl_alert("%s: %s", svgpath, strerror(errno));
            return 1;
         case Fl_Image::ERR_FORMAT:
            // Parsing error
            fl_alert("%s: couldn't decode image", svgpath);
            return 1;
     }
     box->image(svg);  // assign svg object to box
 
     win->end();
     win->show(argc,argv);
     return(Fl::run());
 }
 \endcode

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
  void init_(const char *filename, const char *filedata, Fl_SVG_Image *copy_source);
  Fl_SVG_Image(Fl_SVG_Image *source);
protected:
  virtual int draw_scaled(int X, int Y, int W, int H);
public:
  /** Set this to \c false to allow image re-scaling that alters the image aspect ratio.
   Upon object creation, \c proportional is set to \c true, and the aspect ratio is kept constant.*/
  bool proportional;
  Fl_SVG_Image(const char *filename, const char *filedata = NULL);
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
// End of "$Id$".
//
