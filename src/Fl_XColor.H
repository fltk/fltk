// X-specific structure for storing allocated colors.  Used by fl_color.C,
// fl_draw_image.C, fl_set_color.C.

#include <config.h>

// one of these for each color in fltk's "colormap":
// if overlays are enabled, another one for the overlay
struct Fl_XColor {
  unsigned char r,g,b;	// actual color used by X
  unsigned char mapped;	// true when XAllocColor done
  unsigned long pixel;	// the X pixel to use
};
extern Fl_XColor fl_xmap[/*overlay*/][256];

// mask & shifts to produce xcolor for truecolor visuals:
extern unsigned char fl_redmask, fl_greenmask, fl_bluemask;
extern int fl_redshift, fl_greenshift, fl_blueshift, fl_extrashift;

