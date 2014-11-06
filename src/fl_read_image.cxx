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

#include <FL/x.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "flstring.h"

#ifdef DEBUG
#  include <stdio.h>
#endif // DEBUG

#if defined(__APPLE__)
#  include "fl_read_image_mac.cxx"
#else
#  include <FL/Fl_RGB_Image.H>
#  include <FL/Fl_Window.H>
#  include <FL/Fl_Plugin.H>
#  include <FL/Fl_Device.H>

static uchar *read_win_rectangle(uchar *p, int X, int Y, int w, int h, int alpha);


static void write_image_inside(Fl_RGB_Image *to, Fl_RGB_Image *from, int to_x, int to_y)
/* Copy the image "from" inside image "to" with its top-left angle at coordinates to_x, to_y.
 Also, exchange top and bottom of "from". Image depth can differ between "to" and "from".
 */
{
  int to_ld = (to->ld() == 0? to->w() * to->d() : to->ld());
  int from_ld = (from->ld() == 0? from->w() * from->d() : from->ld());
  uchar *tobytes = (uchar*)to->array + to_y * to_ld + to_x * to->d();
  const uchar *frombytes = from->array + (from->h() - 1) * from_ld;
  for (int i = from->h() - 1; i >= 0; i--) {
    if (from->d() == to->d()) memcpy(tobytes, frombytes, from->w() * from->d());
    else {
      for (int j = 0; j < from->w(); j++) {
        memcpy(tobytes + j * to->d(), frombytes + j * from->d(), from->d());
      }
    }
    tobytes += to_ld;
    frombytes -= from_ld;
  }
}

/* Captures rectangle x,y,w,h from a mapped window or GL window.
 All sub-GL-windows that intersect x,y,w,h, and their subwindows, are also captured.
 
 Arguments when this function is initially called:
 g: a window or GL window
 p: as in fl_read_image()
 x,y,w,h: a rectangle in window g's coordinates
 alpha: as in fl_read_image()
 full_img: NULL
 
 Arguments when this function recursively calls itself:
 g: an Fl_Group
 p: as above
 x,y,w,h: a rectangle in g's coordinates if g is a window, or in g's parent window coords if g is a group
 alpha: as above
 full_img: NULL, or a previously captured image that encompasses the x,y,w,h rectangle and that
 will be partially overwritten with the new capture
 
 Return value:
 An Fl_RGB_Image* of depth 4 if alpha>0 or 3 if alpha = 0 containing the captured pixels.
 */
static Fl_RGB_Image *traverse_to_gl_subwindows(Fl_Group *g, uchar *p, int x, int y, int w, int h, int alpha,
                                               Fl_RGB_Image *full_img)
{
  if ( g->as_gl_window() ) {
    Fl_Plugin_Manager pm("fltk:device");
    Fl_Device_Plugin *pi = (Fl_Device_Plugin*)pm.plugin("opengl.device.fltk.org");
    if (!pi) return full_img;
    Fl_RGB_Image *img = pi->rectangle_capture(g, x, y, w, h); // bottom to top image
    if (full_img) full_img = img; // top and bottom will be exchanged later
    else { // exchange top and bottom to get a proper FLTK image
      uchar *data = ( p ? p : new uchar[img->w() * img->h() * (alpha?4:3)] );
      full_img = new Fl_RGB_Image(data, img->w(), img->h(), alpha?4:3);
      if (!p) full_img->alloc_array = 1;
      if (alpha) memset(data, alpha, img->w() * img->h() * 4);
      write_image_inside(full_img, img, 0, 0);
      delete img;
    }
  }
  else if ( g->as_window() && (!full_img || (g->window() && g->window()->as_gl_window())) ) {
    // the starting window or one inside a GL window
    if (full_img) g->as_window()->make_current();
    uchar *image_data;
    int alloc_img = (full_img != NULL || p == NULL); // false means use p, don't alloc new memory for image
#ifdef __APPLE_CC__
    // on Darwin + X11, read_win_rectangle() sometimes returns NULL when there are subwindows
    do image_data = read_win_rectangle( (alloc_img ? NULL : p), x, y, w, h, alpha); while (!image_data);
#else
    image_data = read_win_rectangle( (alloc_img ? NULL : p), x, y, w, h, alpha);
#endif
    full_img = new Fl_RGB_Image(image_data, w, h, alpha?4:3);
    if (alloc_img) full_img->alloc_array = 1;
  }
  int n = g->children();
  for (int i = 0; i < n; i++) {
    Fl_Widget *c = g->child(i);
    if ( !c->visible() || !c->as_group()) continue;
    if ( c->as_window() ) {
      int origin_x = x; // compute intersection of x,y,w,h and the c window
      if (x < c->x()) origin_x = c->x();
      int origin_y = y;
      if (y < c->y()) origin_y = c->y();
      int width = c->w();
      if (origin_x + width > c->x() + c->w()) width = c->x() + c->w() - origin_x;
      if (origin_x + width > x + w) width = x + w - origin_x;
      int height = c->w();
      if (origin_y + height > c->y() + c->h()) height = c->y() + c->h() - origin_y;
      if (origin_y + height > y + h) height = y + h - origin_y;
      if (width > 0 && height > 0) {
        Fl_RGB_Image *img = traverse_to_gl_subwindows(c->as_window(), p, origin_x - c->x(),
                                                      origin_y - c->y(), width, height, alpha, full_img);
        if (img == full_img) continue;
        int top;
        if (c->as_gl_window()) {
          top = origin_y - y;
        } else {
          top = full_img->h() - (origin_y - y + img->h());
        }
        write_image_inside(full_img, img, origin_x - x, top);
        delete img;
      }
    }
    else traverse_to_gl_subwindows(c->as_group(), p, x, y, w, h, alpha, full_img);
  }
  return full_img;
}

//
// 'fl_read_image()' - Read an image from the current window or off-screen buffer
// this is the version for X11 and WIN32. The mac version is in fl_read_image_mac.cxx

uchar *				// O - Pixel buffer or NULL if failed
fl_read_image(uchar *p,		// I - Pixel buffer or NULL to allocate
              int   X,		// I - Left position
              int   Y,		// I - Top position
              int   w,		// I - Width of area to read
              // negative allows capture of window title bar and frame (X11 only)
              int   h,		// I - Height of area to read
              int   alpha)// I - Alpha value for image (0 for none)
{
  if (w < 0 || fl_find(fl_window) == 0) { // read from off_screen buffer or title bar and frame
    return read_win_rectangle(p, X, Y, w, h, alpha); // this function has an X11 and a WIN32 version
  }
  Fl_RGB_Image *img = traverse_to_gl_subwindows(Fl_Window::current(), p, X, Y, w, h, alpha, NULL);
  uchar *image_data = (uchar*)img->array;
  img->alloc_array = 0;
  delete img;
  return image_data;
}

#ifdef WIN32
#  include "fl_read_image_win32.cxx" // gives the WIN32 version of read_win_rectangle()
#else
#  include <X11/Xutil.h>
#  ifdef __sgi
#    include <X11/extensions/readdisplay.h>
#  else
#    include <stdlib.h>
#  endif // __sgi

// Defined in fl_color.cxx
extern uchar fl_redmask, fl_greenmask, fl_bluemask;
extern int fl_redshift, fl_greenshift, fl_blueshift, fl_extrashift;

//
// 'fl_subimage_offsets()' - Calculate subimage offsets for an axis
static inline int
fl_subimage_offsets(int a, int aw, int b, int bw, int &obw)
{
  int off;
  int ob;

  if (b >= a) {
    ob = b;
    off = 0;
  } else {
    ob = a;
    off = a - b;
  }

  bw -= off;

  if (ob + bw <= a + aw) {
    obw = bw;
  } else {
    obw = (a + aw) - ob;
  }

  return off;
}

// this handler will catch and ignore exceptions during XGetImage
// to avoid an application crash
extern "C" {
  static int xgetimageerrhandler(Display *display, XErrorEvent *error) {
    return 0;
  }
}


static uchar *read_win_rectangle(uchar *p, int X, int Y, int w, int h, int alpha)
{
  XImage	*image;		// Captured image
  int		i, maxindex;	// Looping vars
  int           x, y;		// Current X & Y in image
  int		d;		// Depth of image
  unsigned char *line,		// Array to hold image row
		*line_ptr;	// Pointer to current line image
  unsigned char	*pixel;		// Current color value
  XColor	colors[4096];	// Colors from the colormap...
  unsigned char	cvals[4096][3];	// Color values from the colormap...
  unsigned	index_mask,
		index_shift,
		red_mask,
		red_shift,
		green_mask,
		green_shift,
		blue_mask,
		blue_shift;


  //
  // Under X11 we have the option of the XGetImage() interface or SGI's
  // ReadDisplay extension which does all of the really hard work for
  // us...
  //
  int allow_outside = w < 0;    // negative w allows negative X or Y, that is, window frame
  if (w < 0) w = - w;

#  ifdef __sgi
  if (XReadDisplayQueryExtension(fl_display, &i, &i)) {
    image = XReadDisplay(fl_display, fl_window, X, Y, w, h, 0, NULL);
  } else
#  else
  image = 0;
#  endif // __sgi

  if (!image) {
    // fetch absolute coordinates
    int dx, dy, sx, sy, sw, sh;
    Window child_win;
    
    Fl_Window *win;
    if (allow_outside) win = (Fl_Window*)1;
    else win = fl_find(fl_window);
    if (win) {
      XTranslateCoordinates(fl_display, fl_window,
          RootWindow(fl_display, fl_screen), X, Y, &dx, &dy, &child_win);
      // screen dimensions
      Fl::screen_xywh(sx, sy, sw, sh, fl_screen);
    }
    if (!win || (dx >= sx && dy >= sy && dx + w <= sx+sw && dy + h <= sy+sh)) {
      // the image is fully contained, we can use the traditional method
      // however, if the window is obscured etc. the function will still fail. Make sure we
      // catch the error and continue, otherwise an exception will be thrown.
      XErrorHandler old_handler = XSetErrorHandler(xgetimageerrhandler);
      image = XGetImage(fl_display, fl_window, X, Y, w, h, AllPlanes, ZPixmap);
      XSetErrorHandler(old_handler);
    } else {
      // image is crossing borders, determine visible region
      int nw, nh, noffx, noffy;
      noffx = fl_subimage_offsets(sx, sw, dx, w, nw);
      noffy = fl_subimage_offsets(sy, sh, dy, h, nh);
      if (nw <= 0 || nh <= 0) return 0;

      // allocate the image
      int bpp = fl_visual->depth + ((fl_visual->depth / 8) % 2) * 8;
      char* buf = (char*)malloc(bpp / 8 * w * h);
      image = XCreateImage(fl_display, fl_visual->visual,
	  fl_visual->depth, ZPixmap, 0, buf, w, h, bpp, 0);
      if (!image) {
	if (buf) free(buf);
	return 0;
      }

      XErrorHandler old_handler = XSetErrorHandler(xgetimageerrhandler);
      XImage *subimg = XGetSubImage(fl_display, fl_window, X + noffx, Y + noffy,
                                    nw, nh, AllPlanes, ZPixmap, image, noffx, noffy);
      XSetErrorHandler(old_handler);
      if (!subimg) {
        XDestroyImage(image);
        return 0;
      }
    }
  }

  if (!image) return 0;

#ifdef DEBUG
  printf("width            = %d\n", image->width);
  printf("height           = %d\n", image->height);
  printf("xoffset          = %d\n", image->xoffset);
  printf("format           = %d\n", image->format);
  printf("data             = %p\n", image->data);
  printf("byte_order       = %d\n", image->byte_order);
  printf("bitmap_unit      = %d\n", image->bitmap_unit);
  printf("bitmap_bit_order = %d\n", image->bitmap_bit_order);
  printf("bitmap_pad       = %d\n", image->bitmap_pad);
  printf("depth            = %d\n", image->depth);
  printf("bytes_per_line   = %d\n", image->bytes_per_line);
  printf("bits_per_pixel   = %d\n", image->bits_per_pixel);
  printf("red_mask         = %08x\n", image->red_mask);
  printf("green_mask       = %08x\n", image->green_mask);
  printf("blue_mask        = %08x\n", image->blue_mask);
  printf("map_entries      = %d\n", fl_visual->visual->map_entries);
#endif // DEBUG

  d = alpha ? 4 : 3;

  // Allocate the image data array as needed...
  if (!p) p = new uchar[w * h * d];

  // Initialize the default colors/alpha in the whole image...
  memset(p, alpha, w * h * d);

  // Check that we have valid mask/shift values...
  if (!image->red_mask && image->bits_per_pixel > 12) {
    // Greater than 12 bits must be TrueColor...
    image->red_mask   = fl_visual->visual->red_mask;
    image->green_mask = fl_visual->visual->green_mask;
    image->blue_mask  = fl_visual->visual->blue_mask;

#ifdef DEBUG
    puts("\n---- UPDATED ----");
    printf("fl_redmask       = %08x\n", fl_redmask);
    printf("fl_redshift      = %d\n", fl_redshift);
    printf("fl_greenmask     = %08x\n", fl_greenmask);
    printf("fl_greenshift    = %d\n", fl_greenshift);
    printf("fl_bluemask      = %08x\n", fl_bluemask);
    printf("fl_blueshift     = %d\n", fl_blueshift);
    printf("red_mask         = %08x\n", image->red_mask);
    printf("green_mask       = %08x\n", image->green_mask);
    printf("blue_mask        = %08x\n", image->blue_mask);
#endif // DEBUG
  }

  // Check if we have colormap image...
  if (!image->red_mask) {
    // Get the colormap entries for this window...
    maxindex = fl_visual->visual->map_entries;

    for (i = 0; i < maxindex; i ++) colors[i].pixel = i;

    XQueryColors(fl_display, fl_colormap, colors, maxindex);

    for (i = 0; i < maxindex; i ++) {
      cvals[i][0] = colors[i].red >> 8;
      cvals[i][1] = colors[i].green >> 8;
      cvals[i][2] = colors[i].blue >> 8;
    }

    // Read the pixels and output an RGB image...
    for (y = 0; y < image->height; y ++) {
      pixel = (unsigned char *)(image->data + y * image->bytes_per_line);
      line  = p + y * w * d;

      switch (image->bits_per_pixel) {
        case 1 :
	  for (x = image->width, line_ptr = line, index_mask = 128;
	       x > 0;
	       x --, line_ptr += d) {
	    if (*pixel & index_mask) {
	      line_ptr[0] = cvals[1][0];
	      line_ptr[1] = cvals[1][1];
	      line_ptr[2] = cvals[1][2];
            } else {
	      line_ptr[0] = cvals[0][0];
	      line_ptr[1] = cvals[0][1];
	      line_ptr[2] = cvals[0][2];
            }

            if (index_mask > 1) {
	      index_mask >>= 1;
	    } else {
              index_mask = 128;
              pixel ++;
            }
	  }
          break;

        case 2 :
	  for (x = image->width, line_ptr = line, index_shift = 6;
	       x > 0;
	       x --, line_ptr += d) {
	    i = (*pixel >> index_shift) & 3;

	    line_ptr[0] = cvals[i][0];
	    line_ptr[1] = cvals[i][1];
	    line_ptr[2] = cvals[i][2];

            if (index_shift > 0) {
              index_mask >>= 2;
              index_shift -= 2;
            } else {
              index_mask  = 192;
              index_shift = 6;
              pixel ++;
            }
	  }
          break;

        case 4 :
	  for (x = image->width, line_ptr = line, index_shift = 4;
	       x > 0;
	       x --, line_ptr += d) {
	    if (index_shift == 4) i = (*pixel >> 4) & 15;
	    else i = *pixel & 15;

	    line_ptr[0] = cvals[i][0];
	    line_ptr[1] = cvals[i][1];
	    line_ptr[2] = cvals[i][2];

            if (index_shift > 0) {
              index_shift = 0;
	    } else {
              index_shift = 4;
              pixel ++;
            }
	  }
          break;

        case 8 :
	  for (x = image->width, line_ptr = line;
	       x > 0;
	       x --, line_ptr += d, pixel ++) {
	    line_ptr[0] = cvals[*pixel][0];
	    line_ptr[1] = cvals[*pixel][1];
	    line_ptr[2] = cvals[*pixel][2];
	  }
          break;

        case 12 :
	  for (x = image->width, line_ptr = line, index_shift = 0;
	       x > 0;
	       x --, line_ptr += d) {
	    if (index_shift == 0) {
	      i = ((pixel[0] << 4) | (pixel[1] >> 4)) & 4095;
	    } else {
	      i = ((pixel[1] << 8) | pixel[2]) & 4095;
	    }

	    line_ptr[0] = cvals[i][0];
	    line_ptr[1] = cvals[i][1];
	    line_ptr[2] = cvals[i][2];

            if (index_shift == 0) {
              index_shift = 4;
            } else {
              index_shift = 0;
              pixel += 3;
            }
	  }
          break;
      }
    }
  } else {
    // RGB(A) image, so figure out the shifts & masks...
    red_mask  = image->red_mask;
    red_shift = 0;

    while ((red_mask & 1) == 0) {
      red_mask >>= 1;
      red_shift ++;
    }

    green_mask  = image->green_mask;
    green_shift = 0;

    while ((green_mask & 1) == 0) {
      green_mask >>= 1;
      green_shift ++;
    }

    blue_mask  = image->blue_mask;
    blue_shift = 0;

    while ((blue_mask & 1) == 0) {
      blue_mask >>= 1;
      blue_shift ++;
    }

    // Read the pixels and output an RGB image...
    for (y = 0; y < image->height; y ++) {
      pixel = (unsigned char *)(image->data + y * image->bytes_per_line);
      line  = p + y * w * d;

      switch (image->bits_per_pixel) {
        case 8 :
	  for (x = image->width, line_ptr = line;
	       x > 0;
	       x --, line_ptr += d, pixel ++) {
	    i = *pixel;

	    line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	    line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	    line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
	  }
          break;

        case 12 :
	  for (x = image->width, line_ptr = line, index_shift = 0;
	       x > 0;
	       x --, line_ptr += d) {
	    if (index_shift == 0) {
	      i = ((pixel[0] << 4) | (pixel[1] >> 4)) & 4095;
	    } else {
	      i = ((pixel[1] << 8) | pixel[2]) & 4095;
            }

	    line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	    line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	    line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;

            if (index_shift == 0) {
              index_shift = 4;
            } else {
              index_shift = 0;
              pixel += 3;
            }
	  }
          break;

        case 16 :
          if (image->byte_order == LSBFirst) {
            // Little-endian...
	    for (x = image->width, line_ptr = line;
	         x > 0;
	         x --, line_ptr += d, pixel += 2) {
	      i = (pixel[1] << 8) | pixel[0];

	      line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	      line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	      line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
	    }
	  } else {
            // Big-endian...
	    for (x = image->width, line_ptr = line;
	         x > 0;
	         x --, line_ptr += d, pixel += 2) {
	      i = (pixel[0] << 8) | pixel[1];

	      line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	      line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	      line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
	    }
	  }
          break;

        case 24 :
          if (image->byte_order == LSBFirst) {
            // Little-endian...
	    for (x = image->width, line_ptr = line;
	         x > 0;
	         x --, line_ptr += d, pixel += 3) {
	      i = (((pixel[2] << 8) | pixel[1]) << 8) | pixel[0];

	      line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	      line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	      line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
	    }
	  } else {
            // Big-endian...
	    for (x = image->width, line_ptr = line;
	         x > 0;
	         x --, line_ptr += d, pixel += 3) {
	      i = (((pixel[0] << 8) | pixel[1]) << 8) | pixel[2];

	      line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	      line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	      line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
	    }
	  }
          break;

        case 32 :
          if (image->byte_order == LSBFirst) {
            // Little-endian...
	    for (x = image->width, line_ptr = line;
	         x > 0;
	         x --, line_ptr += d, pixel += 4) {
	      i = (((((pixel[3] << 8) | pixel[2]) << 8) | pixel[1]) << 8) | pixel[0];

	      line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	      line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	      line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
	    }
	  } else {
            // Big-endian...
	    for (x = image->width, line_ptr = line;
	         x > 0;
	         x --, line_ptr += d, pixel += 4) {
	      i = (((((pixel[0] << 8) | pixel[1]) << 8) | pixel[2]) << 8) | pixel[3];

	      line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	      line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	      line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
	    }
	  }
          break;
      }
    }
  }

  // Destroy the X image we've read and return the RGB(A) image...
  XDestroyImage(image);

  return p;
}

#endif // !WIN32

#endif // !__APPLE__

//
// End of "$Id$".
//
