//
// "$Id$"
//
// X11 image reading routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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
 */
uchar *fl_read_image(uchar *p, int X, int Y, int w, int h, int alpha) {
  uchar *image_data = NULL;
  Fl_RGB_Image *img;
  if (fl_find(fl_window) == 0) { // read from off_screen buffer
    img = Fl::screen_driver()->read_win_rectangle(X, Y, w, h);
    if (!img) {
      return NULL;
    }
    img->alloc_array = 1;
  } else {
    img = Fl::screen_driver()->traverse_to_gl_subwindows(Fl_Window::current(), X, Y, w, h, NULL);
  }
  int depth = alpha ? 4 : 3;
  if (img->d() != depth) {
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

//
// End of "$Id$".
//
