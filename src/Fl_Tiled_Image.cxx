//
// "$Id$"
//
// Tiled image code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
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
#include <FL/Fl_Tiled_Image.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>

/**
  The constructors create a new tiled image containing the specified image.
  Use a width and height of 0 to tile the whole window/widget.

  \note Due to implementation constraints in FLTK 1.3.3 and later width
    and height of 0 may not work as expected when used as background image
    in widgets other than windows. You may need to center and clip the
    image (label) and set the label type to FL_NORMAL_LABEL. Doing so will
    let the tiled image fill the whole widget as its background image.
    Other combinations of label flags may or may not work.

  \code
  #include "bg.xpm"
  Fl_Pixmap *bg_xpm = new Fl_Pixmap(bg_xpm);
  Fl_Tiled_Image *bg_tiled = new Fl_Tiled_Image(bg_xpm,0,0);

  Fl_Box *box = new Fl_Box(40,40,300,100,"");
  box->box(FL_UP_BOX);
  box->labeltype(FL_NORMAL_LABEL);
  box->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_CLIP);
  box->image(bg_tiled);
  \endcode

  \note Setting an image (label) for a window may not work as expected due
    to implementation constraints in FLTK 1.3.x and maybe later. The reason
    is the way Fl::scheme() initializes the window's label type and image.
    A possible workaround is to use another Fl_Group as the only child widget
    and to set the background image for this group as described above.

  \todo Fix Fl_Tiled_Image as background image for widgets and windows
    and fix the implementation of Fl::scheme(const char *).
*/
Fl_Tiled_Image::Fl_Tiled_Image(Fl_Image *i,	// I - Image to tile
                               int      W,	// I - Width of tiled area
			       int      H) :	// I - Height of tiled area
  Fl_Image(W,H,0) {
  image_       = i;
  alloc_image_ = 0;

  // giving to the tiled image the screen size may fail with multiscreen
  // configurations, so we leave it with w = h = 0 (STR #3106)
  // if (W == 0) w(Fl::w());
  // if (H == 0) h(Fl::h());
}
/**
  The destructor frees all memory and server resources that are used by
  the tiled image.
*/
  Fl_Tiled_Image::~Fl_Tiled_Image() {
  if (alloc_image_) delete image_;
}


//
// 'Fl_Tiled_Image::copy()' - Copy and resize a tiled image...
//

Fl_Image *			// O - New image
Fl_Tiled_Image::copy(int W,	// I - New width
                     int H) {	// I - New height
  if (W == w() && H == h()) return this;
  else return new Fl_Tiled_Image(image_, W, H);
}


//
// 'Fl_Tiled_Image::color_average()' - Blend colors...
//

void
Fl_Tiled_Image::color_average(Fl_Color c,	// I - Color to blend with
                              float    i) {	// I - Blend fraction
  if (!alloc_image_) {
    image_       = image_->copy();
    alloc_image_ = 1;
  }

  image_->color_average(c, i);
}


//
// 'Fl_Tiled_Image::desaturate()' - Convert the image to grayscale...
//

void
Fl_Tiled_Image::desaturate() {
  if (!alloc_image_) {
    image_       = image_->copy();
    alloc_image_ = 1;
  }

  image_->desaturate();
}


//
// 'Fl_Tiled_Image::draw()' - Draw a tiled image.
//
/**
  Draws a tiled image.

  Tiled images can be used as background images for widgets and windows.
  However, due to implementation constraints, you must take care when
  setting label types and alignment flags. Only certain combinations work as
  expected, others may yield unexpected results and undefined behavior.

  This draw method can draw multiple copies of one image in an area given
  by \p X, \p Y, \p W, \p H.

  The optional arguments \p cx and \p cy can be used to crop the image
  starting at offsets (cx, cy). \p cx and \p cy must be \>= 0 (negative values
  are ignored). If one of the values is greater than the image width or height
  resp. (\p cx \>= image()->w() or \p cy \>= image()->h()) nothing is drawn,
  because the resulting image would be empty.

  After calculating the resulting image size the image is drawn as often
  as necessary to fill the given area, starting at the top left corner.

  If both \p W and \p H are 0 the image is repeated as often as necessary
  to fill the entire window, unless there is a valid clip region. If you
  want to fill only one particular widget's background, then you should
  either set a clip region in your draw() method or use the label alignment
  flags \p FL_ALIGN_INSIDE|FL_ALIGN_CLIP to make sure the image is clipped.

  This may be improved in a later version of the library.
*/
void
Fl_Tiled_Image::draw(int X,	// I - Starting X position
                     int Y,	// I - Starting Y position
		     int W,	// I - Width of area to be filled
		     int H,	// I - Height of area to be filled
		     int cx,	// I - "Source" X position
		     int cy) {	// I - "Source" Y position

  int iw = image_->w();		// effective image width
  int ih = image_->h();		// effective image height

  if (!iw || !ih) return;
  if (cx >= iw || cy >= ih) return;

  if (cx < 0) cx = 0;		// ignore negative values
  if (cy < 0) cy = 0;

  // W and H null means the image is potentially as large as the current window
  // or widget. The latter can not be checked here, hence we use the whole
  // window as well and rely on appropriate clipping. See comments above.
  // This should be fixed! (AlbrechtS, 01 Mar 2015)

  if (W == 0 && H == 0 && Fl_Window::current()) {
    W = Fl_Window::current()->w();
    H = Fl_Window::current()->h();
    X = Y = 0;
  }

  if (W == 0 || H == 0) return;

  fl_push_clip(X, Y, W, H);

  if (cx > 0) iw -= cx;		// crop image
  if (cy > 0) ih -= cy;

  for (int yy = Y; yy < H; yy += ih) {
    if (fl_not_clipped(X,yy,W,ih)) {
      for (int xx = X; xx < W; xx += iw) {
	if (fl_not_clipped(xx,yy,iw,ih)) {
	  image_->draw(xx,yy,iw,ih,cx,cy);
	}
      }
    }
  }
  fl_pop_clip();
}


//
// End of "$Id$".
//
