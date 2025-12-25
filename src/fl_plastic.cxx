//
// "Plastic" scheme drawing routines for the Fast Light Tool Kit (FLTK).
//
// These box types provide a cross between Aqua and KDE buttons; kindof
// like translucent plastic buttons...
//
// Copyright 2001-2005 by Michael Sweet.
// Copyright 2006-2025 by Bill Spitzak and others.
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

/**
  \file src/fl_plastic.cxx
  \brief Implementation of the \c 'plastic' scheme.
*/

// Box drawing code for an obscure box type.
// These box types are in separate files so they are not linked
// in if not used.

#include <FL/Fl.H>
#include <FL/Fl_Scheme.H>
#include <FL/fl_draw.H>

#include <cassert>

// Globals

extern const uchar *fl_gray_ramp();     // in src/fl_boxtype.cxx

// Module globals (static)

static float plastic_average = -1.00f;  // plastic color average: not yet set
static int av_min =  10;                // min. supported average
static int av_def =  75;                // default average (up to FLTK 1.4)
static int av_max = 100;                // max. supported average


// clamp and set color average value

static void set_color_average(int av) {
  if (av < av_min)
    plastic_average = av_min / 100.f;
  else if (av > av_max)
    plastic_average = av_max / 100.f;
  else
    plastic_average = av / 100.f;
}

/**
  Set the color average value of the 'plastic' scheme in percent.

  Legal values are in the range [ 10 ... 100 ], other values are clamped. The
  default value is 75 which is backwards compatible with FLTK 1.4 and earlier.

  Higher values make the colors in boxes etc. appear "more gray" whereas lower
  values make colors appear more like the original color. The recommended value
  is about 40 to 60 but this is left to the user.

  If this method is not called then the environment variable \c FLTK_PLASTIC_AVERAGE
  can be used to set the color average. The environment variable must be a pure
  numeric (integer) value in the given range, otherwise the behavior is undefined.

  However, calling \b this method takes precedence over the environment variable.
  This method can be called at any time (e.g. to view dynamic changes). This will
  permanently change the appearance for all later box / background drawings.

  For details see GitHub Issue 464: "RFE: plastic scheme with faithful colors".

  \note Program developers are supposed to use this method to apply "better"
    values than the default (1.4) look and feel. End users are supposed to use
    the environment variable for programs that \b don't use
    Fl_Scheme::plastic_color_average().

  Include the following header:
  \code
    #include <FL/Fl_Scheme.H>
  \endcode

  \param[in]  av  color average value in the documented range.

  \since 1.5.0
*/
void Fl_Scheme::plastic_color_average(int av) {
  set_color_average(av);
}

// Get 'plastic' color average from environment variable 'FLTK_PLASTIC_AVERAGE'
// unless it has already been set.
// See GitHub Issue # 464: "RFE: plastic scheme with faithful colors"

static float plastic_color_average() {
  if (plastic_average < 0.0f) {               // only once
    char *envvar = fl_getenv("FLTK_PLASTIC_AVERAGE");
    if (envvar) {                             // convert and store env. var.
      int temp = atoi(envvar);                // may be 0, but will be clamped
      set_color_average(temp);                // clamp and set value
    } else {
      plastic_average = av_def / 100.0f;      // set default value
    }
  }
  assert(plastic_average >= 0);
  return plastic_average;
}

inline Fl_Color shade_color(uchar gc, Fl_Color bc) {
  return fl_color_average((Fl_Color)gc, bc, plastic_color_average());
}

static void frame_rect(int x, int y, int w, int h, const char *c, Fl_Color bc) {
  const uchar *g = fl_gray_ramp();
  int b = ((int) strlen(c)) / 4 + 1;

  for (x += b, y += b, w -= 2 * b, h -= 2 * b; b > 1; b --)
  {
    // Draw lines around the perimeter of the button, 4 colors per
    // circuit.
    fl_color(shade_color(g[(int)*c++], bc));
    fl_line(x, y + h + b, x + w - 1, y + h + b, x + w + b - 1, y + h);
    fl_color(shade_color(g[(int)*c++], bc));
    fl_line(x + w + b - 1, y + h, x + w + b - 1, y, x + w - 1, y - b);
    fl_color(shade_color(g[(int)*c++], bc));
    fl_line(x + w - 1, y - b, x, y - b, x - b, y);
    fl_color(shade_color(g[(int)*c++], bc));
    fl_line(x - b, y, x - b, y + h, x, y + h + b);
  }
}

static void frame_round(int x, int y, int w, int h, const char *c, Fl_Color bc) {
  const uchar *g = fl_gray_ramp();
  size_t b = strlen(c) / 4 + 1;

  if (w==h) {
    for (; b > 1; b --, x ++, y ++, w -= 2, h -= 2)
    {
      fl_color(shade_color(g[(int)*c++], bc));
      fl_arc(x, y, w, h, 45.0, 135.0);
      fl_color(shade_color(g[(int)*c++], bc));
      fl_arc(x, y, w, h, 315.0, 405.0);
      fl_color(shade_color(g[(int)*c++], bc));
      fl_arc(x, y, w, h, 225.0, 315.0);
      fl_color(shade_color(g[(int)*c++], bc));
      fl_arc(x, y, w, h, 135.0, 225.0);
    }
  } else if (w>h) {
    int d = h/2;
    for (; b > 1; d--, b --, x ++, y ++, w -= 2, h -= 2)
    {
      fl_color(shade_color(g[(int)*c++], bc));
      fl_arc(x, y, h, h, 90.0, 135.0);
      fl_xyline(x+d, y, x+w-d);
      fl_arc(x+w-h, y, h, h, 45.0, 90.0);
      fl_color(shade_color(g[(int)*c++], bc));
      fl_arc(x+w-h, y, h, h, 315.0, 405.0);
      fl_color(shade_color(g[(int)*c++], bc));
      fl_arc(x+w-h, y, h, h, 270.0, 315.0);
      fl_xyline(x+d, y+h-1, x+w-d);
      fl_arc(x, y, h, h, 225.0, 270.0);
      fl_color(shade_color(g[(int)*c++], bc));
      fl_arc(x, y, h, h, 135.0, 225.0);
    }
  } else if (w<h) {
    int d = w/2;
    for (; b > 1; d--, b --, x ++, y ++, w -= 2, h -= 2)
    {
      fl_color(shade_color(g[(int)*c++], bc));
      fl_arc(x, y, w, w, 45.0, 135.0);
      fl_color(shade_color(g[(int)*c++], bc));
      fl_arc(x, y, w, w, 0.0, 45.0);
      fl_yxline(x+w-1, y+d, y+h-d);
      fl_arc(x, y+h-w, w, w, 315.0, 360.0);
      fl_color(shade_color(g[(int)*c++], bc));
      fl_arc(x, y+h-w, w, w, 225.0, 315.0);
      fl_color(shade_color(g[(int)*c++], bc));
      fl_arc(x, y+h-w, w, w, 180.0, 225.0);
      fl_yxline(x, y+d, y+h-d);
      fl_arc(x, y, w, w, 135.0, 180.0);
    }
  }
}

static void shade_rect(int x, int y, int w, int h, const char *c, Fl_Color bc) {
  const uchar *g = fl_gray_ramp();
  int   i, j;
  int   clen = (int) strlen(c) - 1;
  int   chalf = clen / 2;
  int   cstep = 1;

  if (h < (w * 2)) {
    // Horizontal shading...
    if (clen >= h) cstep = 2;

    for (i = 0, j = 0; j < chalf; i ++, j += cstep) {
      // Draw the top line and points...
      fl_color(shade_color(g[(int)c[i]], bc));
      fl_xyline(x + 1, y + i, x + w - 2);

      fl_color(shade_color(g[c[i] - 2], bc));
      fl_point(x, y + i + 1);
      fl_point(x + w - 1, y + i + 1);

      // Draw the bottom line and points...
      fl_color(shade_color(g[(int)c[clen - i]], bc));
      fl_xyline(x + 1, y + h - i, x + w - 2);

      fl_color(shade_color(g[c[clen - i] - 2], bc));
      fl_point(x, y + h - i);
      fl_point(x + w - 1, y + h - i);
    }

    // Draw the interior and sides...
    i = chalf / cstep;

    fl_color(shade_color(g[(int)c[chalf]], bc));
    fl_rectf(x + 1, y + i, w - 2, h - 2 * i + 1);

    fl_color(shade_color(g[c[chalf] - 2], bc));
    fl_yxline(x, y + i, y + h - i);
    fl_yxline(x + w - 1, y + i, y + h - i);
  } else {
    // Vertical shading...
    if (clen >= w) cstep = 2;

    for (i = 0, j = 0; j < chalf; i ++, j += cstep) {
      // Draw the left line and points...
      fl_color(shade_color(g[(int)c[i]], bc));
      fl_yxline(x + i, y + 1, y + h - 1);

      fl_color(shade_color(g[c[i] - 2], bc));
      fl_point(x + i + 1, y);
      fl_point(x + i + 1, y + h);

      // Draw the right line and points...
      fl_color(shade_color(g[(int)c[clen - i]], bc));
      fl_yxline(x + w - 1 - i, y + 1, y + h - 1);

      fl_color(shade_color(g[c[clen - i] - 2], bc));
      fl_point(x + w - 2 - i, y);
      fl_point(x + w - 2 - i, y + h);
    }

    // Draw the interior, top, and bottom...
    i = chalf / cstep;

    fl_color(shade_color(g[(int)c[chalf]], bc));
    fl_rectf(x + i, y + 1, w - 2 * i, h - 1);

    fl_color(shade_color(g[c[chalf] - 2], bc));
    fl_xyline(x + i, y, x + w - i);
    fl_xyline(x + i, y + h, x + w - i);
  }
}

static void shade_round(int x, int y, int w, int h, const char *c, Fl_Color bc) {
  const uchar *g = fl_gray_ramp();
  int   i;
  int   clen = (int) (strlen(c) - 1);
  int   chalf = clen / 2;

  if (w>h) {
    int d = h/2;
    const int na = 8;
    for (i=0; i<chalf; i++, d--, x++, y++, w-=2, h-=2)
    {
      fl_color(shade_color(g[(int)c[i]], bc));
      fl_pie(x, y, h, h, 90.0, 135.0+i*na);
      fl_xyline(x+d, y, x+w-d);
      fl_pie(x+w-h, y, h, h, 45.0+i*na, 90.0);
      fl_color(shade_color(g[(int)c[i] - 2], bc));
      fl_pie(x+w-h, y, h, h, 315.0+i*na, 405.0+i*na);
      fl_color(shade_color(g[(int)c[clen - i]], bc));
      fl_pie(x+w-h, y, h, h, 270.0, 315.0+i*na);
      fl_xyline(x+d, y+h-1, x+w-d);
      fl_pie(x, y, h, h, 225.0+i*na, 270.0);
      fl_color(shade_color(g[c[(int)clen - i] - 2], bc));
      fl_pie(x, y, h, h, 135.0+i*na, 225.0+i*na);
    }
    fl_color(shade_color(g[(int)c[chalf]], bc));
    fl_rectf(x+d, y, w-h+1, h+1);
    fl_pie(x, y, h, h, 90.0, 270.0);
    fl_pie(x+w-h, y, h, h, 270.0, 90.0);
  } else {
    int d = w/2;
    const int na = 8;
    for (i=0; i<chalf; i++, d--, x++, y++, w-=2, h-=2)
    {
      fl_color(shade_color(g[(int)c[i]], bc));
      fl_pie(x, y, w, w, 45.0+i*na, 135.0+i*na);
      fl_color(shade_color(g[c[i] - 2], bc));
      fl_pie(x, y, w, w, 0.0, 45.0+i*na);
      fl_yxline(x+w-1, y+d, y+h-d);
      fl_pie(x, y+h-w, w, w, 315.0+i*na, 360.0);
      fl_color(shade_color(g[(int)c[clen - i]], bc));
      fl_pie(x, y+h-w, w, w, 225.0+i*na, 315.0+i*na);
      fl_color(shade_color(g[c[clen - i] - 2], bc));
      fl_pie(x, y+h-w, w, w, 180.0, 225.0+i*na);
      fl_yxline(x, y+d, y+h-d);
      fl_pie(x, y, w, w, 135.0+i*na, 180.0);
    }
    fl_color(shade_color(g[(int)c[chalf]], bc));
    fl_rectf(x, y+d, w+1, h-w+1);
    fl_pie(x, y, w, w, 0.0, 180.0);
    fl_pie(x, y+h-w, w, w, 180.0, 360.0);
  }
}

void fl_plastic_up_frame(int x, int y, int w, int h, Fl_Color c) {
  frame_rect(x, y, w, h - 1, "KLDIIJLM", c);
}

static void narrow_thin_box(int x, int y, int w, int h, Fl_Color c) {
  if (h<=0 || w<=0) return;
  const uchar *g = fl_gray_ramp();
  fl_color(shade_color(g[(int)'R'], c));
  fl_rectf(x+1, y+1, w-2, h-2);
  fl_color(shade_color(g[(int)'I'], c));
  if (w > 1) {
    fl_xyline(x+1, y, x+w-2);
    fl_xyline(x+1, y+h-1, x+w-2);
  }
  if (h > 1) {
    fl_yxline(x, y+1, y+h-2);
    fl_yxline(x+w-1, y+1, y+h-2);
  }
}

void fl_plastic_thin_up_box(int x, int y, int w, int h, Fl_Color c) {
  if (w > 4 && h > 4) {
    shade_rect(x + 1, y + 1, w - 2, h - 3, "RQOQSUWQ", c);
    frame_rect(x, y, w, h - 1, "IJLM", c);
  } else {
    narrow_thin_box(x, y, w, h, c);
  }
}

void fl_plastic_up_box(int x, int y, int w, int h, Fl_Color c) {
  if (w > 8 && h > 8) {
    shade_rect(x + 1, y + 1, w - 2, h - 3, "RVQNOPQRSTUVWVQ", c);
    frame_rect(x, y, w, h - 1, "IJLM", c);
  } else {
    fl_plastic_thin_up_box(x, y, w, h, c);
  }
}

void fl_plastic_up_round(int x, int y, int w, int h, Fl_Color c) {
  shade_round(x, y, w, h, "RVQNOPQRSTUVWVQ", c);
  frame_round(x, y, w, h, "IJLM", c);
}

void fl_plastic_down_frame(int x, int y, int w, int h, Fl_Color c) {
  frame_rect(x, y, w, h - 1, "LLLLTTRR", c);
}

void fl_plastic_down_box(int x, int y, int w, int h, Fl_Color c) {
  if (w > 6 && h > 6) {
    shade_rect(x + 2, y + 2, w - 4, h - 5, "STUVWWWVT", c);
    fl_plastic_down_frame(x, y, w, h, c);
  }
  else {
    narrow_thin_box(x, y, w, h, c);
  }
}

void fl_plastic_down_round(int x, int y, int w, int h, Fl_Color c) {
  shade_round(x, y, w, h, "STUVWWWVT", c);
  frame_round(x, y, w, h, "IJLM", c);
}
