//
// "$Id$"
//
// Bitmap header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2017 by Bill Spitzak and others.
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

/* \file
   Fl_Bitmap widget . */

#ifndef Fl_Bitmap_H
#define Fl_Bitmap_H
#include "Fl_Image.H"
#include "Fl_Widget.H" // for fl_uintptr_t

class Fl_Widget;
struct Fl_Menu_Item;

/**
  The Fl_Bitmap class supports caching and drawing of mono-color
  (bitmap) images. Images are drawn using the current color.
*/
class FL_EXPORT Fl_Bitmap : public Fl_Image {
  friend class Fl_Graphics_Driver;
public:
  /** pointer to raw bitmap data */
  const uchar *array;
  /** Non-zero if array points to bitmap data allocated internally */
  int alloc_array;
  
private:
  int prepare(int XP, int YP, int WP, int HP, int &cx, int &cy,
	    int &X, int &Y, int &W, int &H);
  /** for internal use */
  fl_uintptr_t id_;
  float cache_scale_; // graphics scaling value when id_ was computed
  
protected:
  virtual int draw_scaled(int X, int Y, int W, int H);

public:
  /** The constructors create a new bitmap from the specified bitmap data */
  Fl_Bitmap(const uchar *bits, int W, int H) :
    Fl_Image(W,H,0), array(bits), alloc_array(0), id_(0), cache_scale_(1) {data((const char **)&array, 1);}
  /** The constructors create a new bitmap from the specified bitmap data */
  Fl_Bitmap(const char *bits, int W, int H) :
    Fl_Image(W,H,0), array((const uchar *)bits), alloc_array(0), id_(0), cache_scale_(1) {data((const char **)&array, 1);}
  virtual ~Fl_Bitmap();
  virtual Fl_Image *copy(int W, int H);
  Fl_Image *copy() { return copy(w(), h()); }
  virtual void draw(int X, int Y, int W, int H, int cx=0, int cy=0);
  void draw(int X, int Y) {draw(X, Y, w(), h(), 0, 0);}
  virtual void label(Fl_Widget*w);
  virtual void label(Fl_Menu_Item*m);
  virtual void uncache();
};

#endif

//
// End of "$Id$".
//
