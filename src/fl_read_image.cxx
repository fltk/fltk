//
// "$Id$"
//
// X11 image reading routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2014 by Bill Spitzak and others.
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
#include <FL/Fl_Screen_Driver.H>

/**
 Reads an RGB(A) image from the current window or off-screen buffer.
 \param[in] p     pixel buffer, or NULL to allocate one
 \param[in] X,Y   position of top-left of image to read
 \param[in] W,H   width and height of image to read
 \param[in] alpha alpha value for image (0 for none)
 \returns pointer to pixel buffer, or NULL if allocation failed.
 
 The \p p argument points to a buffer that can hold the image and must
 be at least \p W*H*3 bytes when reading RGB images, or \p W*H*4 bytes
 when reading RGBA images. If NULL, fl_read_image() will create an
 array of the proper size which can be freed using <tt>delete[]</tt>.
 
 The \p alpha parameter controls whether an alpha channel is created
 and the value that is placed in the alpha channel. If 0, no alpha
 channel is generated.
 */
uchar *fl_read_image(uchar *p, int X, int Y, int w, int h, int alpha) {
  return Fl::screen_driver()->read_image(p, X, Y, w, h, alpha);
}

//
// End of "$Id$".
//
