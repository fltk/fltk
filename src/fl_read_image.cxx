//
// X11 image reading routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/platform.H>
#include "Fl_Screen_Driver.H"

/**
 Reads an RGB(A) image from the current window or off-screen buffer.
 \param[in] p     pixel buffer, or NULL to allocate one
 \param[in] X,Y   position of top-left of image to read
 \param[in] w,h   width and height of image to read
 \param[in] alpha alpha value for image (0 for none)
 \returns pointer to pixel buffer, or NULL if allocation failed.

 The \p p argument points to a buffer that can hold the image and must
 be at least \p w*h*3 bytes when reading RGB images, or \p w*h*4 bytes
 when reading RGBA images. If NULL, fl_read_image() will create an
 array of the proper size which can be freed using <tt>delete[]</tt>.

 The \p alpha parameter controls whether an alpha channel is created
 and the value that is placed in the alpha channel. If 0, no alpha
 channel is generated.

 \see fl_capture_window()
 */
uchar *fl_read_image(uchar *p, int X, int Y, int w, int h, int alpha) {
  uchar *image_data = NULL;
  Fl_RGB_Image *img;
  if (Fl_Surface_Device::surface()->as_image_surface()) { // read from off_screen buffer
    img = Fl::screen_driver()->read_win_rectangle(X, Y, w, h, 0);
    if (!img) {
      return NULL;
    }
    img->alloc_array = 1;
  } else {
    img = Fl_Screen_Driver::traverse_to_gl_subwindows(Fl_Window::current(), X, Y, w, h, NULL);
  }
  int depth = alpha ? 4 : 3;
  if (img && img->d() != depth) {
    uchar *data = new uchar[img->w() * img->h() * depth];
    if (depth == 4) memset(data, alpha, img->w() * img->h() * depth);
    uchar *d = data;
    const uchar *q;
    int ld = img->ld() ? img->ld() : img->w() * img->d();
    for (int r = 0; r < img->h(); r++) {
      q = img->array + r * ld;
      for (int c = 0; c < img->w(); c++) {
        d[0] = q[0];
        d[1] = q[1];
        d[2] = q[2];
        d += depth; q += img->d();
      }
    }
    Fl_RGB_Image *img2 = new Fl_RGB_Image(data, img->w(), img->h(), depth);
    img2->alloc_array = 1;
    delete img;
    img = img2;
  }
  if (img) {
    if (img->w() != w || img->h() != h) {
      Fl_RGB_Image *img2 = (Fl_RGB_Image*)img->copy(w, h);
      delete img;
      img = img2;
    }
    img->alloc_array = 0;
    image_data = (uchar*)img->array;
    delete img;
  }
  if (p && image_data) {
    memcpy(p, image_data, w * h * depth);
    delete[] image_data;
    image_data = p;
  }
  return image_data;
}

/** Captures the content of a rectangular zone of a mapped window.
 \param win a mapped Fl_Window (derived types including Fl_Gl_Window are also possible)
 \param x,y,w,h window area to be captured. Intersecting sub-windows are captured too.
 \return The captured pixels as an Fl_RGB_Image. The raw and
 drawing sizes of the image can differ. Returns NULL when capture was not successful.
 The image depth may differ between platforms.
 \version 1.4
*/
Fl_RGB_Image *fl_capture_window(Fl_Window *win, int x, int y, int w, int h)
{
  Fl_RGB_Image *rgb = NULL;
  if (win->shown()) {
    rgb = Fl_Screen_Driver::traverse_to_gl_subwindows(win, x, y, w, h, NULL);
    if (rgb) rgb->scale(w, h, 0, 1);
  }
  return rgb;
}
