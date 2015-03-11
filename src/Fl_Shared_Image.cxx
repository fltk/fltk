//
// "$Id$"
//
// Shared image code for the Fast Light Tool Kit (FLTK).
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

#include <stdio.h>
#include <stdlib.h>
#include <FL/fl_utf8.h>
#include "flstring.h"

#include <FL/Fl.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_XBM_Image.H>
#include <FL/Fl_XPM_Image.H>
#include <FL/Fl_Preferences.H>
#include <FL/fl_draw.H>

//
// Global class vars...
//

Fl_Shared_Image **Fl_Shared_Image::images_ = 0;	// Shared images
int	Fl_Shared_Image::num_images_ = 0;	// Number of shared images
int	Fl_Shared_Image::alloc_images_ = 0;	// Allocated shared images

Fl_Shared_Handler *Fl_Shared_Image::handlers_ = 0;// Additional format handlers
int	Fl_Shared_Image::num_handlers_ = 0;	// Number of format handlers
int	Fl_Shared_Image::alloc_handlers_ = 0;	// Allocated format handlers


//
// Typedef the C API sort function type the only way I know how...
//

extern "C" {
  typedef int (*compare_func_t)(const void *, const void *);
}


/** Returns the Fl_Shared_Image* array */
Fl_Shared_Image **Fl_Shared_Image::images() {
  return images_;
}
/** Returns the total number of shared images in the array. */
int Fl_Shared_Image::num_images() {
  return num_images_;
}


//
// 'Fl_Shared_Image::compare()' - Compare two shared images...
//

int
Fl_Shared_Image::compare(Fl_Shared_Image **i0,		// I - First image
                         Fl_Shared_Image **i1) {	// I - Second image
  int i = strcmp((*i0)->name(), (*i1)->name());

  if (i) return i;
  else if (((*i0)->w() == 0 && (*i1)->original_) ||
           ((*i1)->w() == 0 && (*i0)->original_)) return 0;
  else if ((*i0)->w() != (*i1)->w()) return (*i0)->w() - (*i1)->w();
  else return (*i0)->h() - (*i1)->h();
}


/** 
  Creates an empty shared image.
  The constructors create a new shared image record in the image cache.
  
  <P>The constructors are protected and cannot be used directly
  from a program. Use the get() method instead.
*/
Fl_Shared_Image::Fl_Shared_Image() : Fl_Image(0,0,0) {
  name_        = 0;
  refcount_    = 1;
  original_    = 0;
  image_       = 0;
  alloc_image_ = 0;
#if FLTK_ABI_VERSION >= 10304
  scaled_image_= 0;
#endif
}


/** 
  Creates a shared image from its filename and its corresponding Fl_Image* img.
  The constructors create a new shared image record in the image cache.
  
  <P>The constructors are protected and cannot be used directly
  from a program. Use the get() method instead.
*/
Fl_Shared_Image::Fl_Shared_Image(const char *n,		// I - Filename
                                 Fl_Image   *img)	// I - Image
  : Fl_Image(0,0,0) {
  name_ = new char[strlen(n) + 1];
  strcpy((char *)name_, n);

  refcount_    = 1;
  image_       = img;
  alloc_image_ = !img;
  original_    = 1;
#if FLTK_ABI_VERSION >= 10304
  scaled_image_= 0;
#endif

  if (!img) reload();
  else update();
}


//
// 'Fl_Shared_Image::add()' - Add a shared image to the array.
//

void
Fl_Shared_Image::add() {
  Fl_Shared_Image	**temp;		// New image pointer array...

  if (num_images_ >= alloc_images_) {
    // Allocate more memory...
    temp = new Fl_Shared_Image *[alloc_images_ + 32];

    if (alloc_images_) {
      memcpy(temp, images_, alloc_images_ * sizeof(Fl_Shared_Image *));

      delete[] images_;
    }

    images_       = temp;
    alloc_images_ += 32;
  }

  images_[num_images_] = this;
  num_images_ ++;

  if (num_images_ > 1) {
    qsort(images_, num_images_, sizeof(Fl_Shared_Image *),
          (compare_func_t)compare);
  }
}


//
// 'Fl_Shared_Image::update()' - Update the dimensions of the shared images.
//

void
Fl_Shared_Image::update() {
  if (image_) {
    w(image_->w());
    h(image_->h());
    d(image_->d());
    data(image_->data(), image_->count());
  }
}

/**
  The destructor free all memory and server resources that are
  used by the image. The destructor is protected and cannot be
  used directly from a program. Use the Fl_Shared_Image::release() method
  instead.
*/
Fl_Shared_Image::~Fl_Shared_Image() {
  if (name_) delete[] (char *)name_;
  if (alloc_image_) delete image_;
#if FLTK_ABI_VERSION >= 10304
  delete scaled_image_;
#endif
}


//
/** 
  Releases and possibly destroys (if refcount <=0) a shared image. 
  In the latter case, it will reorganize the shared image array so that no hole will occur.
*/
void Fl_Shared_Image::release() {
  int	i;	// Looping var...

  refcount_ --;
  if (refcount_ > 0) return;

  for (i = 0; i < num_images_; i ++)
    if (images_[i] == this) {
      num_images_ --;

      if (i < num_images_) {
        memmove(images_ + i, images_ + i + 1,
               (num_images_ - i) * sizeof(Fl_Shared_Image *));
      }

      break;
    }

  delete this;

  if (num_images_ == 0 && images_) {
    delete[] images_;

    images_       = 0;
    alloc_images_ = 0;
  }
}


//
/** Reloads the shared image from disk */
void Fl_Shared_Image::reload() {
  // Load image from disk...
  int		i;		// Looping var
  FILE		*fp;		// File pointer
  uchar		header[64];	// Buffer for auto-detecting files
  Fl_Image	*img;		// New image

  if (!name_) return;

  if ((fp = fl_fopen(name_, "rb")) != NULL) {
    if (fread(header, 1, sizeof(header), fp)==0) { /* ignore */ }
    fclose(fp);
  } else {
    return;
  }

  // Load the image as appropriate...
  if (memcmp(header, "#define", 7) == 0) // XBM file
    img = new Fl_XBM_Image(name_);
  else if (memcmp(header, "/* XPM */", 9) == 0) // XPM file
    img = new Fl_XPM_Image(name_);
  else {
    // Not a standard format; try an image handler...
    for (i = 0, img = 0; i < num_handlers_; i ++) {
      img = (handlers_[i])(name_, header, sizeof(header));

      if (img) break;
    }
  }

  if (img) {
    if (alloc_image_) delete image_;

    alloc_image_ = 1;

    if ((img->w() != w() && w()) || (img->h() != h() && h())) {
      // Make sure the reloaded image is the same size as the existing one.
      Fl_Image *temp = img->copy(w(), h());
      delete img;
      image_ = temp;
    } else {
      image_ = img;
    }

    update();
  }
}


//
// 'Fl_Shared_Image::copy()' - Copy and resize a shared image...
//

Fl_Image *
Fl_Shared_Image::copy(int W, int H) {
  Fl_Image		*temp_image;	// New image file
  Fl_Shared_Image	*temp_shared;	// New shared image

  // Make a copy of the image we're sharing...
  if (!image_) temp_image = 0;
  else temp_image = image_->copy(W, H);

  // Then make a new shared image...
  temp_shared = new Fl_Shared_Image();

  temp_shared->name_ = new char[strlen(name_) + 1];
  strcpy((char *)temp_shared->name_, name_);

  temp_shared->refcount_    = 1;
  temp_shared->image_       = temp_image;
  temp_shared->alloc_image_ = 1;

  temp_shared->update();

  return temp_shared;
}


//
// 'Fl_Shared_Image::color_average()' - Blend colors...
//

void
Fl_Shared_Image::color_average(Fl_Color c,	// I - Color to blend with
                               float    i) {	// I - Blend fraction
  if (!image_) return;

  image_->color_average(c, i);
  update();
}


//
// 'Fl_Shared_Image::desaturate()' - Convert the image to grayscale...
//

void
Fl_Shared_Image::desaturate() {
  if (!image_) return;

  image_->desaturate();
  update();
}


//
// 'Fl_Shared_Image::draw()' - Draw a shared image...
//
void Fl_Shared_Image::draw(int X, int Y, int W, int H, int cx, int cy) {
#if FLTK_ABI_VERSION >= 10304
  if (!image_) {
    Fl_Image::draw(X, Y, W, H, cx, cy);
    return;
  }
  if (w() == image_->w() && h() == image_->h()) {
    image_->draw(X, Y, W, H, cx, cy);
    return;
  }
  fl_push_clip(X, Y, W, H);
  int done = fl_graphics_driver->draw_scaled(image_, X-cx, Y-cy, w(), h());
  if (!done) {
    if (scaled_image_ && (scaled_image_->w() != w() || scaled_image_->h() != h())) {
      delete scaled_image_;
      scaled_image_ = NULL;
    }
    if (!scaled_image_) {
      Fl_RGB_Scaling previous = RGB_scaling();
      RGB_scaling(scaling_algorithm_); // useless but no harm if image_ is not an Fl_RGB_Image
      scaled_image_ = image_->copy(w(), h());
      RGB_scaling(previous);
    }
    scaled_image_->draw(X-cx, Y-cy, scaled_image_->w(), scaled_image_->h(), 0, 0);
  }
  fl_pop_clip();
#else
  if (image_) image_->draw(X, Y, W, H, cx, cy);
  else Fl_Image::draw(X, Y, W, H, cx, cy);
#endif // FLTK_ABI_VERSION
}

/** Sets the drawing size of the shared image.
 This function gives the shared image its own size, independently from the size of the original image
 that is typically larger than the shared image.
 This can be useful to draw a shared image on a drawing surface whose resolution is higher
 than the drawing unit for this surface. Examples of such drawing surfaces: laser printers,
 PostScript files, PDF printers, retina displays on Apple hardware.

 \param width,height   maximum width and height (in drawing units) to use when drawing the shared image
 \param proportional   if not null, keep the width and height of the shared image proportional to those of its original image
 \param can_expand  if null, the width and height of the shared image will not exceed those of the original image
 
 \version 1.3.4 and requires compiling with FLTK_ABI_VERSION = 10304
 
 Example code: scale an image to fit in a box
 \code
 Fl_Box *b = ...  // a box
 Fl_Shared_Image *shared = Fl_Shared_Image::get("/path/to/picture.jpeg"); // read a picture file
 b->image(shared); // use the shared image as the box image
 b->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_CLIP); // the image is to be drawn centered in the box
 shared->scale(b->w(), b->h(), 1); // set the drawing size of the shared image to the size of the box
 \endcode
 */
void Fl_Shared_Image::scale(int width, int height, int proportional, int can_expand)
{
#if FLTK_ABI_VERSION >= 10304
  w(width);
  h(height);
  if (!image_) return;
  float fw = image_->w() / float(width);
  float fh = image_->h() / float(height);
  if (proportional) {
    if (fh > fw) fw = fh;
    else fh = fw;
  }
  if (!can_expand) {
    if (fw < 1) fw = 1;
    if (fh < 1) fh = 1;
  }
  w(image_->w() / fw);
  h(image_->h() / fh);
#endif
}


Fl_RGB_Scaling Fl_Shared_Image::scaling_algorithm_ = FL_RGB_SCALING_BILINEAR;


//
// 'Fl_Shared_Image::uncache()' - Uncache the shared image...
//

void Fl_Shared_Image::uncache()
{
  if (image_) image_->uncache();
}



/** Finds a shared image from its named and size specifications */
Fl_Shared_Image* Fl_Shared_Image::find(const char *n, int W, int H) {
  Fl_Shared_Image	*key,		// Image key
			**match;	// Matching image

  if (num_images_) {
    key = new Fl_Shared_Image();
    key->name_ = new char[strlen(n) + 1];
    strcpy((char *)key->name_, n);
    key->w(W);
    key->h(H);

    match = (Fl_Shared_Image **)bsearch(&key, images_, num_images_,
                                        sizeof(Fl_Shared_Image *),
                                        (compare_func_t)compare);

    delete key;

    if (match) {
      (*match)->refcount_ ++;
      return *match;
    }
  }

  return 0;
}


/** 
 \brief Find or load an image that can be shared by multiple widgets.
 
 Gets a shared image, if it exists already ; it will return it.
 If it does not exist or if it exist but with other size, 
 then the existing image is deleted and replaced
 by a new image from the n filename of the proper dimension.
 If n is not a valid image filename, then get() will return NULL.
 
 Shared JPEG and PNG images can also be created from memory by using their 
 named memory access constructor.
 
 \param n name of the image
 \param W, H desired size
 
 \see Fl_Shared_Image::find(const char *n, int W, int H)
 \see Fl_Shared_Image::release() 
 \see Fl_JPEG_Image::Fl_JPEG_Image(const char *name, const unsigned char *data)
 \see Fl_PNG_Image::Fl_PNG_Image (const char *name_png, const unsigned char *buffer, int maxsize)
*/
Fl_Shared_Image* Fl_Shared_Image::get(const char *n, int W, int H) {
  Fl_Shared_Image	*temp;		// Image

  if ((temp = find(n, W, H)) != NULL) return temp;

  if ((temp = find(n)) == NULL) {
    temp = new Fl_Shared_Image(n);

    if (!temp->image_) {
      delete temp;
      return NULL;
    }

    temp->add();
  }

  if ((temp->w() != W || temp->h() != H) && W && H) {
    temp = (Fl_Shared_Image *)temp->copy(W, H);
    temp->add();
  }

  return temp;
}

/** Builds a shared image from a pre-existing Fl_RGB_Image
 \param rgb  an Fl_RGB_Image used to build a new shared image.
 \param own_it 1 if the shared image should delete \p rgb when it is itself deleted, 0 otherwise
 \version 1.3.4
 */
Fl_Shared_Image *Fl_Shared_Image::get(Fl_RGB_Image *rgb, int own_it)
{
  Fl_Shared_Image *shared = new Fl_Shared_Image(Fl_Preferences::newUUID(), rgb);
  shared->alloc_image_ = own_it;
  shared->add();
  return shared;
}


/** Adds a shared image handler, which is basically a test function for adding new formats */
void Fl_Shared_Image::add_handler(Fl_Shared_Handler f) {
  int			i;		// Looping var...
  Fl_Shared_Handler	*temp;		// New image handler array...

  // First see if we have already added the handler...
  for (i = 0; i < num_handlers_; i ++) {
    if (handlers_[i] == f) return;
  }

  if (num_handlers_ >= alloc_handlers_) {
    // Allocate more memory...
    temp = new Fl_Shared_Handler [alloc_handlers_ + 32];

    if (alloc_handlers_) {
      memcpy(temp, handlers_, alloc_handlers_ * sizeof(Fl_Shared_Handler));

      delete[] handlers_;
    }

    handlers_       = temp;
    alloc_handlers_ += 32;
  }

  handlers_[num_handlers_] = f;
  num_handlers_ ++;
}



/** Removes a shared image handler */
void Fl_Shared_Image::remove_handler(Fl_Shared_Handler f) {
  int	i;				// Looping var...

  // First see if the handler has been added...
  for (i = 0; i < num_handlers_; i ++) {
    if (handlers_[i] == f) break;
  }

  if (i >= num_handlers_) return;

  // OK, remove the handler from the array...
  num_handlers_ --;

  if (i < num_handlers_) {
    // Shift later handlers down 1...
    memmove(handlers_ + i, handlers_ + i + 1,
           (num_handlers_ - i) * sizeof(Fl_Shared_Handler ));
  }
}


//
// End of "$Id$".
//
