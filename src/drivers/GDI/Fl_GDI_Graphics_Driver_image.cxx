//
// "$Id$"
//
// WIN32 image drawing code for the Fast Light Tool Kit (FLTK).
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
#include "Fl_GDI_Graphics_Driver.h"
#include <FL/Fl.H>
#include <FL/Fl_Printer.H>
#include <FL/fl_draw.H>
#include <FL/x.H>

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

  if (!linedelta) linedelta = W*delta;

  int x, y, w, h;
  fl_clip_box(X,Y,W,H,x,y,w,h);
  if (w<=0 || h<=0) return;
  if (buf) buf += (x-X)*delta + (y-Y)*linedelta;

  static U32 bmibuffer[256+12];
  BITMAPINFO &bmi = *((BITMAPINFO*)bmibuffer);
  if (!bmi.bmiHeader.biSize) {
    bmi.bmiHeader.biSize = sizeof(bmi)-4; // does it use this to determine type?
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biXPelsPerMeter = 0;
    bmi.bmiHeader.biYPelsPerMeter = 0;
    bmi.bmiHeader.biClrUsed = 0;
    bmi.bmiHeader.biClrImportant = 0;
  }
#if USE_COLORMAP
  if (indexed) {
    for (short i=0; i<256; i++) {
      *((short*)(bmi.bmiColors)+i) = i;
    }
  } else
#endif
  if (depth<3) {
    RGBQUAD *bmi_colors = &bmi.bmiColors[0];	// suppress warning (STR #3199)
    for (int i=0; i<256; i++) {
      bmi_colors[i].rgbBlue = (uchar)i;		// bmi.bmiColors[i]...
      bmi_colors[i].rgbGreen = (uchar)i;
      bmi_colors[i].rgbRed = (uchar)i;
      bmi_colors[i].rgbReserved = (uchar)0; // must be zero
    }
  }
  bmi.bmiHeader.biWidth = w;
#if USE_COLORMAP
  bmi.bmiHeader.biBitCount = indexed ? 8 : depth*8;
  int pixelsize = indexed ? 1 : depth;
#else
  bmi.bmiHeader.biBitCount = depth*8;
  int pixelsize = depth;
#endif
  if (depth==2) { // special case: gray with alpha
    bmi.bmiHeader.biBitCount = 32;
    pixelsize = 4;
  }
  int linesize = (pixelsize*w+3)&~3;
  
  static U32* buffer;
  static long buffer_size;
  int blocking = h;
  {int size = linesize*h;
  // when printing, don't limit buffer size not to get a crash in StretchDIBits
  if (size > MAXBUFFER && Fl_Surface_Device::surface() == Fl_Display_Device::display_device()) {
    size = MAXBUFFER;
    blocking = MAXBUFFER/linesize;
  }
  if (size > buffer_size) {
    delete[] buffer;
    buffer_size = size;
    buffer = new U32[(size+3)/4];
  }}
  bmi.bmiHeader.biHeight = blocking;
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
	to += w;
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
    }
    if (Fl_Surface_Device::surface() != Fl_Display_Device::display_device()) {
      // if print context, device and logical units are not equal, so SetDIBitsToDevice
      // does not do the expected job, whereas StretchDIBits does it.
      StretchDIBits(gc, x, y+j-k, w, k, 0, 0, w, k,
		    (LPSTR)((uchar*)buffer+(blocking-k)*linesize),
		    &bmi,
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
			&bmi,
#if USE_COLORMAP
			indexed ? DIB_PAL_COLORS : DIB_RGB_COLORS
#else
			DIB_RGB_COLORS
#endif
			);
      }
  }
}

static int fl_abs(int v) { return v<0 ? -v : v; }

void Fl_GDI_Graphics_Driver::draw_image(const uchar* buf, int x, int y, int w, int h, int d, int l){
  if (fl_abs(d)&FL_IMAGE_WITH_ALPHA) {
    d ^= FL_IMAGE_WITH_ALPHA;
    innards(buf,x,y,w,h,d,l,fl_abs(d),0,0, gc_);
  } else {
    innards(buf,x,y,w,h,d,l,(d<3&&d>-3),0,0, gc_);
  }
}

void Fl_GDI_Graphics_Driver::draw_image(Fl_Draw_Image_Cb cb, void* data,
		   int x, int y, int w, int h,int d) {
  if (fl_abs(d)&FL_IMAGE_WITH_ALPHA) {
    d ^= FL_IMAGE_WITH_ALPHA;
    innards(0,x,y,w,h,d,0,(d<3&&d>-3),cb,data, gc_);
  } else {
    innards(0,x,y,w,h,d,0,(d<3&&d>-3),cb,data, gc_);
  }
}

void Fl_GDI_Graphics_Driver::draw_image_mono(const uchar* buf, int x, int y, int w, int h, int d, int l){
  if (fl_abs(d)&FL_IMAGE_WITH_ALPHA) {
    d ^= FL_IMAGE_WITH_ALPHA;
    innards(buf,x,y,w,h,d,l,1,0,0, gc_);
  } else {
    innards(buf,x,y,w,h,d,l,1,0,0, gc_);
  }
}

void Fl_GDI_Graphics_Driver::draw_image_mono(Fl_Draw_Image_Cb cb, void* data,
		   int x, int y, int w, int h,int d) {
  if (fl_abs(d)&FL_IMAGE_WITH_ALPHA) {
    d ^= FL_IMAGE_WITH_ALPHA;
    innards(0,x,y,w,h,d,0,1,cb,data, gc_);
  } else {
    innards(0,x,y,w,h,d,0,1,cb,data, gc_);
  }
}

void fl_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b) {
#if USE_COLORMAP
  // use the error diffusion dithering code to produce a much nicer block:
  if (fl_palette) {
    uchar c[3];
    c[0] = r; c[1] = g; c[2] = b;
    innards(c,x,y,w,h,0,0,0,0,0,(HDC)fl_graphics_driver->gc());
    return;
  }
#endif
  fl_color(r,g,b);
  fl_rectf(x,y,w,h);
}

// 'fl_create_bitmask()' - Create an N-bit bitmap for masking...
Fl_Bitmask Fl_GDI_Graphics_Driver::create_bitmask(int w, int h, const uchar *data) {
  // this won't work when the user changes display mode during run or
  // has two screens with differnet depths
  Fl_Bitmask bm;
  static uchar hiNibble[16] =
  { 0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0 };
  static uchar loNibble[16] =
  { 0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e,
    0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f };
  int np  = GetDeviceCaps(gc_, PLANES);	//: was always one on sample machines
  int bpp = GetDeviceCaps(gc_, BITSPIXEL);//: 1,4,8,16,24,32 and more odd stuff?
  int Bpr = (bpp*w+7)/8;			//: bytes per row
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

void Fl_GDI_Graphics_Driver::delete_bitmask(Fl_Bitmask bm) {
  DeleteObject((HGDIOBJ)bm);
}

void Fl_GDI_Graphics_Driver::draw(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  if (bm->start(XP, YP, WP, HP, cx, cy, X, Y, W, H)) {
    return;
  }

  HDC tempdc = CreateCompatibleDC(gc_);
  int save = SaveDC(tempdc);
  SelectObject(tempdc, (HGDIOBJ)bm->id_);
  SelectObject(gc_, fl_brush());
  // secret bitblt code found in old MSWindows reference manual:
  BitBlt(gc_, X, Y, W, H, tempdc, cx, cy, 0xE20746L);
  RestoreDC(tempdc, save);
  DeleteDC(tempdc);
}

// TODO: move this into a file with the printer implementations
void Fl_GDI_Printer_Graphics_Driver::draw(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  typedef BOOL (WINAPI* fl_transp_func)  (HDC,int,int,int,int,HDC,int,int,int,int,UINT);
  static fl_transp_func fl_TransparentBlt = NULL;
  static HMODULE hMod = NULL;
  if (!hMod) {
    hMod = LoadLibrary("MSIMG32.DLL");
    if (hMod) fl_TransparentBlt = (fl_transp_func)GetProcAddress(hMod, "TransparentBlt");
  }
  if (!fl_TransparentBlt) {
    Fl_GDI_Graphics_Driver::draw(bm,  XP,  YP,  WP,  HP,  cx,  cy);
    return;
  }
  if (bm->start(XP, YP, WP, HP, cx, cy, X, Y, W, H)) {
    return;
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
  Fl_Offscreen tmp_id = fl_create_offscreen(W, H);
  fl_begin_offscreen(tmp_id);
  fl_color(background);
  fl_rectf(0,0,W,H); // use this color as offscreen background
  fl_color(save_c); // back to bitmap's color
  HDC off_gc = (HDC)fl_graphics_driver->gc();
  tempdc = CreateCompatibleDC(off_gc);
  save = SaveDC(tempdc);
  SelectObject(tempdc, (HGDIOBJ)bm->id_);
  SelectObject(off_gc, fl_brush()); // use bitmap's desired color
  BitBlt(off_gc, 0, 0, W, H, tempdc, 0, 0, 0xE20746L); // draw bitmap to offscreen
  fl_end_offscreen(); // offscreen data is in tmp_id
  SelectObject(tempdc, (HGDIOBJ)tmp_id); // use offscreen data
                                         // draw it to printer context with background color as transparent
  fl_TransparentBlt(gc_, X,Y,W,H, tempdc, cx, cy, bm->w(), bm->h(), RGB(r, g, b) );
  fl_delete_offscreen(tmp_id);
  RestoreDC(tempdc, save);
  DeleteDC(tempdc);
}  


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
  if (!img->id_) img->id_ = (fl_uintptr_t)build_id(img, (void**)&(img->mask_));
  if (img->mask_) {
    HDC new_gc = CreateCompatibleDC(gc_);
    int save = SaveDC(new_gc);
    SelectObject(new_gc, (void*)img->mask_);
    BitBlt(gc_, X, Y, W, H, new_gc, cx, cy, SRCAND);
    SelectObject(new_gc, (void*)img->id_);
    BitBlt(gc_, X, Y, W, H, new_gc, cx, cy, SRCPAINT);
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
  GetWorldTransform(gc_, &old_tr); // storing old transform
  tr.eM11 = float(WP)/float(img->w());
  tr.eM22 = float(HP)/float(img->h());
  tr.eM12 = tr.eM21 = 0;
  tr.eDx =  float(XP);
  tr.eDy =  float(YP);
  ModifyWorldTransform(gc_, &tr, MWT_LEFTMULTIPLY);
  img->draw(0, 0, img->w(), img->h(), 0, 0);
  SetWorldTransform(gc_, &old_tr);
  return 1;
}

void Fl_GDI_Graphics_Driver::uncache(Fl_RGB_Image*, fl_uintptr_t &id_, fl_uintptr_t &mask_)
{
  if (id_) {
    fl_delete_offscreen((Fl_Offscreen)id_);
    id_ = 0;
  }

  if (mask_) {
    fl_delete_bitmask((Fl_Bitmask)mask_);
    mask_ = 0;
  }
}

// 'fl_create_bitmap()' - Create a 1-bit bitmap for drawing...
static Fl_Bitmask fl_create_bitmap(int w, int h, const uchar *data) {
  // we need to pad the lines out to words & swap the bits
  // in each byte.
  int w1 = (w + 7) / 8;
  int w2 = ((w + 15) / 16) * 2;
  uchar* newarray = new uchar[w2*h];
  const uchar* src = data;
  uchar* dest = newarray;
  Fl_Bitmask bm;
  static uchar reverse[16] =	/* Bit reversal lookup table */
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

fl_uintptr_t Fl_GDI_Graphics_Driver::cache(Fl_Bitmap*, int w, int h, const uchar *array) {
  return (fl_uintptr_t)fl_create_bitmap(w, h, array);
}

void Fl_GDI_Graphics_Driver::uncache(Fl_Bitmap *img, fl_uintptr_t &id_) {
  delete_bitmask((Fl_Offscreen)id_);
}

void Fl_GDI_Graphics_Driver::draw(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  if (pxm->prepare(XP, YP, WP, HP, cx, cy, X, Y, W, H)) return;
  if (pxm->mask_) {
    HDC new_gc = CreateCompatibleDC(gc_);
    int save = SaveDC(new_gc);
    SelectObject(new_gc, (void*)pxm->mask_);
    BitBlt(gc_, X, Y, W, H, new_gc, cx, cy, SRCAND);
    SelectObject(new_gc, (void*)pxm->id_);
    BitBlt(gc_, X, Y, W, H, new_gc, cx, cy, SRCPAINT);
    RestoreDC(new_gc,save);
    DeleteDC(new_gc);
  } else {
    copy_offscreen(X, Y, W, H, (Fl_Offscreen)pxm->id_, cx, cy);
  }
}


void Fl_GDI_Printer_Graphics_Driver::draw(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  if (pxm->prepare(XP, YP, WP, HP, cx, cy, X, Y, W, H)) return;
  typedef BOOL (WINAPI* fl_transp_func)  (HDC,int,int,int,int,HDC,int,int,int,int,UINT);
  static HMODULE hMod = NULL;
  static fl_transp_func fl_TransparentBlt = NULL;
  if (!hMod) {
    hMod = LoadLibrary("MSIMG32.DLL");
    if(hMod) fl_TransparentBlt = (fl_transp_func)GetProcAddress(hMod, "TransparentBlt");
  }
  if (fl_TransparentBlt) {
    HDC new_gc = CreateCompatibleDC(gc_);
    int save = SaveDC(new_gc);
    SelectObject(new_gc, (void*)pxm->id_);
    // print all of offscreen but its parts in background color
    fl_TransparentBlt(gc_, X, Y, W, H, new_gc, cx, cy, W, H, pxm->pixmap_bg_color );
    RestoreDC(new_gc,save);
    DeleteDC(new_gc);
  }
  else {
    copy_offscreen(X, Y, W, H, (Fl_Offscreen)pxm->id_, cx, cy);
  }
}

extern uchar **fl_mask_bitmap;

fl_uintptr_t Fl_GDI_Graphics_Driver::cache(Fl_Pixmap *img, int w, int h, const char *const*data) {
  Fl_Offscreen id;
  id = fl_create_offscreen(w, h);
  fl_begin_offscreen(id);
  uchar *bitmap = 0;
  fl_mask_bitmap = &bitmap;
  fl_draw_pixmap(data, 0, 0, FL_BLACK);
  extern UINT win_pixmap_bg_color; // computed by fl_draw_pixmap()
  img->pixmap_bg_color = win_pixmap_bg_color;
  fl_mask_bitmap = 0;
  if (bitmap) {
    img->mask_ = (fl_uintptr_t)fl_create_bitmask(w, h, bitmap);
    delete[] bitmap;
  }
  fl_end_offscreen();
  return (fl_uintptr_t)id;
}

//
// End of "$Id$".
//
