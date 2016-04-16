//
// "$Id$"
//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#if defined(FL_PORTING)
# pragma message "FL_PORTING: optionally implement class Fl_XXX_Image_Surface_Driver for your platform"
Fl_Image_Surface_Driver *Fl_Image_Surface_Driver::newImageSurfaceDriver(int w, int h, int high_res, Fl_Offscreen)
{
  return NULL;
}
#endif


/** Constructor with optional high resolution.
 \param w and \param h give the size in pixels of the resulting image.
 \param high_res if non-zero, the surface pixel size is twice as high and wide as w and h,
 which is useful to draw it later on a high resolution display (e.g., retina display).
 This is implemented for the Mac OS platform only.
 If \p highres is non-zero, use Fl_Image_Surface::highres_image() to get the image data.
 \param pixmap is used internally by FLTK; applications just use its default value.
 \version 1.3.4 (1.3.3 without the highres parameter)
 */
Fl_Image_Surface::Fl_Image_Surface(int w, int h, int high_res, Fl_Offscreen pixmap) : Fl_Widget_Surface(NULL) {
  platform_surface = Fl_Image_Surface_Driver::newImageSurfaceDriver(w, h, high_res, pixmap);
  driver(platform_surface->driver());
}


/** The destructor. */
Fl_Image_Surface::~Fl_Image_Surface() { delete platform_surface; }

void Fl_Image_Surface::origin(int x, int y) {platform_surface->origin(x, y);}

void Fl_Image_Surface::origin(int *x, int *y) {platform_surface->origin(x, y);}

void Fl_Image_Surface::set_current() {platform_surface->set_current();}

/** Stop sending graphics commands to the surface */
void Fl_Image_Surface::end_current() {platform_surface->end_current();}

void Fl_Image_Surface::translate(int x, int y) {platform_surface->translate(x, y);}

void Fl_Image_Surface::untranslate() {platform_surface->untranslate();}

Fl_Offscreen Fl_Image_Surface::offscreen() {return platform_surface->offscreen;}

int Fl_Image_Surface::printable_rect(int *w, int *h)  {return platform_surface->printable_rect(w, h);}


/** Returns an image made of all drawings sent to the Fl_Image_Surface object.
 The returned object contains its own copy of the RGB data.
 The caller is responsible for deleting the image.
 */
Fl_RGB_Image *Fl_Image_Surface::image() {return platform_surface->image();}

/** Returns a possibly high resolution image made of all drawings sent to the Fl_Image_Surface object.
 The Fl_Image_Surface object should have been constructed with Fl_Image_Surface(W, H, 1).
 The returned image is scaled to a size of WxH drawing units and may have a pixel size twice as wide and high.
 The returned object should be deallocated with Fl_Shared_Image::release() after use.
 \version 1.3.4
 */
Fl_Shared_Image* Fl_Image_Surface::highres_image()
{
  Fl_Shared_Image *s_img = Fl_Shared_Image::get(platform_surface->image());
  int width, height;
  platform_surface->printable_rect(&width, &height);
  s_img->scale(width, height);
  return s_img;
}

/** Allows to delete the Fl_Image_Surface object while keeping its underlying Fl_Offscreen
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
   \param w,h     width and height in pixels of the buffer.
   \return    the created graphics buffer.
   */
Fl_Offscreen fl_create_offscreen(int w, int h) {
  int rank = find_slot();
  offscreen_api_surface[rank] = new Fl_Image_Surface(w, h, 0);
  return offscreen_api_surface[rank]->offscreen();
}

/**  Deletion of an offscreen graphics buffer.
   \param ctx     the buffer to be deleted.
   */
void fl_delete_offscreen(Fl_Offscreen ctx) {
  if (!ctx) return;
  for (int i = 0; i < count_offscreens; i++) {
    if (offscreen_api_surface[i] && offscreen_api_surface[i]->offscreen() == ctx) {
      delete offscreen_api_surface[i];
      offscreen_api_surface[i] = NULL;
    }
  }
}

static int stack_current_offscreen[16];
static unsigned stack_height = 0;

/**  Send all subsequent drawing commands to this offscreen buffer.
   \param ctx     the offscreen buffer.
   */
void fl_begin_offscreen(Fl_Offscreen ctx) {
  for (int i = 0; i < count_offscreens; i++) {
    if (offscreen_api_surface[i] && offscreen_api_surface[i]->offscreen() == ctx) {
      offscreen_api_surface[i]->set_current();
      if (stack_height < sizeof(stack_current_offscreen)/sizeof(int)) {
        stack_current_offscreen[stack_height++] = i;
      } else {
        fprintf(stderr, "FLTK fl_begin_offscreen Stack overflow error\n");
      }
      return;
    }
  }
}

/** Quit sending drawing commands to the current offscreen buffer.
   */
void fl_end_offscreen() {
  if (stack_height > 0) {
    int i = stack_current_offscreen[--stack_height];
    offscreen_api_surface[i]->end_current();
  }
}

/** @} */


//
// End of "$Id$".
//
