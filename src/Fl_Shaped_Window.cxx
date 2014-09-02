//
// "$Id$"
//
// Fl_Shaped_Window source file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2014 by Bill Spitzak and others.
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

#include <FL/x.H>
#include <FL/Fl_Shaped_Window.H>
#include <FL/Fl_Bitmap.H>
#include <FL/Fl_Pixmap.H>

#ifdef WIN32
# include <malloc.h> // needed for VisualC2010
#elif !defined(__APPLE__)
#include <dlfcn.h>
#define ShapeBounding			0
#define ShapeSet			0
#endif

/** Create a shaped window with the given size and title */
Fl_Shaped_Window::Fl_Shaped_Window(int w, int h, const char* title)
	: Fl_Window(w, h, title), lw_(0), lh_(0), shape_(0), todelete_(0) {
	  type(FL_SHAPED_WINDOW);
	  border(false);
#if defined(__APPLE__)
	  mask = NULL;
#endif
	}

/** Create a shaped window with the given position, size and title */
Fl_Shaped_Window::Fl_Shaped_Window(int x, int y, int w, int h, const char* title)
	: Fl_Window(x, y, w, h, title), lw_(0), lh_(0), shape_(0), todelete_(0) {
	  type(FL_SHAPED_WINDOW);
	  border(false);
#if defined(__APPLE__)
	  mask = NULL;
#endif
	}

/** Destroys the shaped window but not its associated Fl_Image */
Fl_Shaped_Window::~Fl_Shaped_Window() {
  if (todelete_) delete todelete_;
#if defined(__APPLE__)
  if (mask) {
    CGImageRelease(mask);
  }
#endif
}


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

void Fl_Shaped_Window::draw() {
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (mask && (CGContextClipToMask != NULL)) CGContextClipToMask(fl_gc, CGRectMake(0,0,w(),h()), mask); // requires Mac OS 10.4
  CGContextSaveGState(fl_gc);
# endif
  Fl_Window::draw();
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  CGContextRestoreGState(fl_gc);
# endif
  }

#elif defined(WIN32)

static inline BYTE bit(int x) { return (BYTE)(1 << (x%8)); }

static HRGN bitmap2region(Fl_Bitmap* image) {
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
  BYTE* p, *data = (BYTE*)image->array;
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

void Fl_Shaped_Window::draw() {
  if ((lw_ != w() || lh_ != h()) && shape_) {
    // size of window has changed since last time
    lw_ = w();
    lh_ = h();
    Fl_Bitmap* temp = (Fl_Bitmap*)shape_->copy(lw_, lh_);
    HRGN region = bitmap2region(temp);
    SetWindowRgn(fl_xid(this), region, TRUE); // the system deletes the region when it's no longer needed
    delete temp;
  }
  Fl_Window::draw();
}

#else


#ifndef FL_DOXYGEN
void Fl_Shaped_Window::combine_mask()
{
  typedef void (*XShapeCombineMask_type)(Display*, int, int, int, int, Pixmap, int);
  static XShapeCombineMask_type XShapeCombineMask_f = NULL;
  static int beenhere = 0;
  typedef Bool (*XShapeQueryExtension_type)(Display*, int*, int*);
  int error_base, shapeEventBase;
  if (!beenhere) {
    beenhere = 1;
    fl_open_display();
    void *handle = dlopen(NULL, RTLD_LAZY); // search symbols in executable
    XShapeQueryExtension_type XShapeQueryExtension_f = (XShapeQueryExtension_type)dlsym(handle, "XShapeQueryExtension");
    XShapeCombineMask_f = (XShapeCombineMask_type)dlsym(handle, "XShapeCombineMask");
    // make sure that the X server has the SHAPE extension
    if ( !( XShapeQueryExtension_f && XShapeCombineMask_f && 
	   XShapeQueryExtension_f(fl_display, &shapeEventBase, &error_base) ) ) XShapeCombineMask_f = NULL;
  }
  if (!XShapeCombineMask_f) return;
  lw_ = w();
  lh_ = h();
  Fl_Bitmap* temp = (Fl_Bitmap*)shape_->copy(lw_, lh_);  
  Pixmap pbitmap = XCreateBitmapFromData(fl_display, fl_xid(this), 
					 (const char*)temp->array,
					 temp->w(), temp->h());
  XShapeCombineMask_f(fl_display, fl_xid(this), ShapeBounding, 0, 0, pbitmap, ShapeSet);
  if (pbitmap != None) XFreePixmap(fl_display, pbitmap);
  delete temp;
}
#endif // !FL_DOXYGEN

void Fl_Shaped_Window::draw() {
  if (( lw_ != w() || lh_ != h() ) && shape_) {
    // size of window has changed since last time
    combine_mask();
  }
  Fl_Window::draw();
}

#endif // __APPLE__


void Fl_Shaped_Window::shape_bitmap_(Fl_Bitmap* b) {
  if (todelete_) { delete todelete_; todelete_ = NULL; }
  shape_ = b;
  lw_ = lh_ = 0; // so change in mask is detected
#if defined(__APPLE__)
  if (mask) {
    CGImageRelease(mask);
    mask = NULL;
  }
  if (b) {
    // complement mask bits and perform bitwise inversion of all bytes and also reverse top and bottom
    int bytes_per_row = (b->w() + 7)/8;
    uchar *from = new uchar[bytes_per_row * b->h()];
    for (int i = 0; i < b->h(); i++) {
      uchar *p = (uchar*)b->array + bytes_per_row * i;
      uchar *last = p + bytes_per_row;
      uchar *q = from + (b->h() - 1 - i) * bytes_per_row;
      while (p < last) {
	*q++ = swap_byte(~*p++);
      }
    }
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, from, bytes_per_row * b->h(), MyProviderReleaseData);
    mask = CGImageMaskCreate(b->w(), b->h(), 1, 1, bytes_per_row, provider, NULL, false);
    CFRelease(provider);
  }  
#endif
}


#if defined(__APPLE__) // on the mac, use an 8-bit mask
/* the image can be of any depth
 offset gives the byte offset from the pixel start to the byte used to construct the shape
 */
void Fl_Shaped_Window::shape_alpha_(Fl_RGB_Image* img, int offset) {
  int i, d = img->d(), w = img->w(), h = img->h();
  if (todelete_) { delete todelete_; todelete_ = NULL; }
  shape_ = img;
  lw_ = lh_ = 0; // so change in mask is detected
  if (mask) {
    CGImageRelease(mask);
    mask = NULL;
  }
  if (shape_) {
    // reverse top and bottom and convert to gray scale if img->d() == 3 and complement bits
    int bytes_per_row = w * d;
    uchar *from = new uchar[w * h];
    for ( i = 0; i < h; i++) {
      uchar *p = (uchar*)img->array + bytes_per_row * i + offset;
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
    mask = CGImageMaskCreate(w, h, 8, 8, w, provider, NULL, false);
    CFRelease(provider);
  }  
}

#else

/* the img image can be of any depth
 offset gives the byte offset from the pixel start to the byte used to construct the shape
 */
void Fl_Shaped_Window::shape_alpha_(Fl_RGB_Image* img, int offset) {
  int i, j, d = img->d(), w = img->w(), h = img->h(), bytesperrow = (w+7)/8;
  unsigned u;
  uchar byte, onebit;
  // build an Fl_Bitmap covering the non-fully transparent/black part of the image
  const uchar* bits = new uchar[h*bytesperrow]; // to store the bitmap
  const uchar* alpha = img->array + offset; // points to alpha value of rgba pixels
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
  todelete_ = bitmap;
}

#endif


void Fl_Shaped_Window::shape_pixmap_(Fl_Pixmap* pixmap) {
  Fl_RGB_Image* rgba = new Fl_RGB_Image(pixmap);
  shape_alpha_(rgba, 3);
  delete rgba;
}

/** Set the window's shape with an image.
 The \p img argument can be an Fl_Bitmap, Fl_Pixmap or Fl_RGB_Image.
 \li With Fl_Bitmap or Fl_Pixmap, the shaped window covers the image part where bitmap bits equal one, 
 or where the pixmap is not fully transparent.
 \li With an Fl_RGB_Image with an alpha channel (depths 2 or 4), the shaped window covers the image part
 that is not fully transparent.
 \li With an Fl_RGB_Image of depth 1 (gray-scale) or 3 (RGB), the shaped window covers the non-black image part.
 
 On some platforms, an 8-bit shape-mask is used when \p img is an Fl_RGB_Image: 
 with depths 2 or 4, the image alpha channel becomes the shape mask such that areas with alpha = 0 
 are out of the shaped window; 
 with depths 1 or 3, white and black are in and out of the 
 shaped window, respectively, and other colors give intermediate masking scores.
 */
void Fl_Shaped_Window::shape(const Fl_Image* img) {
  int d = img->d();
  if (d && img->count() >= 2) shape_pixmap_((Fl_Pixmap*)img);
  else if (d == 0) shape_bitmap_((Fl_Bitmap*)img);
  else if (d == 2 || d == 4) shape_alpha_((Fl_RGB_Image*)img, d - 1);
  else if ((d == 1 || d == 3) && img->count() == 1) shape_alpha_((Fl_RGB_Image*)img, 0);
}

//
// End of "$Id$".
//
