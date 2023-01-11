//
// Image drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2020 by Bill Spitzak and others.
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

// I hope a simple and portable method of drawing color and monochrome
// images.  To keep this simple, only a single storage type is
// supported: 8 bit unsigned data, byte order RGB, and pixels are
// stored packed into rows with the origin at the top-left.  It is
// possible to alter the size of pixels with the "delta" argument, to
// add alpha or other information per pixel.  It is also possible to
// change the origin and direction of the image data by messing with
// the "delta" and "linedelta", making them negative, though this may
// defeat some of the shortcuts in translating the image for X.


// A list of assumptions made about the X display:

// bits_per_pixel must be one of 8, 16, 24, 32.

// scanline_pad must be a power of 2 and greater or equal to 8.

// PseudoColor visuals must have 8 bits_per_pixel (although the depth
// may be less than 8).  This is the only limitation that affects any
// modern X displays, you can't use 12 or 16 bit colormaps.

// The mask bits in TrueColor visuals for each color are
// contiguous and have at least one bit of each color.  This
// is not checked for.

// For 24 and 32 bit visuals there must be at least 8 bits of each color.

////////////////////////////////////////////////////////////////

#include <config.h>
#include "Fl_Xlib_Graphics_Driver.H"
#include "../X11/Fl_X11_Screen_Driver.H"
#include "../X11/Fl_X11_Window_Driver.H"
#  include <FL/Fl.H>
#  include <FL/fl_draw.H>
#  include <FL/platform.H>
#  include <FL/Fl_Image_Surface.H>
#  include <FL/Fl_Tiled_Image.H>
#  include "../../Fl_Screen_Driver.H"
#  include "../../Fl_XColor.H"
#  include "../../flstring.h"
#if HAVE_XRENDER
#  include <X11/extensions/Xrender.h>
#  if RENDER_MAJOR * 100 + RENDER_MAJOR < 10
#    define RepeatPad  2
#  endif
#endif // HAVE_XRENDER

static XImage xi;       // template used to pass info to X
static int bytes_per_pixel;
static int scanline_add;
static int scanline_mask;

static void (*converter)(const uchar *from, uchar *to, int w, int delta);
static void (*mono_converter)(const uchar *from, uchar *to, int w, int delta);

static int dir;         // direction-alternator
static int ri,gi,bi;    // saved error-diffusion value

#  if USE_COLORMAP
////////////////////////////////////////////////////////////////
// 8-bit converter with error diffusion

static void color8_converter(const uchar *from, uchar *to, int w, int delta) {
  int r=ri, g=gi, b=bi;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*delta;
    to = to+(w-1);
    d = -delta;
    td = -1;
  } else {
    dir = 1;
    d = delta;
    td = 1;
  }
  for (; w--; from += d, to += td) {
    r += from[0]; if (r < 0) r = 0; else if (r>255) r = 255;
    g += from[1]; if (g < 0) g = 0; else if (g>255) g = 255;
    b += from[2]; if (b < 0) b = 0; else if (b>255) b = 255;
    Fl_Color i = fl_color_cube(r*FL_NUM_RED/256,g*FL_NUM_GREEN/256,b*FL_NUM_BLUE/256);
    Fl_XColor& xmap = fl_xmap[0][i];
    if (!xmap.mapped) {if (!fl_redmask) fl_xpixel(r,g,b); else fl_xpixel(i);}
    r -= xmap.r;
    g -= xmap.g;
    b -= xmap.b;
    *to = uchar(xmap.pixel);
  }
  ri = r; gi = g; bi = b;
}

static void mono8_converter(const uchar *from, uchar *to, int w, int delta) {
  int r=ri, g=gi, b=bi;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*delta;
    to = to+(w-1);
    d = -delta;
    td = -1;
  } else {
    dir = 1;
    d = delta;
    td = 1;
  }
  for (; w--; from += d, to += td) {
    r += from[0]; if (r < 0) r = 0; else if (r>255) r = 255;
    g += from[0]; if (g < 0) g = 0; else if (g>255) g = 255;
    b += from[0]; if (b < 0) b = 0; else if (b>255) b = 255;
    Fl_Color i = fl_color_cube(r*FL_NUM_RED/256,g*FL_NUM_GREEN/256,b*FL_NUM_BLUE/256);
    Fl_XColor& xmap = fl_xmap[0][i];
    if (!xmap.mapped) {if (!fl_redmask) fl_xpixel(r,g,b); else fl_xpixel(i);}
    r -= xmap.r;
    g -= xmap.g;
    b -= xmap.b;
    *to = uchar(xmap.pixel);
  }
  ri = r; gi = g; bi = b;
}

#  endif

////////////////////////////////////////////////////////////////
// 16 bit TrueColor converters with error diffusion
// Cray computers have no 16-bit type, so we use character pointers
// (which may be slow)

#  ifdef U16
#    define OUTTYPE U16
#    define OUTSIZE 1
#    define OUTASSIGN(v) *t = v
#  else
#    define OUTTYPE uchar
#    define OUTSIZE 2
#    define OUTASSIGN(v) int tt=v; t[0] = uchar(tt>>8); t[1] = uchar(tt)
#  endif

static void color16_converter(const uchar *from, uchar *to, int w, int delta) {
  OUTTYPE *t = (OUTTYPE *)to;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*delta;
    t = t+(w-1)*OUTSIZE;
    d = -delta;
    td = -OUTSIZE;
  } else {
    dir = 1;
    d = delta;
    td = OUTSIZE;
  }
  int r=ri, g=gi, b=bi;
  for (; w--; from += d, t += td) {
    r = (r&~fl_redmask)  +from[0]; if (r>255) r = 255;
    g = (g&~fl_greenmask)+from[1]; if (g>255) g = 255;
    b = (b&~fl_bluemask) +from[2]; if (b>255) b = 255;
    OUTASSIGN((
      ((r&fl_redmask)<<fl_redshift)+
      ((g&fl_greenmask)<<fl_greenshift)+
      ((b&fl_bluemask)<<fl_blueshift)
      ) >> fl_extrashift);
  }
  ri = r; gi = g; bi = b;
}

static void mono16_converter(const uchar *from,uchar *to,int w, int delta) {
  OUTTYPE *t = (OUTTYPE *)to;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*delta;
    t = t+(w-1)*OUTSIZE;
    d = -delta;
    td = -OUTSIZE;
  } else {
    dir = 1;
    d = delta;
    td = OUTSIZE;
  }
  uchar mask = fl_redmask & fl_greenmask & fl_bluemask;
  int r=ri;
  for (; w--; from += d, t += td) {
    r = (r&~mask) + *from; if (r > 255) r = 255;
    uchar m = r&mask;
    OUTASSIGN((
      (m<<fl_redshift)+
      (m<<fl_greenshift)+
      (m<<fl_blueshift)
      ) >> fl_extrashift);
  }
  ri = r;
}

// special-case the 5r6g5b layout used by XFree86:

static void c565_converter(const uchar *from, uchar *to, int w, int delta) {
  OUTTYPE *t = (OUTTYPE *)to;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*delta;
    t = t+(w-1)*OUTSIZE;
    d = -delta;
    td = -OUTSIZE;
  } else {
    dir = 1;
    d = delta;
    td = OUTSIZE;
  }
  int r=ri, g=gi, b=bi;
  for (; w--; from += d, t += td) {
    r = (r&7)+from[0]; if (r>255) r = 255;
    g = (g&3)+from[1]; if (g>255) g = 255;
    b = (b&7)+from[2]; if (b>255) b = 255;
    OUTASSIGN(((r&0xf8)<<8) + ((g&0xfc)<<3) + (b>>3));
  }
  ri = r; gi = g; bi = b;
}

static void m565_converter(const uchar *from,uchar *to,int w, int delta) {
  OUTTYPE *t = (OUTTYPE *)to;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*delta;
    t = t+(w-1)*OUTSIZE;
    d = -delta;
    td = -OUTSIZE;
  } else {
    dir = 1;
    d = delta;
    td = OUTSIZE;
  }
  int r=ri;
  for (; w--; from += d, t += td) {
    r = (r&7) + *from; if (r > 255) r = 255;
    OUTASSIGN((r>>3) * 0x841);
  }
  ri = r;
}

////////////////////////////////////////////////////////////////
// 24bit TrueColor converters:

static void rgb_converter(const uchar *from, uchar *to, int w, int delta) {
  int d = delta-3;
  for (; w--; from += d) {
    *to++ = *from++;
    *to++ = *from++;
    *to++ = *from++;
  }
}

static void bgr_converter(const uchar *from, uchar *to, int w, int delta) {
  for (; w--; from += delta) {
    uchar r = from[0];
    uchar g = from[1];
    *to++ = from[2];
    *to++ = g;
    *to++ = r;
  }
}

static void rrr_converter(const uchar *from, uchar *to, int w, int delta) {
  for (; w--; from += delta) {
    *to++ = *from;
    *to++ = *from;
    *to++ = *from;
  }
}

////////////////////////////////////////////////////////////////
// 32bit TrueColor converters on a 32 or 64-bit machine:

#  ifdef U64
#    define STORETYPE U64
#    if WORDS_BIGENDIAN
#      define INNARDS32(f) \
  U64 *t = (U64*)to; \
  int w1 = w/2; \
  for (; w1--; from += delta) {U64 i = f; from += delta; *t++ = (i<<32)|(f);} \
  if (w&1) *t++ = (U64)(f)<<32;
#    else
#      define INNARDS32(f) \
  U64 *t = (U64*)to; \
  int w1 = w/2; \
  for (; w1--; from += delta) {U64 i = f; from += delta; *t++ = ((U64)(f)<<32)|i;} \
  if (w&1) *t++ = (U64)(f);
#    endif
#  else
#    define STORETYPE U32
#    define INNARDS32(f) \
  U32 *t = (U32*)to; for (; w--; from += delta) *t++ = f
#  endif

static void rgbx_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32((unsigned(from[0])<<24)+(from[1]<<16)+(from[2]<<8));
}

static void xbgr_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32((from[0])+(from[1]<<8)+(from[2]<<16));
}

static void xrgb_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32((from[0]<<16)+(from[1]<<8)+(from[2]));
}

static void argb_premul_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32((unsigned(from[3]) << 24) +
             (((from[0] * from[3]) / 255) << 16) +
             (((from[1] * from[3]) / 255) << 8) +
             ((from[2] * from[3]) / 255));
}

static void depth2_to_argb_premul_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32((unsigned(from[1]) << 24) +
            (((from[0] * from[1]) / 255) << 16) +
            (((from[0] * from[1]) / 255) << 8) +
            ((from[0] * from[1]) / 255));
}

static void bgrx_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32((from[0]<<8)+(from[1]<<16)+(unsigned(from[2])<<24));
}

static void rrrx_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32(unsigned(*from) * 0x1010100U);
}

static void xrrr_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32(*from * 0x10101U);
}

static void
color32_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32(
    (from[0]<<fl_redshift)+(from[1]<<fl_greenshift)+(from[2]<<fl_blueshift));
}

static void
mono32_converter(const uchar *from,uchar *to,int w, int delta) {
  INNARDS32(
    (*from << fl_redshift)+(*from << fl_greenshift)+(*from << fl_blueshift));
}

////////////////////////////////////////////////////////////////

static void figure_out_visual() {

  fl_xpixel(FL_BLACK); // setup fl_redmask, etc, in fl_color.cxx
  fl_xpixel(FL_WHITE); // also make sure white is allocated

  static XPixmapFormatValues *pfvlist;
  static int FL_NUM_pfv;
  if (!pfvlist) pfvlist = XListPixmapFormats(fl_display,&FL_NUM_pfv);
  XPixmapFormatValues *pfv;
  for (pfv = pfvlist; pfv < pfvlist+FL_NUM_pfv; pfv++)
    if (pfv->depth == fl_visual->depth) break;
  xi.format = ZPixmap;
  xi.byte_order = ImageByteOrder(fl_display);
//i.bitmap_unit = 8;
//i.bitmap_bit_order = MSBFirst;
//i.bitmap_pad = 8;
  xi.depth = fl_visual->depth;
  xi.bits_per_pixel = pfv->bits_per_pixel;

  if (xi.bits_per_pixel & 7) bytes_per_pixel = 0; // produce fatal error
  else bytes_per_pixel = xi.bits_per_pixel/8;

  unsigned int n = pfv->scanline_pad/8;
  if (pfv->scanline_pad & 7 || (n&(n-1)))
    Fl::fatal("Can't do scanline_pad of %d",pfv->scanline_pad);
  if (n < sizeof(STORETYPE)) n = sizeof(STORETYPE);
  scanline_add = n-1;
  scanline_mask = -n;

#  if USE_COLORMAP
  if (bytes_per_pixel == 1) {
    converter = color8_converter;
    mono_converter = mono8_converter;
    return;
  }
  if (!fl_visual->red_mask)
    Fl::fatal("Can't do %d bits_per_pixel colormap",xi.bits_per_pixel);
#  endif

  // otherwise it is a TrueColor visual:

  int rs = fl_redshift;
  int gs = fl_greenshift;
  int bs = fl_blueshift;

  switch (bytes_per_pixel) {

  case 2:
    // All 16-bit TrueColor visuals are supported on any machine with
    // 24 or more bits per integer.
#  ifdef U16
    xi.byte_order = WORDS_BIGENDIAN;
#  else
    xi.byte_order = 1;
#  endif
    if (rs == 11 && gs == 6 && bs == 0 && fl_extrashift == 3) {
      converter = c565_converter;
      mono_converter = m565_converter;
    } else {
      converter = color16_converter;
      mono_converter = mono16_converter;
    }
    break;

  case 3:
    if (xi.byte_order) {rs = 16-rs; gs = 16-gs; bs = 16-bs;}
    if (rs == 0 && gs == 8 && bs == 16) {
      converter = rgb_converter;
      mono_converter = rrr_converter;
    } else if (rs == 16 && gs == 8 && bs == 0) {
      converter = bgr_converter;
      mono_converter = rrr_converter;
    } else {
      Fl::fatal("Can't do arbitrary 24bit color");
    }
    break;

  case 4:
    if ((xi.byte_order!=0) != WORDS_BIGENDIAN)
      {rs = 24-rs; gs = 24-gs; bs = 24-bs;}
    if (rs == 0 && gs == 8 && bs == 16) {
      converter = xbgr_converter;
      mono_converter = xrrr_converter;
    } else if (rs == 24 && gs == 16 && bs == 8) {
      converter = rgbx_converter;
      mono_converter = rrrx_converter;
    } else if (rs == 8 && gs == 16 && bs == 24) {
      converter = bgrx_converter;
      mono_converter = rrrx_converter;
    } else if (rs == 16 && gs == 8 && bs == 0) {
      converter = xrgb_converter;
      mono_converter = xrrr_converter;
    } else {
      xi.byte_order = WORDS_BIGENDIAN;
      converter = color32_converter;
      mono_converter = mono32_converter;
    }
    break;

  default:
    Fl::fatal("Can't do %d bits_per_pixel",xi.bits_per_pixel);
  }

}

#  define MAXBUFFER 0x40000 // 256k

static void innards(const uchar *buf, int X, int Y, int W, int H,
                    int delta, int linedelta, int mono,
                    Fl_Draw_Image_Cb cb, void* userdata,
                    const bool alpha, GC gc)
{
  if (!linedelta) linedelta = W*abs(delta);

  int dx = 0, dy = 0, w = 0, h = 0;
  fl_clip_box(X, Y, W, H, dx, dy, w, h);
  if (w<=0 || h<=0) return;
  dx -= X;
  dy -= Y;
  if (!bytes_per_pixel) figure_out_visual();
  const unsigned oldbpp = bytes_per_pixel;
  static GC gc32 = None;
  xi.width = w;
  xi.height = h;

  void (*conv)(const uchar *from, uchar *to, int w, int delta) = converter;
  if (mono) conv = mono_converter;
  if (alpha) {
    // This flag states the destination format is ARGB32 (big-endian), pre-multiplied.
    bytes_per_pixel = 4;
    conv = (mono ? depth2_to_argb_premul_converter : argb_premul_converter);
    xi.depth = 32;
    xi.bits_per_pixel = 32;

    // Do we need a new GC?
    if (fl_visual->depth != 32) {
      if (gc32 == None)
        gc32 = XCreateGC(fl_display, fl_window, 0, NULL);
      gc = gc32;
    }
  }

  // See if the data is already in the right format.  Unfortunately
  // some 32-bit x servers (XFree86) care about the unknown 8 bits
  // and they must be zero.  I can't confirm this for user-supplied
  // data, so the 32-bit shortcut is disabled...
  // This can set bytes_per_line negative if image is bottom-to-top
  // I tested it on Linux, but it may fail on other Xlib implementations:
  if (buf && (
#  if 0 // set this to 1 to allow 32-bit shortcut
      delta == 4 &&
#    if WORDS_BIGENDIAN
      conv == rgbx_converter
#    else
      conv == xbgr_converter
#    endif
      ||
#  endif
      conv == rgb_converter && delta==3
      ) && !(linedelta&scanline_add)) {
    xi.data = (char *)(buf+delta*dx+linedelta*dy);
    xi.bytes_per_line = linedelta;

  } else {
    int linesize = ((w*bytes_per_pixel+scanline_add)&scanline_mask)/sizeof(STORETYPE);
    int blocking = h;
    static STORETYPE *buffer;   // our storage, always word aligned
    static long buffer_size;
    {int size = linesize*h;
    if (size > MAXBUFFER) {
      size = MAXBUFFER;
      blocking = MAXBUFFER/linesize;
    }
    if (size > buffer_size) {
      delete[] buffer;
      buffer_size = size;
      buffer = new STORETYPE[size];
    }}
    xi.data = (char *)buffer;
    xi.bytes_per_line = linesize*sizeof(STORETYPE);
    if (buf) {
      buf += delta*dx+linedelta*dy;
      for (int j=0; j<h; ) {
        STORETYPE *to = buffer;
        int k;
        for (k = 0; j<h && k<blocking; k++, j++) {
          conv(buf, (uchar*)to, w, delta);
          buf += linedelta;
          to += linesize;
        }
        XPutImage(fl_display,fl_window,gc, &xi, 0, 0, X+dx, Y+dy+j-k, w, k);
      }
    } else {
      STORETYPE* linebuf = new STORETYPE[(W*delta+(sizeof(STORETYPE)-1))/sizeof(STORETYPE)];
      for (int j=0; j<h; ) {
        STORETYPE *to = buffer;
        int k;
        for (k = 0; j<h && k<blocking; k++, j++) {
          cb(userdata, dx, dy+j, w, (uchar*)linebuf);
          conv((uchar*)linebuf, (uchar*)to, w, delta);
          to += linesize;
        }
        XPutImage(fl_display,fl_window,gc, &xi, 0, 0, X+dx, Y+dy+j-k, w, k);
      }

      delete[] linebuf;
    }
  }

  if (alpha) {
    bytes_per_pixel = oldbpp;
    xi.depth = fl_visual->depth;
    xi.bits_per_pixel = oldbpp * 8;
  }
}

void Fl_Xlib_Graphics_Driver::draw_image_unscaled(const uchar* buf, int x, int y, int w, int h, int d, int l){

  const bool alpha = !!(abs(d) & FL_IMAGE_WITH_ALPHA);
  if (alpha) d ^= FL_IMAGE_WITH_ALPHA;
  const int mono = (d>-3 && d<3);

  innards(buf,x+floor(offset_x_),y+floor(offset_y_),w,h,d,l,mono,0,0,alpha,gc_);
}

void Fl_Xlib_Graphics_Driver::draw_image_unscaled(Fl_Draw_Image_Cb cb, void* data,
                   int x, int y, int w, int h,int d) {

  const bool alpha = !!(abs(d) & FL_IMAGE_WITH_ALPHA);
  if (alpha) d ^= FL_IMAGE_WITH_ALPHA;
  const int mono = (d>-3 && d<3);

  innards(0,x+floor(offset_x_),y+floor(offset_y_),w,h,d,0,mono,cb,data,alpha,gc_);
}

void Fl_Xlib_Graphics_Driver::draw_image_mono_unscaled(const uchar* buf, int x, int y, int w, int h, int d, int l){
  innards(buf,x+floor(offset_x_),y+floor(offset_y_),w,h,d,l,1,0,0,0,gc_);
}

void Fl_Xlib_Graphics_Driver::draw_image_mono_unscaled(Fl_Draw_Image_Cb cb, void* data,
                   int x, int y, int w, int h,int d) {
  innards(0,x+floor(offset_x_),y+floor(offset_y_),w,h,d,0,1,cb,data,0,gc_);
}

void Fl_Xlib_Graphics_Driver::colored_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b) {
  if (fl_visual->depth > 16) {
    Fl_Graphics_Driver::colored_rectf(x, y, w, h, r, g, b);
  } else {
    uchar c[3];
    c[0] = r; c[1] = g; c[2] = b;
    innards(c, floor(x), floor(y), floor(x + w) - floor(x), floor(y + h) - floor(y),
            0,0,0,0,0,0, (GC)gc());
  }
}

unsigned long Fl_Xlib_Graphics_Driver::create_bitmask(int w, int h, const uchar *data) {
  return XCreateBitmapFromData(fl_display, fl_window, (const char *)data,
                               (w+7)&-8, h);
}

void Fl_Xlib_Graphics_Driver::delete_bitmask(fl_uintptr_t bm) {
  XFreePixmap(fl_display, (unsigned long)bm);
}

void Fl_Xlib_Graphics_Driver::draw_fixed(Fl_Bitmap *bm, int X, int Y, int W, int H, int cx, int cy) {
  X = floor(X)+floor(offset_x_);
  Y = floor(Y)+floor(offset_y_);
  cache_size(bm, W, H);
  cx *= scale(); cy *= scale();
  XSetStipple(fl_display, gc_, *Fl_Graphics_Driver::id(bm));
  int ox = X-cx; if (ox < 0) ox += bm->w()*scale();
  int oy = Y-cy; if (oy < 0) oy += bm->h()*scale();
  XSetTSOrigin(fl_display, gc_, ox, oy);
  XSetFillStyle(fl_display, gc_, FillStippled);
  XFillRectangle(fl_display, fl_window, gc_, X, Y, W, H);
  XSetFillStyle(fl_display, gc_, FillSolid);
}


// Composite an image with alpha on systems that don't have accelerated
// alpha compositing...
static void alpha_blend(Fl_RGB_Image *img, int X, int Y, int W, int H, int cx, int cy) {
  if (cx < 0) { W += cx; X -= cx; cx = 0; }
  if (cy < 0) { H += cy; Y -= cy; cy = 0; }
  if (W + cx > img->data_w()) W = img->data_w() - cx;
  if (H + cy > img->data_h()) H = img->data_h() - cy;
  // don't attempt to read outside the window/offscreen buffer limits
  Window root_return;
  int x_return, y_return;
  unsigned int winW, winH;
  unsigned int border_width_return;
  unsigned int depth_return;
  XGetGeometry(fl_display, fl_window, &root_return, &x_return, &y_return, &winW,
               &winH, &border_width_return, &depth_return);
  if (X+W > (int)winW) W = (int)winW-X;
  if (Y+H > (int)winH) H = (int)winH-Y;
  if (W <= 0 || H <= 0) return;
  int ld = img->ld();
  if (ld == 0) ld = img->data_w() * img->d();
  uchar *srcptr = (uchar*)img->array + cy * ld + cx * img->d();

  uchar *dst = fl_read_image(NULL, X, Y, W, H, 0);
  if (!dst) {
    fl_draw_image(srcptr, X, Y, W, H, img->d(), ld);
    return;
  }
  int srcskip = ld - img->d() * W;
  uchar *dstptr = dst;

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

void Fl_Xlib_Graphics_Driver::cache(Fl_RGB_Image *img) {
  Fl_Image_Surface *surface;
  int depth = img->d();
  if (depth == 1 || depth == 3) {
    surface = new Fl_Image_Surface(img->data_w(), img->data_h());
  } else if (fl_can_do_alpha_blending()) {
    Pixmap pixmap = XCreatePixmap(fl_display, RootWindow(fl_display, fl_screen), img->data_w(), img->data_h(), 32);
    surface = new Fl_Image_Surface(img->data_w(), img->data_h(), 0, (Fl_Offscreen)pixmap);
    depth |= FL_IMAGE_WITH_ALPHA;
  } else {
    *Fl_Graphics_Driver::id(img) = 0;
    return;
  }
  Fl_Surface_Device::push_current(surface);
  fl_draw_image(img->array, 0, 0, img->data_w(), img->data_h(), depth, img->ld());
  Fl_Surface_Device::pop_current();
  Fl_Offscreen off = Fl_Graphics_Driver::get_offscreen_and_delete_image_surface(surface);
  int *pw, *ph;
  cache_w_h(img, pw, ph);
  *pw = img->data_w();
  *ph = img->data_h();
  *Fl_Graphics_Driver::id(img) = (fl_uintptr_t)off;
}


void Fl_Xlib_Graphics_Driver::draw_fixed(Fl_RGB_Image *img, int X, int Y, int W, int H, int cx, int cy) {
  X = floor(X)+floor(offset_x_);
  Y = floor(Y)+floor(offset_y_);
  cache_size(img, W, H);
  cx *= scale(); cy *= scale();
  if (img->d() == 1 || img->d() == 3) {
    XCopyArea(fl_display, *Fl_Graphics_Driver::id(img), fl_window, gc_, cx, cy, W, H, X, Y);
    return;
  }
  // Composite image with alpha manually each time...
  push_no_clip();
  float s = scale();
  Fl_Graphics_Driver::scale(1);
  int ox = offset_x_, oy = offset_y_;
  offset_x_ = offset_y_ = 0;
  Fl_X11_Screen_Driver *d = (Fl_X11_Screen_Driver*)Fl::screen_driver();
  int nscreen = Fl_Window_Driver::driver(Fl_Window::current())->screen_num();
  float keep = d->scale(nscreen);
  d->scale(nscreen, 1);
  alpha_blend(img, X, Y, W, H, cx, cy);
  d->scale(nscreen, keep);
  Fl_Graphics_Driver::scale(s);
  offset_x_ = ox; offset_y_ = oy;
  pop_clip();
}

#if HAVE_XRENDER

void Fl_Xlib_Graphics_Driver::draw_rgb(Fl_RGB_Image *rgb, int XP, int YP, int WP, int HP, int cx, int cy) {
  if (!fl_can_do_alpha_blending()) {
    Fl_Graphics_Driver::draw_rgb(rgb, XP, YP, WP, HP, cx, cy);
    return;
  }
  if (!*Fl_Graphics_Driver::id(rgb)) {
    cache(rgb);
  }
  float s = scale();
  int Xs = Fl_Scalable_Graphics_Driver::floor(XP - cx, s);
  int Wfull = Fl_Scalable_Graphics_Driver::floor(XP - cx + rgb->w(), s) - Xs  ;
  int Ys = Fl_Scalable_Graphics_Driver::floor(YP - cy, s);
  int Hfull = Fl_Scalable_Graphics_Driver::floor(YP - cy + rgb->h(), s) - Ys;
  if (Wfull == 0 || Hfull == 0) return;
  bool need_clip = (cx || cy || WP < rgb->w() || HP < rgb->h());
  if (need_clip) push_clip(XP, YP, WP, HP);
  scale_and_render_pixmap( *Fl_Graphics_Driver::id(rgb), rgb->d(),
                          rgb->data_w() / double(Wfull), rgb->data_h() / double(Hfull),
                          Xs + this->floor(offset_x_), Ys + this->floor(offset_y_),
                          Wfull, Hfull);
  if (need_clip) pop_clip();
}

/* Draws with Xrender an Fl_Offscreen with optional scaling and accounting for transparency if necessary.
 XP,YP,WP,HP are in drawing units
 */
int Fl_Xlib_Graphics_Driver::scale_and_render_pixmap(Fl_Offscreen pixmap, int depth, double scale_x, double scale_y, int XP, int YP, int WP, int HP) {
  bool has_alpha = (depth == 2 || depth == 4);
  if (!has_alpha && scale_x == 1 && scale_y == 1) {
    // Fix for a problem visible under XQuartz with test/device and Fl_Image_Surface:
    // the drawn image is fully black. The problem does not occur under linux.
    // Why the problem occurs under XQuartz remains unknown.
    // The fix is to use XCopyArea() when adequate, rather than using Xrender.
    XCopyArea(fl_display, pixmap, fl_window, gc_, 0, 0, WP, HP, XP, YP);
    return 1;
  }
  XRenderPictureAttributes srcattr;
  memset(&srcattr, 0, sizeof(XRenderPictureAttributes));
  static XRenderPictFormat *fmt24 = XRenderFindStandardFormat(fl_display, PictStandardRGB24);
  static XRenderPictFormat *fmt32 = XRenderFindStandardFormat(fl_display, PictStandardARGB32);
  static XRenderPictFormat *dstfmt = XRenderFindVisualFormat(fl_display, fl_visual->visual);
  srcattr.repeat = RepeatPad;
  Picture src = XRenderCreatePicture(fl_display, (Pixmap)pixmap, has_alpha ?fmt32:fmt24,
                                     CPRepeat, &srcattr);
  Picture dst = XRenderCreatePicture(fl_display, fl_window, dstfmt, 0, 0);
  if (!src || !dst) {
    fprintf(stderr, "Failed to create Render pictures (%lu %lu)\n", src, dst);
    return 0;
  }
  Fl_Region r = scale_clip(scale());
  const Region clipr = (Region)clip_region();
  if (clipr)
    XRenderSetPictureClipRegion(fl_display, dst, clipr);
  unscale_clip(r);
  if (scale_x != 1 || scale_y != 1) {
    XTransform mat = {{
      { XDoubleToFixed( scale_x ), XDoubleToFixed( 0 ),       XDoubleToFixed( 0 ) },
      { XDoubleToFixed( 0 ),       XDoubleToFixed( scale_y ), XDoubleToFixed( 0 ) },
      { XDoubleToFixed( 0 ),       XDoubleToFixed( 0 ),       XDoubleToFixed( 1 ) }
    }};
    XRenderSetPictureTransform(fl_display, src, &mat);
    if (Fl_Image::scaling_algorithm() == FL_RGB_SCALING_BILINEAR) {
      XRenderSetPictureFilter(fl_display, src, FilterBilinear, 0, 0);
      // A note at  https://www.talisman.org/~erlkonig/misc/x11-composite-tutorial/ :
      // "When you use a filter you'll probably want to use PictOpOver as the render op,
      // regardless of whether the source picture has an alpha channel or not, since
      // the edges may end up having alpha values after the filter has been applied."
      // suggests this is necessary :
      has_alpha = true;
    }
  }
  XRenderComposite(fl_display, (has_alpha ? PictOpOver : PictOpSrc), src, None, dst, 0, 0, 0, 0,
                   XP, YP, WP, HP);
  XRenderFreePicture(fl_display, src);
  XRenderFreePicture(fl_display, dst);
  return 1;
}

#endif // HAVE_XRENDER

void Fl_Xlib_Graphics_Driver::uncache(Fl_RGB_Image*, fl_uintptr_t &id_, fl_uintptr_t &mask_)
{
  if (id_) {
    XFreePixmap(fl_display, (Pixmap)id_);
    id_ = 0;
  }
}

void Fl_Xlib_Graphics_Driver::cache(Fl_Bitmap *bm) {
  int *pw, *ph;
  cache_w_h(bm, pw, ph);
  *pw = bm->data_w();
  *ph = bm->data_h();
  *Fl_Graphics_Driver::id(bm) = (fl_uintptr_t)create_bitmask(bm->data_w(), bm->data_h(), bm->array);
}

void Fl_Xlib_Graphics_Driver::draw_fixed(Fl_Pixmap *pxm, int X, int Y, int W, int H, int cx, int cy) {
  X = floor(X)+floor(offset_x_);
  Y = floor(Y)+floor(offset_y_);
  cache_size(pxm, W, H);
  cx *= scale(); cy *= scale();
  Fl_Region r2 = scale_clip(scale());
  if (*Fl_Graphics_Driver::mask(pxm)) {
    // make X use the bitmap as a mask:
    XSetClipMask(fl_display, gc_, *Fl_Graphics_Driver::mask(pxm));
    XSetClipOrigin(fl_display, gc_, X-cx, Y-cy);
    if (clip_region()) {
      // At this point, XYWH is the bounding box of the intersection between
      // the current clip region and the (portion of the) pixmap we have to draw.
      // The current clip region is often a rectangle. But, when a window with rounded
      // corners is moved above another window, expose events may create a complex clip
      // region made of several (e.g., 10) rectangles. We have to draw only in the clip
      // region, and also to mask out the transparent pixels of the image. This can't
      // be done in a single Xlib call for a multi-rectangle clip region. Thus, we
      // process each rectangle of the intersection between the clip region and XYWH.
      // See also STR #3206.
      Region r = (Region)XRectangleRegion(X,Y,W,H);
      XIntersectRegion(r, (Region)clip_region(), r);
      int X1, Y1, W1, H1;
      for (int i = 0; i < r->numRects; i++) {
        X1 = r->rects[i].x1;
        Y1 = r->rects[i].y1;
        W1 = r->rects[i].x2 - r->rects[i].x1;
        H1 = r->rects[i].y2 - r->rects[i].y1;
        XCopyArea(fl_display, *Fl_Graphics_Driver::id(pxm), fl_window, gc_, cx + (X1 - X), cy + (Y1 - Y), W1, H1, X1, Y1);
      }
      XDestroyRegion(r);
    } else {
      XCopyArea(fl_display, *Fl_Graphics_Driver::id(pxm), fl_window, gc_, cx, cy, W, H, X, Y);
    }
    // put the old clip region back
    XSetClipOrigin(fl_display, gc_, 0, 0);
    float s = scale(); Fl_Graphics_Driver::scale(1);
    restore_clip();
    Fl_Graphics_Driver::scale(s);
  }
  else XCopyArea(fl_display, *Fl_Graphics_Driver::id(pxm), fl_window, gc_, cx, cy, W, H, X, Y);
  unscale_clip(r2);
}


void Fl_Xlib_Graphics_Driver::cache(Fl_Pixmap *pxm) {
  Fl_Image_Surface *surf = new Fl_Image_Surface(pxm->data_w(), pxm->data_h());
  Fl_Surface_Device::push_current(surf);
  uchar **pbitmap = surf->driver()->mask_bitmap();
  *pbitmap = (uchar*)1;// will instruct fl_draw_pixmap() to compute the image's mask
  fl_draw_pixmap(pxm->data(), 0, 0, FL_BLACK);
  uchar *bitmap = *pbitmap;
  if (bitmap) {
    *Fl_Graphics_Driver::mask(pxm) = (fl_uintptr_t)create_bitmask(pxm->data_w(), pxm->data_h(), bitmap);
    delete[] bitmap;
  }
  *pbitmap = 0;
  Fl_Surface_Device::pop_current();
  Fl_Offscreen id = Fl_Graphics_Driver::get_offscreen_and_delete_image_surface(surf);
  int *pw, *ph;
  cache_w_h(pxm, pw, ph);
  *pw = pxm->data_w();
  *ph = pxm->data_h();
  *Fl_Graphics_Driver::id(pxm) = (fl_uintptr_t)id;
}

void Fl_Xlib_Graphics_Driver::uncache_pixmap(fl_uintptr_t offscreen) {
  XFreePixmap(fl_display, (Pixmap)offscreen);
}
