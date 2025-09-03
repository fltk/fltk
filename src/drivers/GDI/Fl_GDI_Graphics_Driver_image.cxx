//
// Windows image drawing code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

// Unbelievably (since it conflicts with how most PC software works)
// Micro$oft picked a bottom-up and BGR storage format for their
// DIB images.  I'm pretty certain there is a way around this, but
// I can't find any other than the brute-force method of drawing
// each line as a separate image.  This may also need to be done
// if the delta is any amount other than 1, 3, or 4.

////////////////////////////////////////////////////////////////

#include <config.h>
#include "Fl_GDI_Graphics_Driver.H"
#include "../WinAPI/Fl_WinAPI_System_Driver.H"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/platform.H>
#include <FL/Fl_Image_Surface.H>

#define MAXBUFFER 0x40000 // 256k

void fl_release_dc(HWND, HDC); // from Fl_win32.cxx

#if USE_COLORMAP

// error-diffusion dither into the FLTK colormap
static void dither(uchar* to, const uchar* from, int w, int delta) {
  static int ri, gi, bi, dir;
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
    int rr = r*FL_NUM_RED/256;
    r -= rr*255/(FL_NUM_RED-1);
    g += from[1]; if (g < 0) g = 0; else if (g>255) g = 255;
    int gg = g*FL_NUM_GREEN/256;
    g -= gg*255/(FL_NUM_GREEN-1);
    b += from[2]; if (b < 0) b = 0; else if (b>255) b = 255;
    int bb = b*FL_NUM_BLUE/256;
    b -= bb*255/(FL_NUM_BLUE-1);
    *to = uchar(FL_COLOR_CUBE+(bb*FL_NUM_RED+rr)*FL_NUM_GREEN+gg);
  }
  ri = r; gi = g; bi = b;
}

// error-diffusion dither into the FLTK colormap
static void monodither(uchar* to, const uchar* from, int w, int delta) {
  static int ri,dir;
  int r=ri;
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
    r += *from; if (r < 0) r = 0; else if (r>255) r = 255;
    int rr = r*FL_NUM_GRAY/256;
    r -= rr*255/(FL_NUM_GRAY-1);
    *to = uchar(FL_GRAY_RAMP+rr);
  }
  ri = r;
}

#endif // USE_COLORMAP

static int fl_abs(int v) { return v<0 ? -v : v; }

static void innards(const uchar *buf, int X, int Y, int W, int H,
                    int delta, int linedelta, int depth,
                    Fl_Draw_Image_Cb cb, void* userdata, HDC gc)
{
  char indexed = 0;

#if USE_COLORMAP
  indexed = (fl_palette != 0);
#endif

  if (depth==0) depth = 3;
  if (indexed || !fl_can_do_alpha_blending())
    depth = (depth-1)|1;

  if (!linedelta) linedelta = W*fl_abs(delta);

  int x = 0, y = 0, w = 0, h = 0;
  fl_clip_box(X, Y, W, H, x, y, w, h);
  if (w<=0 || h<=0) return;
  if (buf) buf += (x-X)*delta + (y-Y)*linedelta;

  // bmibuffer: BITMAPINFOHEADER + 256 colors (RGBQUAD) + 1 (rounding effects ?)
  static U32 bmibuffer[sizeof(BITMAPINFOHEADER)/4 + 257];
  BITMAPINFO *bmi = (BITMAPINFO*)bmibuffer;
  if (!bmi->bmiHeader.biSize) {
    bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi->bmiHeader.biPlanes = 1;
    bmi->bmiHeader.biCompression = BI_RGB;
    bmi->bmiHeader.biXPelsPerMeter = 0;
    bmi->bmiHeader.biYPelsPerMeter = 0;
    bmi->bmiHeader.biClrUsed = 0;
    bmi->bmiHeader.biClrImportant = 0;
  }
#if USE_COLORMAP
  if (indexed) {
    for (short i=0; i<256; i++) {
      *((short*)(bmi->bmiColors)+i) = i;
    }
  } else
#endif
  if (depth<3) {
    RGBQUAD *bmi_colors = &(bmi->bmiColors[0]); // use pointer to suppress warning (STR #3199)
    for (int i=0; i<256; i++) {
      bmi_colors[i].rgbBlue = (uchar)i;     // = bmi->bmiColors[i]
      bmi_colors[i].rgbGreen = (uchar)i;
      bmi_colors[i].rgbRed = (uchar)i;
      bmi_colors[i].rgbReserved = (uchar)0; // must be zero
    }
  }
  bmi->bmiHeader.biWidth = w;
#if USE_COLORMAP
  bmi->bmiHeader.biBitCount = indexed ? 8 : depth*8;
  int pixelsize = indexed ? 1 : depth;
#else
  bmi->bmiHeader.biBitCount = depth*8;
  int pixelsize = depth;
#endif
  if (depth==2) { // special case: gray with alpha
    bmi->bmiHeader.biBitCount = 32;
    pixelsize = 4;
  }
  int linesize = (pixelsize*w+3)&~3;

  static U32* buffer;
  static long buffer_size;
  int blocking = h;
  {
    int size = linesize * h;
    // when printing, don't limit buffer size not to get a crash in StretchDIBits
    if (size > MAXBUFFER && !fl_graphics_driver->has_feature(Fl_Graphics_Driver::PRINTER)) {
      size = MAXBUFFER;
      blocking = MAXBUFFER / linesize;
    }
    if (size > buffer_size) {
      delete[] buffer;
      buffer_size = size;
      buffer = new U32[(size + 3) / 4];
    }
  }
  bmi->bmiHeader.biHeight = blocking;
  static U32* line_buffer;
  if (!buf) {
    int size = W*delta;
    static int line_buf_size;
    if (size > line_buf_size) {
      delete[] line_buffer;
      line_buf_size = size;
      line_buffer = new U32[(size+3)/4];
    }
  }
  for (int j=0; j<h; ) {
    int k;
    for (k = 0; j<h && k<blocking; k++, j++) {
      const uchar* from;
      if (!buf) { // run the converter:
        cb(userdata, x-X, y-Y+j, w, (uchar*)line_buffer);
        from = (uchar*)line_buffer;
      } else {
        from = buf;
        buf += linedelta;
      }
      uchar *to = (uchar*)buffer+(blocking-k-1)*linesize;
#if USE_COLORMAP
      if (indexed) {
        if (depth<3)
          monodither(to, from, w, delta);
        else
          dither(to, from, w, delta);
      } else
#endif
      {
        int i;
        switch (depth) {
          case 1:
            for (i=w; i--; from += delta) *to++ = *from;
            break;
          case 2:
            for (i=w; i--; from += delta, to += 4) {
              uchar a = from[1];
              uchar gray = (from[0]*a)>>8;
              to[0] = gray;
              to[1] = gray;
              to[2] = gray;
              to[3] = a;
            }
            break;
          case 3:
            for (i=w; i--; from += delta, to += 3) {
              uchar r = from[0];
              to[0] = from[2];
              to[1] = from[1];
              to[2] = r;
            }
            break;
          case 4:
            for (i=w; i--; from += delta, to += 4) {
              uchar a = from[3];
              uchar r = from[0];
              to[0] = (from[2]*a)>>8;
              to[1] = (from[1]*a)>>8;
              to[2] = (r*a)>>8;
              to[3] = from[3];
            }
            break;
        }
      }
    } // for (k = 0; j<h && k<blocking ...)

    if (fl_graphics_driver->has_feature(Fl_Graphics_Driver::PRINTER)) {
      // if print context, device and logical units are not equal, so SetDIBitsToDevice
      // does not do the expected job, whereas StretchDIBits does it.
      StretchDIBits(gc, x, y+j-k, w, k, 0, 0, w, k,
                    (LPSTR)((uchar*)buffer+(blocking-k)*linesize),
                    bmi,
#if USE_COLORMAP
                    indexed ? DIB_PAL_COLORS : DIB_RGB_COLORS
#else
                    DIB_RGB_COLORS
#endif
                    , SRCCOPY );
      delete[] buffer;
      buffer = NULL;
      buffer_size = 0;
    }
    else {
      SetDIBitsToDevice(gc, x, y+j-k, w, k, 0, 0, 0, k,
                        (LPSTR)((uchar*)buffer+(blocking-k)*linesize),
                        bmi,
#if USE_COLORMAP
                        indexed ? DIB_PAL_COLORS : DIB_RGB_COLORS
#else
                        DIB_RGB_COLORS
#endif
                        );
    }
  } // for (int j=0; j<h; )
}

void Fl_GDI_Graphics_Driver::draw_image_unscaled(const uchar* buf, int x, int y, int w, int h, int d, int l){
  if (fl_abs(d)&FL_IMAGE_WITH_ALPHA) {
    d ^= FL_IMAGE_WITH_ALPHA;
    innards(buf,x,y,w,h,d,l,fl_abs(d),0,0, gc_);
  } else {
    innards(buf,x,y,w,h,d,l,(d<3&&d>-3),0,0, gc_);
  }
}

void Fl_GDI_Graphics_Driver::draw_image_unscaled(Fl_Draw_Image_Cb cb, void* data,
                   int x, int y, int w, int h,int d) {
  if (fl_abs(d)&FL_IMAGE_WITH_ALPHA) {
    d ^= FL_IMAGE_WITH_ALPHA;
    innards(0,x,y,w,h,d,0,(d<3&&d>-3),cb,data, gc_);
  } else {
    innards(0,x,y,w,h,d,0,(d<3&&d>-3),cb,data, gc_);
  }
}

void Fl_GDI_Graphics_Driver::draw_image_mono_unscaled(const uchar* buf, int x, int y, int w, int h, int d, int l){
  if (fl_abs(d)&FL_IMAGE_WITH_ALPHA) {
    d ^= FL_IMAGE_WITH_ALPHA;
    innards(buf,x,y,w,h,d,l,1,0,0, gc_);
  } else {
    innards(buf,x,y,w,h,d,l,1,0,0, gc_);
  }
}

void Fl_GDI_Graphics_Driver::draw_image_mono_unscaled(Fl_Draw_Image_Cb cb, void* data,
                   int x, int y, int w, int h,int d) {
  if (fl_abs(d)&FL_IMAGE_WITH_ALPHA) {
    d ^= FL_IMAGE_WITH_ALPHA;
    innards(0,x,y,w,h,d,0,1,cb,data, gc_);
  } else {
    innards(0,x,y,w,h,d,0,1,cb,data, gc_);
  }
}

#if USE_COLORMAP
void Fl_GDI_Graphics_Driver::colored_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b) {
  // use the error diffusion dithering code to produce a much nicer block:
  if (fl_palette) {
    uchar c[3];
    c[0] = r; c[1] = g; c[2] = b;
    innards(c, floor(x), floor(y), floor(x + w) - floor(x), floor(y + h) - floor(y),
            0,0,0,0,0, (HDC)gc());
    return;
  }
  Fl_Graphics_Driver::colored_rectf(x, y, w, h, r, g, b);
}
#endif

// Create an N-bit bitmap for masking...
HBITMAP Fl_GDI_Graphics_Driver::create_bitmask(int w, int h, const uchar *data) {
  // this won't work when the user changes display mode during run or
  // has two screens with different depths
  HBITMAP bm;
  static uchar hiNibble[16] =
  { 0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0 };
  static uchar loNibble[16] =
  { 0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e,
    0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f };
  HDC current_gc = (HDC)Fl_Surface_Device::surface()->driver()->gc();
  int np  = GetDeviceCaps(current_gc, PLANES); //: was always one on sample machines
  int bpp = GetDeviceCaps(current_gc, BITSPIXEL);//: 1,4,8,16,24,32 and more odd stuff?
  int Bpr = (bpp*w+7)/8;                        //: bytes per row
  int pad = Bpr&1, w1 = (w+7)/8, shr = ((w-1)&7)+1;
  if (bpp==4) shr = (shr+1)/2;
  uchar *newarray = new uchar[(Bpr+pad)*h];
  uchar *dst = newarray;
  const uchar *src = data;

  for (int i=0; i<h; i++) {
    // This is slooow, but we do it only once per pixmap
    for (int j=w1; j>0; j--) {
      uchar b = *src++;
      if (bpp==1) {
        *dst++ = (uchar)( hiNibble[b&15] ) | ( loNibble[(b>>4)&15] );
      } else if (bpp==4) {
        for (int k=(j==1)?shr:4; k>0; k--) {
          *dst++ = (uchar)("\377\360\017\000"[b&3]);
          b = b >> 2;
        }
      } else {
        for (int k=(j==1)?shr:8; k>0; k--) {
          if (b&1) {
            *dst++=0;
            if (bpp>8) *dst++=0;
            if (bpp>16) *dst++=0;
            if (bpp>24) *dst++=0;
          } else {
            *dst++=0xff;
            if (bpp>8) *dst++=0xff;
            if (bpp>16) *dst++=0xff;
            if (bpp>24) *dst++=0xff;
          }

          b = b >> 1;
        }
      }
    }

    dst += pad;
  }

  bm = CreateBitmap(w, h, np, bpp, newarray);
  delete[] newarray;

  return bm;
}

void Fl_GDI_Graphics_Driver::delete_bitmask(fl_uintptr_t bm) {
  DeleteObject((HGDIOBJ)bm);
}

void Fl_GDI_Graphics_Driver::draw_fixed(Fl_Bitmap *bm, int X, int Y, int W, int H, int cx, int cy) {
  X = this->floor(X);
  Y = this->floor(Y);
  cache_size(bm, W, H);
  cx = this->floor(cx); cy = this->floor(cy);

  HDC tempdc = CreateCompatibleDC(gc_);
  int save = SaveDC(tempdc);
  SelectObject(tempdc, (HGDIOBJ)*Fl_Graphics_Driver::id(bm));
  SelectObject(gc_, fl_brush());
  // secret bitblt code found in old Windows reference manual:
  BitBlt(gc_, X, Y, W, H, tempdc, cx, cy, 0xE20746L);
  RestoreDC(tempdc, save);
  DeleteDC(tempdc);
}

Fl_GDI_Printer_Graphics_Driver::transparent_f_type Fl_GDI_Printer_Graphics_Driver::TransparentBlt() {
  HMODULE hMod;
  static transparent_f_type fpter = ( (hMod = LoadLibrary("MSIMG32.DLL")) ?
                                             (transparent_f_type)GetProcAddress(hMod, "TransparentBlt") : NULL
                                             );
  return fpter;
}

void Fl_GDI_Printer_Graphics_Driver::draw_bitmap(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  if (Fl_Graphics_Driver::start_image(bm, XP, YP, WP, HP, cx, cy, X, Y, W, H)) {
    return;
  }
  transparent_f_type fl_TransparentBlt = TransparentBlt();
  if (!fl_TransparentBlt) {
    Fl_Graphics_Driver::draw_bitmap(bm,  X,  Y,  W,  H,  cx,  cy);
    return;
  }
  bool recache = false;
  if (*id(bm)) {
    int *pw, *ph;
    cache_w_h(bm, pw, ph);
    recache = (*pw != bm->data_w() || *ph != bm->data_h());
  }
  if (recache || !*id(bm)) {
    bm->uncache();
    cache(bm);
  }
  HDC tempdc;
  int save;
  // algorithm for bitmap output to Fl_GDI_Printer
  Fl_Color save_c = fl_color(); // save bitmap's desired color
  uchar r, g, b;
  Fl::get_color(save_c, r, g, b);
  r = 255-r;
  g = 255-g;
  b = 255-b;
  Fl_Color background = fl_rgb_color(r, g, b); // a color very different from the bitmap's
  Fl_Image_Surface *img_surf = new Fl_Image_Surface(bm->data_w(), bm->data_h());
  Fl_Surface_Device::push_current(img_surf);
  fl_color(background);
  fl_rectf(0,0, bm->data_w(), bm->data_h()); // use this color as offscreen background
  fl_color(save_c); // back to bitmap's color
  HDC off_gc = (HDC)fl_graphics_driver->gc();
  tempdc = CreateCompatibleDC(off_gc);
  save = SaveDC(tempdc);
  SelectObject(tempdc, (HGDIOBJ)*Fl_Graphics_Driver::id(bm));
  SelectObject(off_gc, fl_brush()); // use bitmap's desired color
  BitBlt(off_gc, 0, 0, bm->data_w(), bm->data_h(), tempdc, 0, 0, 0xE20746L); // draw bitmap to offscreen
  Fl_Surface_Device::pop_current();
  SelectObject(tempdc, (HGDIOBJ)img_surf->offscreen()); // use offscreen data
                                         // draw it to printer context with background color as transparent
  float scaleW = bm->data_w()/float(bm->w());
  float scaleH = bm->data_h()/float(bm->h());
  fl_TransparentBlt(gc_, X, Y, W, H, tempdc,
                    int(cx * scaleW), int(cy * scaleH), int(W * scaleW), int(H * scaleH), RGB(r, g, b) );
  delete img_surf;
  RestoreDC(tempdc, save);
  DeleteDC(tempdc);
  if (recache) bm->uncache();
}


// Create a 1-bit mask used for alpha blending
HBITMAP Fl_GDI_Graphics_Driver::create_alphamask(int w, int h, int d, int ld, const uchar *array) {
  HBITMAP bm;
  int bmw = (w + 7) / 8;
  uchar *bitmap = new uchar[bmw * h];
  uchar *bitptr, bit;
  const uchar *dataptr;
  int x, y;
  static uchar dither[16][16] = { // Simple 16x16 Floyd dither
    { 0,   128, 32,  160, 8,   136, 40,  168,
      2,   130, 34,  162, 10,  138, 42,  170 },
    { 192, 64,  224, 96,  200, 72,  232, 104,
      194, 66,  226, 98,  202, 74,  234, 106 },
    { 48,  176, 16,  144, 56,  184, 24,  152,
      50,  178, 18,  146, 58,  186, 26,  154 },
    { 240, 112, 208, 80,  248, 120, 216, 88,
      242, 114, 210, 82,  250, 122, 218, 90 },
    { 12,  140, 44,  172, 4,   132, 36,  164,
      14,  142, 46,  174, 6,   134, 38,  166 },
    { 204, 76,  236, 108, 196, 68,  228, 100,
      206, 78,  238, 110, 198, 70,  230, 102 },
    { 60,  188, 28,  156, 52,  180, 20,  148,
      62,  190, 30,  158, 54,  182, 22,  150 },
    { 252, 124, 220, 92,  244, 116, 212, 84,
      254, 126, 222, 94,  246, 118, 214, 86 },
    { 3,   131, 35,  163, 11,  139, 43,  171,
      1,   129, 33,  161, 9,   137, 41,  169 },
    { 195, 67,  227, 99,  203, 75,  235, 107,
      193, 65,  225, 97,  201, 73,  233, 105 },
    { 51,  179, 19,  147, 59,  187, 27,  155,
      49,  177, 17,  145, 57,  185, 25,  153 },
    { 243, 115, 211, 83,  251, 123, 219, 91,
      241, 113, 209, 81,  249, 121, 217, 89 },
    { 15,  143, 47,  175, 7,   135, 39,  167,
      13,  141, 45,  173, 5,   133, 37,  165 },
    { 207, 79,  239, 111, 199, 71,  231, 103,
      205, 77,  237, 109, 197, 69,  229, 101 },
    { 63,  191, 31,  159, 55,  183, 23,  151,
      61,  189, 29,  157, 53,  181, 21,  149 },
    { 254, 127, 223, 95,  247, 119, 215, 87,
      253, 125, 221, 93,  245, 117, 213, 85 }
  };

  // Generate a 1-bit "screen door" alpha mask; not always pretty, but
  // definitely fast...  In the future we may be able to support things
  // like the RENDER extension in XFree86, when available, to provide
  // true RGBA-blended rendering.  See:
  //
  //     http://www.xfree86.org/~keithp/render/protocol.html
  //
  // for more info on XRender...
  //
  memset(bitmap, 0, bmw * h);

  for (dataptr = array + d - 1, y = 0; y < h; y ++, dataptr += ld)
    for (bitptr = bitmap + y * bmw, bit = 1, x = 0; x < w; x ++, dataptr += d) {
      if (*dataptr > dither[x & 15][y & 15])
        *bitptr |= bit;
      if (bit < 128) bit <<= 1;
      else {
        bit = 1;
        bitptr ++;
      }
    }

  bm = create_bitmask(w, h, bitmap);
  delete[] bitmap;

  return bm;
}


void Fl_GDI_Graphics_Driver::cache(Fl_RGB_Image *img)
{
  Fl_Image_Surface *surface = new Fl_Image_Surface(img->data_w(), img->data_h());
  Fl_Surface_Device::push_current(surface);
  if ((img->d() == 2 || img->d() == 4) && fl_can_do_alpha_blending()) {
    fl_draw_image(img->array, 0, 0, img->data_w(), img->data_h(), img->d()|FL_IMAGE_WITH_ALPHA, img->ld());
  } else {
    fl_draw_image(img->array, 0, 0, img->data_w(), img->data_h(), img->d(), img->ld());
    if (img->d() == 2 || img->d() == 4) {
      *Fl_Graphics_Driver::mask(img) = (fl_uintptr_t)create_alphamask(img->data_w(), img->data_h(), img->d(), img->ld(), img->array);
    }
  }
  Fl_Surface_Device::pop_current();
  Fl_Offscreen offs = Fl_Graphics_Driver::get_offscreen_and_delete_image_surface(surface);
  int *pw, *ph;
  cache_w_h(img, pw, ph);
  *pw = img->data_w();
  *ph = img->data_h();
  *Fl_Graphics_Driver::id(img) = (fl_uintptr_t)offs;
}


void Fl_GDI_Graphics_Driver::draw_fixed(Fl_RGB_Image *img, int X, int Y, int W, int H, int cx, int cy) {
  X = this->floor(X);
  Y = this->floor(Y);
  cache_size(img, W, H);
  cx = this->floor(cx); cy = this->floor(cy);
  if (W + cx > img->data_w()) W = img->data_w() - cx;
  if (H + cy > img->data_h()) H = img->data_h() - cy;
  if (!*Fl_Graphics_Driver::id(img)) {
    cache(img);
  }
  if (*Fl_Graphics_Driver::mask(img)) {
    HDC new_gc = CreateCompatibleDC(gc_);
    int save = SaveDC(new_gc);
    SelectObject(new_gc, (void*)*Fl_Graphics_Driver::mask(img));
    BitBlt(gc_, X, Y, W, H, new_gc, cx, cy, SRCAND);
    SelectObject(new_gc, (void*)*Fl_Graphics_Driver::id(img));
    BitBlt(gc_, X, Y, W, H, new_gc, cx, cy, SRCPAINT);
    RestoreDC(new_gc,save);
    DeleteDC(new_gc);
  } else if (img->d()==2 || img->d()==4) {
    copy_offscreen_with_alpha(X, Y, W, H, (HBITMAP)*Fl_Graphics_Driver::id(img), cx, cy);
  } else {
    copy_offscreen(X, Y, W, H, (Fl_Offscreen)*Fl_Graphics_Driver::id(img), cx, cy);
  }
}


void Fl_GDI_Graphics_Driver::draw_rgb(Fl_RGB_Image *rgb, int XP, int YP, int WP, int HP, int cx, int cy) {
  if (Fl_Graphics_Driver::start_image(rgb, XP, YP, WP, HP, cx, cy, XP, YP, WP, HP)) {
    return;
  }
  if ((rgb->d() % 2) == 0 && !fl_can_do_alpha_blending()) {
    Fl_Graphics_Driver::draw_rgb(rgb, XP, YP, WP, HP, cx, cy);
    return;
  }
  if (!*Fl_Graphics_Driver::id(rgb)) {
    cache(rgb);
  }
  push_clip(XP, YP, WP, HP);
  XP -= cx; YP -= cy;
  WP = rgb->w(); HP = rgb->h();
  cache_size(rgb, WP, HP);
  HDC new_gc = CreateCompatibleDC(gc_);
  int save = SaveDC(new_gc);
  SelectObject(new_gc, (HBITMAP)*Fl_Graphics_Driver::id(rgb));
  if ( (rgb->d() % 2) == 0 ) {
    alpha_blend_(this->floor(XP), this->floor(YP), WP, HP, new_gc, 0, 0, rgb->data_w(), rgb->data_h());
  } else {
    SetStretchBltMode(gc_, (Fl_Image::scaling_algorithm() == FL_RGB_SCALING_BILINEAR ? HALFTONE : BLACKONWHITE));
    StretchBlt(gc_, this->floor(XP), this->floor(YP), WP, HP, new_gc, 0, 0, rgb->data_w(), rgb->data_h(), SRCCOPY);
  }
  RestoreDC(new_gc, save);
  DeleteDC(new_gc);
  pop_clip();
}


void Fl_GDI_Printer_Graphics_Driver::draw_rgb(Fl_RGB_Image *rgb, int XP, int YP, int WP, int HP, int cx, int cy) {
  if (Fl_Graphics_Driver::start_image(rgb, XP, YP, WP, HP, cx, cy, XP, YP, WP, HP)) {
    return;
  }
  XFORM old_tr, tr;
  GetWorldTransform(gc_, &old_tr); // storing old transform
  tr.eM11 = float(rgb->w())/float(rgb->data_w());
  tr.eM22 = float(rgb->h())/float(rgb->data_h());
  tr.eM12 = tr.eM21 = 0;
  tr.eDx =  float(XP);
  tr.eDy =  float(YP);
  ModifyWorldTransform(gc_, &tr, MWT_LEFTMULTIPLY);
  if (*id(rgb)) {
    int *pw, *ph;
    cache_w_h(rgb, pw, ph);
    if ( *pw != rgb->data_w() ||  *ph != rgb->data_h()) rgb->uncache();
  }
  if (!*id(rgb)) cache(rgb);
  draw_fixed(rgb, 0, 0, int(WP / tr.eM11), int(HP / tr.eM22), int(cx / tr.eM11), int(cy / tr.eM22));
  SetWorldTransform(gc_, &old_tr);
}


void Fl_GDI_Graphics_Driver::uncache(Fl_RGB_Image*, fl_uintptr_t &id_, fl_uintptr_t &mask_)
{
  if (id_) {
    DeleteObject((HBITMAP)id_);
    id_ = 0;
  }

  if (mask_) {
    delete_bitmask(mask_);
    mask_ = 0;
  }
}

// 'fl_create_bitmap()' - Create a 1-bit bitmap for drawing...
static HBITMAP fl_create_bitmap(int w, int h, const uchar *data) {
  // we need to pad the lines out to words & swap the bits
  // in each byte.
  int w1 = (w + 7) / 8;
  int w2 = ((w + 15) / 16) * 2;
  uchar* newarray = new uchar[w2*h];
  const uchar* src = data;
  uchar* dest = newarray;
  HBITMAP bm;
  static uchar reverse[16] =    /* Bit reversal lookup table */
  { 0x00, 0x88, 0x44, 0xcc, 0x22, 0xaa, 0x66, 0xee,
    0x11, 0x99, 0x55, 0xdd, 0x33, 0xbb, 0x77, 0xff };

  for (int y = 0; y < h; y++) {
    for (int n = 0; n < w1; n++, src++)
      *dest++ = (uchar)((reverse[*src & 0x0f] & 0xf0) |
                        (reverse[(*src >> 4) & 0x0f] & 0x0f));
    dest += w2 - w1;
  }

  bm = CreateBitmap(w, h, 1, 1, newarray);

  delete[] newarray;

  return bm;
}

void Fl_GDI_Graphics_Driver::cache(Fl_Bitmap *bm) {
  int *pw, *ph;
  cache_w_h(bm, pw, ph);
  *pw = bm->data_w();
  *ph = bm->data_h();
  *Fl_Graphics_Driver::id(bm) = (fl_uintptr_t)fl_create_bitmap(bm->data_w(), bm->data_h(), bm->array);
}

void Fl_GDI_Graphics_Driver::draw_fixed(Fl_Pixmap *pxm, int X, int Y, int W, int H, int cx, int cy) {
  X = this->floor(X);
  Y = this->floor(Y);
  cache_size(pxm, W, H);
  cx = this->floor(cx); cy = this->floor(cy);
  Fl_Region r2 = scale_clip(scale());
  if (*Fl_Graphics_Driver::mask(pxm)) {
    HDC new_gc = CreateCompatibleDC(gc_);
    int save = SaveDC(new_gc);
    SelectObject(new_gc, (void*)*Fl_Graphics_Driver::mask(pxm));
    BitBlt(gc_, X, Y, W, H, new_gc, cx, cy, SRCAND);
    SelectObject(new_gc, (void*)*Fl_Graphics_Driver::id(pxm));
    BitBlt(gc_, X, Y, W, H, new_gc, cx, cy, SRCPAINT);
    RestoreDC(new_gc,save);
    DeleteDC(new_gc);
  } else {
    float s = scale(); Fl_Graphics_Driver::scale(1);
    copy_offscreen(X, Y, W, H, (Fl_Offscreen)*Fl_Graphics_Driver::id(pxm), cx, cy);
    Fl_Graphics_Driver::scale(s);
  }
  unscale_clip(r2);
}

/*  ===== Implementation note about how Fl_Pixmap objects get printed under Windows =====
 Fl_Pixmap objects are printed with the print-specific Fl_GDI_Printer_Graphics_Driver
 which uses the TransparentBlt() system function that can scale the image and treat one
 of its colors as transparent.
 Fl_GDI_Printer_Graphics_Driver::draw_pixmap(Fl_Pixmap *,...) sets need_pixmap_bg_color,
 a static class variable, to 1 and recaches the image. This calls fl_convert_pixmap()
 that checks the value of need_pixmap_bg_color. When this value is not 0, fl_convert_pixmap
 runs in a way that memorizes the list of all colors in the pixmap, computes
 a color absent from this list, uses it for the transparent pixels of the pixmap and puts
 this color value in need_pixmap_bg_color. As a result, the transparent areas of the image
 are correcty handled by the printing operation. Variable need_pixmap_bg_color is ultimately
 reset to 0.
 Fl_GDI_Graphics_Driver::make_unused_color_() which does the color computation mentioned
 above is implemented in file src/fl_draw_pixmap.cxx
 */
void Fl_GDI_Printer_Graphics_Driver::draw_pixmap(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  if (start_image(pxm, XP, YP, WP, HP, cx, cy, X, Y, W, H)) return;
  transparent_f_type fl_TransparentBlt = TransparentBlt();
  if (fl_TransparentBlt) {
    need_pixmap_bg_color = 1;
    pxm->uncache();
    cache(pxm);
    HDC new_gc = CreateCompatibleDC(gc_);
    int save = SaveDC(new_gc);
    SelectObject(new_gc, (void*)*Fl_Graphics_Driver::id(pxm));
    // print all of offscreen but its parts in background color
    float scaleW = pxm->data_w()/float(pxm->w());
    float scaleH = pxm->data_h()/float(pxm->h());
    fl_TransparentBlt(gc_, X, Y, W, H, new_gc,
                      int(cx * scaleW), int(cy * scaleH), int(W * scaleW), int(H * scaleH), need_pixmap_bg_color );
    RestoreDC(new_gc,save);
    DeleteDC(new_gc);
    need_pixmap_bg_color = 0;
  }
  else {
    copy_offscreen(X, Y, W, H, (Fl_Offscreen)*Fl_Graphics_Driver::id(pxm), cx, cy);
  }
}

// Makes an RGB triplet different from all the colors used in the pixmap
// and computes Fl_Graphics_Driver::need_pixmap_bg_color from this triplet
void Fl_GDI_Graphics_Driver::make_unused_color_(uchar &r, uchar &g, uchar &b, int color_count, void **data) {
  typedef struct { uchar r; uchar g; uchar b; } UsedColor;
  UsedColor *used_colors = *(UsedColor**)data;
  int i;
  r = 2; g = 3; b = 4;
  while (1) {
    for ( i=0; i<color_count; i++ )
      if ( used_colors[i].r == r &&
           used_colors[i].g == g &&
           used_colors[i].b == b )
        break;
    if (i >= color_count) {
      free((void*)used_colors);
      *(UsedColor**)data = NULL;
      need_pixmap_bg_color = RGB(r, g, b);
      return;
    }
    if (r < 255) {
      r++;
    } else {
      r = 0;
      if (g < 255) {
        g++;
      } else {
        g = 0;
        b++;
      }
    }
  }
}

void Fl_GDI_Graphics_Driver::cache(Fl_Pixmap *img) {
  Fl_Image_Surface *surf = new Fl_Image_Surface(img->data_w(), img->data_h());
  Fl_Surface_Device::push_current(surf);
  uchar **pbitmap = surf->driver()->mask_bitmap();
  *pbitmap = (uchar*)1;// will instruct fl_draw_pixmap() to compute the image's mask
  fl_draw_pixmap(img->data(), 0, 0, FL_BLACK);
  uchar *bitmap = *pbitmap;
  if (bitmap) {
    *Fl_Graphics_Driver::mask(img) =
      (fl_uintptr_t)create_bitmask(img->data_w(), img->data_h(), bitmap);
    delete[] bitmap;
  }
  *pbitmap = 0;
  Fl_Surface_Device::pop_current();
  Fl_Offscreen id = Fl_Graphics_Driver::get_offscreen_and_delete_image_surface(surf);
  int *pw, *ph;
  cache_w_h(img, pw, ph);
  *pw = img->data_w();
  *ph = img->data_h();
  *Fl_Graphics_Driver::id(img) = (fl_uintptr_t)id;
}

void Fl_GDI_Graphics_Driver::uncache_pixmap(fl_uintptr_t offscreen) {
  DeleteObject((HBITMAP)offscreen);
}
