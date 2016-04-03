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
#include "Fl_Cocoa_Window_Driver.H"
#include "../Quartz/Fl_Quartz_Graphics_Driver.H"
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <FL/x.H>


Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *w)
{
  return new Fl_Cocoa_Window_Driver(w);
}


Fl_Cocoa_Window_Driver::Fl_Cocoa_Window_Driver(Fl_Window *win)
: Fl_Window_Driver(win)
{
}


Fl_Cocoa_Window_Driver::~Fl_Cocoa_Window_Driver()
{
  if (shape_data_) {
    if (shape_data_->mask) {
      CGImageRelease(shape_data_->mask);
    }
    delete shape_data_;
  }
}


void Fl_Cocoa_Window_Driver::take_focus()
{
  Fl_X *x = Fl_X::i(pWindow);
  if (x) x->set_key_window();
}


void Fl_Cocoa_Window_Driver::flush_double() {
  flush_single();
}


void Fl_Cocoa_Window_Driver::flush_overlay()
{
  Fl_Overlay_Window *oWindow = pWindow->as_overlay_window();
  int erase_overlay = (pWindow->damage()&FL_DAMAGE_OVERLAY) | (overlay() == oWindow);
  pWindow->clear_damage((uchar)(pWindow->damage()&~FL_DAMAGE_OVERLAY));

  if (!oWindow->shown()) return;
  pWindow->make_current(); // make sure fl_gc is non-zero
  Fl_X *myi = Fl_X::i(pWindow);
  if (!myi) return; // window not yet created
  if (!myi->other_xid) {
      myi->other_xid = fl_create_offscreen(oWindow->w(), oWindow->h());
      oWindow->clear_damage(FL_DAMAGE_ALL);
  }
    if (oWindow->damage() & ~FL_DAMAGE_EXPOSE) {
      fl_clip_region(myi->region); myi->region = 0;
      if ( myi->other_xid ) {
        fl_begin_offscreen( myi->other_xid );
        fl_clip_region( 0 );
        draw();
        fl_end_offscreen();
      } else {
        draw();
      }
    }
  if (erase_overlay) fl_clip_region(0);
  // on Irix (at least) it is faster to reduce the area copied to
  // the current clip region:
  int X,Y,W,H; fl_clip_box(0,0,oWindow->w(),oWindow->h(),X,Y,W,H);
  if (myi->other_xid) fl_copy_offscreen(X, Y, W, H, myi->other_xid, X, Y);
  
  if (overlay() == oWindow) oWindow->draw_overlay();
}


void Fl_Cocoa_Window_Driver::draw_begin()
{
  if (!Fl_Surface_Device::surface()->driver()->has_feature(Fl_Graphics_Driver::NATIVE)) return;
  CGContextRef gc = (CGContextRef)Fl_Surface_Device::surface()->driver()->gc();
  if (shape_data_) {
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
    if (shape_data_->mask && (&CGContextClipToMask != NULL)) {
      CGContextClipToMask(gc, CGRectMake(0,0,w(),h()), shape_data_->mask); // requires Mac OS 10.4
    }
    CGContextSaveGState(gc);
# endif
  }
  if ( fl_mac_os_version >= 100600 ) {
    // for Mac OS X 10.6 and above, make window with rounded bottom corners
    Fl_X::clip_to_rounded_corners(gc, pWindow->w(), pWindow->h());
  }
}


void Fl_Cocoa_Window_Driver::draw_end()
{
  // on OS X, windows have no frame. Before OS X 10.7, to resize a window, we drag the lower right
  // corner. This code draws a little ribbed triangle for dragging.
  if (fl_mac_os_version < 100700 && !parent() && pWindow->resizable() &&
      (!size_range_set() || minh() != maxh() || minw() != maxw())) {
    int dx = Fl::box_dw(pWindow->box())-Fl::box_dx(pWindow->box());
    int dy = Fl::box_dh(pWindow->box())-Fl::box_dy(pWindow->box());
    if (dx<=0) dx = 1;
    if (dy<=0) dy = 1;
    int x1 = w()-dx-1, x2 = x1, y1 = h()-dx-1, y2 = y1;
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
  if (Fl_Surface_Device::surface()->driver()->has_feature(Fl_Graphics_Driver::NATIVE)) {
    CGContextRef gc = (CGContextRef)Fl_Surface_Device::surface()->driver()->gc();
    if (shape_data_) CGContextRestoreGState(gc);
  }
# endif
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


void Fl_Cocoa_Window_Driver::hide() {
  Fl_X* ip = Fl_X::i(pWindow);
  // MacOS X manages a single pointer per application. Make sure that hiding
  // a toplevel window will not leave us with some random pointer shape, or
  // worst case, an invisible pointer
  if (ip && !parent()) pWindow->cursor(FL_CURSOR_DEFAULT);
  if ( hide_common() ) return;
  Fl_X::q_release_context(ip);
  if ( ip->xid == fl_window )
    fl_window = 0;
  if (ip->region) Fl_Graphics_Driver::XDestroyRegion(ip->region);
  ip->destroy();
  delete ip;
}


void Fl_Cocoa_Window_Driver::fullscreen_on() {
  pWindow->_set_fullscreen();
  /* On OS X < 10.6, it is necessary to recreate the window. This is done
   with hide+show. */
  hide();
  show();
  Fl::handle(FL_FULLSCREEN, pWindow);
}


void Fl_Cocoa_Window_Driver::fullscreen_off(int X, int Y, int W, int H) {
  pWindow->_clear_fullscreen();
  hide();
  resize(X, Y, W, H);
  show();
  Fl::handle(FL_FULLSCREEN, pWindow);
}


void Fl_Cocoa_Window_Driver::decoration_sizes(int *top, int *left,  int *right, int *bottom) {
  *top = 24;
  *left = 2;
  *right = 2;
  *bottom = 2;
}

int Fl_Cocoa_Window_Driver::scroll(int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y, void (*draw_area)(void*, int,int,int,int), void* data)
{
  CGImageRef img = Fl_X::CGImage_from_window_rect(Fl_Window::current(), src_x, src_y, src_w, src_h);
  if (img) {
    ((Fl_Quartz_Graphics_Driver*)fl_graphics_driver)->draw_CGImage(img,dest_x,dest_y,src_w,src_h,0,0,src_w,src_h);
    CFRelease(img);
  }
  return 0;
}

//
// End of "$Id$".
//
