//
// Bitmap drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

/** Create a bit mask */
Fl_Bitmask fl_create_bitmask(int w, int h, const uchar *array) {
  return fl_graphics_driver->create_bitmask(w, h, array);
}

/** delete a bit mask */
void fl_delete_bitmask(Fl_Bitmask bm) {
  return Fl_Graphics_Driver::default_driver().delete_bitmask(bm);
}

// Create a 1-bit mask used for alpha blending
Fl_Bitmask fl_create_alphamask(int w, int h, int d, int ld, const uchar *array) {
  Fl_Bitmask bm;
  int bmw = (w + 7) / 8;
  uchar *bitmap = new uchar[bmw * h];
  uchar *bitptr, bit;
  const uchar *dataptr;
  int x, y;
  static uchar dither[16][16] = { // Simple 16x16 Floyd dither
    { 0,   128, 32,  160, 8,   136, 40,  168,
      2,   130, 34,  162, 10,  138, 42,  170 },
    { 192, 64,  224, 96,  200, 72,  232, 104,
      194, 66,  226, 98,  202, 74,  234, 106 },
    { 48,  176, 16,  144, 56,  184, 24,  152,
      50,  178, 18,  146, 58,  186, 26,  154 },
    { 240, 112, 208, 80,  248, 120, 216, 88,
      242, 114, 210, 82,  250, 122, 218, 90 },
    { 12,  140, 44,  172, 4,   132, 36,  164,
      14,  142, 46,  174, 6,   134, 38,  166 },
    { 204, 76,  236, 108, 196, 68,  228, 100,
      206, 78,  238, 110, 198, 70,  230, 102 },
    { 60,  188, 28,  156, 52,  180, 20,  148,
      62,  190, 30,  158, 54,  182, 22,  150 },
    { 252, 124, 220, 92,  244, 116, 212, 84,
      254, 126, 222, 94,  246, 118, 214, 86 },
    { 3,   131, 35,  163, 11,  139, 43,  171,
      1,   129, 33,  161, 9,   137, 41,  169 },
    { 195, 67,  227, 99,  203, 75,  235, 107,
      193, 65,  225, 97,  201, 73,  233, 105 },
    { 51,  179, 19,  147, 59,  187, 27,  155,
      49,  177, 17,  145, 57,  185, 25,  153 },
    { 243, 115, 211, 83,  251, 123, 219, 91,
      241, 113, 209, 81,  249, 121, 217, 89 },
    { 15,  143, 47,  175, 7,   135, 39,  167,
      13,  141, 45,  173, 5,   133, 37,  165 },
    { 207, 79,  239, 111, 199, 71,  231, 103,
      205, 77,  237, 109, 197, 69,  229, 101 },
    { 63,  191, 31,  159, 55,  183, 23,  151,
      61,  189, 29,  157, 53,  181, 21,  149 },
    { 254, 127, 223, 95,  247, 119, 215, 87,
      253, 125, 221, 93,  245, 117, 213, 85 }
  };

  // Generate a 1-bit "screen door" alpha mask; not always pretty, but
  // definitely fast...  In the future we may be able to support things
  // like the RENDER extension in XFree86, when available, to provide
  // true RGBA-blended rendering.  See:
  //
  //     http://www.xfree86.org/~keithp/render/protocol.html
  //
  // for more info on XRender...
  //
  // MacOS already provides alpha blending support and has its own
  // fl_create_alphamask() function...
  memset(bitmap, 0, bmw * h);

  for (dataptr = array + d - 1, y = 0; y < h; y ++, dataptr += ld)
    for (bitptr = bitmap + y * bmw, bit = 1, x = 0; x < w; x ++, dataptr += d) {
      if (*dataptr > dither[x & 15][y & 15])
        *bitptr |= bit;
      if (bit < 128) bit <<= 1;
      else {
        bit = 1;
        bitptr ++;
      }
    }

  bm = fl_create_bitmask(w, h, bitmap);
  delete[] bitmap;

  return (bm);
}

void Fl_Bitmap::draw(int XP, int YP, int WP, int HP, int cx, int cy) {
  fl_graphics_driver->draw_bitmap(this, XP, YP, WP, HP, cx, cy);
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
    fl_delete_bitmask((Fl_Bitmask)id_);
    id_ = 0;
  }
}

void Fl_Bitmap::label(Fl_Widget* widget) {
  widget->image(this);
}

void Fl_Bitmap::label(Fl_Menu_Item* m) {
  m->label(FL_IMAGE_LABEL, (const char*)this);
}

Fl_Image *Fl_Bitmap::copy(int W, int H) {
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
