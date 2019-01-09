//
// "$Id$"
//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Printer.H>
#include <FL/Fl.H>

const char *Fl_Image_Surface::class_id = "Fl_Image_Surface";

void Fl_Image_Surface::prepare_(int w, int h, int highres) {
  width = w;
  height = h;
#if FL_ABI_VERSION < 10304
  highres = 0;
  if (highres) {/* avoid compiler warning (Linux + Windows */}
#endif
#ifdef __APPLE__
  offscreen = fl_create_offscreen(highres ? 2*w : w, highres ? 2*h : h);
  helper = new Fl_Quartz_Flipped_Surface_(w, h);
  if (highres) {
    CGContextScaleCTM(offscreen, 2, 2);
  }
  driver(helper->driver());
  CGContextSetShouldAntialias(offscreen, false);
  CGContextSaveGState(offscreen);
  CGContextTranslateCTM(offscreen, 0, height);
  CGContextScaleCTM(offscreen, 1.0f, -1.0f);
  CGContextSetRGBFillColor(offscreen, 1, 1, 1, 1);
  CGContextFillRect(offscreen, CGRectMake(0,0,w,h) );
#elif defined(WIN32)
  offscreen = fl_create_offscreen(w, h);
  helper = new Fl_GDI_Surface_();
  driver(helper->driver());
#else
  gc = 0;
  if (!fl_gc) { // allows use of this class before any window is shown
    fl_open_display();
    gc = XCreateGC(fl_display, RootWindow(fl_display, fl_screen), 0, 0);
    fl_gc = gc;
  }
  offscreen = fl_create_offscreen(w, h);
  helper = new Fl_Xlib_Surface_();
  driver(helper->driver());
#endif
}

/** Constructor with optional high resolution.
 \param w and \param h give the size in pixels of the resulting image.
 \param highres if non-zero, the surface pixel size is twice as high and wide as w and h,
 which is useful to draw it later on a high resolution display (e.g., retina display). 
 This is implemented for the Mac OS platform only.
 If \p highres is non-zero, use Fl_Image_Surface::highres_image() to get the image data.
 \version 1.3.4 and requires compilation with -DFL_ABI_VERSION=10304 (1.3.3 without the highres parameter)
 */
Fl_Image_Surface::Fl_Image_Surface(int w, int h, int highres) : Fl_Surface_Device(NULL) {
  prepare_(w, h, highres);
}
#if FLTK_ABI_VERSION < 10304
Fl_Image_Surface::Fl_Image_Surface(int w, int h) : Fl_Surface_Device(NULL) {
  prepare_(w, h, 0);
}
#endif


/** The destructor.
 */
Fl_Image_Surface::~Fl_Image_Surface() {
#ifdef __APPLE__
  void *data = CGBitmapContextGetData((CGContextRef)offscreen);
  free(data);
  CGContextRelease((CGContextRef)offscreen);
  delete (Fl_Quartz_Flipped_Surface_*)helper;
#elif defined(WIN32)
  fl_delete_offscreen(offscreen);
  delete (Fl_GDI_Surface_*)helper;
#else
  fl_delete_offscreen(offscreen);
  if (gc) { XFreeGC(fl_display, gc); fl_gc = 0; }
  delete (Fl_Xlib_Surface_*)helper;
#endif
}

/** Returns an image made of all drawings sent to the Fl_Image_Surface object.
 The returned object contains its own copy of the RGB data.
 Prefer Fl_Image_Surface::highres_image() if the surface was 
 constructed with the highres option on.
 */
Fl_RGB_Image* Fl_Image_Surface::image()
{
  unsigned char *data;
  int W = width, H = height;
#ifdef __APPLE__
  CGContextFlush(offscreen);
  W = CGBitmapContextGetWidth(offscreen);
  H = CGBitmapContextGetHeight(offscreen);
  Fl_X::set_high_resolution(0);
  data = fl_read_image(NULL, 0, 0, W, H, 0);
  fl_gc = 0;
#elif defined(WIN32)
  fl_pop_clip(); 
  data = fl_read_image(NULL, 0, 0, width, height, 0);
  RestoreDC(fl_gc, _savedc); 
  DeleteDC(fl_gc); 
  _ss->set_current(); 
  fl_window=_sw; 
  fl_gc = _sgc;
#else
  fl_pop_clip(); 
  data = fl_read_image(NULL, 0, 0, width, height, 0);
  fl_window = pre_window; 
  previous->set_current();
#endif
  Fl_RGB_Image *image = new Fl_RGB_Image(data, W, H);
  image->alloc_array = 1;
  return image;
}

/** Returns a possibly high resolution image made of all drawings sent to the Fl_Image_Surface object.
 The Fl_Image_Surface object should have been constructed with Fl_Image_Surface(W, H, 1).
 The returned image is scaled to a size of WxH drawing units and may have a pixel size twice as wide and high.
 The returned object should be deallocated with Fl_Shared_Image::release() after use.
 \version 1.3.4 and requires compilation with -DFL_ABI_VERSION=10304
 */
Fl_Shared_Image* Fl_Image_Surface::highres_image()
{
  Fl_Shared_Image *s_img = Fl_Shared_Image::get(image());
  s_img->scale(width, height);
  return s_img;
}

/** Draws a widget in the image surface
 
 \param widget any FLTK widget (e.g., standard, custom, window, GL view) to draw in the image
 \param delta_x and \param delta_y give 
 the position in the image of the top-left corner of the widget
 */
void Fl_Image_Surface::draw(Fl_Widget *widget, int delta_x, int delta_y)
{
  helper->print_widget(widget, delta_x, delta_y);
}


void Fl_Image_Surface::set_current()
{
#if defined(__APPLE__)
  fl_gc = offscreen; fl_window = 0;
  Fl_Surface_Device::set_current();
  Fl_X::set_high_resolution( CGBitmapContextGetWidth(offscreen) > width );
#elif defined(WIN32)
  _sgc=fl_gc; 
  _sw=fl_window;
  _ss = Fl_Surface_Device::surface(); 
  Fl_Surface_Device::set_current();
  fl_gc = fl_makeDC(offscreen); 
   _savedc = SaveDC(fl_gc); 
  fl_window=(HWND)offscreen; 
  fl_push_no_clip();
#else
  pre_window = fl_window; 
  fl_window = offscreen; 
  previous = Fl_Surface_Device::surface(); 
  Fl_Surface_Device::set_current();
  fl_push_no_clip();
#endif
}

#if defined(__APPLE__)

Fl_Quartz_Flipped_Surface_::Fl_Quartz_Flipped_Surface_(int w, int h) : Fl_Quartz_Surface_(w, h) {
}

void Fl_Quartz_Flipped_Surface_::translate(int x, int y) {
  CGContextRestoreGState(fl_gc);
  CGContextSaveGState(fl_gc);
  CGContextTranslateCTM(fl_gc, x, -y);
  CGContextSaveGState(fl_gc);
  CGContextTranslateCTM(fl_gc, 0, height);
  CGContextScaleCTM(fl_gc, 1.0f, -1.0f);
}

void Fl_Quartz_Flipped_Surface_::untranslate() {
  CGContextRestoreGState(fl_gc);
}

const char *Fl_Quartz_Flipped_Surface_::class_id = "Fl_Quartz_Flipped_Surface_";


void Fl_Image_Surface::draw_decorated_window(Fl_Window* win, int delta_x, int delta_y)
{
  int bt = win->decorated_h() - win->h();
  draw(win, delta_x, bt + delta_y ); // draw the window content
  if (win->border()) {
    // draw the window title bar
    helper->translate(delta_x, delta_y);
    CGContextTranslateCTM(fl_gc, 0, bt);
    CGContextScaleCTM(fl_gc, 1, -1);
    void *layer = Fl_X::get_titlebar_layer(win);
    if (layer) {
      Fl_X::draw_layer_to_context(layer, fl_gc, win->w(), bt);
    } else {
      CGImageRef img = Fl_X::CGImage_from_window_rect(win, 0, -bt, win->w(), bt);
      CGContextDrawImage(fl_gc, CGRectMake(0, 0, win->w(), bt), img);
      CGImageRelease(img);
    }
    helper->untranslate();
    CGContextTranslateCTM(fl_gc, delta_x, height+delta_y);
    CGContextScaleCTM(fl_gc, 1.0f, -1.0f);
  }
}

#else

/** Draws a window and its borders and title bar to the image drawing surface. 
 \param win an FLTK window to draw in the image
 \param delta_x and \param delta_y give
 the position in the image of the top-left corner of the window's title bar
*/
void Fl_Image_Surface::draw_decorated_window(Fl_Window* win, int delta_x, int delta_y)
{
#ifdef WIN32
  // draw_decorated_window() will change the current drawing surface, and set it
  // back to us; it's necessary to do some cleaning before
  fl_pop_clip();
  RestoreDC(fl_gc, _savedc);
  DeleteDC(fl_gc);
#elif !defined(__APPLE__)
  fl_pop_clip();
#endif
  helper->draw_decorated_window(win, delta_x, delta_y, this);
}
#endif


//
// End of "$Id$".
//
