//
// "$Id: fl_plastic.cxx,v 1.1.2.15 2003/01/13 19:24:08 easysw Exp $"
//
// "Plastic" drawing routines for the Fast Light Tool Kit (FLTK).
//
// These box types provide a cross between Aqua and KDE buttons; kindof
// like translucent plastic buttons...
//
// Copyright 2001-2002 by Michael Sweet.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

// Box drawing code for an obscure box type.
// These box types are in seperate files so they are not linked
// in if not used.

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "flstring.h"


extern uchar *fl_gray_ramp();

inline Fl_Color shade_color(uchar gc, Fl_Color bc) {
  return fl_color_average((Fl_Color)gc, bc, 0.75f);
}

static void shade_frame(int x, int y, int w, int h, const char *c, Fl_Color bc) {
  uchar *g = fl_gray_ramp();
  int b = strlen(c) / 4 + 1;

  for (x += b, y += b, w -= 2 * b, h -= 2 * b; b > 1; b --)
  {
    // Draw lines around the perimeter of the button, 4 colors per
    // circuit.
    fl_color(shade_color(g[*c++], bc));
    fl_line(x, y + h + b, x + w - 1, y + h + b, x + w + b - 1, y + h);
    fl_color(shade_color(g[*c++], bc));
    fl_line(x + w + b - 1, y + h, x + w + b - 1, y, x + w - 1, y - b);
    fl_color(shade_color(g[*c++], bc));
    fl_line(x + w - 1, y - b, x, y - b, x - b, y);
    fl_color(shade_color(g[*c++], bc));
    fl_line(x - b, y, x - b, y + h, x, y + h + b);
  }
}


static void shade_rect(int x, int y, int w, int h, const char *c, Fl_Color bc)
{
  uchar		*g = fl_gray_ramp();
  int		i, j;
  int		clen = strlen(c) - 1;
  int		chalf = clen / 2;
  int		cstep = 1;

  if (h < (w * 2)) {
    // Horizontal shading...
    if (clen >= h) cstep = 2;

    for (i = 0, j = 0; j < chalf; i ++, j += cstep) {
      // Draw the top line and points...
      fl_color(shade_color(g[c[i]], bc));
      fl_xyline(x + 1, y + i, x + w - 1);

      fl_color(shade_color(g[c[i] - 2], bc));
      fl_point(x, y + i + 1);
      fl_point(x + w - 1, y + i + 1);

      // Draw the bottom line and points...
      fl_color(shade_color(g[c[clen - i]], bc));
      fl_xyline(x + 1, y + h - i, x + w - 1);

      fl_color(shade_color(g[c[clen - i] - 2], bc));
      fl_point(x, y + h - i);
      fl_point(x + w - 1, y + h - i);
    }

    // Draw the interior and sides...
    i = chalf / cstep;

    fl_color(shade_color(g[c[chalf]], bc));
    fl_rectf(x + 1, y + i, w - 2, h - 2 * i + 1);

    fl_color(shade_color(g[c[chalf] - 2], bc));
    fl_yxline(x, y + i, y + h - i);
    fl_yxline(x + w - 1, y + i, y + h - i);
  } else {
    // Vertical shading...
    if (clen >= w) cstep = 2;

    for (i = 0, j = 0; j < chalf; i ++, j += cstep) {
      // Draw the left line and points...
      fl_color(shade_color(g[c[i]], bc));
      fl_yxline(x + i, y + 1, y + h - 1);

      fl_color(shade_color(g[c[i] - 2], bc));
      fl_point(x + i + 1, y);
      fl_point(x + i + 1, y + h - 1);

      // Draw the right line and points...
      fl_color(shade_color(g[c[clen - i]], bc));
      fl_yxline(x + w - 1 - i, y + 1, y + h - 1);

      fl_color(shade_color(g[c[clen - i] - 2], bc));
      fl_point(x + w - 1 - i, y);
      fl_point(x + w - 1 - i, y + h - 1);
    }

    // Draw the interior, top, and bottom...
    i = chalf / cstep;

    fl_color(shade_color(g[c[chalf]], bc));
    fl_rectf(x + i, y + 1, w - 2 * i, h - 2);

    fl_color(shade_color(g[c[chalf] - 2], bc));
    fl_xyline(x + i, y, x + w - i);
    fl_xyline(x + i, y + h - 1, x + w - i);
  }
}


static void up_frame(int x, int y, int w, int h, Fl_Color c) {
//  shade_frame(x, y, w, h - 1, "MNFKKLNO", c);
  shade_frame(x, y, w, h - 1, "KLDIIJLM", c);
}


static void up_box(int x, int y, int w, int h, Fl_Color c) {
//  shade_rect(x + 2, y + 2, w - 4, h - 5, "TXSPPQQRSSTTUVS", c);
  shade_rect(x + 2, y + 2, w - 4, h - 5, "RVQNOPQRSTUVWVQ", c);
//  shade_rect(x + 2, y + 2, w - 4, h - 5, "RTVUTSRSTUWWXWQ", c);
//  shade_rect(x + 2, y + 2, w - 4, h - 5, "RVQNNOOPQQRRSTQ", c);

  up_frame(x, y, w, h, c);
}


static void down_frame(int x, int y, int w, int h, Fl_Color c) {
  shade_frame(x, y, w, h - 1, "LLRRTTLL", c);
}


static void down_box(int x, int y, int w, int h, Fl_Color c) {
  shade_rect(x + 2, y + 2, w - 4, h - 5, "STUVWWWVT", c);

  down_frame(x, y, w, h, c);
}


extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*);


Fl_Boxtype fl_define_FL_PLASTIC_UP_BOX() {
  fl_internal_boxtype(_FL_PLASTIC_UP_BOX, up_box);
  fl_internal_boxtype(_FL_PLASTIC_DOWN_BOX, down_box);
  fl_internal_boxtype(_FL_PLASTIC_UP_FRAME, up_frame);
  fl_internal_boxtype(_FL_PLASTIC_DOWN_FRAME, down_frame);

  return _FL_PLASTIC_UP_BOX;
}


//
// End of "$Id: fl_plastic.cxx,v 1.1.2.15 2003/01/13 19:24:08 easysw Exp $".
//
