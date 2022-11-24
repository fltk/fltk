//
// SVG image code for the Fast Light Tool Kit (FLTK).
//
// Copyright 2017-2022 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include <config.h>

#if defined(FLTK_USE_SVG) || defined(FL_DOXYGEN)

#include <FL/Fl_SVG_Image.H>
#include <FL/fl_utf8.h>
#include <FL/fl_draw.H>
#include <FL/fl_string_functions.h>
#include "Fl_Screen_Driver.H"
#include "Fl_System_Driver.H"
#include <stdio.h>
#include <stdlib.h>

#if !defined(HAVE_LONG_LONG)
static double strtoll(const char *str, char **endptr, int base) {
  return (double)strtol(str, endptr, base);
}
#endif

#ifdef _MSC_VER
#pragma warning (push)                  // Save #pragma warning status
#pragma warning (disable: 4244)         // Switch off conversion warnings
#endif

#define NANOSVG_ALL_COLOR_KEYWORDS      // Include full list of color keywords.
#define NANOSVG_IMPLEMENTATION          // Expands implementation
#include "../nanosvg/nanosvg.h"

#define NANOSVGRAST_IMPLEMENTATION      // Expands implementation
#include "../nanosvg/nanosvgrast.h"

#ifdef _MSC_VER
#pragma warning (pop)                  // Restore #pragma warning status
#endif

#if defined(HAVE_LIBZ)
#include <zlib.h>
#endif

/** The constructor loads the SVG image from the given .svg/.svgz filename or in-memory data.
 \param filename Name of a .svg or .svgz file, or NULL.
 \param svg_data A pointer to the memory location of the SVG image data.
 This parameter allows to load an SVG image from in-memory data, and is used when \p filename is NULL.
 \param length When 0, indicates that \p svg_data contains SVG text, otherwise \p svg_data is
  a  buffer of \p length  bytes containing GZ-compressed SVG data.
 \note In-memory SVG data is parsed by the object constructor and is not used after construction.
 When \p length > 0, parameter \p svg_data may safely be cast from data of type <em>const unsigned char *</em>.
 */
Fl_SVG_Image::Fl_SVG_Image(const char *filename, const char *svg_data, size_t length) : Fl_RGB_Image(NULL, 0, 0, 4) {
  init_(filename, (const unsigned char *)svg_data, NULL, length);
}


// private constructor
Fl_SVG_Image::Fl_SVG_Image(const Fl_SVG_Image *source) : Fl_RGB_Image(NULL, 0, 0, 4) {
  init_(NULL, NULL, source, 0);
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

#if defined(HAVE_LIBZ)

/* Implementation note about decompression of svgz file or in-memory data.
 It seems necessary to use the gzdopen()/gzread() API to inflate a gzip'ed
 file or byte buffer. Writing the in-memory gzip'ed data to an anonymous pipe
 and calling gzread() on the read end of this pipe is a solution for the in-memory case.
 But non-blocking write to the pipe is needed to do that in the main thread,
 and that seems impossible with Windows anonymous pipes.
 Therefore, the anonymous pipe is handled in 2 ways:
 1) Under Windows, a child thread writes to the write end of the pipe and
 the main thread reads from the read end with gzread().
 2) Under Posix systems, the write end of the pipe is made non-blocking
 with a fcntl() call, and the main thread successively writes to the write
 end and reads from the read end with gzread(). This allows to not have
 libfltk_images requiring a threading library.
 */

static char *svg_inflate(gzFile gzf, // can be a file or the read end of a pipe
                         size_t size, // size of compressed data or of file
                         bool is_compressed, // true when file or byte buffer is gzip'ed
                         int fdwrite, // write end of pipe if >= 0
                         const unsigned char *bytes // byte buffer to write to pipe
                         ) {
  size_t rest_bytes = size;
  int l;
  size_t out_size = is_compressed ? 3 * size + 1 : size + 1;
  char *out = (char*)malloc(out_size);
  char *p = out;
  do {
    if (is_compressed && p + size > out + out_size) {
      out_size += size;
      size_t delta = (p - out);
      out = (char*)realloc(out, out_size + 1);
      p = out + delta;
    }
    if ( fdwrite >= 0 && Fl::system_driver()->write_nonblocking_fd(fdwrite, bytes, rest_bytes) ) {
      free(out);
      out = NULL;
      is_compressed = false;
      break;
    }

    l = gzread(gzf, p, (unsigned int)size);
    if (l > 0) {
      p += l; *p = 0;
    }
  } while (is_compressed && l >0);
  gzclose(gzf);
  if (is_compressed) out = (char*)realloc(out, (p-out)+1);
  return out;
}

#endif // defined(HAVE_LIBZ)


void Fl_SVG_Image::init_(const char *filename, const unsigned char *in_filedata, const Fl_SVG_Image *copy_source, size_t length) {
  if (copy_source) {
    filename = NULL;
    in_filedata = NULL;
    counted_svg_image_ = copy_source->counted_svg_image_;
    counted_svg_image_->ref_count++;
  } else {
    counted_svg_image_ = new counted_NSVGimage;
    counted_svg_image_->svg_image = NULL;
    counted_svg_image_->ref_count = 1;
  }
  char *filedata = NULL;
  to_desaturate_ = false;
  average_weight_ = 1;
  proportional = true;
  bool is_compressed = true;

  if (filename || length) { // process file or byte buffer
#if defined(HAVE_LIBZ)
    int fdread, fdwrite = -1;
    if (length) { // process gzip'ed byte buffer
      // Pipe gzip'ed byte buffer into gzlib inflate algorithm.
      // Under Windows, gzip'ed byte buffer is written to pipe by child thread.
      // Under Posix, gzip'ed byte buffer is written to pipe by non-blocking write
      // done by main thread.
      Fl::system_driver()->pipe_support(fdread, fdwrite, in_filedata, length);
    } else { // read or decompress a .svg or .svgz file
      struct stat svg_file_stat;
      fl_stat(filename, &svg_file_stat); // get file size
      fdread = fl_open_ext(filename, 1, 0);
      // read a possibly gzip'ed file and return result as char string
      length = svg_file_stat.st_size;
      is_compressed = (strcmp(filename + strlen(filename) - 5, ".svgz") == 0);
    }
    gzFile gzf =  (fdread >= 0 ? gzdopen(fdread, "rb") : NULL);
    if (gzf) {
      filedata = svg_inflate(gzf, length, is_compressed, fdwrite, in_filedata);
    } else {
      if (fdread >= 0) Fl::system_driver()->close_fd(fdread);
      if (fdwrite >= 0) Fl::system_driver()->close_fd(fdwrite);
    }

#else // ! HAVE_LIBZ
    // without libz, read .svg file
    FILE *fp = fl_fopen(filename, "rb");
    if (fp) {
      fseek(fp, 0, SEEK_END);
      long size = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      filedata = (char*)malloc(size+1);
      if (filedata) {
        if (fread(filedata, 1, size, fp) == size) {
          filedata[size] = '\0';
        } else {
          free(filedata);
          filedata = NULL;
        }
      }
      fclose(fp);
    }
#endif // HAVE_LIBZ
    if (!filedata) ld(ERR_FILE_ACCESS);
  } else { // handle non-gzip'ed svg data as a char string
    // XXX: Make internal copy -- nsvgParse() modifies filedata during parsing (!)
    filedata = in_filedata ? fl_strdup((const char *)in_filedata) : NULL;
  }

  // filedata is NULL or contains SVG data as a char string
  if (filedata) {
    counted_svg_image_->svg_image = nsvgParse(filedata, "px", 96);
    free(filedata);     // made with svg_inflate|malloc|strdup
    if (counted_svg_image_->svg_image->width == 0 || counted_svg_image_->svg_image->height == 0) {
      d(-1);
      ld(ERR_FORMAT);
    } else {
      w(int(counted_svg_image_->svg_image->width + 0.5));
      h(int(counted_svg_image_->svg_image->height + 0.5));
    }
  } else if (copy_source) {
    w(copy_source->w());
    h(copy_source->h());
  }
  rasterized_ = false;
  raster_w_ = raster_h_ = 0;
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
  nsvgRasterizeXY(rasterizer, counted_svg_image_->svg_image, 0, 0, float(fx), float(fy), (uchar* )array, W, H, W*4);
  alloc_array = 1;
  data((const char * const *)&array, 1);
  d(4);
  if (to_desaturate_) Fl_RGB_Image::desaturate();
  if (average_weight_ < 1) Fl_RGB_Image::color_average(average_color_, average_weight_);
  rasterized_ = true;
  raster_w_ = W;
  raster_h_ = H;
}


Fl_Image *Fl_SVG_Image::copy(int W, int H) const {
  Fl_SVG_Image *svg2 = new Fl_SVG_Image(this);
  svg2->to_desaturate_ = to_desaturate_;
  svg2->average_weight_ = average_weight_;
  svg2->average_color_ = average_color_;
  svg2->proportional = proportional;
  svg2->w(W); svg2->h(H);
  return svg2;
}


/** Have the svg data (re-)rasterized using the given \p width and \p height values.
 By default, the resulting image w() and h() will be close to \p width and \p height
 while preserving the width/height ratio of the SVG data.
 If \ref proportional was set to \c false, the image is rasterized to the exact \c width
 and \c height values. In both cases, data_w() and data_h() values are set to w() and h(),
 respectively.
 */
void Fl_SVG_Image::resize(int width, int height) {
  if (ld() < 0 || width <= 0 || height <= 0) {
    return;
  }
  int w1 = width, h1 = height;
  if (proportional) {
    float f = svg_scaling_(width, height);
    w1 = int( counted_svg_image_->svg_image->width*f + 0.5 );
    h1 = int( counted_svg_image_->svg_image->height*f + 0.5 );
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


void Fl_SVG_Image::cache_size_(int &width, int &height) {
  if (proportional) {
    // Keep the rasterized image proportional to its source-level width and height
    // while maintaining it large enough to allow image tiling.
    float f = counted_svg_image_->svg_image->width / counted_svg_image_->svg_image->height;
    if (height * f >= width) width =  int(height * f + 0.5);
    else height = int(width/f + 0.5);
  }
}


void Fl_SVG_Image::draw(int X, int Y, int W, int H, int cx, int cy) {
  /* There may be several pixels per FLTK unit in an area
   of size w() x h() of the display. This occurs, e.g., with Apple retina displays
   and when the display is rescaled.
   The SVG is rasterized to the area dimension in pixels. The image is then drawn
   scaled to its size expressed in FLTK units. With this procedure,
   the SVG image is drawn using the full resolution of the display.
   */
  int w1 = w(), h1 = h();
  int f = fl_graphics_driver->has_feature(Fl_Graphics_Driver::PRINTER) ? 2 : 1;
  int w2 = f*w(), h2 = f*h();
  fl_graphics_driver->cache_size(this, w2, h2);
  resize(w2, h2);
  scale(w1, h1, 0, 1);
  Fl_RGB_Image::draw(X, Y, W, H, cx, cy);
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

/** Makes sure the object is fully initialized.
 This function rasterizes the SVG image, and consequently initializes its \ref array member, if that was not done before. */
void Fl_SVG_Image::normalize() {
  if (!array) resize(w(), h());
}

#endif // FLTK_USE_SVG
