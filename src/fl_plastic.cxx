//
// "$Id: fl_plastic.cxx,v 1.1.2.2 2001/12/01 13:59:50 easysw Exp $"
//
// "Plastic" drawing routines for the Fast Light Tool Kit (FLTK).
//
// These box types provide a 
// Copyright 2001 by Michael Sweet.
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
#include <string.h>


extern uchar *fl_gray_ramp();

inline Fl_Color shade_color(uchar gc, Fl_Color bc) {
  return fl_color_average((Fl_Color)gc, bc, 0.67f);
}

static void shade_frame(int x, int y, int w, int h, const char *c, Fl_Color bc) {
  uchar *g = fl_gray_ramp();
  int b = strlen(c) / 4 + 1;

  for (x += b, y += b, w -= 2 * b, h -= 2 * b; b > 1; b --)
  {
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
  int		xoff, yoff;
  int		cmod, cerr;
  int		clen = strlen(c);


  if (w >= h)
  {
    h ++;
    cmod = clen % h;
    cerr = 0;

    fl_color(shade_color(g[*c], bc));

    for (yoff = 0; yoff < h; yoff ++)
    {
      fl_xyline(x, y + yoff, x + w - 1);

      cerr += cmod;
      if (cerr >= h)
      {
        cerr -= h;
	c ++;

        fl_color(shade_color(g[*c], bc));
      }
    }
  }
  else
  {
    w ++;
    cmod = clen % w;
    cerr = 0;

    fl_color(shade_color(g[*c], bc));

    for (xoff = 0; xoff < w; xoff ++)
    {
      fl_yxline(x + xoff, y, y + h - 1);

      cerr += cmod;
      if (cerr >= w)
      {
        cerr -= w;
	c ++;

        fl_color(shade_color(g[*c], bc));
      }
    }
  }
}


static void up_frame(int x, int y, int w, int h, Fl_Color c) {
  shade_frame(x, y, w, h, "RRSSDLNN", c);
}


static void up_box(int x, int y, int w, int h, Fl_Color c) {
  if (w > 30 && h > 30)
  {
    uchar *g = fl_gray_ramp();
    fl_color(shade_color(g['W'], c));
    fl_rectf(x + 2, y + 2, w - 4, h - 4);
  }
  else
    shade_rect(x + 2, y + 2, w - 4, h - 4, "VUTSSTUVWW", c);

  up_frame(x, y, w, h, c);
}


static void down_frame(int x, int y, int w, int h, Fl_Color c) {
  shade_frame(x, y, w, h, "LLRRTTLL", c);
}


static void down_box(int x, int y, int w, int h, Fl_Color c) {
  if (w > 30 && h > 30)
  {
    uchar *g = fl_gray_ramp();
    fl_color(shade_color(g['T'], c));
    fl_rectf(x + 2, y + 2, w - 4, h - 4);
  }
  else
    shade_rect(x + 2, y + 2, w - 4, h - 4, "STUVWWWWVT", c);

  down_frame(x, y, w, h, c);
}


extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*);


Fl_Boxtype define_FL_PLASTIC_UP_BOX() {
  fl_internal_boxtype(_FL_PLASTIC_UP_BOX, up_box);
  fl_internal_boxtype(_FL_PLASTIC_DOWN_BOX, down_box);
  fl_internal_boxtype(_FL_PLASTIC_UP_FRAME, up_frame);
  fl_internal_boxtype(_FL_PLASTIC_DOWN_FRAME, down_frame);

  return _FL_PLASTIC_UP_BOX;
}


//
// End of "$Id: fl_plastic.cxx,v 1.1.2.2 2001/12/01 13:59:50 easysw Exp $".
//
