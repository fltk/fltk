//
// SVG image code for the Fast Light Tool Kit (FLTK).
//
// Copyright 2017-2024 by Bill Spitzak and others.
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
#include <FL/Fl_Shared_Image.H>
#include <FL/fl_utf8.h>
#include <FL/fl_draw.H>
#include <FL/fl_string_functions.h>
#include "Fl_Screen_Driver.H"
#include "Fl_System_Driver.H"
#include <stdio.h>
#include <stdlib.h>

#include "../nanosvg/nanosvg.h"
#include "../nanosvg/nanosvgrast.h"

#if defined(HAVE_LIBZ)
#include <zlib.h>
#endif


/** Load an SVG image from a file.

 This constructor loads the SVG image from a .svg or .svgz file. The reader
 recognizes if the data is compressed, and decompresses it if zlib is available
 (HAVE_LIBZ).

 \param filename the filename for a .svg or .svgz file
 */
Fl_SVG_Image::Fl_SVG_Image(const char *filename) :
  Fl_RGB_Image(NULL, 0, 0, 4)
{
  init_(filename, NULL, 0);
}


/** Load an SVG image from memory.

 This constructor loads the SVG image from a block of memory. This version is
 commonly used for uncompressed text data, but the reader recognizes if the data
 is compressed, and decompresses it if zlib is available (HAVE_LIBZ).

 \param sharedname if not \c NULL, a shared image will be generated with this name
 \param svg_data a pointer to the memory location of the SVG image data

 \note In-memory SVG data is parsed by the object constructor and is no longer
 needed after construction.
 */
Fl_SVG_Image::Fl_SVG_Image(const char *sharedname, const char *svg_data) :
  Fl_RGB_Image(NULL, 0, 0, 4)
{
  init_(sharedname, (const unsigned char*)svg_data, 0);
}


/** Load an SVG image from memory.

 This constructor loads the SVG image from a block of memory. This version is
 commonly used for compressed binary data, but the reader recognizes if the data
 is uncompressed, and reads it as a text block.

 \param name if not \c NULL, a shared image will be generated with this name
 \param svg_data a pointer to the memory location of the SVG image data
 \param length of \p svg_data or \c 0 if the length is unknown. This will
        protect memory outside of the \p svg_data array from illegal read
        operations for compressed SVG data

 \note In-memory SVG data is parsed by the object constructor and is no longer
 needed after construction.
 */
Fl_SVG_Image::Fl_SVG_Image(const char *name, const unsigned char *svg_data, size_t length) :
  Fl_RGB_Image(NULL, 0, 0, 4)
{
  init_(name, svg_data, length);
}


// private constructor
Fl_SVG_Image::Fl_SVG_Image(const Fl_SVG_Image *source) :
  Fl_RGB_Image(NULL, 0, 0, 4)
{
  counted_svg_image_ = source->counted_svg_image_;
  counted_svg_image_->ref_count++;
  to_desaturate_ = false;
  average_weight_ = 1;
  proportional = true;
  w(source->w());
  h(source->h());
  rasterized_ = false;
  raster_w_ = raster_h_ = 0;
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

// Decompress gzip data in memory
#define CHUNK_SIZE (2048)
static int svg_inflate(uchar *src, size_t src_length, uchar *&dst, size_t &dst_length) {
  // allocate space for decompressed data in chunks
  typedef struct Chunk {
    Chunk() { next = NULL; }
    struct Chunk *next;
    uchar data[CHUNK_SIZE];
  } Chunk;
  Chunk *first = NULL;
  Chunk *chunk = NULL, *next_chunk;

  z_stream stream = { };
  int err = Z_OK;

  dst = 0;
  dst_length = 0;

  stream.next_in = (z_const Bytef *)src;
  stream.avail_in = 0;
  stream.zalloc = (alloc_func)0;
  stream.zfree = (free_func)0;
  stream.opaque = (voidpf)0;

  // initialize zlib for inflating compressed data
  err = inflateInit2(&stream, 31);
  if (err != Z_OK) return err;
  gz_header header = { };
  err = inflateGetHeader(&stream, &header);
  if (err != Z_OK) return err;

  stream.avail_out = 0;
  stream.avail_in = (uInt)(src_length ? src_length : -1);

  // inflate into as many chunks as needed
  do {
    if (stream.avail_out == 0) {
      next_chunk = new Chunk;
      if (!first) first = next_chunk; else chunk->next = next_chunk;
      chunk = next_chunk;
      stream.avail_out = CHUNK_SIZE;
      stream.next_out = chunk->data;
    }
    err = inflate(&stream, Z_NO_FLUSH);
  } while (err == Z_OK);

  inflateEnd(&stream);

  // copy chunk data into a new continuous data block
  if (err == Z_STREAM_END) {
    size_t nn = dst_length = stream.total_out;
    dst = (uchar*)malloc(dst_length+1); // leave room for a trailing NUL
    uchar *d = dst;
    chunk = first;
    while (chunk && nn>0) {
      size_t n = nn > CHUNK_SIZE ? CHUNK_SIZE : nn;
      memcpy(d, chunk->data, n);
      d += n;
      nn -= n;
      chunk = chunk->next;
    }
  }

  // delete all the chunks that we allocated
  chunk = first;
  while (chunk) {
    next_chunk = chunk->next;
    delete chunk;
    chunk = next_chunk;
  }

  return (err == Z_STREAM_END)
          ? Z_OK
          : (err == Z_NEED_DICT)
            ? Z_DATA_ERROR
            : ((err == Z_BUF_ERROR) && stream.avail_out)
              ? Z_DATA_ERROR
              : err;
}


#endif // defined(HAVE_LIBZ)


void Fl_SVG_Image::init_(const char *name, const unsigned char *in_data, size_t length) {
  counted_svg_image_ = new counted_NSVGimage;
  counted_svg_image_->svg_image = NULL;
  counted_svg_image_->ref_count = 1;
  to_desaturate_ = false;
  average_weight_ = 1;
  proportional = true;

  // yes, this is a const cast to avoid duplicating user supplied data
  uchar *data = const_cast<uchar*>(in_data); // 🤨 careful with this, don't overwrite user supplied data in nsvgParse()

  // this is to make it clear what we are doing
  const char *sharedname = data ? name : NULL;
  const char *filename = data ? NULL : name;

  // prepare with error data, so we can just return if an error occurs
  d(-1);
  ld(ERR_FORMAT);
  rasterized_ = false;
  raster_w_ = raster_h_ = 0;

  // if we are reading from a file, just read the entire file into a memory block
  if (!data) {
    FILE *f = fl_fopen(filename, "rb");
    if (f) {
      fseek(f, 0, SEEK_END);
      length = ftell(f);
      fseek(f, 0, SEEK_SET);
      data = (uchar*)malloc(length+1);
      if (data) {
        if (fread((void*)data, 1, length, f) == length) {
          data[length] = 0;
        } else {
          free((void*)data);
          data = NULL;
        }
      }
      fclose(f);
    }
    if (!data) return;
  }

  // now if our data is compressed, we use zlib to infalte it
  if (length==0 || length>10) {
    if (data[0] == 0x1f && data[1] == 0x8b) {
#if defined(HAVE_LIBZ)
      // this is gzip compressed data, so we decompress it and preplace the data array
      uchar *uncompressed_data = NULL;
      size_t uncompressed_data_length = 0;
      int err = svg_inflate(data, length, uncompressed_data, uncompressed_data_length);
      if (err == Z_OK) {
        // replace compressed data with uncompressed data
        if (in_data == NULL) free(data);
        length = (size_t)uncompressed_data_length;
        data = (uchar*)uncompressed_data;
        data[length] = 0;
      } else {
        if (in_data != data) free(data);
        return;
      }
#else
      if (in_data != data) free(data);
      return;
#endif // HAVE_LIBZ
    }
  }

  // now our SVG data should be in text format in `data`, terminated by a NUL
  // nsvgParse is destructive, so if in_data was set, we must duplicate the data first!
  if (in_data == data) {
    if (length) {
      data = (uchar*)malloc(length+1);
      memcpy(data, in_data, length);
      data[length] = 0;
    } else {
      data = (uchar*)fl_strdup((char*)in_data);
    }
  }
  counted_svg_image_->svg_image = nsvgParse((char*)data, "px", 96);
  if (in_data != data) free(data);
  if (counted_svg_image_->svg_image->width != 0 && counted_svg_image_->svg_image->height != 0) {
    w(int(counted_svg_image_->svg_image->width + 0.5));
    h(int(counted_svg_image_->svg_image->height + 0.5));
    d(4);
    ld(0);
  }

  if (sharedname && w() && h()) {
    Fl_Shared_Image *si = new Fl_Shared_Image(sharedname, this);
    si->add();
  }
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
