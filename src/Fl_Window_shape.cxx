//
// "$Id$"
//
// Implementation of Fl_Window::shape(Fl_Image*) for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2015 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#include <config.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Bitmap.H>
#include <FL/Fl_Pixmap.H>
#include <string.h>

#ifdef WIN32
# include <malloc.h> // needed for VisualC2010
#elif !defined(__APPLE__)
#include <config.h>
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#define ShapeBounding			0
#define ShapeSet			0
#endif


#if defined(__APPLE__)

static void MyProviderReleaseData (void *info, const void *data, size_t size) {
  delete[] (uchar*)data;
}

// bitwise inversion of all 4-bit quantities
static const unsigned char swapped[16] = {0,8,4,12,2,10,6,14,1,9,5,13,3,11,7,15};

static inline uchar swap_byte(const uchar b) {
  // reverse the order of bits of byte b: 1->8 becomes 8->1
  return (swapped[b & 0xF] << 4) | swapped[b >> 4];
}

#elif defined(WIN32)

static inline BYTE bit(int x) { return (BYTE)(1 << (x%8)); }

static HRGN bitmap2region(Fl_Image* image) {
  HRGN hRgn = 0;
  /* Does this need to be dynamically determined, perhaps? */
  const int ALLOC_UNIT = 100;
  DWORD maxRects = ALLOC_UNIT;
  
  RGNDATA* pData = (RGNDATA*)malloc(sizeof(RGNDATAHEADER)+(sizeof(RECT)*maxRects));
  pData->rdh.dwSize = sizeof(RGNDATAHEADER);
  pData->rdh.iType = RDH_RECTANGLES;
  pData->rdh.nCount = pData->rdh.nRgnSize = 0;
  SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
  
  const int bytesPerLine = (image->w() + 7)/8;
  BYTE* p, *data = (BYTE*)*image->data();
  for (int y = 0; y < image->h(); y++) {
    // each row, left to right
    for (int x = 0; x < image->w(); x++) {
      int x0 = x;
      while (x < image->w()) {
	p = data + x / 8;
	if (!((*p) & bit(x))) break; // transparent pixel
	x++;
      }
      if (x > x0) {
	RECT *pr;
	/* Add the pixels (x0, y) to (x, y+1) as a new rectangle
	 * in the region
	 */
	if (pData->rdh.nCount >= maxRects) {
	  maxRects += ALLOC_UNIT;
	  pData = (RGNDATA*)realloc(pData, sizeof(RGNDATAHEADER)
				    + (sizeof(RECT)*maxRects));
	}
	pr = (RECT*)&pData->Buffer;
	SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
	if (x0 < pData->rdh.rcBound.left)
	  pData->rdh.rcBound.left = x0;
	if (y < pData->rdh.rcBound.top)
	  pData->rdh.rcBound.top = y;
	if (x > pData->rdh.rcBound.right)
	  pData->rdh.rcBound.right = x;
	if (y+1 > pData->rdh.rcBound.bottom)
	  pData->rdh.rcBound.bottom = y+1;
	pData->rdh.nCount++;
	/* On Windows98, ExtCreateRegion() may fail if the
	 * number of rectangles is too large (ie: >
	 * 4000). Therefore, we have to create the region by
	 * multiple steps.
	 */
	if (pData->rdh.nCount == 2000) {
	  HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER)
				   + (sizeof(RECT)*maxRects), pData);
	  if (hRgn) {
	    CombineRgn(hRgn, hRgn, h, RGN_OR);
	    DeleteObject(h);
	  } else 
	    hRgn = h;
	  pData->rdh.nCount = 0;
	  SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
	}
      }
    }
    /* Go to next row */
    data += bytesPerLine;
  }
  /* Create or extend the region with the remaining rectangles*/
  HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER)
			   + (sizeof(RECT)*maxRects), pData);
  if (hRgn) {
    CombineRgn(hRgn, hRgn, h, RGN_OR);
    DeleteObject(h);
  } else hRgn = h;
  free(pData); // I've created the region so I can free this now, right?
  return hRgn;
}

#else

#ifndef FL_DOXYGEN
void Fl_Window::combine_mask()
{
  typedef void (*XShapeCombineMask_type)(Display*, int, int, int, int, Pixmap, int);
  static XShapeCombineMask_type XShapeCombineMask_f = NULL;
  static int beenhere = 0;
  typedef Bool (*XShapeQueryExtension_type)(Display*, int*, int*);
  if (!beenhere) {
    beenhere = 1;
#if HAVE_DLSYM && HAVE_DLFCN_H
    fl_open_display();
    void *handle = dlopen(NULL, RTLD_LAZY); // search symbols in executable
    XShapeQueryExtension_type XShapeQueryExtension_f = (XShapeQueryExtension_type)dlsym(handle, "XShapeQueryExtension");
    XShapeCombineMask_f = (XShapeCombineMask_type)dlsym(handle, "XShapeCombineMask");
    // make sure that the X server has the SHAPE extension
    int error_base, shapeEventBase;
    if ( !( XShapeQueryExtension_f && XShapeCombineMask_f &&
	   XShapeQueryExtension_f(fl_display, &shapeEventBase, &error_base) ) ) XShapeCombineMask_f = NULL;
#endif
  }
  if (!XShapeCombineMask_f) return;
  shape_data_->lw_ = w();
  shape_data_->lh_ = h();
  Fl_Image* temp = shape_data_->shape_->copy(shape_data_->lw_, shape_data_->lh_);
  Pixmap pbitmap = XCreateBitmapFromData(fl_display, fl_xid(this),
					 (const char*)*temp->data(),
					 temp->w(), temp->h());
  XShapeCombineMask_f(fl_display, fl_xid(this), ShapeBounding, 0, 0, pbitmap, ShapeSet);
  if (pbitmap != None) XFreePixmap(fl_display, pbitmap);
  delete temp;
}
#endif // !FL_DOXYGEN

#endif // __APPLE__


void Fl_Window::shape_bitmap_(Fl_Image* b) {
  shape_data_->shape_ = b;
#if defined(__APPLE__)
  if (b) {
    // complement mask bits and perform bitwise inversion of all bytes and also reverse top and bottom
    int bytes_per_row = (b->w() + 7)/8;
    uchar *from = new uchar[bytes_per_row * b->h()];
    for (int i = 0; i < b->h(); i++) {
      uchar *p = (uchar*)(*b->data()) + bytes_per_row * i;
      uchar *last = p + bytes_per_row;
      uchar *q = from + (b->h() - 1 - i) * bytes_per_row;
      while (p < last) {
        *q++ = swap_byte(~*p++);
      }
    }
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, from, bytes_per_row * b->h(), MyProviderReleaseData);
    shape_data_->mask = CGImageMaskCreate(b->w(), b->h(), 1, 1, bytes_per_row, provider, NULL, false);
    CFRelease(provider);
  }  
#endif
}


#if defined(__APPLE__) // on the mac, use an 8-bit mask
/* the image can be of any depth
 offset gives the byte offset from the pixel start to the byte used to construct the shape
 */
void Fl_Window::shape_alpha_(Fl_Image* img, int offset) {
  int i, d = img->d(), w = img->w(), h = img->h();
  shape_data_->shape_ = img;
  if (shape_data_->shape_) {
    // reverse top and bottom and convert to gray scale if img->d() == 3 and complement bits
    int bytes_per_row = w * d;
    uchar *from = new uchar[w * h];
    for ( i = 0; i < h; i++) {
      uchar *p = (uchar*)(*img->data()) + bytes_per_row * i + offset;
      uchar *last = p + bytes_per_row;
      uchar *q = from + (h - 1 - i) * w;
      while (p < last) {
        if (d == 3) {
          unsigned u = *p++;
          u += *p++;
          u += *p++;
          *q++ = ~(u/3);
        }
        else {
          *q++ = ~(*p);
          p += d;
        }
      }
    }
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, from, w * h, MyProviderReleaseData);
    shape_data_->mask = CGImageMaskCreate(w, h, 8, 8, w, provider, NULL, false);
    CFRelease(provider);
  }  
}

#else

/* the img image can be of any depth
 offset gives the byte offset from the pixel start to the byte used to construct the shape
 */
void Fl_Window::shape_alpha_(Fl_Image* img, int offset) {
  int i, j, d = img->d(), w = img->w(), h = img->h(), bytesperrow = (w+7)/8;
  unsigned u;
  uchar byte, onebit;
  // build an Fl_Bitmap covering the non-fully transparent/black part of the image
  const uchar* bits = new uchar[h*bytesperrow]; // to store the bitmap
  const uchar* alpha = (const uchar*)*img->data() + offset; // points to alpha value of rgba pixels
  for (i = 0; i < h; i++) {
    uchar *p = (uchar*)bits + i * bytesperrow;
    byte = 0;
    onebit = 1;
    for (j = 0; j < w; j++) {
      if (d == 3) {
        u = *alpha;
        u += *(alpha+1);
        u += *(alpha+2);
      }
      else u = *alpha;
      if (u > 0) { // if the pixel is not fully transparent/black
        byte |= onebit; // turn on the corresponding bit of the bitmap
      }
      onebit = onebit << 1; // move the single set bit one position to the left
      if (onebit == 0 || j == w-1) {
        onebit = 1;
        *p++ = byte; // store in bitmap one pack of bits
        byte = 0;
      }
      alpha += d; // point to alpha value of next pixel
    }
  }
  Fl_Bitmap* bitmap = new Fl_Bitmap(bits, w, h);
  bitmap->alloc_array = 1;
  shape_bitmap_(bitmap);
  shape_data_->todelete_ = bitmap;
}

#endif


void Fl_Window::shape_pixmap_(Fl_Image* pixmap) {
  Fl_RGB_Image* rgba = new Fl_RGB_Image((Fl_Pixmap*)pixmap);
  shape_alpha_(rgba, 3);
  delete rgba;
}

#if FLTK_ABI_VERSION < 10303 && !defined(FL_DOXYGEN)
Fl_Window::shape_data_type* Fl_Window::shape_data_ = NULL;
#endif

/** Assigns a non-rectangular shape to the window.
 This function gives an arbitrary shape (not just a rectangular region) to an Fl_Window.
 An Fl_Image of any dimension can be used as mask; it is rescaled to the window's dimension as needed.
 
 The layout and widgets inside are unaware of the mask shape, and most will act as though the window's
 rectangular bounding box is available
 to them. It is up to you to make sure they adhere to the bounds of their masking shape.
 
 The \p img argument can be an Fl_Bitmap, Fl_Pixmap, Fl_RGB_Image or Fl_Shared_Image:
 \li With Fl_Bitmap or Fl_Pixmap, the shaped window covers the image part where bitmap bits equal one,
 or where the pixmap is not fully transparent.
 \li With an Fl_RGB_Image with an alpha channel (depths 2 or 4), the shaped window covers the image part
 that is not fully transparent.
 \li With an Fl_RGB_Image of depth 1 (gray-scale) or 3 (RGB), the shaped window covers the non-black image part.
 \li With an Fl_Shared_Image, the shape is determined by rules above applied to the underlying image.
 The shared image should not have been scaled through Fl_Shared_Image::scale().
 
 Platform details:
 \li On the unix/linux platform, the SHAPE extension of the X server is required.
 This function does control the shape of Fl_Gl_Window instances.
 \li On the MSWindows platform, this function does nothing with class Fl_Gl_Window.
 \li On the Mac platform, OS version 10.4 or above is required. 
 An 8-bit shape-mask is used when \p img is an Fl_RGB_Image:
 with depths 2 or 4, the image alpha channel becomes the shape mask such that areas with alpha = 0
 are out of the shaped window;
 with depths 1 or 3, white and black are in and out of the
 shaped window, respectively, and other colors give intermediate masking scores.
 This function does nothing with class Fl_Gl_Window.

 The window borders and caption created by the window system are turned off by default. They
 can be re-enabled by calling Fl_Window::border(1).
 
 A usage example is found at example/shapedwindow.cxx.
 
 \version 1.3.3 (and requires compilation with FLTK_ABI_VERSION >= 10303)
 */
void Fl_Window::shape(const Fl_Image* img) {
#if FLTK_ABI_VERSION >= 10303
  if (shape_data_) {
    if (shape_data_->todelete_) { delete shape_data_->todelete_; }
#if defined(__APPLE__)
    if (shape_data_->mask) { CGImageRelease(shape_data_->mask); }
#endif
    }
  else {
    shape_data_ = new shape_data_type;
    }
  memset(shape_data_, 0, sizeof(shape_data_type));
  border(false);
  int d = img->d();
  if (d && img->count() >= 2) shape_pixmap_((Fl_Image*)img);
  else if (d == 0) shape_bitmap_((Fl_Image*)img);
  else if (d == 2 || d == 4) shape_alpha_((Fl_Image*)img, d - 1);
  else if ((d == 1 || d == 3) && img->count() == 1) shape_alpha_((Fl_Image*)img, 0);
#endif
}

void Fl_Window::draw() {
  if (shape_data_) {
# if defined(__APPLE__) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
    if (shape_data_->mask && (&CGContextClipToMask != NULL)) {
      CGContextClipToMask(fl_gc, CGRectMake(0,0,w(),h()), shape_data_->mask); // requires Mac OS 10.4
    }
    CGContextSaveGState(fl_gc);
#elif defined(WIN32)
    if ((shape_data_->lw_ != w() || shape_data_->lh_ != h()) && shape_data_->shape_) {
      // size of window has changed since last time
      shape_data_->lw_ = w();
      shape_data_->lh_ = h();
      Fl_Image* temp = shape_data_->shape_->copy(shape_data_->lw_, shape_data_->lh_);
      HRGN region = bitmap2region(temp);
      SetWindowRgn(fl_xid(this), region, TRUE); // the system deletes the region when it's no longer needed
      delete temp;
    }
#elif !(defined(__APPLE__) || defined(WIN32))
    if (( shape_data_->lw_ != w() || shape_data_->lh_ != h() ) && shape_data_->shape_) {
        // size of window has changed since last time
    combine_mask();
    }
# endif
  }

  // The following is similar to Fl_Group::draw(), but ...
  //
  //  - draws the box at (0,0), i.e. with x=0 and y=0 instead of x() and y()
  //  - does NOT draw the label (text)
  //  - draws the image only if FL_ALIGN_INSIDE is set
  //
  // Note: The label (text) of top level windows is drawn in the title bar.
  //   Other windows do not draw their labels at all, unless drawn by their
  //   parent widgets or by special draw() methods (derived classes).

  if (damage() & ~FL_DAMAGE_CHILD) {	 // draw the entire thing
    draw_box(box(),0,0,w(),h(),color()); // draw box with x/y = 0

    if (image() && (align() & FL_ALIGN_INSIDE)) { // draw the image only
      Fl_Label l1;
      memset(&l1,0,sizeof(l1));
      l1.align_ = align();
      l1.image = image();
      if (!active_r() && l1.image && l1.deimage) l1.image = l1.deimage;
      l1.type = labeltype();
      l1.draw(0,0,w(),h(),align());
    }
  }
  draw_children();

#ifdef __APPLE_QUARTZ__
  // on OS X, windows have no frame. Before OS X 10.7, to resize a window, we drag the lower right
  // corner. This code draws a little ribbed triangle for dragging.
  if (fl_mac_os_version < 100700 && fl_gc && !parent() && resizable() &&
      (!size_range_set || minh!=maxh || minw!=maxw)) {
    int dx = Fl::box_dw(box())-Fl::box_dx(box());
    int dy = Fl::box_dh(box())-Fl::box_dy(box());
    if (dx<=0) dx = 1;
    if (dy<=0) dy = 1;
    int x1 = w()-dx-1, x2 = x1, y1 = h()-dx-1, y2 = y1;
    Fl_Color c[4] = {
      color(),
      fl_color_average(color(), FL_WHITE, 0.7f),
      fl_color_average(color(), FL_BLACK, 0.6f),
      fl_color_average(color(), FL_BLACK, 0.8f),
    };
    int i;
    for (i=dx; i<12; i++) {
      fl_color(c[i&3]);
      fl_line(x1--, y1, x2, y2--);
    }
  }
#endif
# if defined(__APPLE__) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (shape_data_) CGContextRestoreGState(fl_gc);
# endif
  
# if defined(FLTK_USE_CAIRO)
  Fl::cairo_make_current(this); // checkout if an update is necessary
# endif
}



//
// End of "$Id$".
//
