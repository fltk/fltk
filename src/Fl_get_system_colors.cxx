//
// "$Id: Fl_get_system_colors.cxx,v 1.6.2.1 1999/10/19 04:50:34 bill Exp $"
//
// System color support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/math.h>

void Fl::background(uchar r, uchar g, uchar b) {
  // replace the gray ramp so that FL_GRAY is this color
  if (!r) r = 1; else if (r==255) r = 254;
  double powr = log(r/255.0)/log((FL_GRAY-FL_GRAY_RAMP)/(FL_NUM_GRAY-1.0));
  if (!g) g = 1; else if (g==255) g = 254;
  double powg = log(g/255.0)/log((FL_GRAY-FL_GRAY_RAMP)/(FL_NUM_GRAY-1.0));
  if (!b) b = 1; else if (b==255) b = 254;
  double powb = log(b/255.0)/log((FL_GRAY-FL_GRAY_RAMP)/(FL_NUM_GRAY-1.0));
  for (int i = 0; i < FL_NUM_GRAY; i++) {
    double gray = i/(FL_NUM_GRAY-1.0);
    Fl::set_color(fl_gray_ramp(i),
		  uchar(pow(gray,powr)*255+.5),
		  uchar(pow(gray,powg)*255+.5),
		  uchar(pow(gray,powb)*255+.5));
  }
}

void Fl::foreground(uchar r, uchar g, uchar b) {
  Fl::set_color(FL_BLACK,r,g,b);
}

void Fl::background2(uchar r, uchar g, uchar b) {
  Fl::set_color(FL_WHITE,r,g,b);
  Fl::set_color(FL_BLACK,get_color(contrast(FL_BLACK,FL_WHITE)));
}

// these are set by Fl::args() and override any system colors:
const char *fl_fg;
const char *fl_bg;
const char *fl_bg2;

#ifdef WIN32

#include <stdio.h>
// simulation of XParseColor:
int fl_parse_color(const char* p, uchar& r, uchar& g, uchar& b) {
  if (*p == '#') p++;
  int n = strlen(p);
  int m = n/3;
  const char *pattern = 0;
  switch(m) {
  case 1: pattern = "%1x%1x%1x"; break;
  case 2: pattern = "%2x%2x%2x"; break;
  case 3: pattern = "%3x%3x%3x"; break;
  case 4: pattern = "%4x%4x%4x"; break;
  default: return 0;
  }
  int R,G,B; if (sscanf(p,pattern,&R,&G,&B) != 3) return 0;
  switch(m) {
  case 1: R *= 0x11; G *= 0x11; B *= 0x11; break;
  case 3: R >>= 4; G >>= 4; B >>= 4; break;
  case 4: R >>= 8; G >>= 8; B >>= 8; break;
  }
  r = R; g = G; b = B;
  return 1;
}

static void
getsyscolor(int what, const char* arg, void (*func)(uchar,uchar,uchar))
{
  if (arg) {
    uchar r,g,b;
    if (!fl_parse_color(arg, r,g,b))
      Fl::error("Unknown color: %s", arg);
    else
      func(r,g,b);
  } else {
    DWORD x = GetSysColor(what);
    func(uchar(x&255), uchar(x>>8), uchar(x>>16));
  }
}

void Fl::get_system_colors() {
  getsyscolor(COLOR_WINDOW,	fl_bg2,Fl::background2);
  getsyscolor(COLOR_WINDOWTEXT,	fl_fg, Fl::foreground);
  getsyscolor(COLOR_BTNFACE,	fl_bg, Fl::background);
}

#else

// For X we should do something. KDE and Gnome store these colors in
// some standard places, where?

static void
getsyscolor(const char *arg, void (*func)(uchar,uchar,uchar)) {
  if (arg) {
    XColor x;
    if (!XParseColor(fl_display, fl_colormap, arg, &x))
      Fl::error("Unknown color: %s", arg);
    else
      func(x.red>>8, x.green>>8, x.blue>>8);
  }
}

void Fl::get_system_colors()
{
  fl_open_display();
  getsyscolor(fl_bg2,Fl::background2);
  getsyscolor(fl_fg, Fl::foreground);
  getsyscolor(fl_bg, Fl::background);
}

#endif

//
// End of "$Id: Fl_get_system_colors.cxx,v 1.6.2.1 1999/10/19 04:50:34 bill Exp $".
//
