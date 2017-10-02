//
// "$Id$"
//
// Shared image header file for the Fast Light Tool Kit (FLTK).
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

/** \file
   Fl_Shared_Image class. */

#ifndef Fl_Shared_Image_H
#  define Fl_Shared_Image_H

#  include "Fl_Image.H"


// Test function for adding new formats
typedef Fl_Image *(*Fl_Shared_Handler)(const char *name, uchar *header,
                                       int headerlen);

// Shared images class.
/**
  This class supports caching, loading, scaling, and drawing of image files.

  Most applications will also want to link against the fltk_images library
  and call the fl_register_images() function to support standard image
  formats such as BMP, GIF, JPEG, and PNG.

  Images can be requested (loaded) with Fl_Shared_Image::get(), find(),
  and some other methods. All images are cached in an internal list of
  shared images and should be released when they are no longer needed.
  A refcount is used to determine if a released image is to be destroyed
  with delete.

  \see Fl_Shared_Image::get()
  \see Fl_Shared_Image::find()
  \see Fl_Shared_Image::release()
*/
class FL_EXPORT Fl_Shared_Image : public Fl_Image {

  friend class Fl_JPEG_Image;
  friend class Fl_PNG_Image;

private:
  static Fl_RGB_Scaling scaling_algorithm_; // method used to rescale RGB source images
#if FLTK_ABI_VERSION >= 10304
  Fl_Image *scaled_image_;
#endif
protected:

  static Fl_Shared_Image **images_;	// Shared images
  static int	num_images_;		// Number of shared images
  static int	alloc_images_;		// Allocated shared images
  static Fl_Shared_Handler *handlers_;	// Additional format handlers
  static int	num_handlers_;		// Number of format handlers
  static int	alloc_handlers_;	// Allocated format handlers

  const char	*name_;			// Name of image file
  int		original_;		// Original image?
  int		refcount_;		// Number of times this image has been used
  Fl_Image	*image_;		// The image that is shared
  int		alloc_image_;		// Was the image allocated?

  static int	compare(Fl_Shared_Image **i0, Fl_Shared_Image **i1);

  // Use get() and release() to load/delete images in memory...
  Fl_Shared_Image();
  Fl_Shared_Image(const char *n, Fl_Image *img = 0);
  virtual ~Fl_Shared_Image();
  void add();
  void update();

public:
  /** Returns the filename of the shared image */
  const char	*name() { return name_; }

  /** Returns the number of references of this shared image.
    When reference is below 1, the image is deleted.
  */
  int		refcount() { return refcount_; }

  /** Returns whether this is an original image.
    Images loaded from a file or from memory are marked \p original as
    opposed to images created as a copy of another image with different
    size (width or height).
    \note This is useful for debugging (rarely used in user code).
    \since FLTK 1.4.0
  */
  int original() { return original_; }

  void		release();
  void		reload();

  virtual Fl_Image *copy(int W, int H);
  Fl_Image *copy() { return copy(w(), h()); }
  virtual void color_average(Fl_Color c, float i);
  virtual void desaturate();
  virtual void draw(int X, int Y, int W, int H, int cx, int cy);
  void draw(int X, int Y) { draw(X, Y, w(), h(), 0, 0); }
  void scale(int width, int height, int proportional = 1, int can_expand = 0);
  virtual void uncache();

  static Fl_Shared_Image *find(const char *name, int W = 0, int H = 0);
  static Fl_Shared_Image *get(const char *name, int W = 0, int H = 0);
  static Fl_Shared_Image *get(Fl_RGB_Image *rgb, int own_it = 1);
  static Fl_Shared_Image **images();
  static int		num_images();
  static void		add_handler(Fl_Shared_Handler f);
  static void		remove_handler(Fl_Shared_Handler f);
  /** Sets what algorithm is used when resizing a source image.
   The default algorithm is FL_RGB_SCALING_BILINEAR.
   Drawing an Fl_Shared_Image is sometimes performed by first resizing the source image
   and then drawing the resized copy. This occurs, e.g., when drawing to screen under Linux or MSWindows
   after having called Fl_Shared_Image::scale().
   This function controls what method is used when the image to be resized is an Fl_RGB_Image.
   \version 1.3.4 and requires compiling with FLTK_ABI_VERSION = 10304
   */
  static void scaling_algorithm(Fl_RGB_Scaling algorithm) {scaling_algorithm_ = algorithm; }
};

//
// The following function is provided in the fltk_images library and
// registers all of the "extra" image file formats that are not part
// of the core FLTK library...
//

FL_EXPORT extern void fl_register_images();

#endif // !Fl_Shared_Image_H

//
// End of "$Id$"
//
