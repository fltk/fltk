//
// "$Id$"
//
// Definition of Apple Cocoa window driver.
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


#include "../../config_lib.h"
#include "Fl_Cocoa_Window_Driver.h"
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl.H>

// class used for Fl_Double_Window but not for Fl_Overlay_Window
class Fl_Cocoa_Double_Window_Driver : public Fl_Cocoa_Window_Driver {
public:
  Fl_Cocoa_Double_Window_Driver(Fl_Window *w) : Fl_Cocoa_Window_Driver(w) {}
  int double_flush(int eraseoverlay) {
    draw();
    return 0;
  }
};

Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *w)
{
  if (w->as_double_window() && !w->as_double_window()->as_overlay_window())
    return new Fl_Cocoa_Double_Window_Driver(w);
  else
    return new Fl_Cocoa_Window_Driver(w);
}


Fl_Cocoa_Window_Driver::Fl_Cocoa_Window_Driver(Fl_Window *win)
: Fl_Window_Driver(win)
{
}

Fl_Cocoa_Window_Driver::~Fl_Cocoa_Window_Driver()
{
  if (shape_data_) {
    if (shape_data_->todelete_) delete shape_data_->todelete_;
    if (shape_data_->mask) {
      CGImageRelease(shape_data_->mask);
    }
    delete shape_data_;
  }
}

extern Fl_Window *fl_xfocus;


void Fl_Cocoa_Window_Driver::take_focus()
{
  Fl_X *x = Fl_X::i(pWindow);
  if (x) x->set_key_window();
}


static void MyProviderReleaseData (void *info, const void *data, size_t size) {
  delete[] (uchar*)data;
}

// bitwise inversion of all 4-bit quantities
static const unsigned char swapped[16] = {0,8,4,12,2,10,6,14,1,9,5,13,3,11,7,15};

static inline uchar swap_byte(const uchar b) {
  // reverse the order of bits of byte b: 1->8 becomes 8->1
  return (swapped[b & 0xF] << 4) | swapped[b >> 4];
}


void Fl_Cocoa_Window_Driver::shape_bitmap_(Fl_Image* b) {
  shape_data_->shape_ = b;
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
}


void Fl_Cocoa_Window_Driver::shape_alpha_(Fl_Image* img, int offset) {
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


void Fl_Cocoa_Window_Driver::shape(const Fl_Image* img) {
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (shape_data_) {
    if (shape_data_->todelete_) { delete shape_data_->todelete_; }
    if (shape_data_->mask) { CGImageRelease(shape_data_->mask); }
  }
  else {
    shape_data_ = new shape_data_type;
  }
  memset(shape_data_, 0, sizeof(shape_data_type));
  pWindow->border(false);
  int d = img->d();
  if (d && img->count() >= 2) shape_pixmap_((Fl_Image*)img);
  else if (d == 0) shape_bitmap_((Fl_Image*)img);
  else if (d == 2 || d == 4) shape_alpha_((Fl_Image*)img, d - 1);
  else if ((d == 1 || d == 3) && img->count() == 1) shape_alpha_((Fl_Image*)img, 0);
#endif
}


void Fl_Cocoa_Window_Driver::draw()
{
  CGContextRef gc = (CGContextRef)Fl_Surface_Device::surface()->driver()->gc();
  if (shape_data_) {
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
    if (shape_data_->mask && (&CGContextClipToMask != NULL)) {
      CGContextClipToMask(gc, CGRectMake(0,0, pWindow->w(), pWindow->h()), shape_data_->mask); // requires Mac OS 10.4
    }
    CGContextSaveGState(gc);
# endif
  }
  Fl_Window_Driver::draw();
  // on OS X, windows have no frame. Before OS X 10.7, to resize a window, we drag the lower right
  // corner. This code draws a little ribbed triangle for dragging.
  if (fl_mac_os_version < 100700 && gc && !pWindow->parent() && pWindow->resizable() ) {
    int dx = Fl::box_dw(pWindow->box())-Fl::box_dx(pWindow->box());
    int dy = Fl::box_dh(pWindow->box())-Fl::box_dy(pWindow->box());
    if (dx<=0) dx = 1;
    if (dy<=0) dy = 1;
    int x1 = pWindow->w()-dx-1, x2 = x1, y1 = pWindow->h()-dx-1, y2 = y1;
    Fl_Color c[4] = {
      pWindow->color(),
      fl_color_average(pWindow->color(), FL_WHITE, 0.7f),
      fl_color_average(pWindow->color(), FL_BLACK, 0.6f),
      fl_color_average(pWindow->color(), FL_BLACK, 0.8f),
    };
    int i;
    for (i=dx; i<12; i++) {
      fl_color(c[i&3]);
      fl_line(x1--, y1, x2, y2--);
    }
  }
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (shape_data_) CGContextRestoreGState(gc);
# endif
}

//
// End of "$Id$".
//
