//
// Bitmap drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
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

/** \fn Fl_Bitmap::Fl_Bitmap(const char *array, int W, int H)
  The constructors create a new bitmap from the specified bitmap data.*/

/** \fn Fl_Bitmap::Fl_Bitmap(const unsigned char *array, int W, int H)
  The constructors create a new bitmap from the specified bitmap data.*/

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Bitmap.H>

#include <stdlib.h>

void Fl_Bitmap::draw(int XP, int YP, int WP, int HP, int cx, int cy) {
  fl_graphics_driver->draw_bitmap(this, XP, YP, WP, HP, cx, cy);
}


/** The constructors create a new bitmap from the specified bitmap data.
 If the provided array is too small to contain all the image data, the
 constructor will not generate the bitmap to avoid illegal memory read
 access and instead set \c data to NULL and \c ld to \c ERR_MEMORY_ACCESS.
 \param bits bitmap data, one pixel per bit, rows are rounded to the next byte
 \param bits_length length of the \p bits array in bytes
 \param W image width in pixels
 \param H image height in pixels
 \see Fl_Bitmap(const char *bits, int bits_length, int W, int H),
      Fl_Bitmap(const uchar *bits, int W, int H)
*/
Fl_Bitmap::Fl_Bitmap(const uchar *bits, int bits_length, int W, int H) :
  Fl_Image(W,H,0),
  array((const uchar *)bits),
  alloc_array(0),
  id_(0),
  cache_w_(0),
  cache_h_(0)
{
  int rowBytes = (W+7)>>3;
  int min_length = rowBytes * H;
  if (bits_length >= min_length) {
    data((const char **)&array, 1);
  } else {
    array = NULL;
    data(NULL, 0);
    ld(ERR_MEMORY_ACCESS);
  }
}


/** The constructors create a new bitmap from the specified bitmap data.
 If the provided array is too small to contain all the image data, the
 constructor will not generate the bitmap to avoid illegal memory read
 access and instead set \c data to NULL and \c ld to \c ERR_MEMORY_ACCESS.
 \param bits bitmap data, one pixel per bit, rows are rounded to the next byte
 \param bits_length length of the \p bits array in bytes
 \param W image width in pixels
 \param H image height in pixels
 \see Fl_Bitmap(const uchar *bits, int bits_length, int W, int H),
      Fl_Bitmap(const char *bits, int W, int H)
 */
Fl_Bitmap::Fl_Bitmap(const char *bits, int bits_length, int W, int H) :
  Fl_Image(W,H,0),
  array((const uchar *)bits),
  alloc_array(0),
  id_(0),
  cache_w_(0),
  cache_h_(0)
{
  int rowBytes = (W+7)>>3;
  int min_length = rowBytes * H;
  if (bits_length >= min_length) {
    data((const char **)&array, 1);
  } else {
    array = NULL;
    data(NULL, 0);
    ld(ERR_MEMORY_ACCESS);
  }
}


/**
  The destructor frees all memory and server resources that are used by
  the bitmap.
*/
Fl_Bitmap::~Fl_Bitmap() {
  uncache();
  if (alloc_array) delete[] (uchar *)array;
}

void Fl_Bitmap::uncache() {
  if (id_) {
    fl_graphics_driver->delete_bitmask(id_);
    id_ = 0;
  }
}

void Fl_Bitmap::label(Fl_Widget* widget) {
  widget->image(this);
}

void Fl_Bitmap::label(Fl_Menu_Item* m) {
  m->label(FL_IMAGE_LABEL, (const char*)this);
}

Fl_Image *Fl_Bitmap::copy(int W, int H) const {
  Fl_Bitmap     *new_image;     // New RGB image
  uchar         *new_array;     // New array for image data

  // Optimize the simple copy where the width and height are the same...
  if (W == data_w() && H == data_h()) {
    new_array = new uchar [H * ((W + 7) / 8)];
    memcpy(new_array, array, H * ((W + 7) / 8));

    new_image = new Fl_Bitmap(new_array, W, H);
    new_image->alloc_array = 1;
    return new_image;
  }
  if (W <= 0 || H <= 0) return 0;

  // OK, need to resize the image data; allocate memory and
  uchar         *new_ptr,       // Pointer into new array
                new_bit,        // Bit for new array
                old_bit;        // Bit for old array
  const uchar   *old_ptr;       // Pointer into old array
  int           sx, sy,         // Source coordinates
                dx, dy,         // Destination coordinates
                xerr, yerr,     // X & Y errors
                xmod, ymod,     // X & Y moduli
                xstep, ystep;   // X & Y step increments


  // Figure out Bresenham step/modulus values...
  xmod   = data_w() % W;
  xstep  = data_w() / W;
  ymod   = data_h() % H;
  ystep  = data_h() / H;

  // Allocate memory for the new image...
  new_array = new uchar [H * ((W + 7) / 8)];
  new_image = new Fl_Bitmap(new_array, W, H);
  new_image->alloc_array = 1;

  memset(new_array, 0, H * ((W + 7) / 8));

  // Scale the image using a nearest-neighbor algorithm...
  for (dy = H, sy = 0, yerr = H, new_ptr = new_array; dy > 0; dy --) {
    for (dx = W, xerr = W, old_ptr = array + sy * ((data_w() + 7) / 8), sx = 0, new_bit = 1;
         dx > 0;
         dx --) {
      old_bit = (uchar)(1 << (sx & 7));
      if (old_ptr[sx / 8] & old_bit) *new_ptr |= new_bit;

      if (new_bit < 128) new_bit <<= 1;
      else {
        new_bit = 1;
        new_ptr ++;
      }

      sx   += xstep;
      xerr -= xmod;

      if (xerr <= 0) {
        xerr += W;
        sx ++;
      }
    }

    if (new_bit > 1) new_ptr ++;

    sy   += ystep;
    yerr -= ymod;
    if (yerr <= 0) {
      yerr += H;
      sy ++;
    }
  }

  return new_image;
}
