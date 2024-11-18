//
// Image drawing code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
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
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Image.H>
#include "flstring.h"

#include <stdlib.h>

void fl_restore_clip(); // from fl_rect.cxx

//
// Base image class...
//

Fl_RGB_Scaling Fl_Image::RGB_scaling_ = FL_RGB_SCALING_NEAREST;

Fl_RGB_Scaling Fl_Image::scaling_algorithm_ = FL_RGB_SCALING_BILINEAR;

/**
 The constructor creates an empty image with the specified
 width, height, and depth. The width and height are in pixels.
 The depth is 0 for bitmaps, 1 for pixmap (colormap) images, and
 1 to 4 for color images.
 */
Fl_Image::Fl_Image(int W, int H, int D) :
  w_(W), h_(H), d_(D), ld_(0), count_(0), data_w_(W), data_h_(H), data_(0L)
{}

/**
  The destructor is a virtual method that frees all memory used
  by the image.
*/
Fl_Image::~Fl_Image() {
}

/**
  If the image has been cached for display, delete the cache
  data. This allows you to change the data used for the image and
  then redraw it without recreating an image object.
*/
void Fl_Image::uncache() {
}

void Fl_Image::draw(int XP, int YP, int, int, int, int) {
  draw_empty(XP, YP);
}

/**
  The protected method draw_empty() draws a box with
  an X in it. It can be used to draw any image that lacks image
  data.
*/
void Fl_Image::draw_empty(int X, int Y) {
  if (w() > 0 && h() > 0) {
    fl_color(FL_FOREGROUND_COLOR);
    fl_rect(X, Y, w(), h());
    fl_line(X, Y, X + w() - 1, Y + h() - 1);
    fl_line(X, Y + h() - 1, X + w() - 1, Y);
  }
}

/**
  Creates a resized copy of the image.

  It is recommended not to call this member function to reduce the size
  of an image to the size of the area where this image will be drawn,
  and to use Fl_Image::scale() instead.

  The new image should be released when you are done with it.

  Note: since FLTK 1.4.0 you can use Fl_Image::release() for all types
  of images (i.e. all subclasses of Fl_Image) instead of operator \em delete
  for Fl_Image's and Fl_Image::release() for Fl_Shared_Image's.

  The new image data will be converted to the requested size. RGB images
  are resized using the algorithm set by Fl_Image::RGB_scaling().

  For the new image the following equations are true:
  - w() == data_w() == \p W
  - h() == data_h() == \p H

 \param[in] W,H  Requested width and height of the new image

  \note The returned image can be safely cast to the same image type as that
  of the source image provided this type is one of Fl_RGB_Image, Fl_SVG_Image,
  Fl_Pixmap, Fl_Bitmap, Fl_Tiled_Image,  Fl_Anim_GIF_Image and Fl_Shared_Image.
  Returned objects copied from images of other, derived, image classes belong
  to the parent class appearing in this list. For example, the copy of an
  Fl_GIF_Image is an object of class Fl_Pixmap.

  \note Since FLTK 1.4.0 this method is 'const'. If you derive your own class
    from Fl_Image or any subclass your overridden methods of 'Fl_Image::copy() const'
    and 'Fl_Image::copy(int, int) const' \b must also be 'const' for inheritance
    to work properly. This is different than in FLTK 1.3.x and earlier where these
    methods have not been 'const'.
*/
Fl_Image *Fl_Image::copy(int W, int H) const {
  return new Fl_Image(W, H, d());
}

/**
  The color_average() method averages the colors in the image with
  the provided FLTK color value.

  The first argument specifies the FLTK color to be used.

  The second argument specifies the amount of the original image to combine
  with the color, so a value of 1.0 results in no color blend, and a value
  of 0.0 results in a constant image of the specified color.

  An internal copy is made of the original image data before changes are
  applied, to avoid modifying the original image data in memory.
*/
void Fl_Image::color_average(Fl_Color, float) {
}

/**
  The desaturate() method converts an image to grayscale.

  If the image contains an alpha channel (depth = 4),
  the alpha channel is preserved.

  An internal copy is made of the original image data before changes are
  applied, to avoid modifying the original image data in memory.
*/
void Fl_Image::desaturate() {
}

// Doxygen documentation in FL/Enumerations.H
Fl_Labeltype fl_define_FL_IMAGE_LABEL() {
  return Fl_Image::define_FL_IMAGE_LABEL();
}

Fl_Labeltype Fl_Image::define_FL_IMAGE_LABEL() {
  Fl::set_labeltype(_FL_IMAGE_LABEL, Fl_Image::labeltype, Fl_Image::measure);
  return _FL_IMAGE_LABEL;
}

/**
  This method is an obsolete way to set the image attribute of a widget
  or menu item.

  \deprecated Please use Fl_Widget::image() or Fl_Widget::deimage() instead.
*/
void Fl_Image::label(Fl_Widget* widget) {
  widget->image(this);
}

/**
  This method is an obsolete way to set the image attribute of a menu item.

  \deprecated Please use Fl_Menu_Item::image() instead.
*/
void Fl_Image::label(Fl_Menu_Item* m) {
  m->label(FL_IMAGE_LABEL, (const char*)this);
}

/**
  Returns a value that is not 0 if there is currently no image available.

  Example use:
  \code
    // [..]
      Fl_Box box(X, Y, W, H);
      Fl_JPEG_Image jpg("/tmp/foo.jpg");
      switch (jpg.fail()) {
        case Fl_Image::ERR_NO_IMAGE:
        case Fl_Image::ERR_FILE_ACCESS:
          fl_alert("/tmp/foo.jpg: %s", strerror(errno));    // shows actual os error to user
          exit(1);
        case Fl_Image::ERR_FORMAT:
          fl_alert("/tmp/foo.jpg: couldn't decode image");
          exit(1);
      }
      box.image(jpg);
  \endcode

  \returns                  Image load failure if non-zero
  \retval 0                 the image was loaded successfully
  \retval ERR_NO_IMAGE      no image was found
  \retval ERR_FILE_ACCESS   there was a file access related error (errno should be set)
  \retval ERR_FORMAT        image decoding failed
  \retval ERR_MEMORY_ACCESS image decoder tried to access memory outside of given memory block
*/
int Fl_Image::fail() const {
  // if no image exists, ld_ may contain a simple error code
  if ((w_ <= 0) || (h_ <= 0) || (d_ <= 0)) {
    if (ld_ == 0)
      return ERR_NO_IMAGE;
    else
      return ld_;
  }
  return 0;
}

void
Fl_Image::labeltype(const Fl_Label *lo,         // I - Label
                    int            lx,          // I - X position
                    int            ly,          // I - Y position
                    int            lw,          // I - Width of label
                    int            lh,          // I - Height of label
                    Fl_Align       la) {        // I - Alignment
  Fl_Image      *img;                           // Image pointer
  int           cx, cy;                         // Image position

  img = (Fl_Image *)(lo->value);

  if (la & FL_ALIGN_LEFT) cx = 0;
  else if (la & FL_ALIGN_RIGHT) cx = img->w() - lw;
  else cx = (img->w() - lw) / 2;

  if (la & FL_ALIGN_TOP) cy = 0;
  else if (la & FL_ALIGN_BOTTOM) cy = img->h() - lh;
  else cy = (img->h() - lh) / 2;

  fl_color((Fl_Color)lo->color);

  img->draw(lx, ly, lw, lh, cx, cy);
}

void
Fl_Image::measure(const Fl_Label *lo,           // I - Label
                  int            &lw,           // O - Width of image
                  int            &lh) {         // O - Height of image
  Fl_Image *img;                                // Image pointer

  img = (Fl_Image *)(lo->value);

  lw = img->w();
  lh = img->h();
}

/** Sets the RGB image scaling method used for copy(int, int).
    Applies to all RGB images, defaults to FL_RGB_SCALING_NEAREST.
*/
void Fl_Image::RGB_scaling(Fl_RGB_Scaling method) {
  RGB_scaling_ = method;
}

/** Returns the currently used RGB image scaling method. */
Fl_RGB_Scaling Fl_Image::RGB_scaling() {
  return RGB_scaling_;
}

/** Sets the drawing size of the image.
 This function controls the values returned by member functions w() and h()
 which in turn control how the image is drawn: the full image data (whose size
 is given by data_w() and data_h()) are drawn scaled
 to an area of the drawing surface sized at w() x h() FLTK units.
 This can make a difference if the drawing surface has more than 1 pixel per
 FLTK unit because the image can be drawn at the full resolution of the drawing surface.
 Examples of such drawing surfaces: HiDPI displays, laser printers, PostScript files, PDF printers.

 \param width,height   maximum values, in FLTK units, that w() and h() should return
 \param proportional   if not null, keep the values returned by w() and h() proportional to
 data_w() and data_h()
 \param can_expand  if null, the values returned by w() and h() will not be larger than
 data_w() and data_h(), respectively
 \note This function generally changes the values returned by the w() and h() member functions.
 In contrast, the values returned by data_w() and data_h() remain unchanged.
 \version 1.4 (1.3.4 and FL_ABI_VERSION for Fl_Shared_Image only)

 Example code: scale an image to fit in a box
 \code
 Fl_Box *b = ...  // a box
 Fl_Image *img = new Fl_PNG_Image("/path/to/picture.png"); // read a picture file
 // set the drawing size of the image to the size of the box keeping its aspect ratio
 img->scale(b->w(), b->h());
 b->image(img); // use the image as the box image
 \endcode
 */
void Fl_Image::scale(int width, int height, int proportional, int can_expand)
{
  if ((width <= data_w() && height <= data_h()) || can_expand) {
    w_ = width;
    h_ = height;
  }
  if (fail()) return;
  if (!proportional && can_expand) return;
  if (!proportional && width <= data_w() && height <= data_h()) return;
  float fw = data_w() / float(width);
  float fh = data_h() / float(height);
  if (proportional) {
    if (fh > fw) fw = fh;
    else fh = fw;
  }
  if (!can_expand) {
    if (fw < 1) fw = 1;
    if (fh < 1) fh = 1;
  }
  w_ = int((data_w() / fw) + 0.5);
  h_ = int((data_h() / fh) + 0.5);
}

/** Draw the image to the current drawing surface rescaled to a given width and height.
 Intended for internal use by the FLTK library.
 \param X,Y position of the image's top-left
 \param W,H width and height for the drawn image
 \return 1
 \deprecated Only for API compatibility with FLTK 1.3.4.
 */
int Fl_Image::draw_scaled(int X, int Y, int W, int H) {
  // transiently set image drawing size to WxH
  int width = w(), height = h();
  scale(W, H, 0, 1);
  draw(X, Y, W, H, 0, 0);
  scale(width, height, 0, 1);
  return 1;
}

/** True after fl_register_images() was called, false before */
bool Fl_Image::register_images_done = false;

//
// RGB image class...
//
size_t Fl_RGB_Image::max_size_ = ~((size_t)0);

int fl_convert_pixmap(const char*const* cdata, uchar* out, Fl_Color bg);


/**
  The constructor creates a new image from the specified data.

  The data array \p bits must contain sufficient data to provide
  \p W * \p H * \p D image bytes and optional line padding, see \p LD.

  \p W and \p H are the width and height of the image in pixels, resp.

  \p D is the image depth and can be:
    - D=1: each uchar in \p bits[] is a grayscale pixel value
    - D=2: each uchar pair in \p bits[] is a grayscale + alpha pixel value
    - D=3: each uchar triplet in \p bits[] is an R/G/B pixel value
    - D=4: each uchar quad in \p bits[] is an R/G/B/A pixel value

  \p LD specifies the line data size of the array, see Fl_Image::ld(int).
  If \p LD is zero, then \p W * \p D is assumed, otherwise \p LD must be
  greater than or equal to \p W * \p D to account for (unused) extra data
  per line (padding).

  The caller is responsible that the image data array \p bits persists as
  long as the image is used.

  This constructor sets Fl_RGB_Image::alloc_array to 0.
  To have the image object control the deallocation of the data array
  \p bits, set alloc_array to non-zero after construction.

  \param[in] bits   The image data array.
  \param[in] W      The width of the image in pixels.
  \param[in] H      The height of the image in pixels.
  \param[in] D      The image depth, or 'number of channels' (default=3).
  \param[in] LD     Line data size (default=0).

  \see Fl_Image::data(), Fl_Image::w(), Fl_Image::h(), Fl_Image::d(), Fl_Image::ld(int)
*/
Fl_RGB_Image::Fl_RGB_Image(const uchar *bits, int W, int H, int D, int LD) :
  Fl_Image(W,H,D),
  array(bits),
  alloc_array(0),
  id_(0),
  mask_(0),
  cache_w_(0), cache_h_(0)
{
    data((const char **)&array, 1);
    ld(LD);
}


/**
 The constructor creates a new image from the specified data.

 If the provided array is too small to contain all the image data, the
 constructor will not generate the image to avoid illegal memory read
 access and instead set \c data to NULL and \c ld to \c ERR_MEMORY_ACCESS.

 \param bits image data
 \param bits_length length of the \p bits array in bytes
 \param W image width in pixels
 \param H image height in pixels
 \param D image depth in bytes, 1 for gray scale, 2 for gray with alpha,
        3 for RGB, and 4 for RGB plus alpha
 \param LD line length in bytes, or 0 to use W*D.

 \see Fl_RGB_Image(const uchar *bits, int W, int H, int D, int LD)
 */
Fl_RGB_Image::Fl_RGB_Image(const uchar *bits, int bits_length, int W, int H, int D, int LD) :
  Fl_Image(W,H,D),
  array(bits),
  alloc_array(0),
  id_(0),
  mask_(0),
  cache_w_(0), cache_h_(0)
{
  if (D == 0) D = 3;
  if (LD == 0) LD = W*D;
  int min_length = LD*(H-1) + W*D;
  if (bits_length >= min_length) {
    data((const char **)&array, 1);
    ld(LD);
  } else {
    array = NULL;
    data(NULL, 0);
    ld(ERR_MEMORY_ACCESS);
  }
}


/**
  The constructor creates a new RGBA image from the specified Fl_Pixmap.

  The RGBA image is built fully opaque except for the transparent area
  of the pixmap that is assigned the \p bg color with full transparency.

  This constructor creates a new internal data array and sets
  Fl_RGB_Image::alloc_array to 1 so the data array is deleted when the
  image is destroyed.
*/
Fl_RGB_Image::Fl_RGB_Image(const Fl_Pixmap *pxm, Fl_Color bg):
  Fl_Image(pxm->data_w(), pxm->data_h(), 4),
  array(0),
  alloc_array(0),
  id_(0),
  mask_(0),
  cache_w_(0), cache_h_(0)
{
  if (pxm && pxm->data_w() > 0 && pxm->data_h() > 0) {
    array = new uchar[data_w() * data_h() * d()];
    alloc_array = 1;
    fl_convert_pixmap(pxm->data(), (uchar*)array, bg);
  }
  data((const char **)&array, 1);
  scale(pxm->w(), pxm->h(), 0, 1);
}


/**
  The destructor frees all memory and server resources that are used by
  the image.
*/
Fl_RGB_Image::~Fl_RGB_Image() {
  uncache();
  if (alloc_array) delete[] (uchar *)array;
}

void Fl_RGB_Image::uncache() {
  Fl_Graphics_Driver::default_driver().uncache(this, id_, mask_);
}

Fl_Image *Fl_RGB_Image::copy(int W, int H) const {
  Fl_RGB_Image  *new_image;     // New RGB image
  uchar         *new_array;     // New array for image data

  // Optimize the simple copy where the width and height are the same,
  // or when we are copying an empty image...
  if ((W == data_w() && H == data_h()) ||
      !w() || !h() || !d() || !array) {
    if (array) {
      // Make a copy of the image data and return a new Fl_RGB_Image...
      new_array = new uchar[W * H * d()];
      if (ld() && (ld() != W  *d())) {
        const uchar *src = array;
        uchar *dst = new_array;
        int dy, dh = H, wd = W*d(), wld = ld();
        for (dy=0; dy<dh; dy++) {
          memcpy(dst, src, wd);
          src += wld;
          dst += wd;
        }
      } else {
        memcpy(new_array, array, W * H * d());
      }
      new_image = new Fl_RGB_Image(new_array, W, H, d());
      new_image->alloc_array = 1;
    } else {
      new_image = new Fl_RGB_Image(array, W, H, d(), ld());
    }
    return new_image;
  }
  if (W <= 0 || H <= 0) return 0;

  // OK, need to resize the image data; allocate memory and create new image
  uchar         *new_ptr;       // Pointer into new array
  const uchar   *old_ptr;       // Pointer into old array
  int           dx, dy,         // Destination coordinates
                line_d;         // stride from line to line

  // Allocate memory for the new image...
  new_array = new uchar [W * H * d()];
  new_image = new Fl_RGB_Image(new_array, W, H, d());
  new_image->alloc_array = 1;

  line_d = ld() ? ld() : data_w() * d();

  if (Fl_Image::RGB_scaling() == FL_RGB_SCALING_NEAREST) {

    int         c,              // Channel number
                sy,             // Source coordinate
                xerr, yerr,     // X & Y errors
                xmod, ymod,     // X & Y moduli
                xstep, ystep;   // X & Y step increments

    // Figure out Bresenham step/modulus values...
    xmod   = data_w() % W;
    xstep  = (data_w() / W) * d();
    ymod   = data_h() % H;
    ystep  = data_h() / H;

    // Scale the image using a nearest-neighbor algorithm...
    for (dy = H, sy = 0, yerr = H, new_ptr = new_array; dy > 0; dy --) {
      for (dx = W, xerr = W, old_ptr = array + sy * line_d; dx > 0; dx --) {
        for (c = 0; c < d(); c ++) *new_ptr++ = old_ptr[c];

        old_ptr += xstep;
        xerr    -= xmod;

        if (xerr <= 0) {
          xerr    += W;
          old_ptr += d();
        }
      }

      sy   += ystep;
      yerr -= ymod;
      if (yerr <= 0) {
        yerr += H;
        sy ++;
      }
    }
  } else {
    // Bilinear scaling (FL_RGB_SCALING_BILINEAR)
    const float xscale = (data_w() - 1) / (float) W;
    const float yscale = (data_h() - 1) / (float) H;
    for (dy = 0; dy < H; dy++) {
      float oldy = dy * yscale;
      if (oldy >= data_h())
        oldy = float(data_h() - 1);
      const float yfract = oldy - (unsigned) oldy;

      for (dx = 0; dx < W; dx++) {
        new_ptr = new_array + dy * W * d() + dx * d();

        float oldx = dx * xscale;
        if (oldx >= data_w())
          oldx = float(data_w() - 1);
        const float xfract = oldx - (unsigned) oldx;

        const unsigned leftx = (unsigned)oldx;
        const unsigned lefty = (unsigned)oldy;
        const unsigned rightx = (unsigned)(oldx + 1 >= data_w() ? oldx : oldx + 1);
        const unsigned righty = (unsigned)oldy;
        const unsigned dleftx = (unsigned)oldx;
        const unsigned dlefty = (unsigned)(oldy + 1 >= data_h() ? oldy : oldy + 1);
        const unsigned drightx = (unsigned)rightx;
        const unsigned drighty = (unsigned)dlefty;

        uchar left[4], right[4], downleft[4], downright[4];
        memcpy(left, array + lefty * line_d + leftx * d(), d());
        memcpy(right, array + righty * line_d + rightx * d(), d());
        memcpy(downleft, array + dlefty * line_d + dleftx * d(), d());
        memcpy(downright, array + drighty * line_d + drightx * d(), d());

        int i;
        if (d() == 4) {
          for (i = 0; i < 3; i++) {
            left[i] = (uchar)(left[i] * left[3] / 255.0f);
            right[i] = (uchar)(right[i] * right[3] / 255.0f);
            downleft[i] = (uchar)(downleft[i] * downleft[3] / 255.0f);
            downright[i] = (uchar)(downright[i] * downright[3] / 255.0f);
          }
        }

        const float leftf = 1 - xfract;
        const float rightf = xfract;
        const float upf = 1 - yfract;
        const float downf = yfract;

        for (i = 0; i < d(); i++) {
          new_ptr[i] = (uchar)((left[i] * leftf +
                   right[i] * rightf) * upf +
                   (downleft[i] * leftf +
                   downright[i] * rightf) * downf);
        }

        if (d() == 4 && new_ptr[3]) {
          for (i = 0; i < 3; i++) {
            new_ptr[i] = (uchar)(new_ptr[i] / (new_ptr[3] / 255.0f));
          }
        }
      }
    }
  }

  return new_image;
}

void Fl_RGB_Image::color_average(Fl_Color c, float i) {
  // Don't average an empty image...
  if (!w() || !h() || !d() || !array) return;

  // Delete any existing pixmap/mask objects...
  uncache();

  // Allocate memory as needed...
  uchar         *new_array,
                *new_ptr;

  if (!alloc_array) new_array = new uchar[data_h() * data_w() * d()];
  else new_array = (uchar *)array;

  // Get the color to blend with...
  uchar         r, g, b;
  unsigned      ia, ir, ig, ib;

  Fl::get_color(c, r, g, b);
  if (i < 0.0f) i = 0.0f;
  else if (i > 1.0f) i = 1.0f;

  ia = (unsigned)(256 * i);
  ir = r * (256 - ia);
  ig = g * (256 - ia);
  ib = b * (256 - ia);

  // Update the image data to do the blend...
  const uchar   *old_ptr;
  int           x, y;
  int   line_i = ld() ? ld() - (data_w()*d()) : 0; // increment from line end to beginning of next line

  if (d() < 3) {
    ig = (r * 31 + g * 61 + b * 8) / 100 * (256 - ia);

    for (new_ptr = new_array, old_ptr = array, y = 0; y < data_h(); y ++, old_ptr += line_i)
      for (x = 0; x < data_w(); x ++) {
        *new_ptr++ = (*old_ptr++ * ia + ig) >> 8;
        if (d() > 1) *new_ptr++ = *old_ptr++;
      }
  } else {
    for (new_ptr = new_array, old_ptr = array, y = 0; y < data_h(); y ++, old_ptr += line_i)
      for (x = 0; x < data_w(); x ++) {
        *new_ptr++ = (*old_ptr++ * ia + ir) >> 8;
        *new_ptr++ = (*old_ptr++ * ia + ig) >> 8;
        *new_ptr++ = (*old_ptr++ * ia + ib) >> 8;
        if (d() > 3) *new_ptr++ = *old_ptr++;
      }
  }

  // Set the new pointers/values as needed...
  if (!alloc_array) {
    array       = new_array;
    alloc_array = 1;

    ld(0);
  }
}

void Fl_RGB_Image::desaturate() {
  // Don't desaturate an empty image...
  if (!w() || !h() || !d() || !array) return;

  // Can only desaturate color images...
  if (d() < 3) return;

  // Delete any existing pixmap/mask objects...
  uncache();

  // Allocate memory for a grayscale image...
  uchar         *new_array,
                *new_ptr;
  int           new_d;

  new_d     = d() - 2;
  new_array = new uchar[data_h() * data_w() * new_d];

  // Copy the image data, converting to grayscale...
  const uchar   *old_ptr;
  int           x, y;
  int   line_i = ld() ? ld() - (data_w()*d()) : 0; // increment from line end to beginning of next line

  for (new_ptr = new_array, old_ptr = array, y = 0; y < data_h(); y ++, old_ptr += line_i)
    for (x = 0; x < data_w(); x ++, old_ptr += d()) {
      *new_ptr++ = (uchar)((31 * old_ptr[0] + 61 * old_ptr[1] + 8 * old_ptr[2]) / 100);
      if (d() > 3) *new_ptr++ = old_ptr[3];
    }

  // Free the old array as needed, and then set the new pointers/values...
  if (alloc_array) delete[] (uchar *)array;

  array       = new_array;
  alloc_array = 1;

  ld(0);
  d(new_d);
}

#define fl_max(a,b) ((a) > (b) ? (a) : (b))
#define fl_min(a,b) ((a) < (b) ? (a) : (b))

typedef struct {int x; int y; int width; int height;} rectangle_int_t;
static void crect_intersect(rectangle_int_t *to, rectangle_int_t *with) {
  int x = fl_max(to->x, with->x);
  to->width = fl_min(to->x + to->width, with->x + with->width) - x;
  if (to->width < 0) to->width = 0;
  int y = fl_max(to->y, with->y);
  to->height = fl_min(to->y + to->height, with->y + with->height) - y;
  if (to->height < 0) to->height = 0;
  to->x = x;
  to->y = y;
}


void Fl_RGB_Image::draw(int XP, int YP, int WP, int HP, int cx, int cy) {
  float s = fl_graphics_driver->scale();
  if (s != int(s) && (cx || cy || WP != w() || HP != h())) {
    // See issue #1128: clipping to a part of the image while the scaling
    // has a fractional value creates problems
    rectangle_int_t r1 = { XP-cx, YP-cy, w(), h() };
    rectangle_int_t r2 = { XP, YP, WP, HP };
    crect_intersect(&r1, &r2);
    // After this, r1.x,r1.y = position of top-left of drawn image part;
    // r1.width,r1.height = size of drawn image part, in FLTK units;
    // fl_max(cx, 0),fl_max(cy, 0) = top-left of drawn part in image.
    int l = (ld() ? ld() : d() * w());
    const uchar *p = array + fl_max(cy, 0) * l + fl_max(cx, 0) * d();
    fl_graphics_driver->draw_image(p, XP, YP, WP, HP, d(), l);
  } else
    fl_graphics_driver->draw_rgb(this, XP, YP, WP, HP, cx, cy);
}

void Fl_RGB_Image::label(Fl_Widget* widget) {
  widget->image(this);
}

void Fl_RGB_Image::label(Fl_Menu_Item* m) {
  m->label(FL_IMAGE_LABEL, (const char*)this);
}
