//
// "$Id$"
//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Device.H>

#if defined(FL_PORTING)
# pragma message "FL_PORTING: optionally implement class Fl_XXX_Image_Surface_Driver for your platform"
Fl_Image_Surface_Driver *Fl_Image_Surface_Driver::newImageSurfaceDriver(int w, int h, int high_res, Fl_Offscreen)
{
  return NULL;
}
#endif


/** Constructor with optional high resolution.
 \param w and \param h set the size of the resulting image. The value of the \p high_res
 parameter controls whether \p w and \p h are interpreted as pixel or FLTK units.
 
 \param high_res If zero, the created image surface is sized at \p w x \p h pixels.
 If non-zero, the pixel size of the created image surface depends on
 the value of the display scale factor (see Fl_Graphics_Driver::scale()):
 the resulting image has the same number of pixels as an area of the display of size
 \p w x \p h expressed in FLTK units.
 If \p highres is non-zero, always use Fl_Image_Surface::highres_image() to get the image data.

 \param off If not null, the image surface is constructed around a pre-existing
 Fl_Offscreen. The caller is responsible for both construction and destruction of this Fl_Offscreen object.
 Is mostly intended for internal use by FLTK.
 \version 1.3.4 (1.3.3 without the \p highres parameter)
 */
Fl_Image_Surface::Fl_Image_Surface(int w, int h, int high_res, Fl_Offscreen off) : Fl_Widget_Surface(NULL) {
  platform_surface = Fl_Image_Surface_Driver::newImageSurfaceDriver(w, h, high_res, off);
  if (platform_surface) driver(platform_surface->driver());
}


/** The destructor. */
Fl_Image_Surface::~Fl_Image_Surface() { delete platform_surface; }

void Fl_Image_Surface::origin(int x, int y) {platform_surface->origin(x, y);}

void Fl_Image_Surface::origin(int *x, int *y) {
  if (platform_surface) platform_surface->origin(x, y);
}

void Fl_Image_Surface::set_current() {
  if (platform_surface) platform_surface->set_current();
}

void Fl_Image_Surface::translate(int x, int y) {
  if (platform_surface) platform_surface->translate(x, y);
}

void Fl_Image_Surface::untranslate() {
  if (platform_surface) platform_surface->untranslate();
}

/** Returns the Fl_Offscreen object associated to the image surface.
 The returned Fl_Offscreen object is deleted when the Fl_Image_Surface object is deleted.
 */
Fl_Offscreen Fl_Image_Surface::offscreen() { 
  return platform_surface ? platform_surface->offscreen : (Fl_Offscreen)0;
}

int Fl_Image_Surface::printable_rect(int *w, int *h)  {return platform_surface->printable_rect(w, h);}


/** Returns an image made of all drawings sent to the Fl_Image_Surface object.
 The returned object contains its own copy of the RGB data.
 The caller is responsible for deleting the image.
 */
Fl_RGB_Image *Fl_Image_Surface::image() {
  bool need_push = (Fl_Surface_Device::surface() != platform_surface);
  if (need_push) Fl_Surface_Device::push_current(platform_surface);
  Fl_RGB_Image *img = platform_surface->image();
  if (need_push) Fl_Surface_Device::pop_current();
  return img;
}

/** Returns a possibly high resolution image made of all drawings sent to the Fl_Image_Surface object.
 The Fl_Image_Surface object should have been constructed with Fl_Image_Surface(W, H, 1).
 The returned Fl_Shared_Image object is scaled to a size of WxH FLTK units and may have a 
 pixel size larger than these values.
 The returned object should be deallocated with Fl_Shared_Image::release() after use.
 \version 1.3.4
 */
Fl_Shared_Image* Fl_Image_Surface::highres_image()
{
  if (!platform_surface) return NULL;
  Fl_Shared_Image *s_img = Fl_Shared_Image::get(image());
  int width, height;
  platform_surface->printable_rect(&width, &height);
  s_img->scale(width, height, 1, 1);
  return s_img;
}

/** Allows to delete the Fl_Image_Surface object while keeping its underlying Fl_Offscreen.
 This member function is intended for internal use by the FLTK library.
 */
Fl_Offscreen Fl_Image_Surface::get_offscreen_before_delete() {
  Fl_Offscreen keep = platform_surface->offscreen;
  platform_surface->offscreen = 0;
  return keep;
}

// implementation of the fl_XXX_offscreen() functions

static Fl_Image_Surface **offscreen_api_surface = NULL;
static int count_offscreens = 0;

static int find_slot(void) { // return an available slot to memorize an Fl_Image_Surface object
  static int max = 0;
  for (int num = 0; num < count_offscreens; num++) {
    if (!offscreen_api_surface[num]) return num;
  }
  if (count_offscreens >= max) {
    max += 20;
    offscreen_api_surface = (Fl_Image_Surface**)realloc(offscreen_api_surface, max * sizeof(void *));
  }
  return count_offscreens++;
}

/** \addtogroup fl_drawings
   @{
   */

/**
   Creation of an offscreen graphics buffer.
   \param w,h     width and height in FLTK units of the buffer.
   \return    the created graphics buffer.
 
 The pixel size of the created graphics buffer is equal to the number of pixels
 in an area of the screen containing the current window sized at \p w,h FLTK units.
 This pixel size varies with the value of the scale factor of this screen.
   */
Fl_Offscreen fl_create_offscreen(int w, int h) {
  int rank = find_slot();
  offscreen_api_surface[rank] = new Fl_Image_Surface(w, h, 1/*high_res*/);
  return offscreen_api_surface[rank]->offscreen();
}

/**  Deletion of an offscreen graphics buffer.
   \param ctx     the buffer to be deleted.
   \note The \p ctx argument must have been created by fl_create_offscreen().
   */
void fl_delete_offscreen(Fl_Offscreen ctx) {
  if (!ctx) return;
  for (int i = 0; i < count_offscreens; i++) {
    if (offscreen_api_surface[i] && offscreen_api_surface[i]->offscreen() == ctx) {
      delete offscreen_api_surface[i];
      offscreen_api_surface[i] = NULL;
      return;
    }
  }
}

/**  Send all subsequent drawing commands to this offscreen buffer.
   \param ctx     the offscreen buffer.
   */
void fl_begin_offscreen(Fl_Offscreen ctx) {
  for (int i = 0; i < count_offscreens; i++) {
    if (offscreen_api_surface[i] && offscreen_api_surface[i]->offscreen() == ctx) {
      Fl_Surface_Device::push_current(offscreen_api_surface[i]);
      return;
    }
  }
}

/** Quit sending drawing commands to the current offscreen buffer.
   */
void fl_end_offscreen() {
  Fl_Surface_Device::pop_current();
}

/** Adapts an offscreen buffer to a changed value of the scale factor.
 The \p ctx argument must have been created by fl_create_offscreen()
 and the calling context must not be between fl_begin_offscreen() and fl_end_offscreen().
 The graphical content of the offscreen is preserved. The current scale factor
 value is given by <tt>Fl_Graphics_Driver::default_driver().scale()</tt>.
 \version 1.4
 */
void fl_scale_offscreen(Fl_Offscreen &ctx) {
  int i, w, h;
  for (i = 0; i < count_offscreens; i++) {
    if (offscreen_api_surface[i] && offscreen_api_surface[i]->offscreen() == ctx) {
      break;
    }
  }
  if (i >= count_offscreens) return;
  Fl_Shared_Image *shared = offscreen_api_surface[i]->highres_image();
  offscreen_api_surface[i]->printable_rect(&w, &h);
  fl_delete_offscreen(ctx);
  ctx = fl_create_offscreen(w, h);
  fl_begin_offscreen(ctx);
  shared->draw(0, 0);
  fl_end_offscreen();
  shared->release();
}

/** @} */


//
// End of "$Id$".
//
