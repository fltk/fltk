//
// "$Id: Fl_Double_Window.cxx,v 1.12.2.4.2.12 2004/08/31 22:00:47 matthiaswm Exp $"
//
// Double-buffered window code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2004 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <config.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/x.H>
#include <FL/fl_draw.H>

// On systems that support double buffering "naturally" the base
// Fl_Window class will probably do double-buffer and this subclass
// does nothing.

#if USE_XDBE

#include <X11/extensions/Xdbe.h>

static int use_xdbe;

static int can_xdbe() {
  static int tried;
  if (!tried) {
    tried = 1;
    int event_base, error_base;
    if (!XdbeQueryExtension(fl_display, &event_base, &error_base)) return 0;
    Drawable root = RootWindow(fl_display,fl_screen);
    int numscreens = 1;
    XdbeScreenVisualInfo *a = XdbeGetVisualInfo(fl_display,&root,&numscreens);
    if (!a) return 0;
    for (int j = 0; j < a->count; j++)
      if (a->visinfo[j].visual == fl_visual->visualid
	  /*&& a->visinfo[j].perflevel > 0*/) {use_xdbe = 1; break;}
    XdbeFreeVisualInfo(a);
  }
  return use_xdbe;
}
#endif

void Fl_Double_Window::show() {
#if !defined(WIN32) && !defined(__APPLE__)
  if (!shown()) { // don't set the background pixel
    fl_open_display();
    Fl_X::make_xid(this);
    return;
  }
#endif
  Fl_Window::show();
}

#ifdef WIN32

// Code used to switch output to an off-screen window.  See macros in
// win32.H which save the old state in local variables.

HDC fl_makeDC(HBITMAP bitmap) {
  HDC new_gc = CreateCompatibleDC(fl_gc);
  SetTextAlign(new_gc, TA_BASELINE|TA_LEFT);
  SetBkMode(new_gc, TRANSPARENT);
#if USE_COLORMAP
  if (fl_palette) SelectPalette(new_gc, fl_palette, FALSE);
#endif
  SelectObject(new_gc, bitmap);
  return new_gc;
}

void fl_copy_offscreen(int x,int y,int w,int h,HBITMAP bitmap,int srcx,int srcy) {
  HDC new_gc = CreateCompatibleDC(fl_gc);
  SelectObject(new_gc, bitmap);
  BitBlt(fl_gc, x, y, w, h, new_gc, srcx, srcy, SRCCOPY);
  DeleteDC(new_gc);
}

extern void fl_restore_clip();

#elif defined(__APPLE_QD__)

GWorldPtr fl_create_offscreen(int w, int h) {
  GWorldPtr gw;
  Rect bounds;
  bounds.left=0; bounds.right=w; bounds.top=0; bounds.bottom=h;
  QDErr err = NewGWorld(&gw, 0, &bounds, 0L, 0L, 0); // 'useTempMem' should not be used (says the Carbon port manual)
  if ( err == -108 )
    { }
//    fl_message( "The application memory is low. Please increase the initial memory assignment.\n" ); 
  if (err!=noErr || gw==0L) return 0L;
  return gw;
}

void fl_copy_offscreen(int x,int y,int w,int h,GWorldPtr gWorld,int srcx,int srcy) {
  Rect src;
  if ( !gWorld ) return;
  src.top = srcy; src.left = srcx; src.bottom = srcy+h; src.right = srcx+w;
  Rect dst;
  GrafPtr dstPort; GetPort(&dstPort);
  dst.top = y; dst.left = x; dst.bottom = y+h; dst.right = x+w;
  RGBColor rgb;
  rgb.red = 0xffff; rgb.green = 0xffff; rgb.blue = 0xffff;
  RGBBackColor( &rgb );
  rgb.red = 0x0000; rgb.green = 0x0000; rgb.blue = 0x0000;
  RGBForeColor( &rgb );
  CopyBits(GetPortBitMapForCopyBits(gWorld), GetPortBitMapForCopyBits(dstPort), &src, &dst, srcCopy, 0L);
}

void fl_delete_offscreen(GWorldPtr gWorld) {
  DisposeGWorld(gWorld);
}

static GrafPtr prevPort;
static GDHandle prevGD;

void fl_begin_offscreen(GWorldPtr gWorld) {
  GetGWorld( &prevPort, &prevGD );
  if ( gWorld )
  {
    SetGWorld( gWorld, 0 ); // sets the correct port
    PixMapHandle pm = GetGWorldPixMap(gWorld);
    Boolean ret = LockPixels(pm);
    if ( ret == false )
    {
      Rect rect;
      GetPortBounds( gWorld, &rect );
      UpdateGWorld( &gWorld, 0, &rect, 0, 0, 0 );
      pm = GetGWorldPixMap( gWorld );
      LockPixels( pm );
    }
    fl_window = 0;
  }
  fl_push_no_clip();
}

void fl_end_offscreen() {
  GWorldPtr currPort;
  GDHandle currGD;
  GetGWorld( &currPort, &currGD );
  fl_pop_clip();
  PixMapHandle pm = GetGWorldPixMap(currPort);
  UnlockPixels(pm);
  SetGWorld( prevPort, prevGD );
  fl_window = GetWindowFromPort( prevPort );
}

extern void fl_restore_clip();

#elif defined(__APPLE_QUARTZ__)

Fl_Offscreen fl_create_offscreen(int w, int h) {
  void *data = calloc(w*h,4);
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  CGContextRef ctx = CGBitmapContextCreate(
    data, w, h, 8, w*4, lut, kCGImageAlphaNoneSkipLast);
  CGColorSpaceRelease(lut);
  return (Fl_Offscreen)ctx;
}

Fl_Offscreen fl_create_offscreen_with_alpha(int w, int h) {
  void *data = calloc(w*h,4);
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  CGContextRef ctx = CGBitmapContextCreate(
    data, w, h, 8, w*4, lut, kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(lut);
  return (Fl_Offscreen)ctx;
}

void fl_copy_offscreen(int x,int y,int w,int h,Fl_Offscreen osrc,int srcx,int srcy) {
  CGContextRef src = (CGContextRef)osrc;
  void *data = CGBitmapContextGetData(src);
  int sw = CGBitmapContextGetWidth(src);
  int sh = CGBitmapContextGetHeight(src);
  CGImageAlphaInfo alpha = CGBitmapContextGetAlphaInfo(src);
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  CGDataProviderRef src_bytes = CGDataProviderCreateWithData( 0L, data, sw*sh*4, 0L);
  CGImageRef img = CGImageCreate( sw, sh, 8, 4*8, 4*sw, lut, alpha,
    src_bytes, 0L, false, kCGRenderingIntentDefault);
  // fl_push_clip();
  CGRect rect = { x, y, w, h };
  Fl_X::q_begin_image(rect, srcx, srcy, sw, sh);
  CGContextDrawImage(fl_gc, rect, img);
  Fl_X::q_end_image();
  CGImageRelease(img);
  CGColorSpaceRelease(lut);
  CGDataProviderRelease(src_bytes);
}

void fl_delete_offscreen(Fl_Offscreen ctx) {
  if (!ctx) return;
  void *data = CGBitmapContextGetData((CGContextRef)ctx);
  CGContextRelease((CGContextRef)ctx);
  if (!data) return;
  free(data);
}

static CGContextRef prev_gc = 0;
static Window prev_window = 0;

void fl_begin_offscreen(Fl_Offscreen ctx) {
  prev_gc = fl_gc;
  prev_window = fl_window;
  fl_gc = (CGContextRef)ctx;
  fl_window = 0;
  //fl_push_no_clip();
  CGContextSaveGState(fl_gc);
  Fl_X::q_fill_context();
}

void fl_end_offscreen() {
  Fl_X::q_release_context();
  //fl_pop_clip();
  fl_gc = prev_gc;
  fl_window = prev_window;
}

extern void fl_restore_clip();

#endif

// Fl_Overlay_Window relies on flush(1) copying the back buffer to the
// front everywhere, even if damage() == 0, thus erasing the overlay,
// and leaving the clip region set to the entire window.

void Fl_Double_Window::flush() {flush(0);}

void Fl_Double_Window::flush(int eraseoverlay) {
  make_current(); // make sure fl_gc is non-zero
  Fl_X *myi = Fl_X::i(this);
  if (!myi->other_xid) {
#if USE_XDBE
    if (can_xdbe()) myi->other_xid =
      XdbeAllocateBackBufferName(fl_display, fl_xid(this), XdbeUndefined);
    else
#endif
#ifdef __APPLE_QD__
    if ( ( !QDIsPortBuffered( GetWindowPort(myi->xid) ) ) || force_doublebuffering_ )
      myi->other_xid = fl_create_offscreen(w(), h());
#elif defined(__APPLE_QUARTZ__)
    if (force_doublebuffering_)
      myi->other_xid = fl_create_offscreen(w(), h());
#else
    myi->other_xid = fl_create_offscreen(w(), h());
#endif
    clear_damage(FL_DAMAGE_ALL);
  }
#if USE_XDBE
  if (use_xdbe) {
    // if this is true, copy rather than swap so back buffer is preserved:
    int copy = (myi->region || eraseoverlay);
    if (myi->backbuffer_bad) { // make sure we do a complete redraw...
      if (myi->region) {XDestroyRegion(myi->region); myi->region = 0;}
      clear_damage(FL_DAMAGE_ALL);
    }
    if (damage()) {
      fl_clip_region(myi->region); myi->region = 0;
      fl_window = myi->other_xid;
      draw();
      fl_window = myi->xid;
    }
    if (!copy) {
      XdbeSwapInfo s;
      s.swap_window = fl_xid(this);
      s.swap_action = XdbeUndefined;
      XdbeSwapBuffers(fl_display, &s, 1);
      myi->backbuffer_bad = 1;
      return;
    }
    // otherwise just use normal copy from back to front:
    myi->backbuffer_bad = 0; // which won't destroy the back buffer...
  } else
#endif
  if (damage() & ~FL_DAMAGE_EXPOSE) {
    fl_clip_region(myi->region); myi->region = 0;
#ifdef WIN32
    HDC _sgc = fl_gc;
    fl_gc = fl_makeDC(myi->other_xid);
    fl_restore_clip(); // duplicate region into new gc
    draw();
    DeleteDC(fl_gc);
    fl_gc = _sgc;
#elif defined(__APPLE__)
    if ( myi->other_xid ) {
      fl_begin_offscreen( myi->other_xid );
      fl_clip_region( 0 );   
      draw();
      fl_end_offscreen();
    } else {
      draw();
    }
#else // X:
    fl_window = myi->other_xid;
    draw();
    fl_window = myi->xid;
#endif
  }
  if (eraseoverlay) fl_clip_region(0);
  // on Irix (at least) it is faster to reduce the area copied to
  // the current clip region:
  int X,Y,W,H; fl_clip_box(0,0,w(),h(),X,Y,W,H);
#ifdef __APPLE__
  if (myi->other_xid) fl_copy_offscreen(X, Y, W, H, myi->other_xid, X, Y);
#else
  fl_copy_offscreen(X, Y, W, H, myi->other_xid, X, Y);
#endif
}

void Fl_Double_Window::resize(int X,int Y,int W,int H) {
  int ow = w();
  int oh = h();
  Fl_Window::resize(X,Y,W,H);
#if USE_XDBE
  if (use_xdbe) return;
#endif
  Fl_X* myi = Fl_X::i(this);
  if (myi && myi->other_xid && (ow != w() || oh != h())) {
    fl_delete_offscreen(myi->other_xid);
    myi->other_xid = 0;
  }
}

void Fl_Double_Window::hide() {
  Fl_X* myi = Fl_X::i(this);
  if (myi && myi->other_xid) {
#if USE_XDBE
    if (!use_xdbe)
#endif
      fl_delete_offscreen(myi->other_xid);
  }
  Fl_Window::hide();
}

Fl_Double_Window::~Fl_Double_Window() {
  hide();
}

//
// End of "$Id: Fl_Double_Window.cxx,v 1.12.2.4.2.12 2004/08/31 22:00:47 matthiaswm Exp $".
//
