//
// "$Id$"
//
// SVG image code for the Fast Light Tool Kit (FLTK).
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

#include <config.h>

#if defined(FLTK_USE_NANOSVG) || defined(FL_DOXYGEN)

#include <FL/Fl_SVG_Image.H>
#include <FL/fl_utf8.h>
#include <FL/fl_draw.H>
#include <FL/Fl_Screen_Driver.H>
#include <stdio.h>
#include <stdlib.h>

#if !defined(HAVE_LONG_LONG)
static double strtoll(const char *str, char **endptr, int base) {
  return (double)strtol(str, endptr, base);
}
#endif

#define NANOSVG_ALL_COLOR_KEYWORDS	// Include full list of color keywords.
#define NANOSVG_IMPLEMENTATION		// Expands implementation
#include "../nanosvg/nanosvg.h"

#define NANOSVGRAST_IMPLEMENTATION	// Expands implementation
#include "../nanosvg/altsvgrast.h"


/** The constructor loads the SVG image from the given .svg filename or in-memory data.
 \param filename A full path and name pointing to an SVG file, or NULL.
 \param filedata A pointer to the memory location of the SVG image data.
 This parameter allows to load an SVG image from in-memory data, and is used when \p filename is NULL.
 \note In-memory SVG data is modified by the object constructor and is no longer used after construction.
 */
Fl_SVG_Image::Fl_SVG_Image(const char *filename, char *filedata) : Fl_RGB_Image(NULL, 0, 0, 4) {
  init_(filename, filedata, NULL);
}


// private constructor
Fl_SVG_Image::Fl_SVG_Image(Fl_SVG_Image *source) : Fl_RGB_Image(NULL, 0, 0, 4) {
  init_(NULL, NULL, source);
}


/** The destructor frees all memory and server resources that are used by the SVG image. */
Fl_SVG_Image::~Fl_SVG_Image() {
  if ( --counted_svg_image_->ref_count <= 0) {
    nsvgDelete(counted_svg_image_->svg_image);
    delete counted_svg_image_;
  }
}


float Fl_SVG_Image::svg_scaling_(int W, int H) {
  float f1 = float(W) / int(counted_svg_image_->svg_image->width+0.5);
  float f2 = float(H) / int(counted_svg_image_->svg_image->height+0.5);
  return (f1 < f2) ? f1 : f2;
}


void Fl_SVG_Image::init_(const char *filename, char *filedata, Fl_SVG_Image *copy_source) {
  if (copy_source) {
    filename = filedata = NULL;
    counted_svg_image_ = copy_source->counted_svg_image_;
    counted_svg_image_->ref_count++;
  } else {
    counted_svg_image_ = new counted_NSVGimage;
    counted_svg_image_->svg_image = NULL;
    counted_svg_image_->ref_count = 1;
  }
  to_desaturate_ = false;
  average_weight_ = 1;
  proportional = true;
  if (filename) {
    filedata = NULL;
    FILE *fp = fl_fopen(filename, "rb");
    if (fp) {
      fseek(fp, 0, SEEK_END);
      size_t size = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      filedata = (char*)malloc(size+1);
      if (filedata) {
        if (fread(filedata, 1, size, fp) == size) filedata[size] = '\0';
        else {
          free(filedata);
          filedata = NULL;
        }
      }
      fclose(fp);
    } else ld(ERR_FILE_ACCESS);
  }
  if (filedata) {
    counted_svg_image_->svg_image = nsvgParse(filedata, "px", 96);
    if (filename) free(filedata);
    if (counted_svg_image_->svg_image->width == 0 || counted_svg_image_->svg_image->height == 0) {
      d(-1);
      ld(ERR_FORMAT);
    } else {
      w(counted_svg_image_->svg_image->width + 0.5);
      h(counted_svg_image_->svg_image->height + 0.5);
    }
  } else if (copy_source) {
    w(copy_source->w());
    h(copy_source->h());
  }
  rasterized_ = false;
}


void Fl_SVG_Image::rasterize_(int W, int H) {
  static NSVGrasterizer *rasterizer = nsvgCreateRasterizer();
  double fx, fy;
  if (proportional) {
    fx = svg_scaling_(W, H);
    fy = fx;
  } else {
    fx = (double)W / counted_svg_image_->svg_image->width;
    fy = (double)H / counted_svg_image_->svg_image->height;
  }
  array = new uchar[W*H*4];
  nsvgAltRasterize(rasterizer, counted_svg_image_->svg_image, 0, 0, fx, fy, (uchar* )array, W, H, W*4);
  alloc_array = 1;
  data((const char * const *)&array, 1);
  d(4);
  if (to_desaturate_) Fl_RGB_Image::desaturate();
  if (average_weight_ < 1) Fl_RGB_Image::color_average(average_color_, average_weight_);
  rasterized_ = true;
  raster_w_ = W;
  raster_h_ = H;
}


Fl_Image *Fl_SVG_Image::copy(int W, int H) {
  Fl_SVG_Image *svg2 = new Fl_SVG_Image(this);
  svg2->to_desaturate_ = to_desaturate_;
  svg2->average_weight_ = average_weight_;
  svg2->average_color_ = average_color_;
  svg2->proportional = proportional;
  svg2->w(W); svg2->h(H);
  return svg2;
}


/** Have the svg data (re-)rasterized using the given width and height values.
 By default, the resulting image w() and h() will preserve the width/height ratio
 of the SVG data.
 If \ref proportional was set to false, the image is rasterized to the given \c width
 and \c height values.*/
void Fl_SVG_Image::resize(int width, int height) {
  if (ld() < 0) {
    return;
  }
  int w1 = width, h1 = height;
  if (proportional) {
    float f = svg_scaling_(width, height);
    w1 = int( int(counted_svg_image_->svg_image->width+0.5)*f + 0.5 );
    h1 = int( int(counted_svg_image_->svg_image->height+0.5)*f + 0.5 );
  }
  w(w1); h(h1);
  if (rasterized_ && w1 == raster_w_ && h1 == raster_h_) return;
  if (array) {
    delete[] array;
    array = NULL;
  }
  uncache();
  rasterize_(w1, h1);
}


void Fl_SVG_Image::draw(int X, int Y, int W, int H, int cx, int cy) {
  static float f = Fl::screen_driver()->retina_factor();
  int w1 = w(), h1 = h();
  /* When f > 1, there may be several pixels per drawing unit in an area
   of size w() x h() of the display. This occurs, e.g., with Apple retina displays.
   The SVG is rasterized to the area dimension in pixels. The image is then drawn
   scaled to its size expressed in drawing units. With this procedure,
   the SVG image is drawn using the full resolution of the display.
   */
  resize(f*w(), f*h());
  if (f == 1) {
    Fl_RGB_Image::draw(X, Y, W, H, cx, cy);
  } else {
    bool need_clip = (cx || cy || W != w1 || H != h1);
    if (need_clip) fl_push_clip(X, Y, W, H);
    fl_graphics_driver->draw_scaled(this, X-cx, Y-cy, w1, h1);
    if (need_clip) fl_pop_clip();
  }
}


void Fl_SVG_Image::desaturate() {
  to_desaturate_ = true;
  Fl_RGB_Image::desaturate();
}


void Fl_SVG_Image::color_average(Fl_Color c, float i) {
  average_color_ = c;
  average_weight_ = i;
  Fl_RGB_Image::color_average(c, i);
}


int Fl_SVG_Image::draw_scaled(int X, int Y, int W, int H) {
  w(W);
  h(H);
  draw(X, Y, W, H, 0, 0);
  return 1;
}

#endif // FLTK_USE_NANOSVG


//
// End of "$Id$".
//
