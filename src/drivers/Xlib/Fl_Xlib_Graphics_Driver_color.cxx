//
// Color functions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

// Implementation of fl_color(i), fl_color(r,g,b).

// FIXME: all the global functions in this file should probably be protected
// members of the driver class. Starting with 1.4 we will allow multiple drivers
// to co-exist, creating conflicts with multipe mapping.

// FIXME: maybe we can forget about color mapping and assume RGB?

// Also code to look at the X visual and figure out the best way to turn
// a color into a pixel value.

// SGI compiler seems to have problems with unsigned char arguments
// being used to index arrays.  So I always copy them to an integer
// before use.

#include <config.h>
#include "Fl_Xlib_Graphics_Driver.H"
#  include "../../Fl_XColor.H"
#  include <FL/Fl.H>
#  include <FL/platform.H>
#  include <FL/fl_draw.H>

extern unsigned fl_cmap[256]; // defined in fl_color.cxx

////////////////////////////////////////////////////////////////
// figure_out_visual() calculates masks & shifts for generating
// pixels in true-color visuals:

uchar fl_redmask;       /**< color mask used in current color map handling */
uchar fl_greenmask;     /**< color mask used in current color map handling */
uchar fl_bluemask;      /**< color mask used in current color map handling */

int fl_redshift;        /**< color shift used in current color map handling */
int fl_greenshift;      /**< color shift used in current color map handling */
int fl_blueshift;       /**< color shift used in current color map handling */
int fl_extrashift;      /**< color shift used in current color map handling */

static uchar beenhere;

static void figure_out_visual() {
  beenhere = 1;
  if (!fl_visual->red_mask || !fl_visual->green_mask || !fl_visual->blue_mask){
#  if USE_COLORMAP
    fl_redmask = 0;
    return;
#  else
    Fl::fatal("Requires true color visual");
#  endif
  }

  // get the bit masks into a more useful form:
  int i,j,m;

  for (i = 0, m = 1; m; i++, m<<=1) if (fl_visual->red_mask & m) break;
  for (j = i; m; j++, m<<=1) if (!(fl_visual->red_mask & m)) break;
  fl_redshift = j-8;
  fl_redmask = (j-i >= 8) ? 0xFF : 0xFF-(255>>(j-i));

  for (i = 0, m = 1; m; i++, m<<=1) if (fl_visual->green_mask & m) break;
  for (j = i; m; j++, m<<=1) if (!(fl_visual->green_mask & m)) break;
  fl_greenshift = j-8;
  fl_greenmask = (j-i >= 8) ? 0xFF : 0xFF-(255>>(j-i));

  for (i = 0, m = 1; m; i++, m<<=1) if (fl_visual->blue_mask & m) break;
  for (j = i; m; j++, m<<=1) if (!(fl_visual->blue_mask & m)) break;
  fl_blueshift = j-8;
  fl_bluemask = (j-i >= 8) ? 0xFF : 0xFF-(255>>(j-i));

  i = fl_redshift;
  if (fl_greenshift < i) i = fl_greenshift;
  if (fl_blueshift < i) i = fl_blueshift;
  if (i < 0) {
    fl_extrashift = -i;
    fl_redshift -= i; fl_greenshift -= i; fl_blueshift -= i;
  } else
    fl_extrashift = 0;

}

Fl_XColor fl_xmap[1][256];

void Fl_Xlib_Graphics_Driver::color(Fl_Color i) {
  if (i & 0xffffff00) {
    unsigned rgb = (unsigned)i;
    color((uchar)(rgb >> 24), (uchar)(rgb >> 16), (uchar)(rgb >> 8));
  } else {
    Fl_Graphics_Driver::color(i);
    if(!gc_) return; // don't get a default gc if current window is not yet created/valid
    XSetForeground(fl_display, gc_, fl_xpixel(i));
  }
}

void Fl_Xlib_Graphics_Driver::color(uchar r,uchar g,uchar b) {
  Fl_Graphics_Driver::color( fl_rgb_color(r, g, b) );
  if(!gc_) return; // don't get a default gc if current window is not yet created/valid
  XSetForeground(fl_display, gc_, fl_xpixel(r,g,b));
}

/** \addtogroup  fl_attributes
    @{ */
////////////////////////////////////////////////////////////////
// Get an rgb color.  This is easy for a truecolor visual.  For
// colormapped it picks the closest color out of the cube in the
// fltk colormap.  However if this color cube entry has been
// requested before, you will get the earlier requested color, and
// even this may be approximated if the X colormap was full.

/**
  Returns the X pixel number used to draw the given rgb color.
  This is the X pixel that fl_color() would use.
  \param[in] r,g,b color components
  \return X pixel number
*/
ulong fl_xpixel(uchar r,uchar g,uchar b) {
  if (!beenhere) figure_out_visual();
#  if USE_COLORMAP
  if (!fl_redmask) {
    // find closest entry in the colormap:
    Fl_Color i =
      fl_color_cube(r*FL_NUM_RED/256,g*FL_NUM_GREEN/256,b*FL_NUM_BLUE/256);
    Fl_XColor &xmap = fl_xmap[Fl_Xlib_Graphics_Driver::fl_overlay][i];
    if (xmap.mapped) return xmap.pixel;
    // if not black or white, change the entry to be an exact match:
    if (i != FL_COLOR_CUBE && i != 0xFF)
      fl_cmap[i] = (r<<24)|(g<<16)|(b<<8);
    return fl_xpixel(i); // allocate an X color
  }
#  endif
  return
    (((r&fl_redmask) << fl_redshift)+
     ((g&fl_greenmask)<<fl_greenshift)+
     ((b&fl_bluemask)<< fl_blueshift)
     ) >> fl_extrashift;
}

////////////////////////////////////////////////////////////////
// Get a color out of the fltk colormap.  Again for truecolor
// visuals this is easy.  For colormap this actually tries to allocate
// an X color, and does a least-squares match to find the closest
// color if X cannot allocate that color.

// calculate what color is actually on the screen for a mask:
static inline uchar realcolor(uchar color, uchar mask) {
#  if 0
  // accurate version if the display has linear gamma, but fl_draw_image
  // works better with the simpler version on most screens...
  uchar m = mask;
  uchar result = color&m;
  for (;;) {
    while (m&mask) {m>>=1; color>>=1;}
    if (!m) break;
    mask = m;
    result |= color&m;
  }
  return result;
#  else
  return (color&mask) | ( (~mask)&(mask>>1) );
#  endif
}

/**
  Returns the X pixel number used to draw the given FLTK color index.
  This is the X pixel that fl_color() would use.
  \param[in] i color index
  \return X pixel number
*/
ulong fl_xpixel(Fl_Color i) {
  if (i & 0xffffff00) {
    return fl_xpixel((i >> 24) & 255, (i >> 16) & 255, (i >> 8) & 255);
  }

  Fl_XColor &xmap = fl_xmap[Fl_Xlib_Graphics_Driver::fl_overlay][i];
  if (xmap.mapped) return xmap.pixel;

  if (!beenhere) figure_out_visual();

  uchar r,g,b;
  {unsigned c = fl_cmap[i]; r=uchar(c>>24); g=uchar(c>>16); b=uchar(c>>8);}

#  if USE_COLORMAP
  Colormap colormap = fl_colormap;
  if (fl_redmask) {
#  endif
    // return color for a truecolor visual:
    xmap.mapped = 2; // 2 prevents XFreeColor from being called
    xmap.r = realcolor(r, fl_redmask);
    xmap.g = realcolor(g, fl_greenmask);
    xmap.b = realcolor(b, fl_bluemask);
    return xmap.pixel =
      (((r&fl_redmask) << fl_redshift)+
       ((g&fl_greenmask)<<fl_greenshift)+
       ((b&fl_bluemask)<< fl_blueshift)
       ) >> fl_extrashift;
#  if USE_COLORMAP
  }
  static XColor *allcolors;
  static int numcolors;

  // I don't try to allocate colors with XAllocColor once it fails
  // with any color.  It is possible that it will work, since a color
  // may have been freed, but some servers are extremely slow and this
  // avoids one round trip:
  if (!numcolors) { // don't try after a failure
    XColor xcol;
    xcol.red = r<<8; xcol.green = g<<8; xcol.blue = b<<8;
    if (XAllocColor(fl_display, colormap, &xcol)) {
      xmap.mapped = 1;
      xmap.r = xcol.red>>8;
      xmap.g = xcol.green>>8;
      xmap.b = xcol.blue>>8;
      return xmap.pixel = xcol.pixel;
    }

    // I only read the colormap once.  Again this is due to the slowness
    // of round-trips to the X server, even though other programs may alter
    // the colormap after this and make decisions here wrong.
    numcolors = fl_visual->colormap_size;
    if (!allcolors) allcolors = new XColor[numcolors];
    for (int p = numcolors; p--;) allcolors[p].pixel = p;
    XQueryColors(fl_display, colormap, allcolors, numcolors);
  }

  // find least-squares match:
  int mindist = 0x7FFFFFFF;
  unsigned int bestmatch = 0;
  for (unsigned int n = numcolors; n--;) {
    XColor &a = allcolors[n];
    int d, t;
    t = int(r)-int(a.red>>8); d = t*t;
    t = int(g)-int(a.green>>8); d += t*t;
    t = int(b)-int(a.blue>>8); d += t*t;
    if (d <= mindist) {bestmatch = n; mindist = d;}
  }
  XColor &p = allcolors[bestmatch];

  // It appears to "work" to not call this XAllocColor, which will
  // avoid another round-trip to the server.  But then X does not
  // know that this program "owns" this value, and can (and will)
  // change it when the program that did allocate it exits:
  if (XAllocColor(fl_display, colormap, &p)) {
    xmap.mapped = 1;
    xmap.pixel = p.pixel;
  } else {
    // However, if that XAllocColor fails, I have to give up and
    // assume the pixel is ok for the duration of the program.  This
    // is due to bugs (?) in the Solaris X and some X terminals
    // where XAllocColor *always* fails when the colormap is full,
    // even if we ask for a color already in it...
    xmap.mapped = 2; // 2 prevents XFreeColor from being called
    xmap.pixel = bestmatch;
  }
  xmap.r = p.red>>8;
  xmap.g = p.green>>8;
  xmap.b = p.blue>>8;
  return xmap.pixel;
#  endif
}

/**
  Free color \p i if used, and clear mapping table entry.
  \param[in] i color index
  \param[in] overlay 0 for normal, 1 for overlay color
*/
void Fl_Xlib_Graphics_Driver::free_color(Fl_Color i, int overlay) {
  if (overlay) return;
  if (fl_xmap[overlay][i].mapped) {
#  if USE_COLORMAP
    Colormap colormap = fl_colormap;
    if (fl_xmap[overlay][i].mapped == 1)
      XFreeColors(fl_display, colormap, &(fl_xmap[overlay][i].pixel), 1, 0);
#  endif
    fl_xmap[overlay][i].mapped = 0;
  }
}

/**
  Set color mapping table entry \p i to color \p c
  \param[in] i color index
  \param[in] c color
*/
void Fl_Xlib_Graphics_Driver::set_color(Fl_Color i, unsigned c) {
  if (fl_cmap[i] != c) {
    free_color(i,0);
    fl_cmap[i] = c;
  }
}

/**
 \}
 */
