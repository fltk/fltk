//
// "$Id$"
//
// Image drawing code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Printer.H>
#include "flstring.h"

#ifdef WIN32
void fl_release_dc(HWND, HDC); // from Fl_win32.cxx
#endif

void fl_restore_clip(); // from fl_rect.cxx

//
// Base image class...
//

Fl_RGB_Scaling Fl_Image::RGB_scaling_ = FL_RGB_SCALING_NEAREST;


/**
 The constructor creates an empty image with the specified
 width, height, and depth. The width and height are in pixels.
 The depth is 0 for bitmaps, 1 for pixmap (colormap) images, and
 1 to 4 for color images.
 */
Fl_Image::Fl_Image(int W, int H, int D) :
  w_(W), h_(H), d_(D), ld_(0), count_(0), data_(0L)
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
  The copy() method creates a copy of the specified
  image. If the width and height are provided, the image is
  resized to the specified size. The image should be deleted (or in
  the case of Fl_Shared_Image, released) when you are done
  with it.
*/
Fl_Image *Fl_Image::copy(int W, int H) {
  return new Fl_Image(W, H, d());
}

/**
  The color_average() method averages the colors in
  the image with the FLTK color value c. The i
  argument specifies the amount of the original image to combine
  with the color, so a value of 1.0 results in no color blend, and
  a value of 0.0 results in a constant image of the specified
  color. 

  An internal copy is made of the original image before
  changes are applied, to avoid modifying the original image.
*/
void Fl_Image::color_average(Fl_Color, float) {
}

/**
  The desaturate() method converts an image to
  grayscale. If the image contains an alpha channel (depth = 4),
  the alpha channel is preserved.
  
  An internal copy is made of the original image before
  changes are applied, to avoid modifying the original image.
*/
void Fl_Image::desaturate() {
}

/**
  The label() methods are an obsolete way to set the
  image attribute of a widget or menu item. Use the
  image() or deimage() methods of the
  Fl_Widget and Fl_Menu_Item classes
  instead.
*/
void Fl_Image::label(Fl_Widget* widget) {
  widget->image(this);
}

/**
  The label() methods are an obsolete way to set the
  image attribute of a widget or menu item. Use the
  image() or deimage() methods of the
  Fl_Widget and Fl_Menu_Item classes
  instead.
*/
void Fl_Image::label(Fl_Menu_Item* m) {
  Fl::set_labeltype(_FL_IMAGE_LABEL, labeltype, measure);
  m->label(_FL_IMAGE_LABEL, (const char*)this);
}

/**
 Returns a value that is not 0 if there is currently no image
 available.

 Example use:
 \code
    [..]
    Fl_Box box(X,Y,W,H);
    Fl_JPEG_Image jpg("/tmp/foo.jpg");
    switch ( jpg.fail() ) {
        case Fl_Image::ERR_NO_IMAGE:
        case Fl_Image::ERR_FILE_ACCESS:
            fl_alert("/tmp/foo.jpg: %s", strerror(errno));    // shows actual os error to user
            exit(1);
        case Fl_Image::ERR_FORMAT:
            fl_alert("/tmp/foo.jpg: couldn't decode image");
            exit(1);
    }
    box.image(jpg);
    [..]
 \endcode

 \return ERR_NO_IMAGE if no image was found
 \return ERR_FILE_ACCESS if there was a file access related error (errno should be set)
 \return ERR_FORMAT if image decoding failed.
 */
int Fl_Image::fail()
{
    // if no image exists, ld_ may contain a simple error code
    if ( (w_<=0) || (h_<=0) || (d_<=0) ) {
        if (ld_==0)
            return ERR_NO_IMAGE;
        else
            return ld_;
    }
    return 0;
}

void
Fl_Image::labeltype(const Fl_Label *lo,		// I - Label
                    int            lx,		// I - X position
		    int            ly,		// I - Y position
		    int            lw,		// I - Width of label
		    int            lh,		// I - Height of label
		    Fl_Align       la) {	// I - Alignment
  Fl_Image	*img;				// Image pointer
  int		cx, cy;				// Image position

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
Fl_Image::measure(const Fl_Label *lo,		// I - Label
                  int            &lw,		// O - Width of image
		  int            &lh) {		// O - Height of image
  Fl_Image *img;				// Image pointer

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
  mask_(0)
{
    data((const char **)&array, 1);
    ld(LD);
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
  Fl_Image(pxm->w(), pxm->h(), 4),
  array(0),
  alloc_array(0),
  id_(0),
  mask_(0)
{
  if (pxm && pxm->w() > 0 && pxm->h() > 0) {
    array = new uchar[w() * h() * d()];
    alloc_array = 1;
    fl_convert_pixmap(pxm->data(), (uchar*)array, bg);
  }
  data((const char **)&array, 1);
}


/**
  The destructor frees all memory and server resources that are used by
  the image.
*/
Fl_RGB_Image::~Fl_RGB_Image() {
#ifdef __APPLE__
  if (id_) CGImageRelease((CGImageRef)id_);
  else if (alloc_array) delete[] (uchar *)array;
#else
  uncache();
  if (alloc_array) delete[] (uchar *)array;
#endif
}

void Fl_RGB_Image::uncache() {
#ifdef __APPLE__
  if (id_) {
    if (mask_) *(bool*)mask_ = false;
    CGImageRelease((CGImageRef)id_);
    id_ = 0;
    mask_ = NULL;
  }
#else
  if (id_) {
    fl_delete_offscreen((Fl_Offscreen)id_);
    id_ = 0;
  }

  if (mask_) {
    fl_delete_bitmask((Fl_Bitmask)mask_);
    mask_ = 0;
  }
#endif
}

Fl_Image *Fl_RGB_Image::copy(int W, int H) {
  Fl_RGB_Image	*new_image;	// New RGB image
  uchar		*new_array;	// New array for image data

  // Optimize the simple copy where the width and height are the same,
  // or when we are copying an empty image...
  if ((W == w() && H == h()) ||
      !w() || !h() || !d() || !array) {
    if (array) {
      // Make a copy of the image data and return a new Fl_RGB_Image...
      new_array = new uchar[w() * h() * d()];
      if (ld() && ld()!=w()*d()) {
        const uchar *src = array;
        uchar *dst = new_array;
        int dy, dh = h(), wd = w()*d(), wld = ld();
        for (dy=0; dy<dh; dy++) {
          memcpy(dst, src, wd);
          src += wld;
          dst += wd;
        }
      } else {
        memcpy(new_array, array, w() * h() * d());
      }
      new_image = new Fl_RGB_Image(new_array, w(), h(), d());
      new_image->alloc_array = 1;

      return new_image;
    } else {
      return new Fl_RGB_Image(array, w(), h(), d(), ld());
    }
  }
  if (W <= 0 || H <= 0) return 0;

  // OK, need to resize the image data; allocate memory and create new image
  uchar		*new_ptr;	// Pointer into new array
  const uchar	*old_ptr;	// Pointer into old array
  int		dx, dy,		// Destination coordinates
		line_d;		// stride from line to line

  // Allocate memory for the new image...
  new_array = new uchar [W * H * d()];
  new_image = new Fl_RGB_Image(new_array, W, H, d());
  new_image->alloc_array = 1;

  line_d = ld() ? ld() : w() * d();

  if (Fl_Image::RGB_scaling() == FL_RGB_SCALING_NEAREST) {

    int		c,		// Channel number
		sy,		// Source coordinate
		xerr, yerr,	// X & Y errors
		xmod, ymod,	// X & Y moduli
		xstep, ystep;	// X & Y step increments

    // Figure out Bresenham step/modulus values...
    xmod   = w() % W;
    xstep  = (w() / W) * d();
    ymod   = h() % H;
    ystep  = h() / H;

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
    const float xscale = (w() - 1) / (float) W;
    const float yscale = (h() - 1) / (float) H;
    for (dy = 0; dy < H; dy++) {
      float oldy = dy * yscale;
      if (oldy >= h())
        oldy = float(h() - 1);
      const float yfract = oldy - (unsigned) oldy;

      for (dx = 0; dx < W; dx++) {
        new_ptr = new_array + dy * W * d() + dx * d();

        float oldx = dx * xscale;
        if (oldx >= w())
          oldx = float(w() - 1);
        const float xfract = oldx - (unsigned) oldx;

        const unsigned leftx = (unsigned)oldx;
        const unsigned lefty = (unsigned)oldy;
        const unsigned rightx = (unsigned)(oldx + 1 >= w() ? oldx : oldx + 1);
        const unsigned righty = (unsigned)oldy;
        const unsigned dleftx = (unsigned)oldx;
        const unsigned dlefty = (unsigned)(oldy + 1 >= h() ? oldy : oldy + 1);
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
  uchar		*new_array,
		*new_ptr;

  if (!alloc_array) new_array = new uchar[h() * w() * d()];
  else new_array = (uchar *)array;

  // Get the color to blend with...
  uchar		r, g, b;
  unsigned	ia, ir, ig, ib;

  Fl::get_color(c, r, g, b);
  if (i < 0.0f) i = 0.0f;
  else if (i > 1.0f) i = 1.0f;

  ia = (unsigned)(256 * i);
  ir = r * (256 - ia);
  ig = g * (256 - ia);
  ib = b * (256 - ia);

  // Update the image data to do the blend...
  const uchar	*old_ptr;
  int		x, y;
  int   line_i = ld() ? ld() - (w()*d()) : 0; // increment from line end to beginning of next line

  if (d() < 3) {
    ig = (r * 31 + g * 61 + b * 8) / 100 * (256 - ia);

    for (new_ptr = new_array, old_ptr = array, y = 0; y < h(); y ++, old_ptr += line_i)
      for (x = 0; x < w(); x ++) {
	*new_ptr++ = (*old_ptr++ * ia + ig) >> 8;
	if (d() > 1) *new_ptr++ = *old_ptr++;
      }
  } else {
    for (new_ptr = new_array, old_ptr = array, y = 0; y < h(); y ++, old_ptr += line_i)
      for (x = 0; x < w(); x ++) {
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
  uchar		*new_array,
		*new_ptr;
  int		new_d;

  new_d     = d() - 2;
  new_array = new uchar[h() * w() * new_d];

  // Copy the image data, converting to grayscale...
  const uchar	*old_ptr;
  int		x, y;
  int   line_i = ld() ? ld() - (w()*d()) : 0; // increment from line end to beginning of next line

  for (new_ptr = new_array, old_ptr = array, y = 0; y < h(); y ++, old_ptr += line_i)
    for (x = 0; x < w(); x ++, old_ptr += d()) {
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

#if !defined(WIN32) && !defined(__APPLE__)
// Composite an image with alpha on systems that don't have accelerated
// alpha compositing...
static void alpha_blend(Fl_RGB_Image *img, int X, int Y, int W, int H, int cx, int cy) {
  int ld = img->ld();
  if (ld == 0) ld = img->w() * img->d();
  uchar *srcptr = (uchar*)img->array + cy * ld + cx * img->d();
  int srcskip = ld - img->d() * W;

  uchar *dst = new uchar[W * H * 3];
  uchar *dstptr = dst;

  fl_read_image(dst, X, Y, W, H, 0);

  uchar srcr, srcg, srcb, srca;
  uchar dstr, dstg, dstb, dsta;

  if (img->d() == 2) {
    // Composite grayscale + alpha over RGB...
    for (int y = H; y > 0; y--, srcptr+=srcskip)
      for (int x = W; x > 0; x--) {
	srcg = *srcptr++;
	srca = *srcptr++;

	dstr = dstptr[0];
	dstg = dstptr[1];
	dstb = dstptr[2];
	dsta = 255 - srca;

	*dstptr++ = (srcg * srca + dstr * dsta) >> 8;
	*dstptr++ = (srcg * srca + dstg * dsta) >> 8;
	*dstptr++ = (srcg * srca + dstb * dsta) >> 8;
      }
  } else {
    // Composite RGBA over RGB...
    for (int y = H; y > 0; y--, srcptr+=srcskip)
      for (int x = W; x > 0; x--) {
	srcr = *srcptr++;
	srcg = *srcptr++;
	srcb = *srcptr++;
	srca = *srcptr++;

	dstr = dstptr[0];
	dstg = dstptr[1];
	dstb = dstptr[2];
	dsta = 255 - srca;

	*dstptr++ = (srcr * srca + dstr * dsta) >> 8;
	*dstptr++ = (srcg * srca + dstg * dsta) >> 8;
	*dstptr++ = (srcb * srca + dstb * dsta) >> 8;
      }
  }

  fl_draw_image(dst, X, Y, W, H, 3, 0);

  delete[] dst;
}
#endif // !WIN32 && !__APPLE__

void Fl_RGB_Image::draw(int XP, int YP, int WP, int HP, int cx, int cy) {
  fl_graphics_driver->draw(this, XP, YP, WP, HP, cx, cy);
}

static int start(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int w, int h, int &cx, int &cy, 
		 int &X, int &Y, int &W, int &H)
{
  // account for current clip region (faster on Irix):
  fl_clip_box(XP,YP,WP,HP,X,Y,W,H);
  cx += X-XP; cy += Y-YP;
  // clip the box down to the size of image, quit if empty:
  if (cx < 0) {W += cx; X -= cx; cx = 0;}
  if (cx+W > w) W = w-cx;
  if (W <= 0) return 1;
  if (cy < 0) {H += cy; Y -= cy; cy = 0;}
  if (cy+H > h) H = h-cy;
  if (H <= 0) return 1;
  return 0;
}

/** Draws an Fl_Image scaled to width \p W & height \p H with top-left corner at \em X,Y
 \return zero when the graphics driver doesn't implement scaled drawing, non-zero if it does implement it.
 */
int Fl_Graphics_Driver::draw_scaled(Fl_Image *img, int X, int Y, int W, int H) {
  return 0;
}

#ifdef __APPLE__
static void imgProviderReleaseData (void *info, const void *data, size_t size)
{
  if (!info || *(bool*)info) delete[] (unsigned char *)data;
  delete (bool*)info;
}

void Fl_Quartz_Graphics_Driver::draw(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  // Don't draw an empty image...
  if (!img->d() || !img->array) {
    img->draw_empty(XP, YP);
    return;
  }
  if (start(img, XP, YP, WP, HP, img->w(), img->h(), cx, cy, X, Y, W, H)) {
    return;
  }
  if (!img->id_) {
    CGColorSpaceRef lut = img->d()<=2 ? CGColorSpaceCreateDeviceGray() : CGColorSpaceCreateDeviceRGB();
    int ld = img->ld();
    if (!ld) ld = img->w() * img->d();
    // If img->alloc_array == 0, the CGImage data provider must not release the image data.
    // If img->alloc_array != 0, the CGImage data provider will take responsibilty of deleting RGB image data after use:
    // when the CGImage is deallocated, the release callback of its data provider
    // (imgProviderReleaseData) is called and can delete the RGB image data.
    // If the CGImage is printed, it is not deallocated until after the end of the page,
    // therefore, with img->alloc_array != 0, the RGB image can be safely deleted any time after return from this function.
    // The previously unused mask_ member allows to make sure the RGB image data is not deleted by Fl_RGB_Image::uncache().
    if (img->alloc_array) img->mask_ = new bool(true);
    CGDataProviderRef src = CGDataProviderCreateWithData(img->mask_, img->array, ld * img->h(),
                                                         img->alloc_array?imgProviderReleaseData:NULL);
    img->id_ = CGImageCreate(img->w(), img->h(), 8, img->d()*8, ld,
                             lut, (img->d()&1)?kCGImageAlphaNone:kCGImageAlphaLast,
                             src, 0L, false, kCGRenderingIntentDefault);
    CGColorSpaceRelease(lut);
    CGDataProviderRelease(src);
  }
  if (img->id_ && fl_gc) {
    if (!img->alloc_array && Fl_Surface_Device::surface()->class_name() == Fl_Printer::class_id && !CGImageGetShouldInterpolate((CGImageRef)img->id_)) {
      // When printing, the image data is used when the page is completed, that is, after return from this function.
      // If the image has alloc_array = 0, we must protect against image data being freed before it is used:
      // we duplicate the image data and have it deleted after use by the release-callback of the CGImage data provider
      Fl_RGB_Image* img2 = (Fl_RGB_Image*)img->copy();
      img2->alloc_array = 0;
      const uchar *img_bytes = img2->array;
      int ld = img2->ld();
      if (!ld) ld = img2->w() * img2->d();
      delete img2;
      img->uncache();
      CGColorSpaceRef lut = img->d()<=2 ? CGColorSpaceCreateDeviceGray() : CGColorSpaceCreateDeviceRGB();
      CGDataProviderRef src = CGDataProviderCreateWithData( NULL, img_bytes, ld * img->h(), imgProviderReleaseData);
      img->id_ = CGImageCreate(img->w(), img->h(), 8, img->d()*8, ld,
                               lut, (img->d()&1)?kCGImageAlphaNone:kCGImageAlphaLast,
                               src, 0L, true, kCGRenderingIntentDefault);
      CGColorSpaceRelease(lut);
      CGDataProviderRelease(src);
    }
    CGRect rect = CGRectMake(X, Y, W, H);
    Fl_X::q_begin_image(rect, cx, cy, img->w(), img->h());
    CGContextDrawImage(fl_gc, rect, (CGImageRef)img->id_);
    Fl_X::q_end_image();
  }
}

int Fl_Quartz_Graphics_Driver::draw_scaled(Fl_Image *img, int XP, int YP, int WP, int HP) {
  int X, Y, W, H;
  fl_clip_box(XP,YP,WP,HP,X,Y,W,H); // X,Y,W,H will give the unclipped area of XP,YP,WP,HP
  if (W == 0 || H == 0) return 1;
  fl_push_no_clip(); // remove the FLTK clip that can't be rescaled
  CGContextSaveGState(fl_gc);
  CGContextClipToRect(fl_gc, CGRectMake(X, Y, W, H)); // this clip path will be rescaled & translated
  CGContextTranslateCTM(fl_gc, XP, YP);
  CGContextScaleCTM(fl_gc, float(WP)/img->w(), float(HP)/img->h());
  img->draw(0, 0, img->w(), img->h(), 0, 0);
  CGContextRestoreGState(fl_gc);
  fl_pop_clip(); // restore FLTK's clip
  return 1;
}

#elif defined(WIN32)
static Fl_Offscreen build_id(Fl_RGB_Image *img, void **pmask)
{
  Fl_Offscreen offs = fl_create_offscreen(img->w(), img->h());
  if ((img->d() == 2 || img->d() == 4) && fl_can_do_alpha_blending()) {
    fl_begin_offscreen(offs);
    fl_draw_image(img->array, 0, 0, img->w(), img->h(), img->d()|FL_IMAGE_WITH_ALPHA, img->ld());
    fl_end_offscreen();
  } else {
    fl_begin_offscreen(offs);
    fl_draw_image(img->array, 0, 0, img->w(), img->h(), img->d(), img->ld());
    fl_end_offscreen();
    if (img->d() == 2 || img->d() == 4) {
      *pmask = fl_create_alphamask(img->w(), img->h(), img->d(), img->ld(), img->array);
    }
  }
  return offs;
}

void Fl_GDI_Graphics_Driver::draw(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  // Don't draw an empty image...
  if (!img->d() || !img->array) {
    img->draw_empty(XP, YP);
    return;
  }
  if (start(img, XP, YP, WP, HP, img->w(), img->h(), cx, cy, X, Y, W, H)) {
    return;
  }
  if (!img->id_) img->id_ = build_id(img, &(img->mask_));
  if (img->mask_) {
    HDC new_gc = CreateCompatibleDC(fl_gc);
    int save = SaveDC(new_gc);
    SelectObject(new_gc, (void*)img->mask_);
    BitBlt(fl_gc, X, Y, W, H, new_gc, cx, cy, SRCAND);
    SelectObject(new_gc, (void*)img->id_);
    BitBlt(fl_gc, X, Y, W, H, new_gc, cx, cy, SRCPAINT);
    RestoreDC(new_gc,save);
    DeleteDC(new_gc);
  } else if (img->d()==2 || img->d()==4) {
    copy_offscreen_with_alpha(X, Y, W, H, (Fl_Offscreen)img->id_, cx, cy);
  } else {
    copy_offscreen(X, Y, W, H, (Fl_Offscreen)img->id_, cx, cy);
  }
}

int Fl_GDI_Printer_Graphics_Driver::draw_scaled(Fl_Image *img, int XP, int YP, int WP, int HP) {
  XFORM old_tr, tr;
  GetWorldTransform(fl_gc, &old_tr); // storing old transform
  tr.eM11 = float(WP)/float(img->w());
  tr.eM22 = float(HP)/float(img->h());
  tr.eM12 = tr.eM21 = 0;
  tr.eDx =  XP;
  tr.eDy =  YP;
  ModifyWorldTransform(fl_gc, &tr, MWT_LEFTMULTIPLY);
  img->draw(0, 0, img->w(), img->h(), 0, 0);
  SetWorldTransform(fl_gc, &old_tr);
  return 1;
}

#else
void Fl_Xlib_Graphics_Driver::draw(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  // Don't draw an empty image...
  if (!img->d() || !img->array) {
    img->draw_empty(XP, YP);
    return;
  }
  if (start(img, XP, YP, WP, HP, img->w(), img->h(), cx, cy, X, Y, W, H)) {
    return;
  }
  if (!img->id_) {
    if (img->d() == 1 || img->d() == 3) {
      img->id_ = fl_create_offscreen(img->w(), img->h());
      fl_begin_offscreen((Fl_Offscreen)img->id_);
      fl_draw_image(img->array, 0, 0, img->w(), img->h(), img->d(), img->ld());
      fl_end_offscreen();
    } else if (img->d() == 4 && fl_can_do_alpha_blending()) {
      img->id_ = fl_create_offscreen_with_alpha(img->w(), img->h());
      fl_begin_offscreen((Fl_Offscreen)img->id_);
      fl_draw_image(img->array, 0, 0, img->w(), img->h(), img->d() | FL_IMAGE_WITH_ALPHA,
                    img->ld());
      fl_end_offscreen();
    }
  }
  if (img->id_) {
    if (img->mask_) {
      // I can't figure out how to combine a mask with existing region,
      // so cut the image down to a clipped rectangle:
      int nx, ny; fl_clip_box(X,Y,W,H,nx,ny,W,H);
      cx += nx-X; X = nx;
      cy += ny-Y; Y = ny;
      // make X use the bitmap as a mask:
      XSetClipMask(fl_display, fl_gc, img->mask_);
      int ox = X-cx; if (ox < 0) ox += img->w();
      int oy = Y-cy; if (oy < 0) oy += img->h();
      XSetClipOrigin(fl_display, fl_gc, X-cx, Y-cy);
    }

    if (img->d() == 4 && fl_can_do_alpha_blending())
      copy_offscreen_with_alpha(X, Y, W, H, img->id_, cx, cy);
    else
      copy_offscreen(X, Y, W, H, img->id_, cx, cy);

    if (img->mask_) {
      // put the old clip region back
      XSetClipOrigin(fl_display, fl_gc, 0, 0);
      fl_restore_clip();
    }
  } else {
    // Composite image with alpha manually each time...
    alpha_blend(img, X, Y, W, H, cx, cy);
  }
}

#endif

void Fl_RGB_Image::label(Fl_Widget* widget) {
  widget->image(this);
}

void Fl_RGB_Image::label(Fl_Menu_Item* m) {
  Fl::set_labeltype(_FL_IMAGE_LABEL, labeltype, measure);
  m->label(_FL_IMAGE_LABEL, (const char*)this);
}

//
// End of "$Id$".
//
