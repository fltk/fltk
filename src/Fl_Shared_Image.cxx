//
// Shared image code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

Fl_Shared_Image **Fl_Shared_Image::images_ = 0; // Shared images
int     Fl_Shared_Image::num_images_ = 0;       // Number of shared images
int     Fl_Shared_Image::alloc_images_ = 0;     // Allocated shared images

Fl_Shared_Handler *Fl_Shared_Image::handlers_ = 0;// Additional format handlers
int     Fl_Shared_Image::num_handlers_ = 0;     // Number of format handlers
int     Fl_Shared_Image::alloc_handlers_ = 0;   // Allocated format handlers


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


/**
  Compares two shared images.

  The order of comparison is:

    -# Image name, usually the filename used to load it
    -# Image width
    -# Image height

  A special case is considered if the width of one of the images is zero
  and the other image is marked \p original. In this case the images match,
  i.e. the comparison returns success (0).

  An image is marked \p original if it was directly loaded from a file or
  from memory as opposed to copied and resized images.

  This comparison is used in Fl_Shared_Image::find() to find an image that
  matches the requested one or to find the position where a new image
  should be entered into the sorted list of shared images.

  It is usually used in two steps:

    -# search with exact width and height
    -# if not found, search again with width = 0 (and height = 0)

  The first step will only return a match if the image exists with the
  same width and height. The second step will match if there is an image
  marked \p original with the same name, regardless of width and height.

  \returns      Whether the images match or their relative sort order (see text).

  \retval       0       the images match
  \retval       <0      Image \p i0 is \e less than image \p i1
  \retval       >0      Image \p i0 is \e greater than image \p i1
*/
int
Fl_Shared_Image::compare(Fl_Shared_Image **i0,          // I - First image
                         Fl_Shared_Image **i1) {        // I - Second image
  int i = strcmp((*i0)->name(), (*i1)->name());

  if (i) return i;
  else if (((*i0)->data_w() == 0 && (*i1)->original_) ||
           ((*i1)->data_w() == 0 && (*i0)->original_)) return 0;
  else if ((*i0)->data_w() != (*i1)->data_w()) return (*i0)->data_w() - (*i1)->data_w();
  else return (*i0)->data_h() - (*i1)->data_h();
}


/**
  Creates an empty shared image.
  The constructors create a new shared image record in the image cache.

  The constructors are protected and cannot be used directly
  from a program. Use the get() method instead.
*/
Fl_Shared_Image::Fl_Shared_Image() : Fl_Image(0,0,0) {
  name_        = 0;
  refcount_    = 1;
  original_    = 0;
  image_       = 0;
  alloc_image_ = 0;
}


/**
  Creates a shared image from its filename and its corresponding Fl_Image* img.
  The constructors create a new shared image record in the image cache.

  The constructors are protected and cannot be used directly
  from a program. Use the get() method instead.
*/
Fl_Shared_Image::Fl_Shared_Image(const char *n,         // I - Filename
                                 Fl_Image   *img)       // I - Image
  : Fl_Image(0,0,0) {
  name_ = new char[strlen(n) + 1];
  strcpy((char *)name_, n);

  refcount_    = 1;
  image_       = img;
  alloc_image_ = !img;
  original_    = 1;

  if (!img) reload();
  else update();
}


/**
  Adds a shared image to the image cache.

  This \b protected method adds an image to the cache, an ordered list
  of shared images. The cache is searched for a matching image whenever
  one is requested, for instance with Fl_Shared_Image::get() or
  Fl_Shared_Image::find().
*/
void
Fl_Shared_Image::add() {
  Fl_Shared_Image       **temp;         // New image pointer array...

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
    int W = w(), H = h();
    w(image_->data_w());
    h(image_->data_h());
    d(image_->d());
    data(image_->data(), image_->count());
    if (W && H) scale(W, H, 0, 1);
  }
}

/**
  The destructor frees all memory and server resources that are
  used by the image.

  The destructor is protected and cannot be used directly from a program.
  Use the Fl_Shared_Image::release() method instead.
*/
Fl_Shared_Image::~Fl_Shared_Image() {
  if (name_) delete[] (char *)name_;
  if (alloc_image_) delete image_;
}


/**
  Releases and possibly destroys (if refcount <= 0) a shared image.

  In the latter case, it will reorganize the shared image array
  so that no hole will occur.
*/
void Fl_Shared_Image::release() {
  int   i;      // Looping var...

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


/** Reloads the shared image from disk. */
void Fl_Shared_Image::reload() {
  // Load image from disk...
  int           i;              // Looping var
  int           count = 0;      // number of bytes read from image header
  FILE          *fp;            // File pointer
  uchar         header[64];     // Buffer for auto-detecting files
  Fl_Image      *img;           // New image

  if (!name_) return;

  if ((fp = fl_fopen(name_, "rb")) != NULL) {
    count = (int)fread(header, 1, sizeof(header), fp);
    fclose(fp);
    if (count == 0)
      return;
  } else {
    return;
  }

  // Load the image as appropriate...
  if (count >= 7 && memcmp(header, "#define", 7) == 0) // XBM file
    img = new Fl_XBM_Image(name_);
  else if (count >= 9 && memcmp(header, "/* XPM */", 9) == 0) // XPM file
    img = new Fl_XPM_Image(name_);
  else {
    // Not a standard format; try an image handler...
    for (i = 0, img = 0; i < num_handlers_; i ++) {
      img = (handlers_[i])(name_, header, count);
      if (img) break;
    }
  }

  if (img) {
    if (alloc_image_) delete image_;

    alloc_image_ = 1;
    image_ = img;
    int W = w();
    int H = h();
    update();
    // Make sure the reloaded image gets the same drawing size as the existing one.
    if (W)
      scale(W, H, 0, 1);
  }
}


//
// 'Fl_Shared_Image::copy()' - Copy and resize a shared image...
//
// Note: intentionally no doxygen docs here.
// For doxygen docs see Fl_Image::copy().

Fl_Image *
Fl_Shared_Image::copy(int W, int H) const {
  Fl_Image              *temp_image;    // New image file
  Fl_Shared_Image       *temp_shared;   // New shared image

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
Fl_Shared_Image::color_average(Fl_Color c,      // I - Color to blend with
                               float    i) {    // I - Blend fraction
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
  if (!image_) {
    Fl_Image::draw(X, Y, W, H, cx, cy);
    return;
  }
  // transiently set the drawing size of image_ to that of the shared image
  int width = image_->w(), height = image_->h();
  image_->scale(w(), h(), 0, 1);
  image_->draw(X, Y, W, H, cx, cy);
  image_->scale(width, height, 0, 1);
}




//
// 'Fl_Shared_Image::uncache()' - Uncache the shared image...
//

void Fl_Shared_Image::uncache()
{
  if (image_) image_->uncache();
}



/** Finds a shared image from its name and size specifications.

  This uses a binary search in the image cache.

  If the image \p name exists with the exact width \p W and height \p H,
  then it is returned.

  If \p W == 0 and the image \p name exists with another size, then the
  \b original image with that \p name is returned.

  In either case the refcount of the returned image is increased.
  The found image should be released with Fl_Shared_Image::release()
  when no longer needed.
*/
Fl_Shared_Image* Fl_Shared_Image::find(const char *name, int W, int H) {
  Fl_Shared_Image       *key,           // Image key
                        **match;        // Matching image

  if (num_images_) {
    key = new Fl_Shared_Image();
    key->name_ = new char[strlen(name) + 1];
    strcpy((char *)key->name_, name);
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
  Find or load an image that can be shared by multiple widgets.

  If the image exists with the requested size, this image will be returned.

  If the image exists, but only with another size, then a new copy with the
  requested size (width \p W and height \p H) will be created as a resized
  copy of the original image. The new image is added to the internal list
  of shared images.

  If the image does not yet exist, then a new image of the proper
  dimension is created from the filename \p name. The original image
  from filename \p name is always added to the list of shared images in
  its original size. If the requested size differs, then the resized
  copy with width \p W and height \p H is also added to the list of
  shared images.

  \note If the sizes differ, then \e two images are created as mentioned above.
        This is intentional so the original image is cached and preserved.
        If you request the same image with another size later, then the
        \b original image will be found, copied, resized, and returned.

  Shared JPEG and PNG images can also be created from memory by using their
  named memory access constructor.

  You should release() the image when you're done with it.

  \param name name of the image
  \param W, H desired size

  \see Fl_Shared_Image::find(const char *name, int W, int H)
  \see Fl_Shared_Image::release()
  \see Fl_JPEG_Image::Fl_JPEG_Image(const char *name, const unsigned char *data)
  \see Fl_PNG_Image::Fl_PNG_Image (const char *name_png, const unsigned char *buffer, int maxsize)
*/
Fl_Shared_Image* Fl_Shared_Image::get(const char *name, int W, int H) {
  Fl_Shared_Image       *temp;          // Image

  if ((temp = find(name, W, H)) != NULL) return temp;

  if ((temp = find(name)) == NULL) {
    temp = new Fl_Shared_Image(name);

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

/** Builds a shared image from a pre-existing Fl_RGB_Image.

 \param[in] rgb         an Fl_RGB_Image used to build a new shared image.
 \param[in] own_it      1 if the shared image should delete \p rgb when
                        it is itself deleted, 0 otherwise

 \version 1.3.4
*/
Fl_Shared_Image *Fl_Shared_Image::get(Fl_RGB_Image *rgb, int own_it)
{
  Fl_Shared_Image *shared = new Fl_Shared_Image(Fl_Preferences::newUUID(), rgb);
  shared->alloc_image_ = own_it;
  shared->add();
  return shared;
}


/** Adds a shared image handler, which is basically a test function
  for adding new image formats.

  This function will be called when an Fl_Shared_Image is to be loaded
  (for instance with Fl_Shared_Image::get()) and the image type is not
  known to FLTK.

  All registered image handlers will be called in the order of registration.
  You should always call fl_register_images() before adding your own
  handlers - unless you need to override a known image file type which
  should be rare.

  \see Fl_Shared_Handler for more information of the function you need
    to define.
*/
void Fl_Shared_Image::add_handler(Fl_Shared_Handler f) {
  int                   i;              // Looping var...
  Fl_Shared_Handler     *temp;          // New image handler array...

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


/** Removes a shared image handler. */
void Fl_Shared_Image::remove_handler(Fl_Shared_Handler f) {
  int   i;                              // Looping var...

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
